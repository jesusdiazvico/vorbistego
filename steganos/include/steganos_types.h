/*                               -*- Mode: C -*- 
 * @file: steganos_types.h
 * @brief: This file defines the common constants, macros and data structures
 *  for the steganographic functions and interfaces of the software TODO!!.
 * @author: Jesus Diaz Vico
 * Maintainer: 
 * Created: vie dec  8 17:28:17 2009 (+0100)
 * Version: 
 * Last-Updated: vie ago 20 20:35:51 2010 (+0200)
 *           By: Jesus
 *     Update #: 216
 * URL: 
 */

#ifndef STEGANOS_TYPES_H
#define STEGANOS_TYPES_H

#include "global_types.h"
#include "numbers.h"

// TODO!!!!!! ESTOY USANDO MEMORIA ESTÁTICA EN LAS VARIABLES DE SS!!!!!

/* Constants */

/**
 * @def I_STEGANOS_OK 
 * @brief Error code for 'No error' to use in functions of integer return type.
 */
#define I_STEGANOS_OK I_OK

/**
 * @def I_STEGANOS_ERR
 * @brief Error code for 'Error occured' to use in functions of integer 
 *  return type.
 */
#define I_STEGANOS_ERR I_ERR

/**
 * @def I_STEGANOS_SYNC_FAIL
 * @brief Error code for 'Unable to synchronize', used when a synhcornization
 *  method is unable to synchronize due to is own nature, i.e., there is no
 *  computation error.
 */
#define I_STEGANOS_SYNC_FAIL 2

/**
 * @def I_STEGANOS_FRAME_SKIP
 * @brief Return ocde used to specify we're not using the current frame to hide
 *  data. This can be due, for example, to a short subliminal capacity of the
 *  frame or to "imperceptibility" reasons.
 */
#define I_STEGANOS_FRAME_SKIP 3

/**
 * @def INDETERMINATE_MARK
 * @brief Value used when there is not enough statistical evidence of the mark
 *  hided in the Vorbis's posts vector being 0 or 1. Used only with ISS 
 *  synchronization method.
 */
#define INDETERMINATE_MARK -1

/**
 * @def VORBIS_MAX_BLOCK
 * @brief Defines the Vorbis maximum number of samples per Vorbis block.
 */
#define VORBIS_MAX_BLOCK 8192

#define VIF_POSIT 63 

/* Macros */

/* Data structures and type definitions */

/**
 * @struct steganos_key_t steganos_types.h "include/steganos_types.h"
 * @brief Defines the key structure to be used in the steganographic layer.
 */
typedef struct /* _steganos_key_t */ {
  byte *key; /**< Key value, of size \f$ \lceil length/BITS\_PER\_BYTE \rceil\f$ bytes. */
  int length; /**< Key length, in bits. */
} steganos_key_t;


/**
 * @typedef int (*hide_method) (byte *plain_mess, int p_m_len, 
 *		                int *floor, float *res, int res_len,
 *		                steganos_key_t *key, byte *sub_mess, 
 *                              int *s_m_len, prng_t *prng)
 *
 * @brief Type definition for hiding functions to use in the residual 
 *  subliminal channel.
 *
 * Functions of this type shall produce a stego message of the same length as the
 * subliminal message, and may or may not use the cover object and/or the key 
 * during the process. In case the cover object and/or the key aren't necessary,
 * the corresponding byte * parameters representing them must be NULL and their 
 * corresponding int's be 0.
 *
 * @param[in] plain_mess Plain message to hide.
 * @param[in] p_m_len Length of plain message in bits.
 * @param[in] floor Floor vector.
 * @param[in] floor_len The length of the floor vector (actually, the floor posts
 *  in Vorbis).
 * @param[in] res Residue vector.
 * @param[in] res_len Number of elements in <i>floor</i> and <i>res</i> vectors.
 * @param[in] key Key to use.
 * @param[in, out] sub_mess String in which the subliminal message will be stored. 
 *  It has to be allocated previously to calling the function.
 * @param[in, out] s_m_len At the input, will store the allocated size (in bits)
 *  of <i>sub_mess</i>. At the output, the length of the resulting sub_message, 
 *  in bits (it may  be different than p_m_len as it may include 
 *  de-synchronization bits).
 * @param[in] prng PRNG in use.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 */
typedef int (*hide_method) (byte *plain_mess, int p_m_len, 
			    int *floor, float *res, int res_len,
			    steganos_key_t *key, byte *sub_mess, 
			    int *s_m_len, prng_t *prng);

