/*                               -*- Mode: C -*- 
 * @file: miscellaneous.c
 * @brief: Several useful functions: xml parsers, type conversions, etc
 * @author: Jesus Diaz Vico
 * Maintainer: 
 * @date: lun ene 11 23:06:41 2010 (+0100)
 * @version: 
 * Last-Updated: mar ago 31 13:43:38 2010 (+0200)
 *           By: Jesus
 *     Update #: 318
 * URL: 
 */
#ifdef STEGO

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <getopt.h>

/* zlib dependencies */
#define CHUNK 16384

#include <assert.h>
#include "zlib.h"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#include "miscellaneous.h"

/** 
 * @fn int parse_validate_steganos_xml(const char *filename, 
 *         	                       xmlChar **names, xmlChar **values, int *elems)
 * @brief Parses and validates the given xml file.
 *
 * Parses and validates the file named <i>filename</i>. The <i>names</i> and
 * <i>values</i> arrays will be allocated here, and must be freed by the user.
 * Oppositely, the <i>elems</i> argument must have been allocated previous to 
 * calling the function. 
 * 
 * @param[in] filename The name of the xml file.
 * @param[out] names An array of pointers to xmlChars which will contain the
 *  names of the read elements.
 * @param[out] values An array of pointers to xmlChars which will contain the
 *  values of the read elements, in the same order as <i>names</i>
 * @param[in, out] elems The size of <i>names</i> and <i>values</i> arrays.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_MISC_OK if no error was present and I_MISC_ERR if an error occured 
 *  with errno updated.
 * @retval I_MISC_OK with errno = 0 (No error).
 * @retval I_MISC_ERR with errno = 0 (Unknown error while reading xml file).
 * @retval I_MISC_ERR with errno = EINVAL (Invalid argument).
 * @retval I_MISC_ERR with errno = EIO (Fail while accessing xml file).
 * @retval I_MISC_INVALID_XML (Invalid xml file).
 */
/* int parse_validate_steganos_xml(const char *filename,  */
/* 				xmlChar **names, xmlChar **values, int *elems) { */

/*   xmlTextReaderPtr reader; */
/*   int ret, ret2, elms, read, i; */
/*   xmlChar **nms, **vls; */


/*   if(!filename) { */
/*     errno = EINVAL; */
/*     message_log("parse_validate_steganos_xml", strerror(errno)); */
/*     return I_MISC_ERR; */
/*   } */
  
/*   /\* This initialize the library and check potential ABI mismatches between the */
/*      version it was compiled for and the actual shared library used *\/ */
/*   LIBXML_TEST_VERSION */
    
/*     /\* Pass some special parsing options to activate DTD attribute defaulting, */
/*        entities substitution and DTD validation *\/ */
/*     reader = xmlReaderForFile(filename, NULL, */
/* 			      XML_PARSE_DTDATTR |  /\* default DTD attributes *\/ */
/* 			      XML_PARSE_NOENT |    /\* substitute entities *\/ */
/* 			      XML_PARSE_DTDVALID); /\* validate with the DTD *\/ */
/*   if(reader) { */

/*     /\* Prepare the return structures *\/ */
/*     if(!(names = (xmlChar **) malloc(sizeof(xmlChar *)*XML_LEAFS))) { */
/*       xmlFreeTextReader(reader); */
/*       message_log("parse_validate_steganos_xml", strerror(errno)); */
/*       return I_MISC_ERR; */
/*     } */
/*     if(!(values = (xmlChar **) malloc(sizeof(xmlChar *)*XML_LEAFS))) { */
/*       xmlFreeTextReader(reader); */
/*       free(names); */
/*       message_log("parse_validate_steganos_xml", strerror(errno)); */
/*       return I_MISC_ERR; */
/*     } */

/*     read = 0; */
/*     ret = xmlTextReaderRead(reader); */
/*     while (ret == 1) { */

/*       if((ret2 = _process_xml_steganos_node(reader, nms, vls, &elms)) != I_MISC_OK) { */
/* 	return ret2; */
/*       } */

