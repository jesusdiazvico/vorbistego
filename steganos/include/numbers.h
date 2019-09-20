/*                               -*- Mode: C -*- 
 * @file: numbers.h
 * @brief: Headers for the file numbers.c from the software TODO!!
 * @author: Jesus Diaz Vico
 * Maintainer: 
 * Created: wed dec  30 17:28:17 2009 (+0100)
 * Version: 
 * Last-Updated: dom jun 20 22:43:57 2010 (+0200)
 *           By: Jesus
 *     Update #: 37
 * URL: 
 */

#ifndef NUMBERS_H
#define NUMBERS_H

#include "global_types.h"

/**
 * @struct prng_t numbers.h "include/numbers.h"
 * @brief Defines the structure to use as PRNG abstraction.
 */
typedef struct /*_prng_t */ {
  void *seed; /**< Seed used in the PRNG */
  long int iters; /**< Numbers of times used */
} prng_t;

/**
 * @fn int prng_init(prng_t *prng)
 * @brief Initializes the PRNG
 *
 * Allocates for the given PRNG
 * @param[in] prng PRNG to allocate.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_OK if no error was present and I_ERR if an error occured 
 *  with errno updated.
 * @retval I_OK with errno = 0 (No error).
 * @retval I_ERR with errno = EINVAL (invalid argument).
 */
int prng_init(prng_t *prng);


/**
 * @fn int prng_free(prng_t *prng)
 * @brief Sets the seed to use to the <i>seed</i>, of <i>size</i> bits.
 *
 * Frees the memory allocated for the given PRNG
 * @param[in] prng PRNG to free.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_OK if no error was present and I_ERR if an error occured 
 *  with errno updated.
 * @retval I_OK with errno = 0 (No error).
 */
int prng_free(prng_t *prng);

/** 
 * @fn int prng_reset_byte(prng_t *prng, byte *seed, int size, long int iters)
 * @brief Recalculate the state of the given PRNG seeded with <i>seed</i> and
 *  used <i>prng->iters</i> times.
 *
 * Recalculates the state of the PRNG <i>prng</i> after being seeded with 
 * <i>seed</i> and iterated <i>iters</i> times.
 *
 * @param[in] prng The PRNG.
 * @param[in] seed The seed used to initialize prng.
 * @param[in] size The size of seed, in bits.
 * @param[in] iters The iterations to "discard".
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_OK if no error was present and I_ERR if an error occured 
 *  with errno updated.
 * @retval I_OK with errno = 0 (No error).
 * @retval I_ERR with errno = EINVAL (invalid argument).
 */
int prng_reset_byte(prng_t *prng, byte *seed, int size, long int iters);

/**
 * @fn int prng_set_seed_byte(prng_t *prng, const byte *seed, const int size)
 * @brief Sets the seed to use to the <i>seed</i>, of <i>size</i> bits.
 *
 * Sets the seed to use for generating [CS]PRN sequences to <i>seed</i>, which
 * will have <i>size</i> bits of length.
 * @param[in] prng PRNG abstraction pointer.
 * @param[in] seed The seed to use, in a byte representation
 * @param[in] size The size of the seed, in bits
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_OK if no error was present and I_ERR if an error occured 
 *  with errno updated.
 * @retval I_OK with errno = 0 (No error).
 * @retval I_ERR with errno = EINVAL (invalid argument).
 */
int prng_set_seed_byte(prng_t *prng, const byte *seed, const int size);

/**
 * @fn int prng_set_seed_uint(prng_t *prng, const unsigned int seed)
 * @brief Sets the seed to use to the <i>seed</i>.
 *
 * Sets the seed to use for generating [CS]PRN sequences to <i>seed</i>, which
 * will have <i>size</i> bits of length.
 * @param[in] prng PRNG abstraction pointer.
 * @param[in] seed The seed to use, in an int representation
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_OK if no error was present and I_ERR if an error occured 
 *  with errno updated.
 * @retval I_OK with errno = 0 (No error).
 * @retval I_ERR with errno = EINVAL (invalid argument).
 */
