/*                               -*- Mode: C -*- 
 * @file: channel.c
 * @brief: This file implements the functions needed to create a subliminal
 * channel within Vorbis audio files with the software Vorbistego.
 * @author: Jesus Diaz Vico
 * Maintainer: 
 * Created: vie dec  18 17:28:17 2009 (+0100)
 * Version: 
 * Last-Updated: vie sep  3 11:09:16 2010 (+0200)
 *           By: Jesus
 *     Update #: 3324
 * URL: 
 */
#ifdef STEGO
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <fenv.h>
#include <gcrypt.h>
#include "steganos_channel.h"
#include "numbers.h"
#include "miscellaneous.h"
#include "vorbis/codec.h"

/** 
 * @fn static int _itu468_var_tol(const float frequency, 
 *                         const float base, float *tolerance)
 * 
 * @brief Calculates the maximum tolerated variation at the given frequency.
 *
 * This function serves to calculate the maximum variation that the given base
 * value, at the given freqency, can stand accordingly to the ITU-R BS. 468-4
 * document. The value returned will be a two element array with the negative
 * and positive directions maximum variations.
 *
 * @param[in] frequency Frequency of the base value.
 * @param[in] base Base value to use for obtaining the tolerances.
 * @param[in, out] tolerance At the output will store the maximum linear variation
 *  proposed in the ITU-R BS. 468-4 document, in both directions [-,+]. The memory
 *  has to be allocated previously to calling the function.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 */
static int _itu468_var_tol(const float frequency, const float base, float *tolerance) {

  float multiplier, x1, y1, x2, y2;
  int i;

  /* Input parameters control */
  if(frequency < 0 || !tolerance) {
    errno = EINVAL;
    message_log("_itu468_var_tolerance", strerror(errno));
    return I_STEGANOS_ERR;
  }

  /** The allowed multiplier is obtained by lineal interpolation, using the
     greatest point lesser than 'frequency' as left extreme and the least point 
     greater than 'frequency' as right extreme of the line. These points are
     obtained from the ITU-R BS. 468-4 document, stored in the global variable
     ITU_R_BS_468. */

  multiplier = -1.f;
  x1 = x2 = y1 = y2 = 0;

  /* Special cases: frequency < ITU_R_BS_468[0][0] and 
     frequency > ITU_R_BS_468[ITU_R_BS_468_SIZE-1][0] */
  if(frequency < ITU_R_BS_468[0][0]) {
    multiplier = ITU_R_BS_468[0][1];
  } else if (frequency > ITU_R_BS_468[ITU_R_BS_468_SIZE-1][0]) {
    multiplier = ITU_R_BS_468[ITU_R_BS_468_SIZE-1][1];
  } else {

    /* Locating left and right extremes */
    for(i=0; i<ITU_R_BS_468_SIZE; i++) {
      if(ITU_R_BS_468[i][0] > frequency) {
	x1 = ITU_R_BS_468[i-1][0];
	y1 = ITU_R_BS_468[i-1][1];
	x2 = ITU_R_BS_468[i][0];
	y2 = ITU_R_BS_468[i][1];
	break;
      } else if(ITU_R_BS_468[i][0] == frequency) {
	multiplier = ITU_R_BS_468[i][1];
	break;
      }
    }
    
    if(multiplier < 0) {
      if(linear_interpolation_y(x1, y1, x2, y2, frequency, &multiplier) == I_STEGANOS_ERR) {
	return I_STEGANOS_ERR;
      }
    }
   
  }

  /* Shouldn't happen TODO! check */
  if(multiplier < 0) {
    message_log("_itu468_var_tolerance", "Wrong tolerance calculation");
    return I_STEGANOS_ERR;
  }

  /* Now use the obtained multiplier to calculate the maximum negative and
     positive variations. See the project documentation for a complete 
     demonstration of the formulae used here. */
  /* TODO! referencia a la documentación? */
  if(base > 0) {
    /* Maximum negative variation */
    tolerance[0] = base*(-multiplier);
    /* Maximum positive variation */
    tolerance[1] = base*multiplier;
  } else {
    /* Maximum negative variation */
    tolerance[0] = base*multiplier;
    /* Maximum positive variation */
    tolerance[1] = base*(-multiplier);
  }

  return I_STEGANOS_OK;

}

/**
 * @fn static int _get_subliminal_data(steganos_state_t *ss, byte *data, 
 *                       const int d_len, int *floor, float *residue, 
 *                       const int res_len, byte *sub_data, int *size)
 * @brief Obtains the next 'size' bits of subliminal data (if available) using
 *  the method specified in ss->hide_method.
 *
 * This function obtains the greatest possible number of bits less or equal to
 * 'size' from the data bitstream and transforms them to a subliminal bitstream
 * applying the hiding method specified by ss->hide_method. In a successful 
 * execution, the size and sub_data values are updated to the final size and 
 * subliminal data values obtained, and the residue and original data vectors 
 * will always be left unchanged. This function also adds the synchronization
 * header fields required depending on the synchronization mode in the protocol
 * state structure. The protocol state must be SYNCHRONIZED at the input and
 * won't change during the execution.
 *
 * @note It's very important to take into account that this functions adds the
 *  needed metadata to the bitstream at the beginning of its execution and 
 *  makes no difference with pure data when hiding. So, if the function
 *  receives a <i>size</i> of 58 bits and it has to add 24 bits of metadata,
 *  only 58-24=34 of the returned <i>sub_data</i> bits will be pure data. If the
 *  current frame and channel is the first of the communication process, a first
 *  "communication"-header will be also included.
 * 
 * @param[in] ss Internal state structure. The protocol state must be 
 * SYNCHRONIZED at the input and won't change during the execution.
 * @param[in] data The remaining data to hide.
 * @param[in] d_len The length of the data parameter, in bits.
 * @param[in] floor The current frame and channel floor vector. Depending on the
 *  hiding method selected, it may or may not be used.
 * @param[in] residue The current frame and channel residue vector.
 * @param[in] res_len The length of the residue vector, in bytes.
 * @param[in, out] sub_data The array that will store the subliminal 
 *  data prepared to be written in the residue.
 * @param[in, out] size The maximum amount of subliminal bits to hide as input 
 *  (i.e., the allocated memory for sub_data) and the final data retrieved at 
 *  the output, also in bits, including header, data, and tail (if any) 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 * @retval I_STEGANOS_ERR with errno = EPROTO (not enough subliminal space)
 * @see steganos_state_t
 */
static int _get_subliminal_data(steganos_state_t *ss, byte *data, const int d_len, 
				int *floor, float *residue, const int res_len,
				byte *sub_data, int *size) {

  int usage_bits, free_bits, header_bits, data_bits, tail_bits, meta_data_bits;
  int tmp_size, i, size_field, aux, meta_data_bytes, header_bytes;
  int data_bytes, tail_bytes;
  byte *meta_data;


  /* Input parameters control */
  if(!ss || d_len < 0 ||
     !floor || !residue || res_len <= 0 || !sub_data || *size <= 0) {
    errno = EINVAL;
    message_log("_get_subliminal_data", strerror(errno));
    return I_STEGANOS_ERR;
  }

  /* Calculate the header, tail and total (header+data+tail)=meta_data sizes */
  usage_bits = *size; /* Desired amount of potential subliminal bits to use */
  free_bits = usage_bits;
  header_bits = SIZE_FIELD_BITS; /* Always needed */
  data_bits = 0;
  tail_bits = 0;
  meta_data_bits = 0;

  if(ss->synchro_method == RES_HEADER || 
     ss->synchro_method == FORCED_RES_HEADER) {
    header_bits += SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE;
  }
  
  free_bits -= header_bits;

  if(!ss->desync) {

    if(free_bits <= 0) {
      message_log("_get_subliminal_data", 
		  "Not enough subliminal space in the current channel-frame");
      errno = EPROTO;
      return I_STEGANOS_ERR;
    }

    /* Test if we reach the end of the subliminal data to send */
    if(d_len <= free_bits) {
      data_bits = d_len;
    } else {
      data_bits = free_bits;
    }
 
  } else {
    data_bits = 0;
  }

  header_bytes = (int) ceilf((float)header_bits/(float)BITS_PER_BYTE);
  data_bytes = (int) ceilf((float)data_bits/(float)BITS_PER_BYTE);
  tail_bytes = (int) ceilf((float)tail_bits/(float)BITS_PER_BYTE);
  
  meta_data_bits = header_bits + data_bits + tail_bits;

  if(meta_data_bits > usage_bits) {
#ifdef STEGANOS_DEBUG
    {
      char message[50];
      memset(message, 0, 50*sizeof(char));
      sprintf(message, "%d) Not enough subliminal capacity", ss->iters);
      message_log("_get_subliminal_data", message);
    }
#endif
    *size = 0;
    errno = EPROTO;
    return I_STEGANOS_ERR;
  }

  meta_data_bytes = (int) ceilf((float)meta_data_bits/(float)BITS_PER_BYTE);

  /* Allocate memory for the data+meta_data bytestring */
  if(!(meta_data = (byte *) malloc(sizeof(byte)*meta_data_bytes))) {
    message_log("_get_subliminal_data", strerror(errno));
    return I_STEGANOS_ERR;
  }

  /* Reset the whole string */
  memset(meta_data, 0, sizeof(byte)*meta_data_bytes);

  /* Only if we're using RES_HEADER we must synchronize here */
  if(ss->synchro_method == RES_HEADER || 
     ss->synchro_method == FORCED_RES_HEADER) {
    for(i=0; i<SYNCHRO_HEADER_BYTES_RES; i++) {
      meta_data[i] = SYNCHRO_HEADER[i];
    }
    
  } 

  /* Fill size field */
  /** In the current approach, the header's size field will indicate the maximum
      amount of subliminal data this channel will shelter. The final data hidden
      might be lesser, though. Such cases must not be contemplated as errors. */
  size_field = data_bits + tail_bits;
  for(i=0; i<SIZE_FIELD_BITS; i++) {
    aux = ceilf((float)SIZE_FIELD_BITS/(float)BITS_PER_BYTE)-(i/BITS_PER_BYTE)-1;
    if(ss->synchro_method == RES_HEADER || 
       ss->synchro_method == FORCED_RES_HEADER) {
      aux += SYNCHRO_HEADER_BYTES_RES;
    }
    if((size_field >> i) % 2) {
      meta_data[aux] |=
	(1 << (i % BITS_PER_BYTE));
    }
  }

  /* Add the pure datastream at the end, note that we are lefting an empty (0'ed)
     space for the 'size' field, that must be fulfilled once we know the exact
     amount of bits we will hide in the current frame and channel */
  memcpy(&meta_data[header_bytes], data, data_bytes); // <-- A veces falla en valgrind TODO!!

  /* Hack: if size_field is not BITS_PER_BYTE-multiple, we place the remainder bits
     at the least significant positions of the last byte */
  if(size_field % BITS_PER_BYTE) {
    meta_data[meta_data_bytes-1] >>= 
      (BITS_PER_BYTE - (size_field % BITS_PER_BYTE));
  }

  tmp_size = meta_data_bytes*BITS_PER_BYTE;

/* #ifdef STEGANOS_DEBUG */
/*   { */
/*     int kk; */
/*     char sdata[400]; */
    
/*     memset(sdata, 0, 400*sizeof(char)); */
/*     for(kk=0; kk<meta_data_bytes; kk++) { */
/*       sprintf(&sdata[2*kk], "%X", meta_data[kk]&0xF0); */
/*       sprintf(&sdata[2*kk+1], "%X", meta_data[kk]&0x0F);      */
/*     } */
/*     message_log("_get_subliminal_data", sdata);  */
/*   } */
/* #endif */

  /* Uses the selected hiding method */
  if(tmp_size) {

    if(ss->hide(meta_data, meta_data_bits, floor, residue, 
		res_len, ss->hiding_key, meta_data, &tmp_size, 
		ss->prng) == I_STEGANOS_ERR) {
      free(meta_data);
      return I_STEGANOS_ERR;
    }

  }

  /* Undo hack (now with tmp_size): I should find a more elegant 
     way of doing this... */
  if(tmp_size % BITS_PER_BYTE) {
    meta_data[meta_data_bytes-1] <<= (BITS_PER_BYTE - (tmp_size % BITS_PER_BYTE));
  }

  *size = tmp_size;
  memcpy(sub_data, meta_data, sizeof(byte)*meta_data_bytes);
  free(meta_data);

  return I_STEGANOS_OK;
}