/*       for(i=0; i<elms; i++) { */
/* 	names[read+i] = nms[i]; */
/* 	values[read+i] = vls[i]; */
/*       } */

/*       /\* Free garbage... I don't like how this is done... *\/ */
/*       free(nms); */
/*       free(vls); */

/*       read += elms; */

/*       ret = xmlTextReaderRead(reader); */

/*     } */

/*     *elems = read; */

/*     /\* Once the document has been fully parsed check the validation results *\/ */
/*     if (xmlTextReaderIsValid(reader) != 1) { */
/*       message_log("parse_validate_steganos_xml", "xmlTextReaderIsValid"); */
/*       return I_MISC_INVALID_XML; */
/*     } */

/*     xmlFreeTextReader(reader); */

/*     /\* If error during read or there were remaining nodes... *\/ */
/*     if (ret != 0) { */
/*       message_log("parse_validate_steganos_xml", "Unknown parse error"); */
/*       return I_MISC_ERR; */
/*     } */
/*   } else { */
/*     errno = EIO; */
/*     message_log("parse_validate_steganos_xml", strerror(errno)); */
/*     return I_MISC_ERR; */
/*   } */
  
/*   /\* Cleanup function for the XML library *\/ */
/*   xmlCleanupParser(); */

/*   /\* This is to debug memory for regression tests *\/ */
/*   xmlMemoryDump(); */
 
/*   return I_MISC_OK; */
  
/* } */

/** 
 * @fn int _process_xml_steganos_node(xmlTextReaderPtr reader, 
 *		             xmlChar **names, xmlChar **values, int *elems)
 * @brief Processes a <i>steganos_config.xml</i> node.
 *
 * This function recognises and processes all valid nodes which may appear in a
 * <i>steganos_config.xml</i> file, returning two arrays of xmlChar strings, the
 * first containing the number of elements read, and the second, its'
 * corresponding values, mantaining the ordering. The field <i>elems</i> will be
 * updated with the size of the previous arrays. The function allocates the
 * needed memory, while the caller must free the returned arrays. If any error
 * occurs during the execution <i>names</i>, <i>values</i> both be set to NULL.
 * Oppositely to <i>names</i> and <i>values</i> the <i>elems</i> argument must 
 * have been allocated previous to calling the function. 
 * 
 * @param[in] reader The xml file reader.
 * @param[out] names An array of pointers to xmlChars which will contain the
 *  names of the read elements.
 * @param[out] values An array of pointers to xmlChars which will contain the
 *  values of the read elements, in the same order as <i>names</i>
 * @param[in, out] elems The size of <i>names</i> and <i>values</i> arrays.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_MISC_OK if no error was present and I_MISC_ERR if an error occured with
 *  errno updated.
 * @retval I_MISC_OK with errno = 0 (No error).
 * @retval I_MISC_ERR with errno = 0 (Unknown error while reading xml file).
 * @retval I_MISC_ERR with errno = EINVAL (Invalid argument).
 * @retval I_MISC_INVALID_XML (Invalid xml file).
 */
/* int _process_xml_steganos_node(xmlTextReaderPtr reader,  */
/* 			       xmlChar **names, xmlChar **values, int *elems) { */

/*   const xmlChar *name, *value; */

  
/*   /\* Input parameters control *\/ */
/*   if(!reader || !elems) { */
/*     errno = EINVAL; */
/*     message_log("_process_xml_steganos_node", strerror(errno)); */
/*     return I_MISC_ERR; */
/*   } */
 
/*   if(!(name = xmlTextReaderConstName(reader))) { */
/*     message_log("_process_xml_steganos_node", "xmlTextReaderConstName"); */
/*     return I_MISC_INVALID_XML; */
/*   } */
  
/*   value = xmlTextReaderConstValue(reader); */

/*   /\* Processing depends on the node type *\/ */
/*   /\* TODO!! no se si funcionara esto *\/ */
/*   if(!xmlStrcmp(name, (unsigned char *) XML_KEY)) { */

