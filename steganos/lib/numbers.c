/*                               -*- Mode: C -*- 
 * @file: numbers.c
 * @brief: This file implements common functions in project TODO!! meant to
 *  provide an interface for PRN sequences managing, sorting number sequences
 *  and searching a given number in a given sequence...
 * @author: Jesus Diaz Vico
 * Maintainer: 
 * Created: wed dec  30 17:28:17 2009 (+0100)
 * Version: 
 * Last-Updated: lun ago 23 19:51:03 2010 (+0200)
 *           By: Jesus
 *     Update #: 186
 * URL: 
 */
#ifdef STEGO
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "numbers.h"
#include "miscellaneous.h"

#include <limits.h>

int prng_init(prng_t *prng) {

  /* Input parameters control */
  if(!prng) {
    errno = EINVAL;
    message_log("prng_init", strerror(errno));
    return I_ERR;
  }

  if(!(prng->seed = (unsigned int *) malloc(sizeof(unsigned int)))) {
    message_log("set_seed", strerror(errno));
    return I_ERR;
  }

  prng->iters = 0; 

  return I_OK;
}

int prng_free(prng_t *prng) {

  if(prng->seed) {
    free(prng->seed); 
    prng->seed = NULL;
  }

  return I_OK;
}

int prng_reset_byte(prng_t *prng, byte *seed, int size, long int iters) {

  int i, r;


  /* Input parameters control */
  if(!prng || !seed || size <= 0) {
    errno = EINVAL;
    message_log("prng_reset_byte", strerror(errno));
    return I_ERR;
  }

  if(prng_set_seed_byte(prng, seed, size) == I_ERR) {
    return I_ERR;
  }
  
  for(i=0; i<iters; i++) {    
    /* The modulo doesn't matter here */
    if(prng_get_random_int(prng, 100, &r) == I_ERR) {
      return I_ERR;
    }
  }

  prng->iters = 0;

  return I_OK;

}

int prng_set_seed_byte(prng_t *prng, const byte *seed, const int size) {
  
  int i, final_size; /* TODO!! quitar cuando cambie de prng? */
  unsigned int ui_seed;

  /* Input parameters control */
  if(!prng || !seed || size <= 0) {
    errno = EINVAL;
    message_log("prng_set_seed_byte", strerror(errno));
    return I_ERR;
  }

  if((unsigned int) size > (sizeof(int)*BITS_PER_BYTE)) {
    final_size = sizeof(int)*BITS_PER_BYTE; 
  } else {
    final_size = size;
  }
  
  /* TODO!! esto hace que cualquier semilla de más de sizeof(int)*BITS_PER_BYTE 
     bits (= 32 bits) se trunque a 32 bits!! Ahora estoy usando claves de 128
     bits que por lo tanto se truncan a 1/4 de su longitud!!!! buscar una 
     solución a esto. */
  final_size /= BITS_PER_BYTE;

  ui_seed = 0;
  for(i=0; i<final_size; i++) {
    ui_seed += (((unsigned int) seed[i]) << (BITS_PER_BYTE*i));
  }

  srandom(ui_seed);
  *((unsigned int *)prng->seed) = ui_seed;
  prng->iters = 0;
  return I_OK;
}

int prng_set_seed_uint(prng_t *prng, const unsigned int seed) {
  
  /* Input parameters control */
  if(!prng) {
    errno = EINVAL;
    message_log("prng_set_seed_int", strerror(errno));
    return I_ERR;
  }

  srandom(seed);
  *((unsigned int *)prng->seed) = seed;
  prng->iters = 0;

  return I_OK;
}

int prng_get_random_int(prng_t *prng, const int modulo, int *r) {

  int rnd;
  long int lrnd;

  /* Input parameters control */
  if(!prng || modulo <= 0 || !r) {
    errno = EINVAL;
    message_log("get_random_int", strerror(errno));
    return I_ERR;
  }

  lrnd = random();
  if(lrnd > INT_MAX) rnd = INT_MAX;
  else rnd = lrnd;

  *r = (int) (((float)modulo*rnd)/(RAND_MAX+1.0));
  prng->iters++;
  return I_OK;
}

int linear_interpolation_y(const float x1, const float y1,
			    const float x2, const float y2, 
			    const float x, float *y) {


  /* Input parameter control */
  if(!y) {
    errno = EINVAL;
    message_log("linear_interpolation_y", strerror(errno));
    return I_ERR;
  }

  /* Simple lineal interpolation */
  *y = y1 + (x - x1)*((y2-y1)/(x2-x1));

  return I_OK;

}

int seek(int *sequence, int seq_len, int number, int *exist) {

  int i;


  /* Input parameter control */
  if(!sequence || seq_len <= 0 || !exist) {
    errno = EINVAL;
    message_log("seek", strerror(errno));
    return I_ERR;
  }

  /* For now, just lineal search TODO!! use a better one? */
  for(i=0; i<seq_len; i++) {

    if(sequence[i] == number) {
      *exist = 1; /* Founded! */
      return I_OK;
    }

  }

  /* Not founded! */
  *exist = 0;

  return I_OK;
  
}

int seek_interval(int *sequence, int seq_len, int lower, int higher, int *exist) {

  int i;


  /* Input parameter control */
  if(!sequence || seq_len <= 0 || lower > higher || !exist) {
    errno = EINVAL;
    message_log("seek_interval", strerror(errno));
    return I_ERR;
  }

  for(i=0; i<seq_len; i++) {

    if(sequence[i] >= lower || sequence[i] <= higher) {
      *exist = 1; /* Founded! */
      return I_OK;
    }

  }

  /* Not founded! */
  *exist = 0;
  return I_OK;
  
}

/* numbers.c ends here */
#endif