/**
 * @fn static int _write_subliminal_data(steganos_state_t *ss, byte *data, const int d_len,
 *                                float *residue, const int res_len, int *written)
 * @brief Writes data in the subliminal channel.
 *
 * Goes through every residue in the order specified by the PRN seeded with the
 * hiding key and writes in the subliminal channel the number of bits nearest to
 * the value specified by <i>d_len</i>, being <i>d_len</i> the upper limit.
 *
 * @note This function uses the PRNG function <i>get_random_int</i>, therefore,
 *  the PRNG must have been seeded previously with the same key used for hiding
 *  in order to get the correct data from the residue/floor vectors.
 *
 * @param[in, out] ss Internal state structure.
 * @param[in] data The data to write in the residue.
 * @param[in] d_len The amount of bits in data that have to be written.
 * @param[in, out] residue The current frame and channel residue vector, 
 *  updated to the subliminal residue after a successful execution.
 * @param[in] res_len The number of residual values in residue.
 * @param[out] written The amount of <i>data</i> that has been possible to write.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 * @retval I_STEGANOS_ERR with errno = ERANGE (tried to read or write in a foreign 
 *  memory location).
 * @see steganos_state_t
 */
static int _write_subliminal_data(steganos_state_t *ss, byte *data, const int d_len,
				  float *residue, const int res_len, int *written) {
  
  int i, j, pos, max_bits, min_bits, curr_value, sub_value, tmp_written;
  int upper, lower, nearest, diff, bit, neg, rd_byte, rd_bit;

 
  /* Input parameters control */ // TODO!! todo controlado?
  if(!ss || !data || d_len < 0 || 
     !residue || res_len <= 0 || !written) {
    errno = EINVAL;
    message_log("_write_subliminal_data", strerror(errno));
    return I_STEGANOS_ERR;
  }

  tmp_written = 0;
  for(i=0; i<res_len; i++) {
    
    pos = ss->res_lineup[i];
    if(residue[pos] < 0) {
      neg = 1;
    } else {
      neg = 0;
    }

    /* Gets the maximum and minimum bit capacity "estimations" */
    max_bits = ss->res_max_capacity[pos];
    min_bits = ss->res_min_capacity[pos];
    
    /* Gets the "estimated" and recommended range, as we will have to manage
       integers, we take the ceil and floor for the upper and lower limits */
    if((residue[pos] < 0 && ss->variation_limit[pos][0] < 0) ||
       (residue[pos] > 0 && ss->variation_limit[pos][0] > 0)) {
/*     if(signbit(residue[pos]) == signbit(ss->variation_limit[pos][0])) { */
      upper = ceil(residue[pos]+ss->variation_limit[pos][0]);
      lower = floor(residue[pos]+ss->variation_limit[pos][1]);
    } else {
      lower = ceil(residue[pos]+ss->variation_limit[pos][0]); 
      upper = floor(residue[pos]+ss->variation_limit[pos][1]);
    }

    /* The more number of bits we can hide, the better */
    sub_value = 0;

    /* Get the next 'max_bits' subliminal bits... but if there isn't room enough
       for max_bits, we reduce it. */
    if((max_bits + tmp_written) > d_len) {
      max_bits = d_len - tmp_written;
      if(max_bits < min_bits) {
	min_bits = max_bits;
      }
    }

    for(j=0; j<max_bits; j++) {
      rd_byte = (tmp_written+j)/BITS_PER_BYTE;
      rd_bit = (BITS_PER_BYTE-1) - ((tmp_written+j) % BITS_PER_BYTE);
      bit = (data[rd_byte] >> rd_bit) % 2;
      sub_value |= (bit << (max_bits - j - 1));
    }

    /* This inner loop will get the greatest number of subliminal bits that fit
       in the current residue, so it'll start with max_bits and will try every
       option until min_bits. The first of the values that fits in the range, 
       will be hided. If no value fits, the value which minimizes the differnece
       with the range (using the lower and upper limits as references) will be 
       stored, to be used in the case a "relaxation protocol" is desired to be 
       used. */
    diff = INT_MAX; nearest = INT_MAX;
    for(j=max_bits; j>=min_bits && j>0; j--) {        
      curr_value = sub_value + (1 << j);

      if(curr_value >= abs(lower) && curr_value <= abs(upper)) { /* Fits!! */
	residue[pos] = curr_value;
	tmp_written += j;
	break;
      } else {
	
	if(diff > abs(curr_value-lower) || diff > abs(curr_value-upper)) {
	  nearest = curr_value;
	  if(abs(curr_value-lower) < abs(curr_value-upper)) {
	    diff = abs(curr_value-lower);
	  } else {
	    diff = abs(curr_value-upper);
	  }
	}

      }
      
      /* Discard the least significant subliminal bit */
      sub_value >>= 1;
      
    }

    /* If none of the possible values fit the recommended range, we choose the
       option that minimizes the commited error, which will be stored in the 
       'nearest' variable. */
    if(j < min_bits || (j == 0 && diff != INT_MAX )) {
      if(nearest == INT_MAX) {
	message_log("_write_subliminal_data", "Unknown error");
	return I_STEGANOS_ERR;
      }
      residue[pos] = nearest;
      while(abs(nearest) > 1) { /* Efficient integer binary logarithm */
	tmp_written++;
	nearest = (nearest >> 1);
      }
    }

    if(neg) {
      residue[pos] *= -1;
    }

    /* @todo Adjust the last residue if it's not completely filled! */
    if(tmp_written >= d_len) {
      break;
    }
   
  }

  *written = tmp_written;

  return I_STEGANOS_OK;
}

/**
 * @fn static int _read_subliminal_residue(const float residue, byte *data, int *read)
 * @brief Reads the subliminal data hided in the given residue.
 *
 * Recovers the subliminal bits hided in the received residue. Note that the bits
 * obtained may not be plain message bits (disregarding the may have been 
 * ciphered) in the sense that they can have been subject to other 
 * transformations depending on the hiding method used. This function recovers the
 * raw hidden data.
 *
 * @param[in] residue The residual value from which we want to recover data.
 * @param[out] data The bitstream in which we will store the read bits.
 * @param[out] read The number of bits read from the residue.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 * @retval I_STEGANOS_ERR with errno = ERANGE (wrong residual value read)
 * @see steganos_state_t
 */
static int _read_subliminal_residue(const float residue, byte *data, int *read) {

  float fvalue;
  int i, ivalue, msab, tmp_read, bit;
  byte *tmp;


  /* Input parameters control */
  if(!data || !read) {
    errno = EINVAL;
    message_log("_read_subliminal_residue", strerror(errno));
    return I_STEGANOS_ERR;
  }

  /* If the received abs(residue) is 0 or 1, there is nothing to read */
  if(residue >= -1 && residue <= 1) {
    *read = 0;
    return I_STEGANOS_OK;
  }
  
  fvalue = fabs(residue);

  /* We don't want to fill data with crap until we know everything has gone OK */
  if(!(tmp = (byte *) malloc(sizeof(byte)*sizeof(float)))) {
    message_log("_read_subliminal_residue", strerror(errno));
    return I_STEGANOS_ERR;
  }

  memset(tmp, 0, sizeof(float));

  /* Get the bits at the left of the Most Significant Active bit */
  msab = (int) floor(logf(fvalue)/logf(2.f));
  fvalue -= (1 << msab); /* Discard the MSbit */

  /* Range check */
  if(fvalue < INT_MIN || fvalue > INT_MAX) {
    errno = ERANGE;
    message_log("_read_subliminal_residue", strerror(errno));
    return I_STEGANOS_ERR;
  } else {
    ivalue = (int) fvalue;
  }

  /* The remaining bits (of the integer representation) are all subliminal and we
     must read them from the most to the least significant bit */
  tmp_read = 0;
  for(i=msab-1; i>=0; i--) {
    /* Gets the subliminal bit */
    bit = ((1 << i) & ivalue) >> i;
    /* Writes it in it's corresponding location of tmp */
    tmp[i/BITS_PER_BYTE] |= (bit << (i % BITS_PER_BYTE));
    tmp_read++;
  }

  *read = tmp_read;
  memcpy(data, tmp, ceil((float) tmp_read / (float) BITS_PER_BYTE));
  free(tmp);

  return I_STEGANOS_OK;
}


/**
 * @fn static int _loudness_noise_control(steganos_state_t *ss, const float *residue, 
			   float *sub_residue, const int res_len)
 * @brief This function tries to limit the effect of the subliminal data in the
 *  loudness and the noise perceived by a final "listener".
 *
 * @param[in, out] ss Internal state structure. 
 * @param[in] residue The original residue, used as reference to measure the
 *  modifications.
 * @param[in, out] sub_residue The modified residue, containing the subliminal
 *  data. In a successful execution, the sub_residue may be changed reducing
 *  the loudness variation and noise introduced by the subliminal data. The
 *  subliminal bits will never be changed, as they contain the subliminal data.
 * @param[in] res_len Number of residual values in residue and sub_residue.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 * @see steganos_state_t
 */