/*     if(!(names = (xmlChar **) malloc(sizeof(xmlChar *)*2))) { */
/*       message_log("_process_xml_steganos_node", strerror(errno)); */
/*       return I_MISC_ERR; */
/*     } */
/*     if(!(values = (xmlChar **) malloc(sizeof(xmlChar *)*2))) { */
/*       free(names); */
/*       message_log("_process_xml_steganos_node", strerror(errno)); */
/*       return I_MISC_ERR; */
/*     } */

/*     if(xmlTextReaderRead(reader) != 1) { */
/*       free(names); */
/*       free(values); */
/*       message_log("_process_xml_steganos_node", "xmlTextReaderRead != 1"); */
/*       return I_MISC_ERR; */
/*     } */

/*     /\* A key must be composed by XML_KEY_LENGTH and XML_KEY_BITSTREAM elements *\/ */
/*     if(!(name = xmlTextReaderConstName(reader))) { */
/*       free(names); */
/*       free(values); */
/*       message_log("_process_xml_steganos_node", "!xmlTextReaderConstName"); */
/*       return I_MISC_INVALID_XML; */
/*     } */
    
/*     value = xmlTextReaderConstValue(reader); */
/*     if(!xmlStrcmp(name, (unsigned char *) XML_KEY_LENGTH)) { */
/*       names[0] = xmlStrdup(name); */
/*       values[0] = xmlStrdup(value); */
/*     } else { */
/*       free(names); */
/*       free(values); */
/*       message_log("_process_xml_steganos_node", "Wrong key length field"); */
/*       return I_MISC_INVALID_XML; */
/*     } */

/*     if(xmlTextReaderRead(reader) != 1) { */
/*       message_log("_process_xml_steganos_node", "xmlTextReaderRead != 1"); */
/*       return I_MISC_ERR; */
/*     } */

/*     if(!(name = xmlTextReaderConstName(reader))) { */
/*       message_log("_process_xml_steganos_node", "!xmlTextReaderConstName"); */
/*       return I_MISC_INVALID_XML; */
/*     } */
    
/*     value = xmlTextReaderConstValue(reader); */
/*     if(!xmlStrcmp(name, (unsigned char *) XML_KEY_BITSTREAM)) { */
/*       names[1] = xmlStrdup(name); */
/*       values[1] = xmlStrdup(value);       */
/*     } else { */
/*       message_log("_process_xml_steganos_node", "Wrong key bitstream field"); */
/*       return I_MISC_INVALID_XML; */
/*     } */
  
/*     if(!names[0] || !names[1] || !values[0] || !values[1]) { */
/*       if(names[0]) free(names[0]); */
/*       if(names[1]) free(names[1]); */
/*       if(values[0]) free(values[0]); */
/*       if(values[1]) free(values[1]); */
/*       if(!elems) free(elems); */
/*       message_log("_process_xml_steganos_node", "Unknown error"); */
/*       return I_MISC_ERR; */
/*     } */
    
/*     *elems = 2; */

/*     return I_MISC_OK; */
    
/*   }  */
  
/*   /\* Otherwise, we just have to read one element *\/ */
/*   else if(!xmlStrcmp(name, (unsigned char *) XML_SYNCHROMODE) || */
/* 	  !xmlStrcmp(name, (unsigned char *) XML_HIDEMETHOD) || */
/* 	  !xmlStrcmp(name, (unsigned char *) XML_AGGRESS)) { */
/*     if(!(names = (xmlChar **) malloc(sizeof(xmlChar *)))) { */
/*       message_log("_process_xml_steganos_node", strerror(errno)); */
/*       return I_MISC_ERR; */
/*     } */
/*     if(!(values = (xmlChar **) malloc(sizeof(xmlChar *)))) { */
/*       free(names); */
/*       message_log("_process_xml_steganos_node", strerror(errno)); */
/*       return I_MISC_ERR; */
/*     } */
    
/*     names[0] = xmlStrdup(name); */
/*     values[0] = xmlStrdup(value); */
        
