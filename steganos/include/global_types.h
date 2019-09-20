/*                               -*- Mode: C -*- 
 * @file: global_types.h
 * @brief: Global types definitions for the software TODO!!
 * @author: Jesus Diaz Vico
 * Maintainer: 
 * @date: lun ene 11 17:23:55 2010 (+0100)
 * @version: 
 * Last-Updated: lun ago 16 23:52:39 2010 (+0200)
 *           By: Jesus
 *     Update #: 31
 * URL: 
 */

#ifndef GLOBAL_TYPES_H
#define GLOBAL_TYPES_H

#include <stdint.h>

/* Constants */

/**
 * @def I_OK 
 * @brief Error code for 'No error' to use in functions of integer return type.
 */
#define I_OK 0

/**
 * @def I_ERR
 * @brief Error code for 'Error occured' to use in functions of integer 
 *  return type.
 */
#define I_ERR 1

/**
 * @def BITS_PER_BYTE
 * @brief Modify for different byte sizes
 */
#define BITS_PER_BYTE 8


/* Data structures and type definitions */

/**
 * @def byte
 * @brief byte will be used as an alias to unsigned char
 */
typedef unsigned char byte;

/**
 * @struct vorbis_config_t
 * @brief Stores the needed information about the Vorbis block and look 
 *  configuration. Is used to avoid circular dependencies.
 */
typedef struct /* _vorbis_config_t */ {
  int rate;
  int pcmend;
  int mult;
  int *postlist;  
  int *forward_index;
  int posts_len;
} vorbis_config_t;

#endif /* GLOBAL_TYPES_H */

/* global_types.h ends here */