static int _loudness_noise_control(steganos_state_t *ss, const float *residue, 
			   float *sub_residue, const int res_len) {


  /* Input parameters control */ // TODO!! todo controlado?
  if(!ss || !residue || !sub_residue || res_len <= 0) {
    errno = EINVAL;
    message_log("_loudness_noise_control", strerror(errno));
    return I_STEGANOS_ERR;
  }

  /** Noise control: No noise control for now...*/

  /** Loudness control: No loudness control for now...*/

  return I_STEGANOS_OK;
}

/**
 * @fn static int _residue_variation(const float residue, const float frequency, 
 *                            float *variation_limit)
 * @brief Calculates the negative and positive variations for the given residual
 *  value and frequency.
 *
 * Calculates the negative and positive maximum variations for the given residual
 * value at the given frequency using the noise tolerances proposed in 
 * ITU-R BS. 468-4.
 *
 * @param[in] residue The original residue value.
 * @param[in] frequency The residual value frequency.
 * @param[out] variation_limit a 2-dimension array, which will store the
 *  {negative, positive} maximum variations.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 */
static int _residue_variation(const float residue, const float frequency, 
			      float *variation_limit) {


  /* Input parameters control */
  if(frequency < 0 || !variation_limit) {
    errno = EINVAL;
    message_log("_residue_variation", strerror(errno));
    return I_STEGANOS_ERR;
  }

  /** The model used for obtaining the maximum tolerances is ITU-R BS. 468-4 */
  if(_itu468_var_tol(frequency, residue, variation_limit) == I_STEGANOS_ERR) {
    return I_STEGANOS_ERR;
  }

  return I_STEGANOS_OK;
}

int steganos_state_init(steganos_state_t *ss, int da, int hide_method, 
			int sync_method, char *key, int keylen) {
  
  /* Input parameter's control */
  if(!ss || !key || keylen < 128) {
    errno = EINVAL;
    message_log("steganos_state_init", strerror(errno));
    return I_STEGANOS_ERR;
  }

  if(da < 1 || da > 10) {
    errno = EINVAL;
    message_log("steganos_state_init", "Wrong aggressiveness");
    return I_STEGANOS_ERR;
  }

  if(hide_method < 0 || hide_method > 1) {
    errno = EINVAL;
    message_log("steganos_state_init", "Wrong hiding method");
    return I_STEGANOS_ERR;
  }

  if(sync_method < 0 || sync_method > 1) {
    errno = EINVAL;
    message_log("steganos_state_init", "Wrong synchronization method");
    return I_STEGANOS_ERR;
  }

  /* Master key: will be used to derive the hiding and synchro subkeys */
  if(!(ss->master_key = (steganos_key_t *) malloc(sizeof(steganos_key_t)))) {
    message_log("steganos_state_init", strerror(errno));
    return I_STEGANOS_ERR;
  }
  
  if(steganos_key_init((byte *) key, keylen, 
	       ss->master_key) == I_STEGANOS_ERR) {
    free(ss->master_key);
    return I_STEGANOS_ERR;
  }

  ss->synchro_key = NULL;
  ss->hiding_key = NULL;

  /* ss->synchro_method will be set to the value obtained by parsing the field
     synchro_method from the file steganos_config.xml' */
  ss->synchro_method = sync_method;
  ss->desync = 0;

  /* ss->synchronize will be set in agreement with synchro_method_et */
  if(ss->synchro_method == ISS) {
    ss->synchronize = synchro_iss;
    ss->desynchronize= desynchro_iss;
  } else {
    ss->synchronize = NULL; /* Currently, just ISS or RES_HEADER */
    ss->desynchronize = desynchro_res_header;
  }

  /* ss->hide_method will be set to the value obtained by parsing the field 
     hide_method from the file 'steganos_config.xml' */
  ss->hide_method = hide_method;

  /* ss->hide will be set in agreement with hide_mode_et */
  if(ss->hide_method == PARITY_BITS) 
    ss->hide = parity_bits_method;
  else 
    ss->hide = void_method;

  /* ss->da will be set to the value obtained by parsing the field aggressiveness
     from the file 'steganos_config.xml' */
  ss->da = da;
  ss->ra = ss->da;

  memset(ss->variation_limit, 0, sizeof(float)*(2*VORBIS_MAX_BLOCK));
   
  ss->max_fc_capacity = 0.f;
  ss->min_fc_capacity = 0.f;

  memset(ss->res_max_capacity, 0, sizeof(int)*VORBIS_MAX_BLOCK);
  memset(ss->res_min_capacity, 0, sizeof(int)*VORBIS_MAX_BLOCK);
  memset(ss->res_lineup, 0, sizeof(int)*VORBIS_MAX_BLOCK);
  memset(ss->res_occupied, 0, sizeof(int)*VORBIS_MAX_BLOCK);
  memset(ss->out, 0, sizeof(int)*VIF_POSIT+2);

  if(!(ss->prng = (prng_t *) malloc(sizeof(prng_t)))) {
    message_log("steganos_state_init", strerror(errno));
    return I_STEGANOS_ERR;
  }

  if(prng_init(ss->prng) == I_ERR) {
    free(ss->prng); ss->prng = NULL;
    return I_STEGANOS_ERR;
  }

  ss->status = I_STEGANOS_OK;
  ss->sent = 0;
  ss->read = 0;
  ss->aligned = 0;
  ss->total_sub_capacity = 0;
  ss->metadata_sent = 0;
  ss->iters = 0;

  return I_STEGANOS_OK;
   
}

int steganos_state_reset_iter(steganos_state_t *ss) {

  if(!ss) {
    errno = EINVAL;
    message_log("steganos_state_reset_iter", strerror(errno));
    return I_STEGANOS_ERR;
  }

  ss->max_fc_capacity = 0.f;
  ss->min_fc_capacity = 0.f;
  ss->status = I_STEGANOS_OK;
  ss->aligned = 0;

  steganos_free_packet_keys(ss);

  memset(ss->variation_limit, 0, sizeof(float)*(2*VORBIS_MAX_BLOCK));  
  memset(ss->res_max_capacity, 0, sizeof(int)*VORBIS_MAX_BLOCK);
  memset(ss->res_min_capacity, 0, sizeof(int)*VORBIS_MAX_BLOCK);
  memset(ss->res_lineup, 0, sizeof(int)*VORBIS_MAX_BLOCK);
  memset(ss->res_occupied, 0, sizeof(int)*VORBIS_MAX_BLOCK);
  memset(ss->out, 0, sizeof(int)*VIF_POSIT+2);
 
  if(!(ss->prng)) {

    if(!(ss->prng = (prng_t *) malloc(sizeof(prng_t)))) {
      message_log("steganos_state_reset_iter", strerror(errno));
      return I_STEGANOS_ERR;
    }

    if(prng_init(ss->prng) == I_ERR) {
      free(ss->prng); ss->prng = NULL;
      return I_STEGANOS_ERR;    
    }

  } 

  return I_STEGANOS_OK;

}

int steganos_state_free(steganos_state_t *ss) {

  if(!ss) {
    return I_STEGANOS_OK;
  }

  if(ss->master_key) {
    free(ss->master_key->key);
    free(ss->master_key);
  }

  /* steganos_key_t *synchro_key */
  if(ss->synchro_key) {
    free(ss->synchro_key->key);
    free(ss->synchro_key);
  }

  /* steganos_key_t *hiding_key */
  if(ss->hiding_key) {
    free(ss->hiding_key->key);
    free(ss->hiding_key);
  }

  if(ss->prng) {
    if(prng_free(ss->prng) == I_ERR) {
      return I_STEGANOS_ERR;
    }
  }

  free(ss->prng);
  ss->prng = NULL;

  return I_STEGANOS_OK;

}

int steganos_prepare_packet_keys(vorbis_config_t *vc, steganos_state_t *ss) {

  gcry_error_t gce;
  gcry_cipher_hd_t chd;
  byte *buffer, *digest;
  unsigned int md_len, buffer_len, forward_len;

  /** @todo Currently, the hiding and synchro subkeys are the same. It will be
            nice to derive them in a different way. */

  /* Input parameters control */
  if(!vc || !ss) {
    errno = EINVAL;
    message_log("steganos_prepare_packet_keys", strerror(errno));
    return I_STEGANOS_ERR;
  }

  /* Initialize the cipher algorithm to obtain the current packet key */
  gce = gcry_cipher_open(&chd, GCRY_CIPHER_ARCFOUR, GCRY_CIPHER_MODE_STREAM,
			 GCRY_CIPHER_SECURE);
  if(gce != GPG_ERR_NO_ERROR) {
    message_log("steganos_prepare_packet_keys", gcry_strerror(gce));
    return I_CRYPTOS_ERR;
  }

  gce = gcry_cipher_setkey(chd, ss->master_key->key, 
			   ss->master_key->length/BITS_PER_BYTE);
  if(gce != GPG_ERR_NO_ERROR) {
    message_log("steganos_prepare_packet_keys", gcry_strerror(gce));
    gcry_cipher_close(chd);
    return I_STEGANOS_ERR;
  }

  md_len = gcry_md_get_algo_dlen(GCRY_MD_MD5);
  if(!(digest = (byte *) malloc(sizeof(byte)*md_len))) {
    message_log("steganos_prepare_packet_keys", gcry_strerror(gce));
    gcry_cipher_close(chd);
    return I_STEGANOS_ERR;
  }
  memset(digest, 0, md_len);

  forward_len = (int) ceilf((float)(vc->posts_len*sizeof(*vc->forward_index)));
  if(forward_len < md_len) buffer_len = md_len;
  else buffer_len = forward_len;

  if(!(buffer = (byte *) malloc(sizeof(byte)*buffer_len))) {
    gcry_cipher_close(chd);
    message_log("steganos_prepare_packet_keys", strerror(errno));
    return I_STEGANOS_ERR;
  }
  memset(buffer, 0, buffer_len);

  memcpy(buffer, vc->forward_index, buffer_len);

  /* Obtain md5(vc->forward_index) */
  gcry_md_hash_buffer(GCRY_MD_MD5, digest, buffer, buffer_len);
  free(buffer);

  /* Cipher the result */
  gce = gcry_cipher_encrypt(chd, digest, md_len, NULL, 0);
  if(gce != GPG_ERR_NO_ERROR) {
    gcry_cipher_close(chd);
    free(buffer);
    message_log("steganos_prepare_packet_keys", gcry_strerror(gce));
    return I_STEGANOS_ERR;
  }  

  gcry_cipher_close(chd);

  /* Set it as frame specific keys */
  if(!ss->hiding_key) {
    if(!(ss->hiding_key = (steganos_key_t *) malloc(sizeof(steganos_key_t)))) {
      message_log("steganos_prepare_packet_keys", strerror(errno));
      free(digest);
      return I_STEGANOS_ERR;
    }
  }
  
  if(steganos_key_init(digest, 128, ss->hiding_key) == I_STEGANOS_ERR) {
    free(digest);
    message_log("steganos_prepare_packet_keys", gcry_strerror(gce));
    return I_STEGANOS_ERR;    
  }

  if(!ss->synchro_key) {
    if(!(ss->synchro_key = (steganos_key_t *) malloc(sizeof(steganos_key_t)))) {
      message_log("steganos_prepare_packet_keys", strerror(errno));
      free(digest);
      return I_STEGANOS_ERR;
    }
  }
  
  if(steganos_key_init(digest, 128, ss->synchro_key) == I_STEGANOS_ERR) {
    free(digest);
    message_log("steganos_prepare_packet_keys", gcry_strerror(gce));
    return I_STEGANOS_ERR;    
  }
  
  free(digest);
  return I_STEGANOS_OK;

}