/*     if(!names[0] || !values[0]) { */
/*       if(names[0]) free(names[0]); */
/*       if(values[0]) free(values[0]); */
/*       if(!elems) free(elems); */
/*       message_log("_process_xml_steganos_node", "Unknown error"); */
/*       return I_MISC_ERR; */
/*     } */

/*     *elems = 1; */
/*     return I_MISC_OK; */
    
/*   } else { */
/*     message_log("_process_xml_steganos_node", "Unknown parse error"); */
/*     return I_MISC_ERR; */
/*   } */

/*   message_log("_process_xml_steganos_node", "Unknown error"); */
    
/*   return I_MISC_ERR; */
/* } */

int to_byte(void *stream, size_t nmemb, size_t size, byte *byte_stream) {

  size_t i, j;
  int element;


  /* Input parameters control */ // TODO!! Todo controlado??
  if(!stream || nmemb <= 0 || size <= 0 || !byte_stream) {
    errno = EINVAL;
    message_log("to_byte", strerror(errno));
    return I_MISC_ERR;
  }

  /* Go through all stream's elements */ 
  for(i=1; i<=nmemb; i++) {
    
    /* Going through all the i-th element's bytes, using big-endian ordering. */
    for(j=1; j<=size; j++) {
      element = ((int *) stream)[i-1];  /* TODO!! parche!! 
					 si size > sizeof(int) esto casca */
      byte_stream[i*size-j] = element >> (j*BITS_PER_BYTE);
    }
    
  }

  return I_MISC_OK;

}

int print_bytestream(int fd, byte *stream, int length) {

  int i, f;
  pid_t pid;
  char *sdata, filename[20], bkln[2];


  if(fd < -1 || !stream || length <= 0) {
    errno = EINVAL;
    message_log("print_bytestream", strerror(errno));    
    return I_MISC_ERR;
  }
  
  if(fd == -1) {
    pid = getpid();
    memset(filename, 0, sizeof(char)*20);
    sprintf(filename, "log_%d", pid);
    if(!(f = open(filename, O_WRONLY | O_APPEND | O_CREAT, 
		  S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH)))
      return I_MISC_ERR;
  } else {
    f = fd;
  }
  
  /*   write(f, stream, length); */
  
  if(!(sdata = (char *) malloc(sizeof(char)*(length*2+1)))) {
    return I_MISC_ERR;
  }
  
  memset(sdata, 0, sizeof(char)*(length*2+1));
  
  for(i=0; i<length; i++) {
    sprintf(&sdata[2*i], "%X", stream[i]&0xF0);
    sprintf(&sdata[2*i+1], "%X", stream[i]&0x0F);
  }
  
  /*   fprintf(f, "%s", sdata); */
  if(write(f, sdata, length*2+1) == -1) {
    return I_MISC_ERR;
  }


  bkln[0] = '\n';
  bkln[1] = 0;
  if(write(f, bkln, 1) == -1) {
    return I_MISC_ERR;
  }

  if(fd == -1) close(f);
  
  free(sdata);
  
  return I_MISC_OK;

}

int bitstream_rol(byte *bitstream, int length, int rot) {

  int i, bytes_length, jump, r;
  byte carry, buff, *aux;


  if(!bitstream || length < 0 || rot < 0) {
    errno = EINVAL;
    message_log("bitstream_rol", strerror(errno));
    return I_MISC_ERR;
  }

  if(!rot) {
    return I_MISC_OK;
  }

  bytes_length = (int) ceilf((float)length/(float)BITS_PER_BYTE);
  if(!(aux = (byte *) malloc(sizeof(byte)*bytes_length))) {
    message_log("bitstream_rol", strerror(errno));
    return I_MISC_ERR;
  }
  memcpy(aux, bitstream, bytes_length);

  jump = rot/BITS_PER_BYTE;
  r = rot % BITS_PER_BYTE;

  carry = 0;
  for(i=bytes_length-jump-1; i>=0; i--) {
    buff = (bitstream[i+jump] & (0xFF << (BITS_PER_BYTE - r))) >> (BITS_PER_BYTE - r);
    aux[i] = (bitstream[i+jump] << r) | carry;
    carry = buff;
  }

  if(jump)
    memset(&aux[bytes_length-jump], 0, jump);
  memcpy(bitstream, aux, bytes_length);

  free(aux);
  return I_MISC_OK;

}