int prng_set_seed_uint(prng_t *prng, const unsigned int seed);

/**
 * @fn int prng_get_random_int(prng_t *prng, const int modulo, int *r)
 * @brief Feeds <i>r</i> with a pseudo random integer.
 *
 * Obtains a new pseudo random number and stores it in <i>r</i>.
 * 
 * @param[in] prng PRNG abstraction pointer.
 * @param[in] modulo The pseudo random number generated will take a value 
 *  between 0 and <i>modulo-1</i>. If <i>modulo</i> equals 0, the function
 *  won't apply any modular arithmetic.
 * @param[out] r Will store the pseudo random number generated.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_OK if no error was present and I_ERR if an error occured 
 *  with errno updated.
 * @retval I_OK with errno = 0 (No error).
 * @retval I_ERR with errno = EINVAL (invalid argument).
 */
int prng_get_random_int(prng_t *prng, const int modulo, int *r);

/**
 * @fn int linear_interpolation_y(const float x1, const float y1,
 *                                 const float x2, const float y2, 
 *	          		   const float x, float *y)
 * @brief Stores the result of the lineal interpolation between the points
 *  [x1,y1] and [x2,y2] in [x,y].
 *
 *  Given [x1, y1], [x2, y2] and x, stores the result of the lineal interpolation
 *  between the points [x1,y1] and [x2,y2] in [x,y].
 *
 * @param[in] x1 The first point x-coordinate.
 * @param[in] y1 The first point y-coordinate.
 * @param[in] x2 The second point x-coordinate.
 * @param[in] y2 The second point y-coordinate.
 * @param[in] x The x coordinate of the new point.
 * @param[out] y Will store the resulting new point y-coordinate.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_OK if no error was present and I_ERR if an error occured 
 *  with errno updated.
 * @retval I_OK with errno = 0 (No error).
 * @retval I_ERR with errno = EINVAL (invalid argument).
 */
int linear_interpolation_y(const float x1, const float y1,
			    const float x2, const float y2, 
			    const float x, float *y);

/**
 * @fn int seek(int *sequence, int seq_len, int number, int *exist)
 * @brief Looks for <i>number</i> in <i>sequence</i>, updating <i>exist</i> in
 *  consequence.
 *
 * Looks for <i>number</i> in <i>sequence</i>. After a successful run, 
 * <i>exist</i> will equal to 1 if the number exists and 0 otherwise.
 * @param[in] sequence A list of numbers.
 * @param[in] seq_len The number of elements in sequence.
 * @param[in] number The number to look for.
 * @param[out] exist Will be 1 if the number exist and 0 otherwise, in a 
 *  successful run of the function.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_OK if no error was present and I_ERR if an error occured 
 *  with errno updated.
 * @retval I_OK with errno = 0 (No error).
 * @retval I_ERR with errno = EINVAL (invalid argument).
 */
int seek(int *sequence, int seq_len, int number, int *exist);

/**
 * @fn int seek_interval(int *sequence, int seq_len, int lower, int higher, 
 *                       int *exist)
 * @brief Looks if there is any number in <i>sequence</i> belonging to the interval 
 * defined by [<i>lower, higher</i>], updating <i>exist</i> in consequence.
 *
 * Looks if there is any number in <i>sequence</i> belgoning to the interval 
 * defined by [<i>lower, higher</i>]. After a successful run, <i>exist</i> will
 * equal to 1 if the number exists and 0 otherwise.
 * @param[in] sequence A list of numbers.
 * @param[in] seq_len The number of elements in sequence.
 * @param[in] lower The lower end of the interval.
 * @param[in] higher The higher end of the interval.
 * @param[out] exist Will be 1 if the number exist and 0 otherwise, in a 
 *  successful run of the function.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_OK if no error was present and I_ERR if an error occured 
 *  with errno updated.
 * @retval I_OK with errno = 0 (No error).
 * @retval I_ERR with errno = EINVAL (invalid argument).
 */
int seek_interval(int *sequence, int seq_len, int lower, int higher, int *exist);

#endif /* NUMBERS_H */

/* numbers.h ends here */
