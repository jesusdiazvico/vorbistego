/*                               -*- Mode: C -*- 
 * @file: channel.h
 * @brief: Headers for the file channel.c from the software TODO!!
 * @author: Jesus Diaz Vico
 * Maintainer: 
 * Created: vie dec  18 17:28:17 2009 (+0100)
 * Version: 
 * Last-Updated: sáb ago 21 21:18:49 2010 (+0200)
 *           By: Jesus
 *     Update #: 165
 * URL: 
 */

#ifndef CHANNEL_H
#define CHANNEL_H

#include "global_types.h"
#include "steganos_types.h"
#include "numbers.h"

/**
 * @def DISTRESS_SYNC_BYTES
 * @brief Size, in bytes, of the "distress" header, used to resynchronize
 *  when using streaming mode.
 */
#define DISTRESS_SYNC_BYTES 3

/**
 * @def DISTRESS_SYNC_BITS
 * @brief Size, in bits, of the "distress" header, used to resynchronize
 *  when using streaming mode.
 */
#define DISTRESS_SYNC_BITS DISTRESS_SYNC_BYTES*BITS_PER_BYTE

/**
 * @def FILE_SIZE_BYTES
 * @brief Size, in bytes, of the field that will mark the amount of subliminal
 *  data that we will try to hide at a given audio file
 */
#define FILE_SIZE_BYTES 4

/**
 * @def FILE_SIZE_BITS
 * @brief Size, in bits, of the field that will mark the amount of subliminal
 *  data that we will try to hide at a given audio file
 */
#define FILE_SIZE_BITS FILE_SIZE_BYTES*BITS_PER_BYTE

/**
 * @def SYNCHRO_HEADER_BYTES_RES
 * @brief Length of the synchronization header field when using RES_HEADER
 *  synchronization mode. Doesn't include the size field bytes.
 */
#define SYNCHRO_HEADER_BYTES_RES 1

/**
 * @def SIZE_FIELD_BITS
 * @brief Length of the synchronization field addressed to indicate the 
 *  number of subliminal bits hided in a given frame and channel.
 */
#define SIZE_FIELD_BITS 8

/**
 * @def RES_HEADER_BITS
 * @brief The size in bits of THE WHOLE header when using RES_HEADER 
 *  synchronization method.
 */
#define RES_HEADER_BITS SYNCHRO_HEADER_BYTES_RES*BITS_PER_BYTE+SIZE_FIELD_BITS

/**
 * @def MAX_SUBLIMINAL_SIZE
 * @brief Maximum number of subliminal bits that could be hidden in a residue.
 */
#define MAX_SUBLIMINAL_SIZE ((1 << SIZE_FIELD_BITS) - 1)

/**
 * @def BITS_PARITY
 * @brief Number of bits to use to calculate the parity in the parity bit method
 */
#define BITS_PARITY 2

/**
 * @def ITU_R_BS_468_SIZE
 * @brief Size of the global static array ITU_R_BS_468
 */
#define ITU_R_BS_468_SIZE 20

/**
 * @var SYNCHRO_HEADER
 * @brief Defines the synchronization header to use. It must have the same
 *  length, in bits, as specified in BITS_SYNCHRO_HEADER
 * @see BITS_SYNCHRO_HEADER
 */
static const int SYNCHRO_HEADER[SYNCHRO_HEADER_BYTES_RES]={
  0xFF
};

/**
 * @var DISTRESS_SYNC_HEADER
 * @brief Defines the distress synchro header used in streaming mode. This header
 *  will occur once per file hided, while the SYNCHRO_HEADER will occur once per
 *  frame with subliminal data.
 * @see DISTRESS_SYNC_BYTES
 */
static const int DISTRESS_SYNC_HEADER[DISTRESS_SYNC_BYTES]={
  0xFF,
  0xFF,
  0xFF
};

/**
 * @var ITU_R_BS_468
 * @brief Precomputed tolerances for each center frequency, from ITU-R BS.468-4
 *
 * The values are presented in a double array, in which each element first
 * component is the center frequency and the second component is the factor
 * which, once multiplied by the current residual/floor value, will give the
 * proposed variation tolerance according to the dB noise tolerance specified in
 * ITU-R BS.458-4 for this frequency. To obtain the variations, the formula used
 * is cur*(pow(10, -ITU_468[freq])-1), where cur represents the current residual 
 * or floor value, freq the current frequency and ITU_468[freq] the dB tolerance 
 * proposed in ITU-R BS.468-4 for this frequency. For more details see
 * TODO!! enlace a la parte de la memoria donde se demuestra esta fórmula
 * TODO!! por qué no sale en doxygen?
 */