int steganos_free_packet_keys(steganos_state_t *ss) {

  if(!ss) {
    return I_STEGANOS_OK;
  }

  if(ss->hiding_key) {
    free(ss->hiding_key->key);
    ss->hiding_key->key = NULL;
    free(ss->hiding_key);
  }
  
  ss->hiding_key = NULL;

  if(ss->synchro_key) {
    free(ss->synchro_key->key);
    ss->synchro_key->key = NULL;
    free(ss->synchro_key);
  }

  ss->synchro_key = NULL;

  return I_STEGANOS_OK;
}

int set_subliminal_capacity_limit(steganos_state_t *ss, float *residue, 
				  const long rate, const int res_len) {

  float max_fc_capacity, min_fc_capacity, esrv, osrv, nbits;
  int i, aux;


  /* Input parameters control */
  if(!ss || !residue || res_len <= 0) {
    errno = EINVAL;
    message_log("set_subliminal_capacity_limit", strerror(errno));
    return I_STEGANOS_ERR;
  }

  max_fc_capacity = 0.f;
  min_fc_capacity = 0.f;

  /* Reset the limits */
  for(i=0; i<res_len; i++) {
    if(_residue_variation(residue[i], i*((float)rate/(2.f*res_len)),
			  ss->variation_limit[i]) == I_STEGANOS_ERR) {
      return I_STEGANOS_ERR;
    }
    
    /* Update maximum capacity */
    if((residue[i] < 0 && ss->variation_limit[i][0] < 0) ||
       (residue[i] > 0 && ss->variation_limit[i][0] > 0)) {
      esrv = ss->variation_limit[i][0];
      osrv = ss->variation_limit[i][1];
    } else {
      osrv = ss->variation_limit[i][0];
      esrv = ss->variation_limit[i][1];
    }

    /** <i> ss->res_max_capacity (MCRVC) </i> and <i>ss->res_min_capacity</i>
	(mCRVC) calculation example:
	<ul>
	<li>Current Residual Value (CRV) = 17</li>
	<li>Residue Limit Variations (RLV) = [-7.6, 15.89]</li>
	<li>Equally Signed Residue Variation (ESRV) = 15.89</li>
	<li>Opposite Signed Residue Variation (OSRV) = -7.6</li>
    	<li>Maximum Current Residual Value Capacity (MCRVC) = 
	ceil(log2(fabs(CRV+ESRV)))-1 = ceil(log2(32.89)) - 1 = 5 bits</li>
    	<li>minimum Current Residual Value Capacity (mCRVC) = 
	ceil(log2(CRV+OSRV))-1 = ceil(log2(9.4)) - 1 = 3 bits</li>
	</ul>
	<i>Note: if CRV+ESRV is an integer number, the results in MCRVC and
	mCRVC are the obtained previously + 1.</i><br>

	Taking the ESRV and OSRV to obtain the MCRVC and mCRVC, respectively, 
	is due to the fact that when the variation made to the original residue
	is made in the same sense (positive or negative) than the residue, we 
	will have at least the same quantity of bits to modify (and probably 
	more), while having less bits to modify if we were to modify the original
	value in the opposite sense.

     */   

    /* Obtain maximum subliminal capacity for the current residue:
       Alternative binary logarithm implementation (more efficient than logf) */
    aux = (int) rintf(fabs(residue[i]+esrv));
    nbits = 0;
    while(aux > 1) {
      nbits++;
      aux >>= 1;
    }

    /* Range control, shouldn't happen */
    if(nbits > BITS_PER_BYTE*sizeof(int)) {
      nbits = BITS_PER_BYTE*sizeof(int);
    }

    /* If the previous gives a positive (and >= 1) number of bits to modify 
       means we can hide info in the current residual value */
    if(nbits >= 1) {
      
      /* Update current residual capacity and maximum frame and channel capacity */
      ss->res_max_capacity[i] = nbits; /* This means the nbits least significant bits
					  of the residual value i are prone to shelter
					  subliminal bits*/
      max_fc_capacity += nbits;

    }

    /* Obtain minimum subliminal capacity for the current residue
       Alternative binary logarithm implementation (more efficient than logf) */
    aux = (int) rintf(fabs(residue[i]+osrv));
    nbits = 0;
    while(aux > 1) {
      nbits++;
      aux >>= 1;
    }

    /* Range control, shouldn't happen */
    if(nbits > BITS_PER_BYTE*sizeof(int)) {
      nbits = BITS_PER_BYTE*sizeof(int);
    }

    /* If the previous gives a positive (and >= 1) number of bits to modify 
       means we can hide info in the current residual value */
    if(nbits >= 1) {

      /* Update current residual capacity and maximum frame and channel capacity */
      ss->res_min_capacity[i] = nbits; /* This means the nbits least significant bits
					  of the residual value i are prone to shelter
					  subliminal bits*/
      min_fc_capacity += nbits;

    }

/*     if(!i) */
/*       fprintf(stderr, "%d: res: %.3f, max_c: %.3f, min_c: %.3f; max_var: %.3f, min_var : %.3f\n", */
/* 	      ss->iters, residue[i], ss->res_max_capacity[i], ss->res_min_capacity[i], esrv, osrv); */
    
    if(ss->res_min_capacity[i] > ss->res_max_capacity[i]) {
      nbits = ss->res_min_capacity[i];
      ss->res_min_capacity[i] = ss->res_max_capacity[i];
      ss->res_max_capacity[i] = nbits;
    }


  }

  /* Update the remaining internal state structure variables */
  ss->max_fc_capacity = max_fc_capacity;
  ss->min_fc_capacity = min_fc_capacity;

  return I_STEGANOS_OK;
}

int hide_data(steganos_state_t *ss, byte *data, const int d_len, 
	      int *floor, float *residue, const int res_len, int *size) {

  float usage, *sub_residue, p;
  long int prng_iters;
  int iusage, write, written, fail;
  byte *sub_data;


  /* Input parameters control */
  if(!ss || d_len < 0 ||
     !floor || !residue || res_len <= 0 || !size) {
    errno = EINVAL;
    message_log("hide_data", "Wrong input");
    return I_STEGANOS_ERR;
  }

  /** To calculate the desired number of bits we will try to hide, we use the
      <i>desired aggresiveness (da></i> and <i>real aggresiveness (ra)</i>
      values. The first represents the share of subliminal channel (in 1/10) which
      we want to use for subliminal transmission, and the second, represents the
      share of subliminal channel <i>really</i> used until the present moment.
      So we'll use <i>ra</i> as corrector to achieve a final share as near as 
      possible to <i>da</i>. Many approaches can be used for this purpose, but
      we'll use the simplest one for now, i.e., the aggresiveness used for the
      current frame will be \f$ (da - ra) + da \f$. This way
      we guarantee that each frame will "theorethically" correct the deviation
      made previously, as \f$ \frac{(da - ra) + da + ra}{2} = da\f$, and, for
      any given frame, <i>ra</i> represents, as said, the real aggresiveness 
      until that very frame.
  */
  p = (ss->da - ss->ra) + ss->da;
  if(p < 0) p = 0;
  if(p > 10) p = 10;
  usage = (p*ss->max_fc_capacity)/(10.f);

  /* #ifdef STEGANOS_DEBUG */
  /*   { */
  /*     char siter[10], susage[200]; */

  /*     memset(siter, 0, 10*sizeof(char)); */
  /*     memset(susage, 0, 200*sizeof(char)); */
  /*     sprintf(siter, "%d)", ss->iters); */
  /*     sprintf(susage, "max: %.3f, usage: %f, da: %d, ra: %.3f, p: %.3f",  */
  /* 	    ss->max_fc_capacity, usage, ss->da, ss->ra, p); */
  /*     message_log(siter, susage); */
  /*   } */
  /* #endif */

  /* TODO!! ss->ra must range from 0 to 10 */

  /* Range control, usage shouldn't be greater than MAX_CAPACITY in any case */
  iusage = 0;
  if(usage > MAX_SUBLIMINAL_SIZE) {
    iusage = MAX_SUBLIMINAL_SIZE;
  } else if(usage < 0) {
    iusage = 0;
  } else { 
    iusage = (int) rint(usage); /* May raise some warning, but it's controlled */
  }

  /* If we are using ISS as synchronization method, the maximum amount of
     subliminal bits we can send is MAX_SUBLIMINAL_SIZE-1 given that the
     RES_HEADER is 0xFF, because if we send a 0xFF as size field, we will
     not be able to distinguish between a FORCED_RES_HEADER synchronization
     or an actual frame with ISS and 0xFF subliminal bits */
  if(iusage == MAX_SUBLIMINAL_SIZE)
    if(ss->synchro_method == ISS || FORCED_RES_HEADER) iusage--;

  /* The subliminal data will have at most iusage bits of length */
  sub_data = (byte *) malloc(sizeof(byte)*
			     ((int) ceilf((float)iusage/(float)BITS_PER_BYTE)));
  if(!sub_data) {
    message_log("hide_data", strerror(errno));
    return I_STEGANOS_ERR;
  }

  /* We use a copy of the residue vector, we don't want to change it before
     before everything has been done (so that if an error occurs, no change 
     will be made) */
  if(!(sub_residue = (float *) malloc(sizeof(float)*res_len))) {
    free(sub_data);
    sub_data = NULL;
    message_log("hide_data", strerror(errno));
    return I_STEGANOS_ERR;  
  }   
  
  /* It is possible that, although having estimated a max capacity, given the 
     received sequence of subliminal bits to hide (of which we can't modify
     the order it is given in) we can't hide as much data as we thought, and
     if that happens, we have to reduce the size field (this shouldn happen
     less as ss->da decreases) */
  write = iusage;
  written = 0;
  fail = write - written;
  prng_iters = ss->prng->iters;

  while(fail) {

    memcpy(sub_residue, residue, res_len*sizeof(*residue));
    memset(sub_data, 0, 
	   (int)(ceilf((float)iusage/(float)BITS_PER_BYTE)*sizeof(byte)));

    /* Retrieve 'iusage' bits of subliminal message prepared to be hided */
    if(_get_subliminal_data(ss, data, d_len, floor, residue, res_len,
			    sub_data, &write) == I_STEGANOS_ERR) {
      free(sub_residue);
      free(sub_data);
      sub_data = NULL;
      return I_STEGANOS_ERR;
    }
        
    /* Write the usage subliminal bits in the ordering marked by the PRNG
       seeded with the hiding key */
    if(_write_subliminal_data(ss, sub_data, write, sub_residue, 
			      res_len, &written) == I_STEGANOS_ERR) {
      free(sub_residue);
      free(sub_data);
      sub_data = NULL;
      return I_STEGANOS_ERR;
    }

    if(written > write) {
      message_log("hide_data", "Unknown error in _write_subliminal_data");
      free(sub_residue);
      free(sub_data);
      sub_data = NULL;
      return I_STEGANOS_ERR;
    }

    fail = write - written;
    if(fail) {

      if(prng_reset_byte(ss->prng, ss->hiding_key->key, ss->hiding_key->length,
			 prng_iters) == I_ERR) {
	free(sub_residue);
	free(sub_data);
	sub_data = NULL;
	return I_STEGANOS_ERR;
      }

    }

    write = written;

  }

#ifdef STEGANOS_DEBUG
  {
    char siter[50], sdata[400];
    unsigned long int i;
    
    memset(siter, 0, 50*sizeof(char));
    memset(sdata, 0, 400*sizeof(char));
    sprintf(siter, "%d) sending %d bits", ss->iters, write);
    for(i=0; i<ceilf((float)write/(float)BITS_PER_BYTE); i++) {
      sprintf(&sdata[2*i], "%X", sub_data[i]&0xF0);
      sprintf(&sdata[2*i+1], "%X", sub_data[i]&0x0F);
      
    }
    message_log(siter, sdata);
  }
#endif

  /* Control the loudness variation and noise introduced by the 
     subliminal data */
  /* Not yet implemented... */
  /*   if(_loudness_noise_control(ss, residue, sub_residue,  */
  /* 			     res_len) == I_STEGANOS_ERR) { */
  /*     return I_STEGANOS_ERR;     */
  /*   } */
  
  *size = 0;
  if((ss->synchro_method == ISS && written >= SIZE_FIELD_BITS) ||
     (ss->synchro_method == RES_HEADER && 
      written >= (SIZE_FIELD_BITS+(SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE)))) {

    *size = written;
    ss->metadata_sent += *size;
    
    /* We have to discard the header bits from the total amount of data hided */
    *size -= SIZE_FIELD_BITS;
    if(ss->synchro_method == RES_HEADER || ss->synchro_method == FORCED_RES_HEADER) {
      *size -= (SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE);
    }

    /* Everything has gone OK, copy the result */
    memcpy(residue, sub_residue, res_len*sizeof(*residue));
    //    free(sub_residue);

  } 

  free(sub_residue);

  /* Update the real aggressiveness til this moment */
  ss->total_sub_capacity += ss->max_fc_capacity;   
  ss->ra = 10.f*((float)ss->metadata_sent/(float)ss->total_sub_capacity); 

  free(sub_data);
  sub_data = NULL;
        
  return I_STEGANOS_OK;

}