/**
 * @typedef int(*sync_method) (vorbis_config_t *vc, steganos_key_t *key,
 *                             int *posts, float *residue, void *cfg, 
 *                             int decoding, int *bit, prng_t *prng)
 * @brief Type definition for synchronization functions to use in the protocol.
 *
 * Functions of this type shall solve the synchronization problem between sender
 * and receiver using the "both-known" data (stored in <i>ss</i> and the floor 
 * and residue vectors, both of length <i>res_len</i>. Depending on the specific
 * method use, the <i>floor</i> and <i>residue</i> vectors will or won't be used.
 *
 * @param[in] vc Vorbis block and look information.
 * @param[in] key Key to use
 * @param[in, out] posts Current floor posts vector. May be subject to changes
 *  depending on the concrete method used. See synchronization methods.
 * @param[in, out] residue Current residue vector. May be subject to changes
 *  depending on the concrete method used. See synchronization methods.
 * @param[in] cfg This parameter will be used to pass any argument needed in a
 *  concrete synchronization method.
 * @param[in] decoding Active if the function is called from the decoder's side,
 * @param[in, out] bit At the input, the desired bit to include if we are at the
 *  encoder's side; at the ouput, the received bit if we are at the decoder's 
 *  side (i.e. 1 if we're telling the receiver there is hidden data and 0 if 
 *  not).
 * @param[in] prng PRNG in use
 *
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 * @retval I_STEGANOS_SYNC_FAIL with errno = 0 (Unable to synchronize).
 *
 * @see synchro_method_et
 * @see iss_synchronization
 */
typedef int (*sync_method) (vorbis_config_t *vc, steganos_key_t *key, int *posts, 
			    float *residue, void *cfg, int decoding, int *bit,
			    prng_t *prng);


/**
 * @typedef int (*desync_method) (vorbis_config_t *vc, int *res_lineup, 
 *                                int *posts, int *floor, float *residue, 
 *                                void *cfg, steganos_key_t *hiding_key,
 *                                hide_method hide, prng_t *prng)
 * @brief Type definition for desynchronization functions to use in the protocol.
 *
 * Functions of this type shall solve the false positive synchronization problem
 * that may happen depending on the synchro_method used. Depending on the
 * specific method in use, the <i>floor</i> and <i>residue</i> vectors will or 
 * won't be used.
 *
 * @param[in] vorbis_config_t Vorbis configuration data.
 * @param[in] res_lineup Residue ordering
 * @param[in, out] posts Current floor posts vector. May be subject to changes
 *  depending on the concrete method used. See synchronization methods.
 * @param[in, out] floor Current floor vector. May be subject to changes
 *  depending on the concrete method used. See synchronization methods.
 * @param[in, out] residue Current residue vector. May be subject to changes
 *  depending on the concrete method used. See synchronization methods.
 * @param[in] cfg This parameter will be used to pass any argument needed in a
 *  concrete synchronization method.
 * @param[in] hiding_key Key for hiding
 * @param[in] hide Hiding method (pointer to function) in use in the current
 *  frame and channel.
 * @param[in] prng PRNG in use
 *
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 *
 * @see synchro_method_et
 * @see iss_synchronization
 */
typedef int (*desync_method) (vorbis_config_t *vc, int *res_lineup, int *posts,
			      int *floor, float *residue, void *cfg, 
			      steganos_key_t *hiding_key, hide_method hide,
			      prng_t *prng);

/* /\** */
/*  * @var protocol_op_send_et */
/*  * @brief Enumeration for the different possible operations at encoder's side, */
/*  *  namely SYNC or HIDE. */
/*  *\/ */
/* typedef enum { */
/*   SYNC = 0, /\**< Synchronization operation. *\/ */
/*   HIDE = 1  /\**< Hide operation *\/ */
/* } protocol_op_send_et; */

/**
 * @var synchro_method_et
 * @brief Enumeration for the different synchronization modes available.
 */
typedef enum {
  RES_HEADER = 0, /**< Classic mode: synchronization header in residue subliminal
		     channel */
  ISS = 1, /**< Improved Spread Spectrum algorithm within floor vector (uses 
	      synchro_key) */
  FORCED_RES_HEADER = 2 /**< Used when ISS is the default option but for some 
			   reason we can't use it at a given moment, and also
			   to prevent false negatives at the decoder's side. */
} synchro_method_et;

/**
 * @var hide_method_et
 * @brief Enumeration for the different methods of hiding data in the residues.
 */
typedef enum {
  DIRECT_HIDING = 0, /**< Just write the data in the given locations. */
  PARITY_BITS = 1 /**< Applies the parity method by Anderson and Petitcolas. */
} hide_method_et;


typedef struct /* _iss_cfg_t */ {
  float alpha; /**< alpha in s = x + (alpha*b - lambda*x)*u */
  float lambda; /**< lambda in s = x + (alpha*b - lambda*x)*u */
  float sigma; /**< The deviation to use in the watermarking random sequence */
  float *u; /**< The watermark */
  float u_norm; /**< Norm of the watermark */
} iss_cfg_t;