static const float ITU_R_BS_468[ITU_R_BS_468_SIZE][2]={
  {31.5f, 0.584893192},//2.f},
  {63.f, 0.380384265},//1.4f},
  {100.f, 0.258925412},//1.f},
  {200.f, 0.216186001},//0.85f},
  {400.f, 0.174897555},//0.7f},
  {800.f, 0.135010816},//0.55f},
  {1000.f, 0.122018454},//0.5f},
  {2000.f, 0.122018454},//0.5f},
  {3150.f, 0.122018454},//0.5f},
  {4000.f, 0.122018454},//0.5f},
  {5000.f, 0.122018454},//0.5f},
  {6300.f, 0.f},
  {7100.f, 0.047128548},//0.2f},
  {8000.f, 0.096478196},//0.4f},
  {9000.f, 0.148153621},//0.6f},
  {10000.f, 0.202264435},//0.8f},
  {12500.f, 0.318256739},//1.2f},
  {14000.f, 0.380384265},//1.4f},
  {16000.f, 0.445439771},//1.6f},
  {20000.f, 0.584893192}//2.f}
};

static const float FLOOR1[256]={
  1.0649863e-07F, 1.1341951e-07F, 1.2079015e-07F, 1.2863978e-07F,
  1.3699951e-07F, 1.4590251e-07F, 1.5538408e-07F, 1.6548181e-07F,
  1.7623575e-07F, 1.8768855e-07F, 1.9988561e-07F, 2.128753e-07F,
  2.2670913e-07F, 2.4144197e-07F, 2.5713223e-07F, 2.7384213e-07F,
  2.9163793e-07F, 3.1059021e-07F, 3.3077411e-07F, 3.5226968e-07F,
  3.7516214e-07F, 3.9954229e-07F, 4.2550680e-07F, 4.5315863e-07F,
  4.8260743e-07F, 5.1396998e-07F, 5.4737065e-07F, 5.8294187e-07F,
  6.2082472e-07F, 6.6116941e-07F, 7.0413592e-07F, 7.4989464e-07F,
  7.9862701e-07F, 8.5052630e-07F, 9.0579828e-07F, 9.6466216e-07F,
  1.0273513e-06F, 1.0941144e-06F, 1.1652161e-06F, 1.2409384e-06F,
  1.3215816e-06F, 1.4074654e-06F, 1.4989305e-06F, 1.5963394e-06F,
  1.7000785e-06F, 1.8105592e-06F, 1.9282195e-06F, 2.0535261e-06F,
  2.1869758e-06F, 2.3290978e-06F, 2.4804557e-06F, 2.6416497e-06F,
  2.8133190e-06F, 2.9961443e-06F, 3.1908506e-06F, 3.3982101e-06F,
  3.6190449e-06F, 3.8542308e-06F, 4.1047004e-06F, 4.3714470e-06F,
  4.6555282e-06F, 4.9580707e-06F, 5.2802740e-06F, 5.6234160e-06F,
  5.9888572e-06F, 6.3780469e-06F, 6.7925283e-06F, 7.2339451e-06F,
  7.7040476e-06F, 8.2047000e-06F, 8.7378876e-06F, 9.3057248e-06F,
  9.9104632e-06F, 1.0554501e-05F, 1.1240392e-05F, 1.1970856e-05F,
  1.2748789e-05F, 1.3577278e-05F, 1.4459606e-05F, 1.5399272e-05F,
  1.6400004e-05F, 1.7465768e-05F, 1.8600792e-05F, 1.9809576e-05F,
  2.1096914e-05F, 2.2467911e-05F, 2.3928002e-05F, 2.5482978e-05F,
  2.7139006e-05F, 2.8902651e-05F, 3.0780908e-05F, 3.2781225e-05F,
  3.4911534e-05F, 3.7180282e-05F, 3.9596466e-05F, 4.2169667e-05F,
  4.4910090e-05F, 4.7828601e-05F, 5.0936773e-05F, 5.4246931e-05F,
  5.7772202e-05F, 6.1526565e-05F, 6.5524908e-05F, 6.9783085e-05F,
  7.4317983e-05F, 7.9147585e-05F, 8.4291040e-05F, 8.9768747e-05F,
  9.5602426e-05F, 0.00010181521F, 0.00010843174F, 0.00011547824F,
  0.00012298267F, 0.00013097477F, 0.00013948625F, 0.00014855085F,
  0.00015820453F, 0.00016848555F, 0.00017943469F, 0.00019109536F,
  0.00020351382F, 0.00021673929F, 0.00023082423F, 0.00024582449F,
  0.00026179955F, 0.00027881276F, 0.00029693158F, 0.00031622787F,
  0.00033677814F, 0.00035866388F, 0.00038197188F, 0.00040679456F,
  0.00043323036F, 0.00046138411F, 0.00049136745F, 0.00052329927F,
  0.00055730621F, 0.00059352311F, 0.00063209358F, 0.00067317058F,
  0.00071691700F, 0.00076350630F, 0.00081312324F, 0.00086596457F,
  0.00092223983F, 0.00098217216F, 0.0010459992F, 0.0011139742F,
  0.0011863665F, 0.0012634633F, 0.0013455702F, 0.0014330129F,
  0.0015261382F, 0.0016253153F, 0.0017309374F, 0.0018434235F,
  0.0019632195F, 0.0020908006F, 0.0022266726F, 0.0023713743F,
  0.0025254795F, 0.0026895994F, 0.0028643847F, 0.0030505286F,
  0.0032487691F, 0.0034598925F, 0.0036847358F, 0.0039241906F,
  0.0041792066F, 0.0044507950F, 0.0047400328F, 0.0050480668F,
  0.0053761186F, 0.0057254891F, 0.0060975636F, 0.0064938176F,
  0.0069158225F, 0.0073652516F, 0.0078438871F, 0.0083536271F,
  0.0088964928F, 0.009474637F, 0.010090352F, 0.010746080F,
  0.011444421F, 0.012188144F, 0.012980198F, 0.013823725F,
  0.014722068F, 0.015678791F, 0.016697687F, 0.017782797F,
  0.018938423F, 0.020169149F, 0.021479854F, 0.022875735F,
  0.024362330F, 0.025945531F, 0.027631618F, 0.029427276F,
  0.031339626F, 0.033376252F, 0.035545228F, 0.037855157F,
  0.040315199F, 0.042935108F, 0.045725273F, 0.048696758F,
  0.051861348F, 0.055231591F, 0.058820850F, 0.062643361F,
  0.066714279F, 0.071049749F, 0.075666962F, 0.080584227F,
  0.085821044F, 0.091398179F, 0.097337747F, 0.10366330F,
  0.11039993F, 0.11757434F, 0.12521498F, 0.13335215F,
  0.14201813F, 0.15124727F, 0.16107617F, 0.17154380F,
  0.18269168F, 0.19456402F, 0.20720788F, 0.22067342F,
  0.23501402F, 0.25028656F, 0.26655159F, 0.28387361F,
  0.30232132F, 0.32196786F, 0.34289114F, 0.36517414F,
  0.38890521F, 0.41417847F, 0.44109412F, 0.46975890F,
  0.50028648F, 0.53279791F, 0.56742212F, 0.60429640F,
  0.64356699F, 0.68538959F, 0.72993007F, 0.77736504F,
  0.82788260F, 0.88168307F, 0.9389798F, 1.F,
};