int unhide_data(steganos_state_t *ss, int *floor, float *residue, 
		const int res_len, byte **data, int *read) {

  byte *bitstream, *floatbyte;
  int i, j, tmp_read, aux, sub_size, same, wr_byte, wr_bit, bit;
  int repeat;
  char sdata[1000]; // TODO!! borrar

  /* Input parameters control */ // TODO!! todo controlado?
  if(!ss || !floor || !residue || 
     res_len <= 0 || !data || !read) {
    errno = EINVAL;
    message_log("unhide_data", strerror(errno));
    *read = 0;
    return I_STEGANOS_ERR;
  }

  if(!(bitstream = (byte *) 
       malloc(sizeof(byte)*
	      ceilf((float)MAX_SUBLIMINAL_SIZE/(float)BITS_PER_BYTE)))) {
    message_log("unhide_data", strerror(errno));
    *read = 0;
    return I_STEGANOS_ERR;
  }
  
  memset(bitstream, 0, ceilf((float)MAX_SUBLIMINAL_SIZE/(float)BITS_PER_BYTE));

  /* Auxiliar buffer */
  if(!(floatbyte = (byte *) malloc(sizeof(byte)*sizeof(float)))) {
    message_log("unhide_data", strerror(errno));
    free(bitstream);
    *read = 0;
    return I_STEGANOS_ERR;    
  }

  tmp_read = 0;
  memset(sdata, 0, 1000*sizeof(char)); // TODO!! Borrar

  /* If we are using the RES_HEADER synchronization method, we will first
     check for the first SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE bits, if they are
     equal to the SYNCHRO_HEADER, we will continue reading, else, there is no
     subliminal data to recover in the current residue. */
  i = 0;
  if(ss->synchro_method == RES_HEADER || 
     ss->synchro_method == FORCED_RES_HEADER) {

    while(tmp_read < SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE) {

      /* Not enough capacity in the frame */
      if(i == res_len) {
	free(bitstream);
	free(floatbyte);
	*read = 0;
	return I_STEGANOS_OK;
      }

      memset(floatbyte, 0, sizeof(float));
      if(_read_subliminal_residue(residue[ss->res_lineup[i]], 
				  floatbyte,
				  &aux) == I_STEGANOS_ERR){
	free(bitstream);
	free(floatbyte);
	*read = 0;
	return I_STEGANOS_ERR;
      }

      /* Copy the 'aux' read bits in their place into bitstream */
      for(j=aux-1; j>=0; j--) {
	bit = (floatbyte[j/BITS_PER_BYTE] >> (j % BITS_PER_BYTE)) % 2;
	wr_byte = tmp_read / BITS_PER_BYTE;
	wr_bit = (BITS_PER_BYTE-1) - (tmp_read % BITS_PER_BYTE);
	bitstream[wr_byte] |= (bit << wr_bit);
	tmp_read++;
      }
            
      i++;

    }

    /* Apply the un-hiding method.
       Note: For now, the two existing methods are selfinvertibles. */
    memcpy(sdata, bitstream, SYNCHRO_HEADER_BYTES_RES); // TODO!!! borrar
    aux = SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE;
    if(ss->hide(bitstream, SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE, floor, 
		residue, res_len, ss->hiding_key, bitstream, &aux, 
		ss->prng) == I_STEGANOS_ERR) {
      free(bitstream);
      free(floatbyte);
      *read = 0;
      return I_STEGANOS_ERR;
    }

    if(aux != SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE) {
      message_log("unhide_data", "Wrong output size after unhide");
      free(bitstream);
      free(floatbyte);
      *read = 0;
      return I_STEGANOS_ERR;
    }
    
    /* Now we've read, at least, the first SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE 
       bits of the subliminal channel, and we can test if it matches with the
       SYNCHRO_HEADER */
    same = 1;
    for(j=0; j<SYNCHRO_HEADER_BYTES_RES; j++) {
      if(bitstream[j] != SYNCHRO_HEADER[j]) {
	same = 0;
      }
    }
    if(!same) {
      *read = 0; /* No match, nothing to read, nothing read */
      free(bitstream);
      free(floatbyte);
      {
	char siter[20], ssdata[2000];
	int kk;
	memset(siter, 0, 20);
	memset(ssdata, 0, 2000);
	sprintf(siter, "%d) ", ss->iters);
	for(kk=0; kk<SYNCHRO_HEADER_BYTES_RES; kk++) {
	  sprintf(&ssdata[2*kk], "%X", sdata[kk]&0xF0);
	  sprintf(&ssdata[2*kk+1], "%X", sdata[kk]&0x0F);
	}
	sprintf(&ssdata[2*kk+1], "Ignoring frame");
	message_log(siter, ssdata);
      }

      return I_STEGANOS_OK;
    }

    /* Now that we know there are subliminal bits hidden, we discard the 
       SYNCHRO_HEADER to make the following code compatible with the other hiding
       methods. */
    // TODO!! this fails in case of non byte multiple headers!!
    if(tmp_read > SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE) {

      aux = ceilf((float)tmp_read/(float)BITS_PER_BYTE)-SYNCHRO_HEADER_BYTES_RES;  
      for(j=0; j<aux; j++) {
	bitstream[j] = bitstream[j+SYNCHRO_HEADER_BYTES_RES];
      }
      for(j=aux; j<ceilf((float)tmp_read/(float)BITS_PER_BYTE); j++) {
	bitstream[j] = 0;
      }

    } else {
      memset(bitstream, 0, SYNCHRO_HEADER_BYTES_RES*sizeof(byte));
    }
    
    tmp_read -= SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE;

  }

  /* Once here, whichever the hiding method is, we have to read SIZE_FIELD_BITS
     bits to know how many subliminal bits are hided in the current residue. */
  repeat = 1;
  while(repeat) {

    while(tmp_read < SIZE_FIELD_BITS) {

      /* Not enough capacity in the frame */
      if(i == res_len) {
	free(bitstream);
	free(floatbyte);
	*read = 0;
	return I_STEGANOS_OK;
      }

      memset(floatbyte, 0, sizeof(float));
      if(_read_subliminal_residue(residue[ss->res_lineup[i]], 
				  floatbyte,
				  &aux) == I_STEGANOS_ERR){
	free(bitstream);
	free(floatbyte);
	*read = 0;
	return I_STEGANOS_ERR;
      }

      /* Copy the 'aux' read bits in their place into bitstream */
      for(j=aux-1; j>=0; j--) {
	bit = (floatbyte[j/BITS_PER_BYTE] >> (j % BITS_PER_BYTE)) % 2;
	wr_byte = tmp_read / BITS_PER_BYTE;
	wr_bit = (BITS_PER_BYTE-1) - (tmp_read % BITS_PER_BYTE);
	bitstream[wr_byte] |= (bit << wr_bit);
	tmp_read++;
      }

      i++;
    }

    // TODO!! borrar
    if(ss->synchro_method != ISS)
      memcpy(&sdata[SYNCHRO_HEADER_BYTES_RES], bitstream, 1);
    else 
      memcpy(sdata, bitstream, 1);

    /* Apply the un-hiding method.
       Note: For now, the two existing methods are selfinvertibles. */
    aux = SIZE_FIELD_BITS;
    if(ss->hide(bitstream, SIZE_FIELD_BITS, floor, residue, res_len,
		ss->hiding_key, bitstream, &aux, ss->prng) == I_STEGANOS_ERR) {
      free(bitstream);
      free(floatbyte);
      *read = 0;
      return I_STEGANOS_ERR;
    }
 
    if(aux != SIZE_FIELD_BITS) {
      message_log("unhide_data", "Wrong output size after unhide");
      free(bitstream);
      free(floatbyte);
      *read = 0;
      return I_STEGANOS_ERR;
    }

    /* Set the number of bits to read */
    sub_size = 0;
    aux = ceil((float)SIZE_FIELD_BITS/(float)BITS_PER_BYTE);
    for(j=0; j<aux; j++) {
      /* As always, using big endian ordering */
      sub_size += (((int)bitstream[j]) << ((aux - j - 1) * BITS_PER_BYTE));
    }

    // TODO!! this fails in case of non byte multiple headers!!
    if(tmp_read > SIZE_FIELD_BITS) {
    
      aux = ceilf((float)tmp_read/(float)BITS_PER_BYTE) -SIZE_FIELD_BITS/BITS_PER_BYTE;  
      for(j=0; j<aux; j++) {
	bitstream[j] = bitstream[j+SIZE_FIELD_BITS/BITS_PER_BYTE];
      }
      for(j=aux; j<ceilf((float)tmp_read/(float)BITS_PER_BYTE); j++) {
	bitstream[j] = 0;
      }
    
    } else {
      memset(bitstream, 0, (SIZE_FIELD_BITS/BITS_PER_BYTE)*sizeof(byte));
    }   

    tmp_read -= SIZE_FIELD_BITS;

    /* Might happen that we receive a size of 0xFF using ISS. This is not possible as
       this is reserved to FORCE_RES_HEADER synchronization, so, if that happens, we
       have to discard it and read the next SIZE_FIELD_BITS, those will be the real
       subliminal size. */
    if(sub_size == 0xFF) {
      repeat = 1;
    } else {
      repeat = 0;
    }

  }
  
  /* Read proper data */
  while(i<res_len && tmp_read < sub_size) {
  
    memset(floatbyte, 0, sizeof(float));
    if(_read_subliminal_residue(residue[ss->res_lineup[i]], 
				floatbyte,
				&aux) == I_STEGANOS_ERR){
      free(bitstream);
      free(floatbyte);
      *read = 0;
      return I_STEGANOS_ERR;
    }

    /* Copy the 'aux' read bits in their place into bitstream */
    for(j=aux-1; j>=0; j--) {
      bit = (floatbyte[j/BITS_PER_BYTE] >> (j % BITS_PER_BYTE)) % 2;
      wr_byte = tmp_read / BITS_PER_BYTE;
      wr_bit = (BITS_PER_BYTE-1) - (tmp_read % BITS_PER_BYTE);
      bitstream[wr_byte] |= (bit << wr_bit);
      tmp_read++;
    }

    i++;

  }

  /* Hack: if sub_size is not BITS_PER_BYTE-multiple, we place the remainder bits
     at the least significant positions of the last byte */
  if(sub_size % BITS_PER_BYTE) {
    bitstream[sub_size/BITS_PER_BYTE] >>= (BITS_PER_BYTE - (sub_size % BITS_PER_BYTE));
  }

  // TODO!! Borrar!!
  if(ss->synchro_method == ISS)
    memcpy(&sdata[1], bitstream, (int) ceilf((float)sub_size/(float)BITS_PER_BYTE));
  else
    memcpy(&sdata[SYNCHRO_HEADER_BYTES_RES+1], bitstream,
	   (int) ceilf((float)sub_size/(float)BITS_PER_BYTE));

  /* Apply the un-hiding method.
     Note: For now, the two existing methods are selfinvertibles. */
  aux = sub_size;  
  if(sub_size) {
    if(ss->hide(bitstream, sub_size, floor, residue, res_len,
		ss->hiding_key, bitstream, &aux, ss->prng) == I_STEGANOS_ERR){
      free(bitstream);
      free(floatbyte);
      *read = 0;
      return I_STEGANOS_ERR;
    }
  }

  /* Undo hack: I should find a more elegant way of doing this... */
  if(sub_size % BITS_PER_BYTE) {
    bitstream[sub_size/BITS_PER_BYTE] <<= (BITS_PER_BYTE - (sub_size % BITS_PER_BYTE));
  }

/*   { */
/*     char siter[20], ssdata[2000]; */
/*     int kk, ssize; */

/*     ssize = ceilf((float)sub_size/(float)BITS_PER_BYTE)+SYNCHRO_HEADER_BYTES_RES+1; */
/*     if(ss->synchro_method != ISS)  */
/*       ssize += SYNCHRO_HEADER_BYTES_RES; */

/*     memset(siter, 0, 20); */
/*     memset(ssdata, 0, 2000); */
/*     sprintf(siter, "%d)", ss->iters); */
/*     for(kk=0; kk<ssize; kk++) { */
/*       sprintf(&ssdata[2*kk], "%X", sdata[kk]&0xF0); */
/*       sprintf(&ssdata[2*kk+1], "%X", sdata[kk]&0x0F); */
/*     } */
/*     message_log(siter, ssdata); */

/*   } */
 
  free(floatbyte);

  /* The number of subliminal bits read and the actual number of subliminal bits
     sent (the number in the header's size field) may not be the same. We will
     always return the least of them. */
  if(aux < sub_size) {
    *read = aux;
  } else {
    *read = sub_size;
  }

  if(*read == 0) {
    free(bitstream);
    return I_STEGANOS_OK;
  }

  *data = bitstream;
  return I_STEGANOS_OK;

}

