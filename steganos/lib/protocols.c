/*                               -*- Mode: C -*- 
 * @file: protocols.c
 * @brief: This file implements the functions relative to the different protocol
 *  layers of the software Vorbistego and their interfaces.
 *
 * Currently there are two protocol layers. The cryptographic layer and the
 * steganographic layer, the former addressing security issues and de latter
 * addressing the stablishment of the subliminal channel. Every interface 
 * relative to the cryptographic protocol will be preceeded by the prefix 
 * <i>cryptos_</i>, while every interface relative to the steganographic one
 * will be preceeded by <i>steganos_</i>.
 *
 * @author: Jesus Diaz Vico
 * Maintainer: 
 * @date: dom ene 10 22:34:16 2010 (+0100)
 * @version: 
 * Last-Updated: lun ago 30 14:16:48 2010 (+0200)
 *           By: Jesus
 *     Update #: 1943
 * URL: 
 */

#ifdef STEGO
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "protocols.h"
#include "steganos_channel.h"
#include "cryptos_channel.h"
#include "miscellaneous.h"
#include "numbers.h"

int cryptos_forward(cryptos_config_t *cc, cryptos_protocol_buffer_t *cb, 
 		    uint64_t data_size) {

  struct stat buf;
  byte *tmp, *packet;
  uint64_t written, effective_data_size, packet_size;
  ssize_t rc;

  /* Input parameters control */
  if(!cc || !cb) {
    errno = EINVAL;
    message_log("cryptos_forward", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  /* If the remaining data in the buffer is at least big enough to cover
     a whole stego-frame, we don't have work to do yet. */
  if(cb->buffer_used >= MAX_SUBLIMINAL_SIZE) {
    return I_CRYPTOS_OK;
  }

  /* If the received data_size is 0 or we receive a data_size > than the
     default packet size, the size is changed to the default. */
  if(!data_size || data_size > cc->default_data_size) {
    effective_data_size = cc->default_data_size;
  } else {
    effective_data_size = data_size;
  }

  packet_size = effective_data_size + CRYPTOS_HEADER_LEN + cc->md_len;

  if(!(packet = (byte *) malloc(sizeof(byte)*packet_size))) {
    message_log("cryptos_forward", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  memset(packet, 0, sizeof(byte)*packet_size);

  if(!(tmp = (byte *) malloc(sizeof(byte)*effective_data_size))) {
    message_log("cryptos_forward", strerror(errno));
    free(packet);
    return I_CRYPTOS_ERR;
  }

  memset(tmp, 0, sizeof(byte)*effective_data_size);

  /* Otherwise, prepare a new CRYPTOS packet */

  /* Locate the file descriptor at the offset indicated by cb->offset */
  if(lseek(cb->fd, cb->offset, SEEK_SET) == -1) {
    message_log("cryptos_forward", strerror(errno));
    free(packet);
    free(tmp);
    return I_CRYPTOS_ERR;
  }

  /* Try to read the given amount of data */
  if((rc = read(cb->fd, tmp, sizeof(byte)*effective_data_size)) == -1) {
    message_log("cryptos_forward", strerror(errno));
    free(packet);
    free(tmp);
    return I_CRYPTOS_ERR;
  }

  if(fstat(cb->fd, &buf)) {
    message_log("cryptos_forward", strerror(errno));
    free(packet);
    free(tmp);
    return I_CRYPTOS_ERR;
  }

  /* Test for EOT */
  if((uint64_t) (cb->offset + rc) == (uint64_t) buf.st_size) {
    cc->packet = 0;
  }

  written = 0;
  if(produce_packet(cc, tmp, rc, packet, packet_size,
		    &written) == I_CRYPTOS_ERR) {
    free(packet);
    free(tmp);
    return I_CRYPTOS_ERR;
  }

  free(tmp);

#ifdef STEGANOS_DEBUG
  {

    char spacket[50], sdata[4000];
    uint64_t i;

    memset(spacket, 0, 50*sizeof(char));
    memset(sdata, 0, 4000*sizeof(char));
    sprintf(spacket, "Packet %lu produced (%lu Bytes)", cc->packet-1, packet_size);
    for(i=0; i<packet_size; i++) {
      sprintf(&sdata[2*i], "%X", packet[i]&0xF0);
      sprintf(&sdata[2*i+1], "%X", packet[i]&0x0F);
	
    }
    message_log(spacket, sdata);
    
  }
#endif

  if(/*size*/written != (uint64_t) rc) {
    message_log("cryptos_forward", "Unknown error");
    free(packet);
    return I_CRYPTOS_ERR;
  }

  memcpy(&cb->buffer[cb->buffer_used], packet, packet_size);
  free(packet);

  cb->buffer_used += packet_size;
  cb->offset += written;

  return I_CRYPTOS_OK;
}

int cryptos_inverse(cryptos_config_t *cc, cryptos_protocol_buffer_t *cb) {

  byte *tmp;
  uint64_t read, data;
  int rc;

  if(!cc || !cb) {
    errno = EINVAL;
    message_log("cryptos_inverse", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  if(!(tmp = (byte *) malloc(sizeof(byte)*cc->default_data_size))) {
    errno = EINVAL;
    message_log("cryptos_inverse", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  memset(tmp, 0, cc->default_data_size);
  read = 0;

  /* If we get an error parsing the packet, it was malformed or had an
     unexpected EMISSION or PACKET ids */
  data = cc->default_data_size;
  if((rc = parse_packet(cc, cb->buffer, cb->buffer_used, tmp, 
			&data, &read)) == I_CRYPTOS_ERR) {
    
    /* If, here read != 0 means we've read a corrupted header (with wrong SYNC
       field), and we have to discard the read bytes. */
    if(read) {
      memmove(cb->buffer, &cb->buffer[read], (cb->buffer_size - read));
      cb->buffer_used -= read;
      memset(&cb->buffer[cb->buffer_used], 0, cb->buffer_size - cb->buffer_used);
    }

    free(tmp);
    return I_CRYPTOS_ERR;

  }

  /* If the packet was complete, but the integrity check failed, discard 
     the packet */
  if(rc == I_CRYPTOS_CHECK_FAIL) {

    /* Remove it from the buffer */
    memmove(cb->buffer, &cb->buffer[read], (cb->buffer_size - read));
    cb->buffer_used -= read;
    memset(&cb->buffer[cb->buffer_used], 0, cb->buffer_size - cb->buffer_used);
    free(tmp);

    return I_CRYPTOS_CHECK_FAIL;
  }

  /* If read equals 0, the packet wasn't complete, so we have no new data */
  if(!read) {
    free(tmp);
    return I_CRYPTOS_OK;
  }

  /* Otherwise, discard the already read packet bytes from the buffer and
     write the data in the specified fd */
  memmove(cb->buffer, &cb->buffer[read], cb->buffer_size - read);
  cb->buffer_used -= read;
  memset(&cb->buffer[cb->buffer_used], 0, cb->buffer_size - cb->buffer_used);

  if((rc = write(cb->fd, tmp, read-CRYPTOS_HEADER_LEN-cc->md_len)) == -1) {
    message_log("cryptos_inverse", strerror(errno));
    free(tmp);
    return I_CRYPTOS_ERR;
  }

  free(tmp);

  if((uint64_t) rc != read-CRYPTOS_HEADER_LEN-cc->md_len) {
    message_log("cryptos_inverse", "Unknown error");
    return I_CRYPTOS_ERR;
  }
  
  return I_CRYPTOS_OK;

}

int steganos_vorbis_config_init(vorbis_config_t *vc, int rate, int pcmend, int mult, 
				int *postlist, int *forward_index, int posts) {

  if(!vc || rate <= 0 || pcmend <= 0 || mult <= 0 || !postlist || 
     !forward_index || posts <= 0) {
    errno = EINVAL;
    message_log("steganos_vorbis_config_init", strerror(errno));
    return I_STEGANOS_ERR;
  }

  vc->rate = rate;
  vc->pcmend = pcmend;
  vc->mult = mult;
  vc->postlist = postlist;
  vc->forward_index = forward_index;
  vc->posts_len = posts;

  return I_STEGANOS_OK;
}

int steganos_vorbis_config_free(vorbis_config_t *vc) {

  if(!vc) {
    return I_STEGANOS_OK;
  }

  return I_STEGANOS_OK;
  
}

int steganos_forward(steganos_state_t *ss, vorbis_config_t *vc, int *floor, 
		     int *posts, float *residue, byte *buffer, size_t buffer_len,
		     int *hided) {
  
  int size, d_len, d_len_bits, i, rc, pcmend, posts_len, rate, aux, carry_len;
  byte *data;

  /* Input parameters control */
  if(!ss || !vc || (!buffer && !ss->desync) || !hided) {  // TODO!! todo controlado?
    errno = EINVAL;
    message_log("steganos_forward", strerror(errno));
    return I_STEGANOS_ERR;
  }

  /* Seed the pseudo random number generator with the hiding key. */
  if(prng_set_seed_byte(ss->prng, ss->hiding_key->key, ss->hiding_key->length) == 
     I_STEGANOS_ERR) {
    return I_STEGANOS_ERR;
  }

  /* If we just have to desynchronize, do it here, it'll save time */
  if(ss->desync) {

    /* Calculate residue lineup for hiding. We'll always need it. */
    if(!ss->aligned) {
      if(calculate_residue_lineup(ss, vc->pcmend/2) == I_STEGANOS_ERR) {
	return I_STEGANOS_ERR;
      }	
    }

    rc = ss->desynchronize(vc, ss->res_lineup, posts, floor, residue, NULL, 
			   ss->hiding_key, ss->hide, ss->prng);
    return rc;
  }

  rate = vc->rate;
  pcmend = vc->pcmend;
  posts_len = vc->posts_len;
  
  /* Analyze residue vector. */
  if(set_subliminal_capacity_limit(ss, residue,
				   rate, pcmend/2) == I_STEGANOS_ERR) {
    return I_STEGANOS_ERR;
  }

  /* In the current implementation, we will always be able to hide at least
     ss->min_fc_capacity bits, so if that limit isn't enough even to hide the
     headers plus a minimum of data, we exit. */
  /* @todo Use here something more practical that takes into account the header
     overload among other things... we are allowing to introduce <i>x</i>
     metainfo bits for a sole single info bit! */
  if(ss->synchro_method == RES_HEADER || ss->synchro_method == FORCED_RES_HEADER) {
    if(ss->min_fc_capacity <= (SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE)) {
      return I_STEGANOS_FRAME_SKIP;
    }
  }

  /* Once here, we know there still are bits to send */
  d_len = buffer_len;
  if(d_len > MAX_SUBLIMINAL_SIZE) {
    d_len = MAX_SUBLIMINAL_SIZE;
  }
    
  /* Allocate a (unsigned char *) for, at maximum, MAX_SUBLIMINAL_SIZE bits */
  if(!(data = (byte *) malloc(sizeof(byte)*buffer_len))) {
    message_log("steganos_forward", strerror(errno));
    return I_STEGANOS_ERR;
  }
    
  memcpy(data, buffer, d_len);

  /* Discard the bits that may have been sent in previous packets */
  carry_len = ss->sent % BITS_PER_BYTE;
  if(carry_len) {
    for(i=0; i<d_len-1; i++) {
      data[i] <<= (ss->sent % BITS_PER_BYTE);
      data[i] |= (data[i+1] >> (BITS_PER_BYTE - (ss->sent % BITS_PER_BYTE)));
    }
    data[d_len-1] <<= (ss->sent % BITS_PER_BYTE);
    d_len_bits = d_len*BITS_PER_BYTE - (ss->sent % BITS_PER_BYTE);
  } else {
    d_len_bits = d_len*BITS_PER_BYTE;
  }

  /* Calculate residue lineup for hiding if not done yet. We'll always need it. */
  if(!ss->aligned) {
    if(calculate_residue_lineup(ss, vc->pcmend/2) == I_STEGANOS_ERR) {
      return I_STEGANOS_ERR;
    }
  }

  if(hide_data(ss, data, d_len_bits, floor,
	       residue, pcmend/2, &size) == I_STEGANOS_ERR) {
    if(data) free(data);
    return I_STEGANOS_ERR;
  }

  ss->sent += size;

  if((carry_len + (size % BITS_PER_BYTE)) >= BITS_PER_BYTE) {
    size += BITS_PER_BYTE;
  }

  *hided = size;

  if(data) free(data);

  return I_STEGANOS_OK;
  
}

int steganos_inverse(steganos_state_t *ss, vorbis_config_t *vc, int *posts, 
		     int *floor, float *residue, float sigma, byte *buffer,
		     int buffer_sz, int *sd_read) {

  iss_cfg_t iss_cfg;
  int read, i, aux, bit, pcmend, posts_len, hack, carry_prev, read_w_carry;
  byte *data, buff;
  static byte carry=0;
  static int carry_len=0;


  /* Input parameters control */
  if(!ss || !vc || !posts || !floor || !residue || 
     (sigma <= 0 && ss->synchro_method == ISS) ||
     !buffer || buffer_sz <= 0 || !sd_read) { // TODO!! todo controlado?
    errno = EINVAL;
    message_log("steganos_inverse", strerror(errno));
    return I_STEGANOS_ERR;
  }

  pcmend = vc->pcmend;
  posts_len = vc->posts_len;
  hack = 0;

  if(ss->synchro_method == ISS) {
    
    if(iss_cfg_init(ss, posts, vc->posts_len, sigma, &iss_cfg) == 
       I_STEGANOS_ERR) {
      ss->status = I_STEGANOS_SYNC_FAIL;
      return I_STEGANOS_ERR;
    }

    if(ss->synchronize(vc, NULL, posts, residue, &iss_cfg, 1, &bit, ss->prng) ==
       I_STEGANOS_ERR) {
      return I_STEGANOS_ERR;
    }

    iss_cfg_free(&iss_cfg);

  }
  

  /* If we're using ISS as synchro method, and the bit received is 0, before exit
     we have to check if it is a false negative, and we do so by comparing the 
     first SYNCHRO_HEADER_BYTES_RES bytes with the SYNCHRO_HEADER  */
  if(ss->synchro_method == ISS && !bit) {
    ss->synchro_method = FORCED_RES_HEADER;
  }


  /* Seed the pseudo random number generator with the hiding key to obtain the
     same sequence used by the sender. */
  if(prng_set_seed_byte(ss->prng, ss->hiding_key->key, ss->hiding_key->length) == 
     I_STEGANOS_ERR) {
    if(ss->synchro_method == FORCED_RES_HEADER) ss->synchro_method = ISS;
    return I_STEGANOS_ERR;
  }

  /* Calculate residue lineup for hiding. We'll always need it. */
  if(!ss->aligned) {
    i=0;
    while(i<vc->pcmend/2) {
      
      if(prng_get_random_int(ss->prng, vc->pcmend/2, &aux) == I_STEGANOS_ERR){
	if(ss->synchro_method == FORCED_RES_HEADER) ss->synchro_method = ISS;
	return I_STEGANOS_ERR;
      }
      
      /* Check if this residual value has been already chosen */
      if(!ss->res_occupied[aux]) {
	ss->res_lineup[i] = aux;
	ss->res_occupied[aux] = 1;
	i++;
      }     
    }
    ss->aligned = 1;
  }

  /* Recover data */
  if(unhide_data(ss, floor, residue, pcmend/2, &data, &read) == I_STEGANOS_ERR) {
    if(ss->synchro_method == FORCED_RES_HEADER) ss->synchro_method = ISS;
    return I_STEGANOS_ERR;
  }

  if(ss->synchro_method == FORCED_RES_HEADER) ss->synchro_method = ISS;

  *sd_read = 0;
  read_w_carry = carry_len + read;
  if(read) {

    carry_prev = carry_len;

    /* If we have carry from previous stegano-frames... */
    if(carry_len) {

      /* We have to rotate all bits in data 'carry_len' bits to the right */
      aux = read/BITS_PER_BYTE;
      if(((read%BITS_PER_BYTE)+carry_len) > BITS_PER_BYTE) aux++;
      for(i=0; i<aux; i++) {
	buff = data[i]; // Buffer for the next carry
	data[i] >>= carry_len;
	data[i] |= carry;
	carry = buff << (BITS_PER_BYTE - carry_len);
      }

      if(read % BITS_PER_BYTE) {
	carry |= (data[read/BITS_PER_BYTE] >> carry_len);
      }

      carry_len = read % BITS_PER_BYTE;
      if((carry_len + carry_prev) >= BITS_PER_BYTE) {
 	read += BITS_PER_BYTE;
	read -= carry_len;
	if(carry_len + carry_prev == BITS_PER_BYTE) {
	  data[read/BITS_PER_BYTE-1] = carry;
	}
	carry_len = (carry_len + carry_prev) % BITS_PER_BYTE;
      } else {
	read -= carry_len;
	carry_len += carry_prev;
      }

      carry &= (0xFF << (BITS_PER_BYTE - carry_len));
     
    } else {

      /* If not, and read is not BITS_PER_BYTE multiple, store the
	 remainder bits as carry */
      if(read % BITS_PER_BYTE) {
	carry = data[read/BITS_PER_BYTE];
	carry_len = read % BITS_PER_BYTE;
	read -= carry_len;
      }

    }
    
#ifdef STEGANOS_DEBUG
    {
      char siter[500], sdata[400];

      memset(siter, 0, 500*sizeof(char));
      memset(sdata, 0, 400*sizeof(char));
      sprintf(siter, "%d) %d bits read + %d bits carry => pass %d bits and %d bits new carry",
	      ss->iters, read_w_carry - carry_prev, carry_prev, read, carry_len);
      for(i=0; i<ceilf((float)read/(float)BITS_PER_BYTE); i++) {
	sprintf(&sdata[2*i], "%X", data[i]&0xF0);
	sprintf(&sdata[2*i+1], "%X", data[i]&0x0F);
	
      }
      message_log(siter, sdata);
    }
#endif
    
    /* Leaving the carry bits, read must be BITS_PER_BYTE-multiple */
    if(read % BITS_PER_BYTE) {
      errno = EBADMSG;
      message_log("steganos_inverse", "Unknown error");
      return I_STEGANOS_ERR;
      
    }    

    if(read)
      memcpy(buffer, data, read/BITS_PER_BYTE);

    *sd_read = read/BITS_PER_BYTE;
    ss->read += read;

/*     *sd_read = read/BITS_PER_BYTE; */
/*     memcpy(buffer, data, *sd_read); */

    free(data);

  }
  
  return I_STEGANOS_OK;

}

/* protocols.c ends here */
#endif