/** 
 * @fn int steganos_state_init(steganos_state_t *ss, int da, int hide_method, 
 *                             int sync_method, char *key, int keylen)
 * @brief Initializes the members of the received stegano_state_t structure.
 * 
 * This function prepares the steganographic protocol for a new whole step, 
 * allocating and initializing the steganos state structure to the default
 * values or those specified as arguments.
 * @param[in] ss Pointer to the steganographic protocol state variable.
 * @param[in] da Desired aggressiveness, i.e., the share of subliminal capacity
 *               to use, in 1/10 (e.g. 2 means use 20%)
 * @param[in] hide_method Hide method to use, must be one of the defined in
 *            hide_method_et
 * @param[in] sync_method Sync method to use, must be one of the defined in
 *            sync_method_et
 * @param[in] key Master key to use for deriving subkeys
 * @param[in] keylen Length of <i>key</i> in bits. Must be at least 128.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 *
 * @see steganos_state_t 
 */
int steganos_state_init(steganos_state_t *ss, int da, int hide_method, 
			int sync_method, char *key, int keylen);

/** 
 * @fn int steganos_state_reset_iter(steganos_state_t *ss)
 * @brief Resets the members of the steganos state structure that change
 *  from one iteration to the next
 *
 * @param[in] ss Pointer to the steganographic protocol state variable.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 *
 * @see steganos_state_t 
 */