int parity_bits_method(byte *plain_mess, int p_m_len,
		       int *floor, float *res, int res_len, 
		       steganos_key_t *key, byte *sub_mess, 
		       int *s_m_len, prng_t *prng) {

  int p_m_r, s_m_wr, s_m_bytes, parity, run_length;
  int readbit, readbyte, readelem, floor_bits, rnd, i;
  byte *tmp;


  /* Input parameters control */ // TODO!! todo controlado?
  if(!plain_mess || p_m_len <= 0 || !floor || res_len <= 0 || 
     !sub_mess || !key || !s_m_len || *s_m_len <= 0 ||
     !prng) {
    errno = EINVAL;
    message_log("parity_bits_method", strerror(errno));
    return I_STEGANOS_ERR;
  }

  s_m_bytes = ceilf((float)(*s_m_len)/(float)BITS_PER_BYTE);
  if(!(tmp = (byte *) malloc(sizeof(byte)*s_m_bytes))) {
    message_log("parity_bits_method", strerror(errno));
    return I_STEGANOS_ERR;
  }
  memset(tmp, 0, s_m_bytes);

  p_m_r = 0;
  s_m_wr = 0;
  run_length = 0;
  floor_bits = res_len*BITS_PER_BYTE*sizeof(*floor);

  while(p_m_r < p_m_len) {
    
    /* Get the next plain_mess bit */
    readbyte = p_m_r / BITS_PER_BYTE;
    readbit = p_m_r % BITS_PER_BYTE;
    parity = (plain_mess[readbyte] >> readbit) % 2;

    /* The next sub_mess bit will be the result of XORing the next
       PARITY_BITS random bits with the current plain_mess bit */
    for(i=0; i<BITS_PARITY; i++) {

      /* Get a new pseudo random number */
      if(prng_get_random_int(prng, floor_bits, &rnd) == I_STEGANOS_ERR) {
	free(tmp);
	return I_STEGANOS_ERR;
      }

      readbyte = rnd / BITS_PER_BYTE;
      readelem = readbyte / sizeof(*floor);
      readbit = (rnd % BITS_PER_BYTE)+((readbyte % sizeof(*floor))*BITS_PER_BYTE);      
      parity ^= ((floor[readelem] >> readbit) % 2);

    }

    /* Write the obtained parity in the corresponding sub_mess bit */
    readbyte = s_m_wr / BITS_PER_BYTE;
    readbit = s_m_wr % BITS_PER_BYTE;
    
    /* Memory overflow control */
    if(readbyte >= s_m_bytes) {
      if(!(tmp = (byte *) realloc((byte *) tmp, sizeof(byte)*(s_m_bytes+1)))) {
	free(tmp);
	return I_STEGANOS_ERR;
      }
      s_m_bytes++;
    }

    tmp[readbyte] |= (parity << readbit);
    p_m_r++;
    s_m_wr++;

  }

  /* Prepare the return */
  if((int) ceilf((float)(*s_m_len)/(float)BITS_PER_BYTE) < s_m_bytes) {
    if(!(sub_mess = (byte *) realloc((byte *) sub_mess, sizeof(byte)*s_m_bytes))) {
      free(tmp);
      return I_STEGANOS_ERR;
    }
  }

  memcpy(sub_mess, tmp, s_m_bytes);
  *s_m_len = s_m_wr;  
  free(tmp);

  return I_STEGANOS_OK;
}

int void_method(byte *plain_mess, int p_m_len, 
		int *floor, float *res, int res_len,
		steganos_key_t *key, byte *sub_mess, 
		int *s_m_len, prng_t *prng) {


  /* Input parameters control */
  if(!plain_mess || p_m_len <= 0 || !sub_mess) {
    errno = EINVAL;
    message_log("void_method", strerror(errno));
    return I_STEGANOS_ERR;
  }

  /* sub_mess and plain_mess may point to the same direction... */
  if(sub_mess != plain_mess) {
    memcpy(sub_mess, plain_mess, (int) ceilf((float)p_m_len/(float)BITS_PER_BYTE));
  }

  *s_m_len = p_m_len;

  return I_STEGANOS_OK;
}

static void render_line0(int x0,int x1,int y0,int y1,int *d){
  int dy=y1-y0;
  int adx=x1-x0;
  int ady=abs(dy);
  int base=dy/adx;
  int sy=(dy<0?base-1:base+1);
  int x=x0;
  int y=y0;
  int err=0;

  ady-=abs(base*adx);

  d[x]=y;
  while(++x<x1){
    err=err+ady;
    if(err>=adx){
      err-=adx;
      y+=sy;
    }else{
      y+=base;
    }
    d[x]=y;
  }
}