int bitstream_ror(byte *bitstream, int length, int rot) {

  int i, bytes_length, jump, r;
  byte carry, buff, *aux;


  if(!bitstream || length < 0 || rot < 0) {
    errno = EINVAL;
    message_log("bitstream_ror", strerror(errno));
    return I_MISC_ERR;
  }

  if(!rot) {
    return I_MISC_OK;
  }

  bytes_length = (int) ceilf((float)length/(float)BITS_PER_BYTE);
  if(!(aux = (byte *) malloc(sizeof(byte)*bytes_length))) {
    message_log("bitstream_ror", strerror(errno));
    return I_MISC_ERR;
  }
  memcpy(aux, bitstream, bytes_length);

  jump = rot/BITS_PER_BYTE;
  r = rot % BITS_PER_BYTE;

  carry = 0;
  for(i=jump; i<bytes_length; i++) {
    if(r) 
      buff = (bitstream[i-jump] & (0xFF >> (BITS_PER_BYTE - r))) << (BITS_PER_BYTE - r);
    aux[i] = (bitstream[i-jump] >> r); 
    if(r) {
      aux[i] |= carry;
      carry = buff;
    }
  }

  memset(aux, 0, jump);
  memcpy(bitstream, aux, bytes_length);

  free(aux);
  return I_MISC_OK;

}

int message_log(const char *caller, const char *message) {

#ifdef STEGANOS_DEBUG

  FILE *fd;
  pid_t pid;
  char filename[20];

  pid = getpid();
  memset(filename, 0, sizeof(char)*20);
  sprintf(filename, "log_%d", pid);
  if(!(fd = fopen(filename, "a"))) {
    return I_MISC_ERR;
  }

  if(!caller) {

    if(message) {
      fprintf(fd, "%ld: unknown caller: %s\n", time(NULL), message);
    } else {
      fprintf(fd, "%ld: unknown caller: unknown message\n", time(NULL));
    }

    fclose(fd);

    return I_MISC_OK;

  } else {

    if(message) {
      fprintf(fd, "%ld: %s: %s\n", time(NULL), caller, message);
    } else {
      fprintf(fd, "%ld: %s: unknown message\n", time(NULL), caller);
    }

  }

  fclose(fd);

#endif

  return I_MISC_OK;

}

struct option long_options_enc[] = {
  {"sdelayfr", required_argument, 0, 0},
  {"sda", required_argument, 0, 0},
  {"sfile", required_argument, 0, 0},
  {"shm", required_argument, 0, 0},
  {"ssm", required_argument, 0, 0},
  {"ssigma", required_argument, 0, 0},
  {"skey", required_argument, 0, 0},
  {"sca", required_argument, 0, 0},
  {"scmda", required_argument, 0, 0},
  {"schmac", no_argument, 0, 0},
  {"sciv", required_argument, 0, 0},
  {"scem", required_argument, 0, 0},
  {"scpkt", required_argument, 0, 0},
  {"scdds", required_argument, 0, 0},
  {"quiet", no_argument, 0, 0},
  {"force-read-file", no_argument, 0, 0},
  {NULL,0,0,0}
};

struct option long_options_dec[] = {
  {"sfile", required_argument, 0, 0},
  {"shm", required_argument, 0, 0},
  {"ssm", required_argument, 0, 0},
  {"ssigma", required_argument, 0, 0},
  {"skey", required_argument, 0, 0},
  {"sca", required_argument, 0, 0},
  {"scmda", required_argument, 0, 0},
  {"schmac", no_argument, 0, 0},
  {"scem", required_argument, 0, 0},
  {"scpkt", required_argument, 0, 0},
  {"quiet", no_argument, 0, 0},
  {"force-read-file", no_argument, 0, 0},
  {NULL,0,0,0}
};