int steganos_state_reset_iter(steganos_state_t *ss);

/** 
 * @fn int steganos_state_free(steganos_state_t *ss)
 * @brief Frees the members of the received stegano_state_t structure. The
 *  structure itself must be freed separately.
 *
 * @param[in] ss Pointer to the steganographic protocol state variable.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 *
 * @see steganos_state_t 
 */
int steganos_state_free(steganos_state_t *ss);

/** 
 * @fn int steganos_prepare_packet_keys(vorbis_config_t *vc, 
 *                                             steganos_state_t *ss)
 * @brief Manipulates the keys to be ready to be used in a new steganos packet
 *
 * The key used to hide/sync is obtained following this flow:
 *  1) Cipher md5(vc->forward_index) with ARCFOUR, using the master key
 *  2) Use the resulting ciphertext as frame specific keys
 * 
 * @param[in] vc Vorbis config structure
 * @param[in, out] ss Steganos state structure. The internal variables 
 *  ss->hiding_key and ss->synchro_key will be updated upon successful execution.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 */
int steganos_prepare_packet_keys(vorbis_config_t *vc, steganos_state_t *ss);

/** 
 * @fn static int steganos_free_packet_keys(steganos_state_t *ss)
 * @brief Frees the keys used in the current steganos packet
 * 
 * @param[in] ss Steganos state structure
 *
 * @return I_STEGANOS_OK 
 */
int steganos_free_packet_keys(steganos_state_t *ss);

/**
 * @fn int set_subliminal_capacity_limit(steganos_state_t *ss, float *residue, 
 *                                       const long rate, const int res_len)
 * @brief Determines and sets the subliminal capacity limit for the current
 *  frame and channel.
 *
 * This function determines the subliminal capacity limit for the current
 * frame and channel residue, updating the steganos_state_t internal state 
 * structure with the values obtained. This limits will be stored in the 
 * (float *)[2] field as [-, +] maximum variations to introduce in the 
 * corresponding residual value of the frame and channel's residue vector. Also
 * updates the maximum subliminal capacity for the whole frame and channel
 * current residue. The protocol state must be IDLE at the input and MEASURED
 * after a successful execution. Also initializes the ss->res_mask variable, 
 * which can be seen as some kind of "steganosgraphic mask", calculated initially
 * from the maximum residual variations here obtained. The protocol state 
 * structure variables must have been allocated and reset previously.
 * 
 * @param[in, out] ss Internal state structure. It's field capacity_limit will be
 *  updated in a successful execution. The protocol state must be IDLE at the 
 *  input and MEASURED after a successful execution.
 * @param[in] residue The current frame and channel residue vector.
 * @param[in] rate The sampling rate used (in Hz), to obtain the frequency 
 *  corresponding to each residue.
 * @param[in] res_len Length of the residue and frequencies vectors.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 * @see steganos_state_t
 */
int set_subliminal_capacity_limit(steganos_state_t *ss, float *residue, 
				  const long rate, const int res_len);

/**
 * @fn int hide_data(steganos_state_t *ss, byte *data, const int d_len, 
 *                   int *floor, float *residue, const int res_len, int *size)
 * @brief Determines the final usage of the residue subliminal channel for the 
 *  current frame and channel by getting the desired size of subliminal data to
 *  hide in it.
 *
 * This function first retrieves the next 'ss->aggressiveness*ss->max_fc_capacity'
 * bits of subliminal data to hide in the current frame and channel residue, 
 * using the chosen hiding function. This let us make a more precise estimation
 * of the final effects of the changes we will introduce in the original data.
 * Once this is done, the nearest integer size of bits is hided in the residue 
 * vector, giving the final subliminal-residue vector. Note that this function
 * shouldn't be called if ss->aggressiveness*ss->max_fc_capacity is not big
 * enough to store the meta-information required (plus some subliminal info).
 * 
 * @param[in, out] ss Internal state structure. The function updates the 
 *  ss->protocol_send_state field, which must be SYNCHRONIZED at the input and
 *  will be CONTROLLED after a successful execution.
 * @param[in] data The remaining bitstream to hide. Note that can have greater, 
 *  equal or less size than the subliminal channel usage.
 * @param[in] d_len The length of the 'data' stream, in bits.
 * @param[in] floor The current frame and channel floor vecotr. Depending on the
 *  hiding method selected, it may or may not be used.
 * @param[in, out] residue The current frame and channel residue vector. After a
 *  successful execution of the function, it will store the subliminal residue.
 * @param[in] res_len The length of the residue (and floor) vector.
 * @param[out] size The final number of bits of pure data (excluding headers) 
 *  hided in the current frame and channel residue.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated
 * @retval I_STEGANOS_OK with errno = 0 (No error)
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument)
 * @see steganos_state_t
 */