int synchro_iss(vorbis_config_t *vc, steganos_key_t *key, int *posts, 
		float *residue, void *cfg, int decoding, int *bit, prng_t *prng) {

  iss_cfg_t *icfg;
  float r, var[2], posts_mean;
  int *work, i, j, b, *floor_ref, *floor_new, rc, rate, pcmend, mult;
  int lx, hx, ly, hy, current, previous, remake, old, variation;
  int *forward_index, *postlist, posts_len;


  /* Input parameters control */
  if(!vc || !posts || !cfg || !bit ||
     (!decoding && *bit != 0 && *bit != 1)) { // TODO!! todo controlado?
    errno = EINVAL;
    message_log("synchro_iss", strerror(errno));
    return I_STEGANOS_ERR;
  }

  rate = vc->rate;
  pcmend = vc->pcmend;
  mult = vc->mult;
  postlist = vc->postlist;
  forward_index = vc->forward_index;
  posts_len = vc->posts_len;
  icfg = (iss_cfg_t *) cfg;

  /* First of all, we have to calculate the original floor as would be calculated
     by the receiver may no subliminal data be hidden. This floor will be used as
     reference when estimating the dB variations introduced by the watermark. */
  if(!(floor_ref = (int *) malloc(sizeof(int)*pcmend/2))) {
    message_log("synchro_iss", strerror(errno));
    return I_STEGANOS_ERR;
  }
  
  if(iss_simulate_floor(vc, posts, floor_ref) == I_STEGANOS_ERR) {
    return I_STEGANOS_ERR;
  }

  /* Copy the posts to the working vector, to prevent modifying the original
     when encoding in case an error occurs. */
  if(!(work = (int *) malloc(sizeof(int)*posts_len))) {
    message_log("synchro_iss", strerror(errno));
    free(floor_ref);
    return I_STEGANOS_ERR;
  }

  /* Copy the posts vector to work, unsetting the "floor1_step2_flags", 
     calculate the projection of the floor over the watermark */
  posts_mean = 0.f;
  for(i=0; i<posts_len;i++) {
    work[i] = posts[i]&0x7fff;
    posts_mean += work[i];
  }
  posts_mean /= (float) posts_len;

  r = 0.f;
  for(i=0; i<posts_len;i++) {
    r += (work[i]-posts_mean)*icfg->u[i]; /* We want the statistic to be
					     centered in 0. */
  }

  /* Actually, here u_norm is the norm*N (N here is floor_len) as we use it to
     get the statistic r, which contains 1/N both in the numerator and
     denominator in it's formula and is therefore cancelled out in the result */
  r /= icfg->u_norm;

  /* Here, if we are decoding, we just return the sign of r as the received bit,
     being 0 if r < 0 and 1 if r > 0. But the posts vector, although follows a 
     Gausssian probability distribution (see Vorbis floor1 comments), does not
     have a mean centered in 0, so we have to shift it. */
  if(decoding) {
    free(floor_ref);
    free(work);
    if(r < 0) {
      *bit = 0;
      return I_STEGANOS_OK;
    } else if(r > 0 ) {
      *bit = 1;
      return I_STEGANOS_OK;
    } else if(r == 0) {
      message_log("synchro_iss",
		  "Not enough evidence to determine the presence of a watermark");
      *bit = INDETERMINATE_MARK;
#ifdef STEGANOS_DEBUG 
      message_log("synchro_iss", 
		  "Not enough statistical evidence to determine watermark presence");
#endif
      return I_STEGANOS_SYNC_FAIL;
    }
  }

  /* If we are here, it's because we are encoding, r stores the projection of
     the floor over the watermark (the inner product) and *bit the bit we want
     to hide. */
  if(*bit) {
    b = 1;
  } else {
    b = -1;
  }
 
  /* Floor synchronized "as it comes" */
  if(b*r > icfg->alpha/icfg->lambda) { /* See formula (31) from
					  Malvar's and Florencio's */
    for(i=0; i<posts_len; i++) {
      posts[i] = work[i];
    }
    free(floor_ref);
    free(work);
    return I_STEGANOS_OK;
  }

  /* Otherwise, we have to mark it */
  /**
     The Give Up concept explained in section V.B in Malvar's and Florencio's
     is simulated here in the following way:
	
     1) For each element in the posts vector, we get the variation to
     introduce if we were using a simple linear ISS.
     2) We calculate the floor that will be obtained by the decoder.
     3) We calculate the dB variance induced this way, taking as reference the
     the floor_ref vector.
     4) If it is acceptable given the ITU-R BS. 468-4 tolerances, we give it
     as valid. If not, we reduce the modifications until it an acceptable
     point.
     5) Once Obtained the final posts vector, we calculate again the floor as
     will be obtained by the decoder, and if we get the hided bit, means
     we have succeed in hiding it with an allowable distortion (Note that
     this approach assumes no noise will be introduced during 
     transmission).
  */

  if(!(floor_new = (int *) malloc(sizeof(int)*pcmend/2))) {
    message_log("synchro_iss", strerror(errno));
    free(floor_ref);
    free(work);
    return I_STEGANOS_ERR;
  }

  /* Introduce the watermark using linear ISS (formula 13 in Malvar's
     and Flornecio's) */
  for(i=0; i<posts_len; i++) {
    variation = (int) rint((icfg->alpha*b - icfg->lambda*r)*icfg->u[i]);
    if((work[i] + variation >= 0) && (work[i] + variation <= 255)) {
      work[i] += variation;
    }
  }

  /* generate quantized floor equivalent to what we'd unpack in decode */
  /* render the lines */
  hx=0;
  lx=0;
  ly=(work[0])*mult;
 
  for(i=1;i<posts_len;i++){

    /* Reset the "working" floor vector */
    memset(floor_new, 0, sizeof(int)*pcmend/2);

    previous=forward_index[i-1];
    current=forward_index[i];
    hy=work[current];
    if(hy==work[current]){
      hy*=mult;
      hx=postlist[current];
  
      render_line0(lx,hx,ly,hy,floor_new);

      /* Check if the variations introduced are allowable according to the ITU-R
         BS. 468-4 document specifications. */
      remake = 0;

      /* lx and hx are the lower and higher ends of the current interpolated
	 floor segment */
      for(j=lx; j<hx; j++) {

	/* var will store the maximum negative and positive variations the j-th
	   element in the floor could withstand accordingly to ITU-R BS. 468-4.
	   Note that despite most of the floor values aren't being actually
	   sent, we still have to control the distortion introduced in them. */
	var[0] = var[1] = 0.f;
	if(_itu468_var_tol(j*((float)rate/((float)pcmend)),
			   FLOOR1[floor_ref[j]],
			   var) == I_STEGANOS_ERR) {
	  free(work);
	  free(floor_ref);
	  free(floor_new);
	  return I_STEGANOS_ERR;
	}
	
	if((FLOOR1[floor_new[j]] < (FLOOR1[floor_ref[j]] + var[0])) ||
	   (FLOOR1[floor_new[j]] > (FLOOR1[floor_ref[j]] + var[1]))) {
	  
	  /* We reduce the variations to the maximum allowed */
	  /* Lower end */
	  rc = _itu468_var_tol(lx*((float)rate/((float)pcmend)),
			       FLOOR1[floor_ref[lx]],
			       var);
	  
	  /* We reduce the watermark, but it have to preserve the direction */
	  old = work[previous];
	  if(work[previous] > (posts[previous]&0x7fff)) {
	    while(FLOOR1[work[previous]] > (FLOOR1[(posts[previous]&0x7fff)]+var[1]) &&
		  work[previous] > (posts[previous]&0x7fff)) {
	      work[previous]--;
	    }
	  } else {
	    while(FLOOR1[work[previous]] < (FLOOR1[(posts[previous]&0x7fff)]+var[0]) &&
		  work[previous] < (posts[previous]&0x7fff)) {
	      work[previous]++;
	    }
	  }
	  
	  if(old != work[previous]) remake++;
	  
	  /* Higher end */
	  if(i==posts_len-1) {
	    rc += _itu468_var_tol(hx*((float)rate/((float)pcmend)),
				  FLOOR1[posts[1]&0x7fff],
				  var);
	  } else {
	    rc += _itu468_var_tol(hx*((float)rate/((float)pcmend)),
				  FLOOR1[floor_ref[hx]],
				  var);
	  }
	  
	  /* We reduce the watermark, but it have to preserve the direction */
	  old = work[current];
	  if(work[current] > (posts[current]&0x7fff)) {
	    while(FLOOR1[work[current]] > (FLOOR1[(posts[current]&0x7fff)]+var[1]) &&
		  work[current] > (posts[current]&0x7fff)) {
	      work[current]--;
	    }
	  } else {
	    while(FLOOR1[work[current]] < (FLOOR1[(posts[current]&0x7fff)]+var[0]) &&
		  work[current] < (posts[current]&0x7fff)) {
	      work[current]++;
	    }
	  }

	  if(old != work[current]) remake++;

	  if(rc) {
	    free(work);
	    free(floor_ref);
	    free(floor_new);
	    return I_STEGANOS_ERR;
	  }

	  /* Since we have modified the working "posts" vector, we have to repeat
	     the distortion control, to see if it is allowable with the reduction
	     made. */
	  if(remake) {
	    break;
	  }
	}
	
      }

      /* If there has been changes, we repeat the last iteration with the new
	 posts */
      if(remake) {
	remake = 0;
	i--; /* This will neutralize the "continue" */
	continue;
      }

      lx=hx;
      ly=hy;
      
    }

  }
  
  /* At this point either we have failed synchronizing, or we have a candidate,
     but we still have to see if it is "strong" enough. */
  free(floor_ref);
  
  /* Simulate the receiver's side */
  posts_mean = 0.f;
  for(i=0; i<posts_len;i++) {
    posts_mean += work[i];
  }
  posts_mean /= (float) posts_len;

  r = 0.f;
  for(i=0; i<posts_len; i++) {
    r += (work[i]-posts_mean)*icfg->u[i];
  }
  r /= icfg->u_norm; /* This is the statistic obtained by the receiver */

  /* If the decoder will correctly interpret the watermark, we've succeed */
  if((r<0 && b == -1) || (r>0 && b == 1)) {

    /* The working vector will be the new (watermarked) floor */
    memcpy(posts, work, posts_len*sizeof(*posts));
    free(work);
    free(floor_new);
    return I_STEGANOS_OK;
    
  }

  free(work);
  free(floor_new);

  /* The posts vector is not prone to be watermarked, i.e., its too strong in the
     opposite direction. */
#ifdef STEGANOS_DEBUG
  {
    char message[200];
    memset(message, 0, 200*sizeof(char));
    sprintf(message, "Unable to %d mark", *bit);
    message_log("synchro_iss", message);
  }
#endif
  *bit = !(*bit);
  return I_STEGANOS_SYNC_FAIL;

}

