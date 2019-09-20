/*                               -*- Mode: C -*- 
 * @file: miscellaneous.h
 * @brief: Header file for miscellaneous.c
 * @author: Jesus Diaz Vico
 * Maintainer: 
 * @date: lun ene 11 23:10:28 2010 (+0100)
 * @version: 
 * Last-Updated: mar ago 31 13:19:41 2010 (+0200)
 *           By: Jesus
 *     Update #: 39
 * URL: 
 */

#ifndef MISCELLANEOUS_H
#define MISCELLANEOUS_H

#include <stdint.h>

//#include <libxml/xmlreader.h>

#include "steganos_types.h"
#include "global_types.h"

/* Constants */

/**
 * @def I_MISC_OK 
 * @brief Error code for 'No error' to use in functions of integer return type.
 */
#define I_MISC_OK 0

/**
 * @def I_MISC_ERR
 * @brief Error code for 'Error occured' to use in functions of integer return type.
 */
#define I_MISC_ERR 1

/**
 * @def DEFAULT_CONFIG_FILE
 * @brief Name that the configuration file must have when the desired configuration
 *  parameters have to be read from a file.
 *
 *  The contesnts of the configuration file must have the same structure as a 
 *  command line call, but starting with vorbistego_cfg instead of ./oggenc or 
 * ./oggdec. E.g.:
 *  vorbistego_cfg --shm 0 --ssm 0 --sfile stegfile --skey 1234567812345678
 */
#define DEFAULT_CONFIG_FILE "vorbistego_cfg"

/**
 * @struct fw_options_t miscellaneous.h
 * @brief Defines the structure for forward side (sender) options. This structure
 *  will be used to store params introduced by a configuration file.
 */
typedef struct {
  int delayfr;          /**< Number of frames to skip before hiding */
  int da;               /**< Default desired aggressiveness */
  char *sfile;          /**< Subliminal input/output file */
  int hide_method;      /**< Hiding method, if at the end is still -1, error */
  int sync_method;      /**< Default synchronization method, if at the end is still -1, error */
  float sigma;          /**< Default sigma */
  char *skey;           /**< Key */
  char *sca;            /**< Cipher algorithm */
  char *scmda;          /**< Digest algorithm */
  int schmac;           /**< Hmac flag */
  char *sciv;           /**< IV */
  uint64_t scem;        /**< Emission ID */
  uint64_t scpkt;       /**< First packet ID */
  int scdds;            /**< Default data size for crypto-packets */  
  int quiet;            /**< Quiet mode indicator */
  int force;            /**< Indicates obligation to use the config from file */
} fw_options_t;

/**
 * @struct inv_options_t miscellaneous.h
 * @brief Defines the structure for inverse side (receiver) options. This structure
 *  will be used to store params introduced by a configuration file.
 */
typedef struct {
  char *sfile;          /**< Subliminal input/output file */
  int hide_method;      /**< Hiding method, if at the end is still -1, error */
  int sync_method;      /**< Default synchronization method, if at the end is still -1, error */
  float sigma;          /**< Default sigma */
  char *skey;           /**< Key */
  char *sca;            /**< Cipher algorithm */
  char *scmda;          /**< Digest algorithm */
  int schmac;           /**< Hmac flag */
  uint64_t scem;        /**< Emission ID */
  uint64_t scpkt;       /**< First packet ID */
  int quiet;            /**< Quiet mode indicator */
  int force;            /**< Indicates obligation to use the config from file */
} inv_options_t;



/* Functions' headers*/

/* int parse_validate_steganos_xml(const char *filename,  */
/* 				xmlChar **names, xmlChar **values, int *elems); */
/* int _process_xml_node(xmlTextReaderPtr reader); */
/* int _process_xml_steganos_node(xmlTextReaderPtr reader,  */
/* 			       xmlChar **names, xmlChar **values, int *elems); */

/**
 * @fn int to_byte(void *stream, size_t nmemb, size_t size, byte *byte_stream)
 *
 * @brief Converts the given pointer of generic type to a byte pointer.
 *
 * Converts the pointer <i>stream</i>, composed by <i>nmemb</i> elements of size
 * <i>size</i> bytes each into a byte pointer, stored in <i>byte_stream</i>. 
 * Uses big endian ordering.
 *
 * @param[in] stream The void pointer to convert.
 * @param[in] nmemb The number of elements in <i>stream</i>.
 * @param[in] size The size, in bytes, of each <i>stream</i>'s element.
 * @param[in, out] byte_stream The pointer which will store the byte conversion
 *  of <i>stream</i>. Enough memory must have been allocated previously to 
 *  calling the function, which must be, at least nmemb*size bytes.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_MISC_OK if no error was present and I_MISC_ERR if an error occured 
 *  with errno updated.
 * @retval I_MISC_OK with errno = 0 (No error).
 * @retval I_MISC_ERR with errno = EINVAL (invalid argument).
 */