/**
 * @struct steganos_state_t steganos_types.h "include/steganos_types.h"
 * @brief Defines the global internal state of the steganosgraphic protocol.
 *
 * @see synchro_method_et
 * @see sync_method
 * @see desync_method
 * @see hide_method_et
 * @see hide_method
 * @see steganos_key_t
 */
typedef struct /*_steganos_state_t*/ {
  vorbis_config_t *vc; /**< Vorbis configuration data needed during the 
			  steganographic protocol. */
  steganos_key_t *master_key; /**< Master key used to derive the subkeys. */
  steganos_key_t *synchro_key; /**< Key to use for synchronization. */
  steganos_key_t *hiding_key;  /**< Key to use for hiding the data in the
				  residues. */
  synchro_method_et synchro_method; /**< Synchronization method (enum type). Can
				       change from one frame and channel to
				       another. */
  sync_method synchronize; /**< Synchronization method (pointer to function)
				 to use in the current frame and channel. */
  desync_method desynchronize; /**< Desynchronization method (pointer to
				     function) */
  int desync; /**< Boolean. Active when desynchronization must be done. */
  hide_method_et hide_method; /**< Hiding method (enum type) to use in the
				 current frame and channel residue vectors. */
  hide_method hide; /**< Hiding method (pointer to function) to use in the
                           current frame and channel. */
  unsigned int da; /**<  Desired aggressiveness of the protocol.
		      Remains the same through the whole process.
		      Ranges from 1 to 10, meaning we want to use
		      the 10,20, ... 100% of the total subliminal
		      capacity. */
  float ra; /**< The aggressiveness is used as a guide to the global share of the
	       subliminal channel we desire to use, but due to the stochastic
	       nature of the method, we shall use it as a guide rather than
	       a strict quantity. The <i>real aggressiveness</i> will change
	       from channel to channel and will be used as a correction
	       indicator. */
  float variation_limit[VORBIS_MAX_BLOCK][2]; /**<  Variation limit per residual
						 line. Stores the [-, +] maximum
						 variations to introduce in each
						 residual value. Changes from one
						 frame and channel to another. */
  float max_fc_capacity; /**< Stores the maximum subliminal capacity for the
			    current frame and channel residue, in bits. */
  float min_fc_capacity; /**< Stores the minimum subliminal capacity for the
			    current frame and channel residue, in bits. */
  int res_max_capacity[VORBIS_MAX_BLOCK]; /**< Will store the maximum number of
					     subliminal bits a given residual
					     element should shelter. This value
					     should be seen also as a guide. */
  int res_min_capacity[VORBIS_MAX_BLOCK]; /**< Will store the minimum number of
					     subliminal bits a given residual
					     element should shelter. This value
					     should be seen also as a guide. */
  int res_lineup[VORBIS_MAX_BLOCK]; /**< Will store the ordering of the residual
				       elements for hiding/unhiding purposes. */
  int res_occupied[VORBIS_MAX_BLOCK]; /**< Will be used to mark the residual
					 values which will be used to write in
					 or read from whithin the same frame.
				         This array will prevent from writing/
				         reading in/from the same residual value
				         more than once. */
  unsigned long int sent; /**< Total amount of _pure_ data (excluding metadata) 
			     already hided in the audio stream. */
  unsigned long int read; /**< Total amount of _pure_ data (excluding metadata) 
			     already recoverred from the stego-audio stream. */
  long int metadata_sent; /**< The same as sent, but including metadata */
  long int remaining; /**< Total amount of _pure_ data to send/receive. */
/*   int fd; /\**< Descriptor of the file containing the subliminal data to send *\/ */
  int out[VIF_POSIT+2]; /**< Vector with the posts values to write in the ogg 
			   packet when using ISS. */
  int status; /**< Status code. Used to control "inter fucntion" errors */
  int posts_mode; /**< With ISS, this variable indicates the way we're trying to
		     mark the floor posts. 1 means we're trying to mark it with
		     a 1 bit, 0 means we're using the original posts, and
		     -1 means we're trying to mark it with a 0 bit. */
  int aligned; /**< Boolean signaling that we've already calculated the residue
		  lineup. Avoids recalculating it when considering several
	          different stego-frame-channels. */
  long int total_sub_capacity; /**< The total subliminal capacity at a given
				  instant. Useful for statistics and 
				  debugging. Only meaningful when encoding. */
  int iters; /**< Usefull when debugging and printing statistics, stores how 
		many iterations of the steganographic protocol has been run at
		a given instant. */
  prng_t *prng; /**< PRNG abstraction */

} steganos_state_t;

#endif /* STEGANOS_TYPES_H */

/* steganos_types.h ends here */