int parse_options(char *cfg_file, fw_options_t *fw, inv_options_t *inv, int sender) {

  FILE *fd;
  char **_argv, _st_argv[31][100];
  int option_index, rc, ret, _argc, i, optind_aux;
  char c, str[100];

  /* Declare an set the params to their default values */
  int delayfr=0;          /* Number of frames to skip before hiding */
  int da=3;               /* Default desired aggressiveness */
  char *sfile=NULL;       /* Subliminal input/output file */
  int hide_method=-1;     /* Hiding method, if at the end is still -1, error */
  int sync_method=-1;     /* Default synchronization method, if at the end is still -1, error */
  float sigma=1.f;        /* Default sigma */
  char *skey=NULL;        /* Key */
  char *sca=NULL;         /* Cipher algorithm */
  char *scmda=NULL;       /* Digest algorithm */
  int schmac=0;           /* Hmac flag */
  char *sciv=NULL;        /* IV */
  uint64_t scem=0;        /* Emission ID */
  uint64_t scpkt=1;       /* First packet ID */
  int scdds=0;            /* Default data size for crypto-packets */  
  int quiet=0;            /* Quiet mode indicator */
  int force=0;            /* Force to read config from file */

  if(!cfg_file ||
     (sender && !fw) ||
     (!sender && !inv)) {
    errno = EINVAL;
    message_log("parse_options", strerror(errno));
    return I_MISC_ERR;
  }

  if(!(fd = fopen(cfg_file, "r"))) {
    message_log("parse_options", strerror(errno));
    return I_MISC_ERR;
  }

  for(i=0; i<14; i++) 
    memset(_st_argv[i], 0, 100);  

  i=0; c = 0; _argc = 0;
  memset(str, 0, 100);
  while(c != '\n') {
    c = fgetc(fd);
    if(c == EOF) {
      fprintf(stderr, "Unexpected End Of File\n");
      fclose(fd);
      errno = EBADF;
      return I_MISC_ERR;
    } else if(isspace(c)) {
      if(_argc > 31) {
	fprintf(stderr, "Unexpected param: %s\n", str);
	fclose(fd);
	errno = EINVAL;
	return I_MISC_ERR;
      }
      strcpy(_st_argv[_argc], str);
      _argc++;
      memset(str, 0,100);
      i = 0;
    } else {
      str[i] = c;
      i++;
    }
  }

  if(strcmp(_st_argv[0], cfg_file)) {
    fprintf(stderr, "Bad configuration file\n");
    errno = EBADF;
    fclose(fd);
    return I_MISC_ERR;
  }

  /* Allocate dynamic _argv */
  _argv = (char **) malloc(sizeof(char*)*_argc);
  if(!_argv) {
    fprintf(stderr, "%s", strerror(errno));
    fclose(fd);
    return I_MISC_ERR;
  }

  for(i=0; i<_argc; i++) {
    if(!(_argv[i] = (char *) malloc(sizeof(char)*(strlen(_st_argv[i])+1)))) {
      fprintf(stderr, "%s", strerror(errno));
      free(_argv);
      fclose(fd);
      return I_MISC_ERR;
    }
    memset(_argv[i], 0, strlen(_st_argv[i])+1);
    strcpy(_argv[i], _st_argv[i]);
  }

  rc = I_MISC_OK; ret = 0;
  option_index = 0;
  optind_aux = optind;
  optind = 1;
  while(ret != -1) {
    if(sender) {
      ret = getopt_long(_argc, _argv, "", long_options_enc, &option_index);
    } else {
      ret = getopt_long(_argc, _argv, "", long_options_dec, &option_index);
    }

    if(ret == -1) break;
    if(ret == '?') {
      rc = I_MISC_ERR;;
      goto free;
    }

    if(sender) {
      if(!strcmp(long_options_enc[option_index].name, "sdelayfr")) {
	if(sscanf(optarg, "%d", &delayfr)
	   != 1) {
	  fprintf(stderr, "Error: delay frames \"%s\" not recognised\n", optarg);
	  rc = I_MISC_ERR;;
	  goto free;
	}                 
      }
      else if(!strcmp(long_options_enc[option_index].name, "sda")) {
	if(sscanf(optarg, "%d", &da)
	   != 1) {
	  fprintf(stderr, "Error: desired aggressiveness \"%s\" not recognised\n", optarg);
	  rc = I_MISC_ERR;
	  goto free;
	}
	if(da < 1 || da > 10) {
	  fprintf(stderr, "Error: wrong aggressiveness \"%s\" (Must be in [1,10])\n", optarg);
	  rc = I_MISC_ERR;;
	  goto free;
	}
      }
      else if(!strcmp(long_options_enc[option_index].name, "sfile")) {
	sfile = strdup(optarg);
      }
      else if(!strcmp(long_options_enc[option_index].name, "shm")) {
	if(sscanf(optarg, "%d", &hide_method)
	   != 1) {
	  fprintf(stderr, "Error: hiding method \"%s\" not recognised\n", optarg);
	  rc = I_MISC_ERR;;
	  goto free;
	}
      }
      else if(!strcmp(long_options_enc[option_index].name, "ssm")) {
	if(sscanf(optarg, "%d", &sync_method)
	   != 1) {
	  fprintf(stderr, "Error: synchronization method \"%s\" not recognised\n", optarg);
	  rc = I_MISC_ERR;;
	  goto free;
	}
      }
      else if(!strcmp(long_options_enc[option_index].name, "ssigma")) {
	if(sscanf(optarg, "%f", &sigma)
	   != 1) {
	  fprintf(stderr, "Error: sigma \"%s\" not recognised\n", optarg);
	  rc = I_MISC_ERR;;
	  goto free;
	}
      }
      else if(!strcmp(long_options_enc[option_index].name, "skey")) {
	skey = strdup(optarg);
      }
      else if(!strcmp(long_options_enc[option_index].name, "sca")) {
	sca = strdup(optarg);
      }
      else if(!strcmp(long_options_enc[option_index].name, "scmda")) {
	scmda = strdup(optarg);
      }
      else if(!strcmp(long_options_enc[option_index].name, "schmac")) {
	schmac = 1;
      }
      else if(!strcmp(long_options_enc[option_index].name, "sciv")) {
	sciv = strdup(optarg);
      }
      else if(!strcmp(long_options_enc[option_index].name, "scem")) {
	if(sscanf(optarg, "%lu", &scem)
	   != 1) {
	  fprintf(stderr, "Error: Emission ID \"%s\" not recognised\n", optarg);
	  rc = I_MISC_ERR;;
	  goto free;
	}
      }
      else if(!strcmp(long_options_enc[option_index].name, "scpkt")) {
	if(sscanf(optarg, "%lu", &scpkt)
	   != 1) {
	  fprintf(stderr, "Error: Packet ID \"%s\" not recognised\n", optarg);
	  rc = I_MISC_ERR;;
	  goto free;
	}
      }
      else if(!strcmp(long_options_enc[option_index].name, "scdds")) {
	if(sscanf(optarg, "%d", &scdds)
	   != 1) {
	  fprintf(stderr, "Error: Default data size \"%s\" not recognised\n", optarg);
	  rc = I_MISC_ERR;;
	  goto free;
	}
      } 
      else if(!strcmp(long_options_enc[option_index].name, "quiet")) {
	quiet = 1;
      } 
      else if(!strcmp(long_options_enc[option_index].name, "force-read-file")) {
	force = 1;
      } else {
	fprintf(stderr, "Error: Unrecognised parameter\n");
	rc = I_MISC_ERR;;
	goto free;
      }
    } else {      
      if(!strcmp(long_options_dec[option_index].name, "sfile")) {
	sfile = strdup(optarg);
      }
      else if(!strcmp(long_options_dec[option_index].name, "shm")) {
	if(sscanf(optarg, "%d", &hide_method)
	   != 1) {
	  fprintf(stderr, "Error: hiding method \"%s\" not recognised\n", optarg);
	  rc = I_MISC_ERR;;
	  goto free;
	}
      }
      else if(!strcmp(long_options_dec[option_index].name, "ssm")) {
	if(sscanf(optarg, "%d", &sync_method)
	   != 1) {
	  fprintf(stderr, "Error: synchronization method \"%s\" not recognised\n", optarg);
	  rc = I_MISC_ERR;;
	  goto free;
	}
      }
      else if(!strcmp(long_options_dec[option_index].name, "ssigma")) {
	if(sscanf(optarg, "%f", &sigma)
	   != 1) {
	  fprintf(stderr, "Error: sigma \"%s\" not recognised\n", optarg);
	  rc = I_MISC_ERR;;
	  goto free;
	}
      }
      else if(!strcmp(long_options_dec[option_index].name, "skey")) {
	skey = strdup(optarg);
      }
      else if(!strcmp(long_options_dec[option_index].name, "sca")) {
	sca = strdup(optarg);
      }
      else if(!strcmp(long_options_dec[option_index].name, "scmda")) {
	scmda = strdup(optarg);
      }
      else if(!strcmp(long_options_dec[option_index].name, "schmac")) {
	schmac = 1;
      }
      else if(!strcmp(long_options_dec[option_index].name, "scem")) {
	if(sscanf(optarg, "%lu", &scem)
	   != 1) {
	  fprintf(stderr, "Error: Emission ID \"%s\" not recognised\n", optarg);
	  rc = I_MISC_ERR;;
	  goto free;
	}
      }
      else if(!strcmp(long_options_dec[option_index].name, "scpkt")) {
	if(sscanf(optarg, "%lu", &scpkt)
	   != 1) {
	  fprintf(stderr, "Error: Packet ID \"%s\" not recognised\n", optarg);
	  rc = I_MISC_ERR;;
	  goto free;
	}
      }
      else if(!strcmp(long_options_dec[option_index].name, "quiet")) {
	quiet = 1;
      }
      else if(!strcmp(long_options_dec[option_index].name, "force-read-file")) {
	force = 1;
      } else {
	fprintf(stderr, "Error: Unrecognised parameter\n");
	rc = I_MISC_ERR;;
	goto free;
      }

    }
   
  }

  /* Check required params */
  if(!skey) {
    fprintf(stderr, "Error: missing key\n");
    rc = I_MISC_ERR;;
    goto free;   
  }
  if(!sfile) {
    fprintf(stderr, "Error: missing input/output file\n");
    rc = I_MISC_ERR;;
    goto free;
  }
  if(hide_method == -1) {
    fprintf(stderr, "Error: missing hiding method\n");
    rc = I_MISC_ERR;;
    goto free;
  }
  if(sync_method == -1) {
    fprintf(stderr, "Error: missing synchronization method\n");
    rc = I_MISC_ERR;;
    goto free;
  }

  /* Update parameters */
  if(fw && sender) {
    fw->delayfr = delayfr;
    fw->da = da;
    fw->sfile = sfile;
    fw->hide_method = hide_method;
    fw->sync_method = sync_method;
    fw->sigma = sigma;
    fw->skey = skey;
    fw->sca = sca;
    fw->scmda = scmda;
    fw->schmac = schmac;
    fw->sciv = sciv;
    fw->scem = scem;
    fw->scpkt = scpkt;
    fw->scdds = scdds;
    fw->quiet = quiet;
    fw->force = force;
  } else {
    inv->sfile = sfile;
    inv->hide_method = hide_method;
    inv->sync_method = sync_method;
    inv->sigma = sigma;
    inv->skey = skey;
    inv->sca = sca;
    inv->scmda = scmda;
    inv->schmac = schmac;
    inv->scem = scem;
    inv->scpkt = scpkt;
    inv->quiet = quiet;
    inv->force = force;
  }

 free:
  
  fclose(fd);
  for(i=0; i<_argc; i++) {
    free(_argv[i]); 
    _argv[i] = NULL;
  }
  free(_argv); 
  _argv = NULL;

  /* Restore the previous optind */
  optind = optind_aux;

  return rc;

}

int zlib_def(FILE *source, FILE *dest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

int zlib_inf(FILE *source, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
      return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

void zlib_zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}


/* miscellaneous.c ends here */
#endif