int hide_data(steganos_state_t *ss, byte *data, const int d_len, 
	      int *floor, float *residue, const int res_len, int *size);

/**
 * @fn int unhide_data(steganos_state_t *ss, int *floor, float *residue, 
 *                     const int res_len, byte **data, int *read)
 * @brief Reads subliminal data from a subliminal residue.
 *
 * This function recovers the subliminal data hided in the current residue. If
 * the ISS synchronization mode is in use, and this function is called, means
 * the residue surely shelters subliminal data. If the RES_HEADER synchro mode
 * is in use, we have to look for the SYNCHRO_HEADER first, which will tell us
 * if we have to recover any data. At the end of a successful execution, 
 * <i>data</i> will point to the recovered data and <i>read</i> store the number
 * of bits read. If no data has been recovered, <i>read</i> will be 0.
 *
 * @note This function uses the PRNG function <i>get_random_int</i>, therefore,
 *  the PRNG must have been seeded previously with the same key used for hiding
 *  in order to get the correct data from the residue/floor vectors.
 *
 * @param[in, out] ss Internal state structure. The protocol state must be 
 *  ANALYZED at the input and READ after a successful execution.
 * @param[in] floor The floor vector of the current frame and channel.
 * @param[in] residue The residue vector of the current frame and channel, 
 *  containing the subliminal data, if any.
 * @param[in] res_len The length of the residue and floor vectors.
 * @param[in, out] data Pointer to the bitstream which will store the subliminal
 *  data read after a successful execution. 
 * @param[in, out] read Will store the final amount of data that has been 
 *  possible to read, in bits.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 * @see steganos_state_t
 */
int unhide_data(steganos_state_t *ss, int *floor, float *residue, 
			 const int res_len, byte **data, int *read);
/* int _get_subliminal_data(steganos_state_t *ss, byte *data, const int d_len,  */
/* 			int *floor, float *residue, const int res_len, */
/* 			byte *sub_data, int *size); */
/* int _write_subliminal_data(steganos_state_t *ss, byte *data, const int d_len, */
/* 			  float *residue, const int res_len, int *written); */
/* int _read_subliminal_residue(const float residue, byte *data, int *read); */
/* int _loudness_noise_control(steganos_state_t *ss, const float *residue,  */
/* 			   float *sub_residue, const int res_len); */
/* int _residue_variation(const float residue, const float frequency,  */
/* 		       float *variation_limit); */

/**
 * @fn int parity_bits_method(byte *plain_mess, int p_m_len, 
 *                            int *floor, float *res, int res_len,
 *                            steganos_key_t *key, byte *sub_mess, 
 *                            int *s_m_len, prng_t *prng)
 * @brief Implementation of Anderson's and Petitcolas's parity bit method.
 *
 * Implementation of Anderson's and Petitcolas's parity bit method. See "On The 
 * Limits of Steganosgraphy" by Anderson and Petitcolas. The function keeps the
 * (*hide_method) typedef format.
 *
 * @note This function uses the PRNG function <i>get_random_int</i>, therefore,
 *  the PRNG must have been seeded previously with the same key used for hiding
 *  in order to get the correct data from the residue/floor vectors.
 *
 * @param[in] plain_mess Plain message to hide.
 * @param[in] p_m_len Length of plain message in bits.
 * @param[in] floor Floor vector.
 * @param[in] res Residue vector.
 * @param[in] res_len Number of elements in <i>floor</i> and <i>res</i> vectors.
 * @param[in] key Key to use.
 * @param[in, out] sub_mess String in which the subliminal message will be stored. 
 *  It has to be allocated previously to calling the function.
 * @param[in, out] s_m_len At the input, will store the allocated size (in bits)
 *  of <i>sub_mess</i>. At the output, the length of the resulting sub_message, 
 *  in bits (it may  be different than p_m_len as it may include 
 *  de-synchronization bits), or discard them.
 * @param[in] prng PRNG in use.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 * @see hide_method
 */