int to_byte(void *stream, size_t nmemb, size_t size, byte *byte_stream);

/** 
 * @fn int print_bytestream(int fd, byte *stream, int length)
 * @brief Prints a bytestream in fd.
 * 
 * @param[in] fd The file descriptor to print in. -1 to write in the system log
 * @param[in] stream The stream to print.
 * @param[in] length The length of <i>stream</i> in bytes.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_MISC_OK if no error was present and I_MISC_ERR if an error occured with
 *  errno updated. 
 * @retval I_MISC_OK with errno = 0 (No error).
 * @retval I_MISC_ERR with errno = EINVAL (Invalid argument).
 */
int print_bytestream(int fd, byte *stream, int length);

/** 
 * @fn int bitstream_rol(byte *bitstream, int length, int rot)
 * @brief Rotates bitstream rot bits to the left.
 * 
 * @param[in, out] bitstream The bit stream to rotate at the input, the
 *  rotated resulting bitstream at the output.
 * @param[in] length The length of bitstream, in bits.
 * @param[in] rot The number of bits to rotate to the left.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_MISC_OK if no error was present and I_MISC_ERR if an error occured with
 *  errno updated. 
 * @retval I_MISC_OK with errno = 0 (No error).
 * @retval I_MISC_ERR if unable to open log file, with errno updated to
 *  consequently (see fopen man page).
 */
int bitstream_rol(byte *bitstream, int length, int rot);

/** 
 * @fn int bitstream_ror(byte *bitstream, int length, int rot)
 * @brief Rotates bitstream rot bits to the right.
 * 
 * @param[in, out] bitstream The bit stream to rotate at the input, the
 *  rotated resulting bitstream at the output.
 * @param[in] length The length of bitstream, in bits.
 * @param[in] rot The number of bits to rotate to the right.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_MISC_OK if no error was present and I_MISC_ERR if an error occured with
 *  errno updated. 
 * @retval I_MISC_OK with errno = 0 (No error).
 * @retval I_MISC_ERR if unable to open log file, with errno updated to
 *  consequently (see fopen man page).
 */
int bitstream_ror(byte *bitstream, int length, int rot);

/** 
 * @fn int message_log(const char *caller, const char *message)
 * @brief Writes the specified message in the log file log_<pid>.
 * 
 * @param[in] caller Name of the calling function.
 * @param[in] message The message to write in the log.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_MISC_OK if no error was present and I_MISC_ERR if an error occured with
 *  errno updated. 
 * @retval I_MISC_OK with errno = 0 (No error).
 * @retval I_MISC_ERR if unable to open log file, with errno updated to
 *  consequently (see fopen man page).
 */
int message_log(const char *caller, const char *message);

/** 
 * @fn int parse_options(char *cfg_file, fw_options_t *fw, inv_options_t *inv, 
 *                       int sender);
 * @brief Parses the configuration options from <i>cfg_file</i>. Just the
 *  vorbistego variant of oggenc/oggdec are accepted as valid parameters!
 *
 * @param[in] cfg_file The configuration file. Must have the same structure
 *  as a normal command-line call, except that it must begin with the name
 *  of the file (currently, it must be the value in DEFAULT_CONFIG_FILE. E.g.:
 *  vorbistego_cfg --shm 0 --ssm 0 --sfile stegfile --skey 1234567812345678
 *
 * @param[in] cfg_file The config filename
 * @param[out] fw The forward options to fill when sender is set to 1
 * @param[out] inv The inverse options to fill when sender is set to 0
 * @param[in] sender When set to 1, indicates we must fill the forward config.
 *  When set to 0, indicates we must fill the inverse config.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_MISC_OK if no error was present and I_MISC_ERR if an error occured with
 *  errno updated. 
 * @retval I_MISC_OK with errno = 0 (No error).
 * @retval I_MISC_ERR if unable to open log file, with errno updated to
 *  consequently (see fopen man page).
 */
int parse_options(char *cfg_file, fw_options_t *fw, inv_options_t *inv, int sender);

/* The following functions are extracted from the zlib's zpipe.c example program
   and are exactly the same except the names. */

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int zlib_def(FILE *source, FILE *dest, int level);

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int zlib_inf(FILE *source, FILE *dest);

/* report a zlib or i/o error */
void zlib_zerr(int ret);

#endif /* MISCELLANEOUS_H */

/* miscellaneous.h ends here */