int iss_cfg_init(steganos_state_t *ss, int *posts, int posts_len, 
		 float sigma, iss_cfg_t *iss_cfg) {

  float noise_var, posts_var, posts_mean, u_var, lambda_opt, alpha, aux;
  float *u, u_norm;
  int i, rnd;

  /* Input parameters control */
  if(!posts || posts_len <= 0 || !iss_cfg) {
    errno = EINVAL;
    message_log("iss_cfg_init", strerror(errno));
    return I_STEGANOS_ERR;
  }

  iss_cfg->sigma = sigma; /** @todo Somehow deduce a good value for sigma */
  u_var = iss_cfg->sigma * iss_cfg->sigma;

  /** @todo We're assuming there is no noise in the channel, i.e., the system
      won't be attacked. A good noise statistical model might improve this 
      thing. */
  noise_var = 0.f;

  /* Obtain the mean value of the posts vector elements */ 
  /* I don't like how this is done. Seems inefficient */
  posts_mean = 0.f;
  for(i=0; i<posts_len; i++) {
    posts_mean += ((float)(posts[i]&0x7fff)/(float)posts_len);
  }

/*   fprintf(stderr, "posts_mean: %.6f\n", posts_mean); */

  /* Obtain the standard deviation of the posts vector elements */
  posts_var = 0.f;  
  for(i=0; i<posts_len; i++) {
    posts_var += ((float)((posts[i]&0x7fff) - posts_mean)*
		  ((posts[i]&0x7fff) - posts_mean))/((float)posts_len);
  }
  
  /* First part of the formula (20) for an optimized lambda */
  lambda_opt = (1+noise_var/posts_var+(posts_len*u_var)/posts_var);
  aux = lambda_opt;

  /* Second part of the formula */
  lambda_opt -= sqrtf(aux*aux - 4*(posts_len*u_var/posts_var));
  
  /* Check for math exceptions */
  if(fetestexcept(FE_INVALID) && errno == EDOM) {
    message_log("iss_cfg_init", strerror(errno));
    return I_STEGANOS_ERR;
  }

  lambda_opt *= 0.5f;
  
  iss_cfg->lambda = lambda_opt;

  /* Formula (33) for alpha */
  alpha = (posts_len*u_var - lambda_opt*lambda_opt*posts_var)/(posts_len*u_var);
  alpha = sqrtf(alpha);

  /* Check for math exceptions */
  if(fetestexcept(FE_INVALID) && errno == EDOM) {
    message_log("iss_cfg_init", strerror(errno));
    return I_STEGANOS_ERR;
  }

  iss_cfg->alpha = alpha;

  /* Finally, initialize the watermark */

  /* Seed the pseudo random number generator with the synchronization key. */
  /** @todo Add something frame-channel dependent to the key used to seed the
      PRNG for generating the marking sequence. */
  if(prng_set_seed_byte(ss->prng, ss->synchro_key->key, ss->synchro_key->length) == 
     I_STEGANOS_ERR) {
    return I_STEGANOS_ERR;
  }

  if(!(u = (float *) malloc(sizeof(float)*posts_len))) {
    message_log("iss_cfg_init", strerror(errno));
    return I_STEGANOS_ERR;
  }
  
  memset(u, 0, posts_len*sizeof(float)); 

  u_norm = 0.f;
  for(i=0; i<posts_len; i++) {

    /* Get a new pseudo random number */
    if(prng_get_random_int(ss->prng, 2, &rnd) == I_STEGANOS_ERR) {
      free(u);
      return I_STEGANOS_ERR;
    }

    if(rnd) {
      u[i] = iss_cfg->sigma;
    } else {
      u[i] = -iss_cfg->sigma;
    }
    
    u_norm += iss_cfg->sigma*iss_cfg->sigma;

  }

  iss_cfg-> u = u;
  iss_cfg->u_norm = u_norm;

  return I_STEGANOS_OK;

}

int iss_cfg_free(iss_cfg_t *iss_cfg) {

  if(iss_cfg) {
    if(iss_cfg->u) free(iss_cfg->u);
  }

  return I_STEGANOS_OK;
  
}

int iss_simulate_floor(vorbis_config_t *vc, int *posts, int *floor) {

  int hx, lx, ly, hy, current, j, pcmend, posts_len, *forward_index;
  int *postlist, mult;


  /* Input parameters control */
  if(!posts || !floor) {
    errno = EINVAL;
    message_log("iss_simulate_floor", strerror(errno));
    return I_STEGANOS_ERR;
  }

  pcmend = vc->pcmend;
  mult = vc->mult;
  postlist = vc->postlist;
  posts_len = vc->posts_len;
  forward_index = vc->forward_index;

  /* NOTE!! This code is the same (just some variable renaming) that the one
     at the end of 'floor1_encode' function in lib/floor1.c libvorbis file */

  /* generate quantized floor equivalent to what we'd unpack in decode */
  /* render the lines */
  hx=0;
  lx=0;
  ly=posts[0]*mult;
  for(j=1;j<posts_len;j++){
    current=forward_index[j];
    hy=posts[current]&0x7fff;
    if(hy==posts[current]){
	  
      hy*=mult;
      hx=postlist[current];

      render_line0(lx,hx,ly,hy,floor);
      lx=hx;
      ly=hy;
    }
  }

  for(j=hx;j<pcmend/2;j++) floor[j]=ly; /* be certain */
  return I_STEGANOS_OK;

}

int desynchro_res_header(vorbis_config_t *vc, int *res_lineup, int *posts, int *floor,
			 float *residue, void *cfg, steganos_key_t *hiding_key,
			 hide_method hide, prng_t *prng) {

  byte bitstream[10], floatbyte[10];
  int i, j, aux, wr_byte, wr_bit, tmp_read, same, bit, first=-1;
  

  /* Input parameters control */
  if(!vc || !residue || !res_lineup) { // TODO!! todo controlado?
    errno = EINVAL;
    message_log("desynchro_res_header", strerror(errno));
    return I_STEGANOS_ERR;
  }

  /* Take the first SYNCHRO_HEADER_BYTES_RES bits, if they coincide
     with the SYNCHRO_HEADER, change at least one. */
  i = 0; tmp_read = 0;
  memset(bitstream, 0, 10*sizeof(byte));
  while(tmp_read < SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE && i < vc->pcmend/2) {

    memset(floatbyte, 0, 10*sizeof(byte));

    if(_read_subliminal_residue(residue[res_lineup[i]], 
				floatbyte,
				&aux) == I_STEGANOS_ERR){
      message_log("desynchro_res_header", "Unable to desynchronize");
      return I_STEGANOS_ERR;
    }
    
    /* Copy the 'aux' read bits in their place into bitstream */
    for(j=aux-1; j>=0; j--) {
      if(first == -1) first = i;
      bit = (floatbyte[j/BITS_PER_BYTE] >> (j % BITS_PER_BYTE)) % 2;
      wr_byte = tmp_read / BITS_PER_BYTE;
      wr_bit = (BITS_PER_BYTE-1) - (tmp_read % BITS_PER_BYTE);
      bitstream[wr_byte] |= (bit << wr_bit);
      tmp_read++;
    }

    i++;
   
  }

  /* Apply the un-hiding method.
     Note: For now, the two existing methods are selfinvertibles. */
  aux = SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE;;
  if(hide(bitstream, SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE, floor, 
	  residue, vc->pcmend/2, hiding_key, bitstream, 
	  &aux, prng) == I_STEGANOS_ERR) {
    message_log("desynchro_res_header", "Unable to desynchronize");
    return I_STEGANOS_ERR;
  }
  
  if(aux != SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE) {
    message_log("desynchro_res_header", "Unable to desynchronize");
    return I_STEGANOS_ERR;
  }
  
  /* Now we've read, at least, the first SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE 
     bits of the subliminal channel, and we can test if it matches with the
     SYNCHRO_HEADER */
  same = 1;
  for(j=0; j<SYNCHRO_HEADER_BYTES_RES; j++) {
    if(bitstream[j] != SYNCHRO_HEADER[j]) {
      same = 0;
    }
  }
  
  /* If the decoder will receive a SYNC header, we just change the LSB of the
     first residual line in the given ordering */
  if(same) {
    if(((int) residue[res_lineup[first]]) % 2) residue[res_lineup[first]]--;
    else residue[res_lineup[first]]++;
  }
  
  return I_STEGANOS_OK;
}

int desynchro_iss(vorbis_config_t *vc, int *res_lineup, int *posts, int *floor,
		  float *residue, void *cfg, steganos_key_t *hiding_key,
		  hide_method hide, prng_t *prng) {

  int i, rc, val, size;
  byte desync[2];

  /* Input parameters control */
  if(!res_lineup || !residue) {
    errno = EINVAL;
    message_log("desynchro_iss", strerror(errno));
    return I_STEGANOS_ERR;
  }

  /* To desynchronize in ISS we just force the first SIZE_FIELD_BITS to be 0,
     so that the receiver will read 0 subliminal bits. */

  /** @todo This might not be a good idea, and could lead to audio artifacts
      or even steganographic fingerprints (despite being the subliminal residues
      dispersed with a PRNG), but for now, I'm in a hurry to make this work, so
      I'll rethink this as soon as possible. */
  memset(desync, 0, 2);
  size = 8;
  if((rc = hide(desync, size, floor, residue, vc->pcmend/2, hiding_key,
		desync, &size, prng)) == I_STEGANOS_ERR) {
    return I_STEGANOS_ERR;
  }

  for(i=0; i<SIZE_FIELD_BITS; i++) {
    
    /* Each iteration embeds one subliminal bit */
    val = 0x2 | ((desync[0] >> (BITS_PER_BYTE - i - 1)) % 2);

    if(residue[res_lineup[i]] < 0)
      residue[res_lineup[i]] = -val;
    else
      residue[res_lineup[i]] = val;

  }

  return I_STEGANOS_OK; 
  
}

int steganos_key_init(byte *byte_key, const int key_len, steganos_key_t *key) {

  int fbytes, ibytes;
  

  /* Input parameters control */
  if(!byte_key || key_len <= 0 || !key) {
    errno = EINVAL;
    message_log("steganos_key_init", strerror(errno));
    return I_STEGANOS_ERR;
  }

  fbytes = ceil((float) key_len / (float) BITS_PER_BYTE);

  /* Range control */
  if(fbytes < INT_MIN || fbytes > INT_MAX) {
    errno = ERANGE;
    message_log("steganos_key_init", strerror(errno));
    return I_STEGANOS_ERR;
  }

  ibytes = (int) fbytes;
  if(!(key->key = (byte *) malloc(sizeof(byte)*ibytes))) {
    message_log("steganos_key_init", strerror(errno));
    return I_STEGANOS_ERR;
  }

  memcpy(key->key, byte_key, ibytes);
  key->length = key_len;

  return I_STEGANOS_OK; 
  
}

int calculate_residue_lineup(steganos_state_t *ss, const int res_len) {

  int aux, i;

  if(!ss || !ss->prng || res_len <= 0) {
    errno = EINVAL;
    message_log("align_residue", strerror(errno));
    return I_STEGANOS_ERR;
  }

  memset(ss->res_lineup, 0, VORBIS_MAX_BLOCK*sizeof(int));
  memset(ss->res_occupied, 0, VORBIS_MAX_BLOCK*sizeof(int));

  /* Calculate residue lineup */
  i=0;    
  while(i<res_len) {
      
    if(prng_get_random_int(ss->prng, res_len, &aux) == I_STEGANOS_ERR){
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

  return I_STEGANOS_OK;
  
}

/* channel.c ends here */

#endif