int parity_bits_method(byte *plain_mess, int p_m_len,
		       int *floor, float *res, int res_len, 
		       steganos_key_t *key, byte *sub_mess, 
		       int *s_m_len, prng_t *prng);

/**
 * @fn int void_method(byte *plain_mess, int p_m_len, 
 *                     int *floor, float *res, int res_len,
 *                     steganos_key_t *key, byte *sub_mess, 
 *                     int *s_m_len, prng_t *prng)
 * @brief Applies no hiding method to the subliminal data.
 *
 * This hiding method simply returns the first p_m_len bits of plain_mess.
 *
 * @param[in] plain_mess Plain message to hide.
 * @param[in] p_m_len Length of plain message in bits.
 * @param[in] floor Floor vector.
 * @param[in] res Residue vector.
 * @param[in] res_len Number of elements in <i>floor</i> and <i>res</i> vectors.
 * @param[in] key Key to use.
 * @param[in, out] sub_mess String in which the subliminal message will be stored. 
 *  It has to be allocated previously to calling the function.
 * @param[in, out] s_m_len At the input, will store the allocated size (in bits)
 *  of <i>sub_mess</i>. At the output, the length of the resulting sub_message, 
 *  in bits (it may  be different than p_m_len as it may include 
 *  de-synchronization bits).
 * @param[in] prng PRNG in use. Here, NULL.
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 * @see hide_method
 */
int void_method(byte *plain_mess, int p_m_len, 
		int *floor, float *res, int res_len,
		steganos_key_t *key, byte *sub_mess, 
		int *s_m_len, prng_t *prng);

/**
 * @fn int synchro_iss (vorbis_config_t *vc, steganos_key_t *key, int *posts, 
 *		        float *residue, void *cfg, int decoding, int *bit,
 *                      prng_t *prng)
 * @brief Implementation of ISS watermarking for synchronization method.
 *
 * Implements the Improved Spread Spectrum watermarking technique introduced by
 * Malvar and Florencio (in "Improved Spread Spectrum: a new modulation technique
 * for robust watermarking") to synchronize sender and receiver by marking the 
 * floor vector.
 *
 * @param[in] vc Vorbis block and look information.
 * @param[in] key Key to use
 * @param[in, out] posts Current floor posts vector. After a successful 
 *  execution, it will be modified as needed.
 * @param[in, out] residue Current residue vector. Not used here.
 * @param[in] cfg Pointer to a structure of iss_cfg_t with the alpha, lambda
 *  and sigma parameters needed for ISS.
 * @param[in] decoding Active if the function is called from the decoder's side,
 * @param[in, out] bit At the input, the desired bit to include if we are at the
 *  encoder's side; at the ouput, the received bit if we are at the decoder's 
 *  side (i.e. 1 if we're telling the receiver there is hidden data and 0 if 
 *  not).
 * @param[in] prng PRNG in use.
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
int synchro_iss(vorbis_config_t *vc, steganos_key_t *key, int *posts, 
		float *residue, void *cfg, int decoding, int *bit,
		prng_t *prng);

/** 
 * @fn int iss_cfg_init(steganos_state_t *ss, int *posts, int posts_len, 
 *                      float sigma, iss_cfg_t *iss_cfg)
 * 
 * @brief Initializes the sigma, lambda and alpha values needed for ISS 
 * accordingly to the formulas from Malvar and Florencio's.
 * 
 * @param[in] ss Internal state structure.
 * @param[in] posts Post vector.
 * @param[in] posts_len Number of elements in the posts vector.
 * @param[in] sigma Sigma parameter (watermark's strength)
 * @param[in, out] iss_cfg ISS configuration structure.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 *
 * @see iss_synchronization
 */
int iss_cfg_init(steganos_state_t *ss, int *posts, int posts_len,
		 float sigma, iss_cfg_t *iss_cfg);

/** 
 * @fn int iss_cfg_free(iss_cfg_t *iss_cfg)
 * 
 * @brief Frees the memory allocated for a iss_cfg_t structure.
 * 
 * @param[in, out] iss_cfg ISS configuration structure to free.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 *
 * @see iss_synchronization
 */
int iss_cfg_free(iss_cfg_t *iss_cfg);

/** 
 * @fn int iss_simulate_floor(vorbis_config_t *vc, int *posts, int *floor)
 * 
 * @brief Simulates the floor that will be calculated by te receiver may the post
 *  vector be like <i>posts</i>
 *
 * @param[in] vc Voribs block and look configuration data.
 * @param[in, out] floor The floor vector, as will be calculated by the receiver.
 *  The memory must be allocated previously to calling the function.
 * @param[in] posts The posts vector.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 */
int iss_simulate_floor(vorbis_config_t *vc, int *posts, int *floor);

/**
 * @typedef int desynchro_res_header(vorbis_config_t *vc, int *res_lineup, 
 *                                   int *posts, int *floor, float *residue, 
 *                                   void *cfg, steganos_key_t *hiding_key,
 *                                   hide_method hide, prng_t *prng)
 *
 * @brief Type definition for desynchronization functions to use in the protocol.
 *
 * Functions of this type shall solve the false positive synchronization problem
 * that may happen depending on the synchro_method used. Depending on the
 * specific method in use, the <i>floor</i> and <i>residue</i> vectors will or 
 * won't be used.
 *
 * @param[in] vc Vorbis block and look info.
 * @param[in] res_lineup Residue ordering
 * @param[in, out] posts Current floor posts vector.
 * @param[in] floor Floor vector
 * @param[in, out] residue Current residue vector. Will be changed if it comes
 *  "naturally" synchronized.
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
int desynchro_res_header(vorbis_config_t *vc, int *res_lineup, int *posts,
			 int *floor, float *residue, void *cfg, 
			 steganos_key_t *hiding_key, hide_method hide,
			 prng_t *prng);

/**
 * @typedef int desynchro_iss(vorbis_config_t *vc, int *res_lineup, 
 *                            int *posts, int *floor, float *residue, 
 *                            void *cfg, steganos_key_t *hiding_key,
 *                            hide_method hide, prng_t *prng)
 *
 * @brief Desynchronization function when using ISS synchronization method.
 *
 * This function should be called when it is not possible to hide information
 * in a given frame-channel and the synchronization method in use is ISS. This
 * function will set the first SIZE_FIELD_BITS of the given residue to 0, 
 * ensuring this way that, despite the receiver will receive a 1-marked floor,
 * he'll read 0 bytes of data.
 *
 * @param[in] vc Vorbis block and look info.
 * @param[in] res_lineup Residue ordering
 * @param[in, out] posts Current floor posts vector.
 * @param[in] floor Floor vector
 * @param[in, out] residue Current residue vector. Will be changed if it comes
 *  "naturally" synchronized.
 * @param[in] cfg ISS configuration options (sigma, alpha, etc.)
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
int desynchro_iss(vorbis_config_t *vc, int *res_lineup, int *posts, 
		   int *floor, float *residue, void *cfg, 
		   steganos_key_t *hiding_key, hide_method hide, 
		   prng_t *prng);

/* int _itu468_var_tol(const float frequency, const float base, float *tolerance); */

/** 
 * @fn int steganos_key_init(byte *byte_key, const int key_len, steganos_key_t *key)
 * @brief Allocates memory for the key fields and sets them to the received 
 *  values.
 * 
 * Allocates memory for the key fields and sets them to the received values.
 *
 * @param[in] byte_key The key value.
 * @param[in] key_len The length of the <i>byte_key</i> argument, in bits.
 * @param[in, out] key A pointer to a previously allocated steganos_key_t variable in
 *  which we'll store the key's members.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument). 
 * @retval I_STEGANOS_ERR with errno = ERANGE (invalid key size).
 */
int steganos_key_init(byte *byte_key, const int key_len, steganos_key_t *key);

/** 
 * @fn calculate_residue_lineup(steganos_state_t *ss, const int res_len)
 * @brief Calculates the residue lineup using the prng in ss.
 * 
 * @param[in] ss Steganos state structure.
 * @param[in] res_len Length of the current residue vector.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK
 * @retval I_STEGANOS_ERR
 */
int calculate_residue_lineup(steganos_state_t *ss, const int res_len);

#endif /* CHANNEL_H */

/* channel.h ends here */
