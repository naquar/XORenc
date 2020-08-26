// Warning: Best read if using a monospaced/fixed-width font and tab width of 4.
#include <argon2.h>
#include <libscrypt.h>
#include <libavutil/md5.h>

/** ================================================================================

	This file is part of 'XORenc'.

	'XORenc' is a "XOR-based" data encryption tool.


	License:

	The MIT License (MIT)

	Copyright (c) 2019 Renan Souza da Motta <renansouzadamotta@yahoo.com>

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to
	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
	the Software, and to permit persons to whom the Software is furnished to do so,
	subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
	COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
	IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
	CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

	================================================================================ */

const uint32_t XORENC_MIN_PASSWORD_LENGTH = 8;
const size_t   XORENC_FILE_BLOCK_SIZE     = 1024 * 1024; // 1024 bytes * 1024 = 1 MiB
const char*    XORENC_SALT                = "3XsCYUXjzoubgVeWADLV65iVhpbkGd1A6FUYiHVf4gzn735b";

typedef enum {
	Direct=1,
	Derived
} TXORencKeyType;

typedef struct {
	void*  data;
	size_t length;
} TXORencKey;

typedef struct {
	TXORencKeyType key_type; // the type of the key
} TXORencParams;

typedef struct {
	uint8_t* data;
	size_t   length;
} TXORencHash;

/** ----------------------------------------------------------------------------------------

	XORenc_int2hex:

		Converts input number to hexadecimal string;

	Parameters:

		n   -> The number to be converted.

		pad -> Zero pad the generated number to 'x'.

	Return value:

		Returns pointer to result string.

	---------------------------------------------------------------------------------------- */
char* XORenc_int2hex(size_t n, unsigned int pad, bool lowercase) {

	const int MAX_LEN = 8; // max length of input 'n' (in bytes)

	char* temp   = calloc(1, (MAX_LEN * 2)+1);
	char* RESULT = calloc(1, (MAX_LEN * 2)+1);
	
	size_t lpp0; // loop exclusive variable


	if ((sizeof(n) > MAX_LEN) || (pad > (MAX_LEN * 2))) {
		// exceeds maximum of MAX_LEN bytes
		return NULL;
	}
	
	
	// start conversion
	for (lpp0=0; lpp0 < (MAX_LEN * 2); lpp0++) {
		// get current hexadecimal digit
		char digit = (n & 0x0F);
		
		// it is 0-9 (decimal)
		temp[(MAX_LEN * 2)-1-lpp0] = (digit + 0x30);
		
		// not decimal?! Add +7...:)
		if (digit > 0x09) {
			temp[(MAX_LEN * 2)-1-lpp0] += 0x07;
			
			if (lowercase) {
				temp[(MAX_LEN * 2)-1-lpp0] += 0x20;
			}
		}
		
		// go to next digit
		n >>= 4;
	}
	
	
	// set padding
	if (pad > 0) {
		RESULT = strcpy(RESULT, &temp[(MAX_LEN * 2)-pad]);
	}
	else {
		// pad to nearest
		lpp0 = 0;
		
		while (temp[lpp0] == '0') {
			lpp0++;
		}

		RESULT = strcpy(RESULT, &temp[lpp0]);
	}
	
	free(temp);

	
	return RESULT;
}



/** ----------------------------------------------------------------------------------------

	XORenc_md5:

		Perform MD5 computation of input data.
      
		Result digest is written in binary, as an array of byte in 'digest_b' (16 bytes).
      
		Result digest is written as string, as an array of char in 'digest_s' (32 bytes).
      	
		Attention because 'digest_s' is NOT null terminated.

	Parameters:

		data     -> Block of input data to be encrypted.
        
		data_len -> Length of input data (in bytes).
        
		digest_b -> Where result digest will be written to (binary).

		digest_s -> Where result digest will be written to (string).

	---------------------------------------------------------------------------------------- */
void XORenc_md5(const uint8_t* data, const size_t data_len, uint8_t* digest_b, char* digest_s) {

	size_t lpp0;
	struct AVMD5* md5_ctx = calloc(1, av_md5_size);
  
	av_md5_init(md5_ctx);

	av_md5_sum(digest_b, data, data_len);
  

	for (lpp0=0; lpp0 < 16; lpp0++) {
		char* tmp = XORenc_int2hex(digest_b[lpp0], 2, true);
  	
		digest_s[(lpp0*2)+0] = tmp[0];
		digest_s[(lpp0*2)+1] = tmp[1];
  	
		free(tmp);
	}


	free(md5_ctx);
}



/** ----------------------------------------------------------------------------------------

	XORenc_hash_scrypt:

		Generate 'Scrypt' hash of 'XORENC_FILE_BLOCK_SIZE' in length.
      
		Result hash is written in binary, as an array of byte in 'hash.data'.

	Parameters:

		pass     -> Input password as string.
        
		pass_len -> Length of password string.
        
		salt     -> Input salt as string.
        
		salt_len -> Length of salt string.
        
	Return value:

		Returns derived data as TXORencHash.

	---------------------------------------------------------------------------------------- */
TXORencHash XORenc_hash_scrypt(	const char*  pass,
								const size_t pass_len,
								const char*  salt,
								const size_t salt_len ) {

	TXORencHash RESULT;

	/* ******* --- XORenc_hash_scrypt --- ******* */

	RESULT.data   = NULL;
	RESULT.length = XORENC_FILE_BLOCK_SIZE;

	RESULT.data = malloc(XORENC_FILE_BLOCK_SIZE);

	if (RESULT.data == NULL) {
		RESULT.length = 0;

		return RESULT;
	}
	
	
	// set 'Scrypt' parameters and generate hash
	const char*  password        = pass;          // password
	const size_t password_length = pass_len;      // password length
	const char*  salt_           = salt;          // salt
	const size_t salt_length     = salt_len;      // salt length
	uint64_t     scrypt_N        = 1024 * 32;     // CPU and RAM cost
	uint32_t     scrypt_r        = 16;            // RAM cost
	uint32_t     scrypt_p        = 2;             // CPU cost (parallelisation)
	uint8_t*     hash            = RESULT.data;   // where to write result hash
	const size_t hash_len        = RESULT.length; // length of hash to write

	int r = libscrypt_scrypt((uint8_t*)password, password_length, (uint8_t*)salt_, salt_length, scrypt_N, scrypt_r, scrypt_p, hash, hash_len);

	if (r != 0) {
		// operation failed! :(
		free(RESULT.data);
		
		RESULT.data = NULL;
		
		return RESULT;
	}


	return RESULT;
}



/** ----------------------------------------------------------------------------------------

	XORenc_hash_argon2:

		Generate 'Argon2' hash of 'XORENC_FILE_BLOCK_SIZE' in length.
      
		Result hash is written in binary, as an array of byte in 'hash.data'.

	Parameters:

		pass     -> Input password as string.
        
		pass_len -> Length of password string.
        
		salt     -> Input salt as string.
        
		salt_len -> Length of salt string.
        
	Return value:

		Returns derived data as TXORencHash.

	---------------------------------------------------------------------------------------- */
TXORencHash XORenc_hash_argon2(	const char*  pass,
								const size_t pass_len,
								const char*  salt,
								const size_t salt_len ) {

	TXORencHash RESULT;

	/* ******* --- XORenc_hash_argon2 --- ******* */

	RESULT.data   = NULL;
	RESULT.length = XORENC_FILE_BLOCK_SIZE;

	RESULT.data = malloc(XORENC_FILE_BLOCK_SIZE);

	if (RESULT.data == NULL) {
		RESULT.length = 0;
	
		return RESULT;
	}
	
	
	// set 'Argon2' parameters and generate hash
	const uint32_t iterations      = 7;             // number of iterations
	const uint32_t memory          = 131072;        // memory in KiB
	const uint32_t threads         = 2;             // number of threads
	const char*    password        = pass;          // password
	const size_t   password_length = pass_len;      // password length
	const char*    salt_           = salt;          // salt
	const size_t   salt_length     = salt_len;      // salt length
	uint8_t*       hash            = RESULT.data;   // where to write result hash
	const size_t   hash_len        = RESULT.length; // length of hash to write

	int r = argon2i_hash_raw(iterations, memory, threads, password, password_length, salt_, salt_length, hash, hash_len);
		
	if (r != ARGON2_OK) {
		// operation failed! :(
		free(RESULT.data);
		
		RESULT.data = NULL;
		
		return RESULT;
	}


	return RESULT;
}



bool XORenc_key_is_byte_sequence(const char* key) {

	// check if input key is of the format: 'XX XX XX...' -> Where 'X' is anything in the range 0-9 and A-F.
	
	size_t lpp0;
	
	/* ******* --- XORenc_key_is_byte_sequence --- ******* */
	
	if (strlen(key) < 2) {
		// not a valid byte sequence key
		return false;
	}
	
	
	// length of key must be a multiple of 3 or a multiple of 3-1
	if ((((strlen(key) + 0) % 3) == 0) || (((strlen(key) + 1) % 3) == 0)) {
		// length is OK, it can be a byte sequence key; let's check it! :)
		lpp0 = 0;
		
		while (lpp0 < strlen(key)) {
			if (((key[lpp0] < '0') || (key[lpp0] > '9')) && ((key[lpp0] < 'A') || (key[lpp0] > 'F'))) {
				// not a valid byte sequence key
				return false;
			}
			
			lpp0 += 1;
			
			if (((key[lpp0] < '0') || (key[lpp0] > '9')) && ((key[lpp0] < 'A') || (key[lpp0] > 'F'))) {
				// not a valid byte sequence key
				return false;
			}
			
			lpp0 += 1;
			
			if (lpp0 < strlen(key)) {
				// check for a space
				if (key[lpp0] != ' ') {
					return false;
				}
			}
			
			lpp0 += 1;
		}
	}
	else {
		return false;
	}
	
	// survived until here, return 'true' :)
	return true;
}



/** ----------------------------------------------------------------------------------------

	XORenc_key_load:

		Load a key from file or command line (option '--key, -k') and returns it.
      
		This function can be used to load keys when encrypting with 'XORenc_encrypt_direct'.

	Parameters:

		str       -> Refers to path to a key file or key in the format:
        
			'XX XX XX...'
                 
			Where 'X' is anything in the range 0-9 and A-F.
        
		block     -> Block of key to be retrieved (0->whole key is loaded; >=1->first/Nth block).
        
		block_len -> The actual length of the block or the key.
        
	Return value:
			
		The pointer to key data of size 'block_len'.
				
		Returns 'NULL' if it fails.

	---------------------------------------------------------------------------------------- */
TXORencKey XORenc_key_load(const char* str, const size_t block) {

	TXORencKey RESULT;
	FILE*      fd0;
	// loop vars
	size_t lpp0, lpp1;
	
	/* ******* --- XORenc_key_load --- ******* */
	
	RESULT.data = malloc(XORENC_FILE_BLOCK_SIZE);
	
	if (RESULT.data == NULL) {
		return RESULT;
	}


	// check if file exists and is accessible
	if (access(str, R_OK) == 0) {
		// file exists, it is key file :)
		fd0 = fopen(str, "rb");
		
		if (fd0 == NULL) {
			// could not open file
			free(RESULT.data);
			
			RESULT.data   = NULL;
			RESULT.length = 0;
			
			return RESULT;
		}


		if (block == 0) {
			// single block load
			RESULT.length = fread(RESULT.data, 1, XORENC_FILE_BLOCK_SIZE, fd0);
			
			if (RESULT.length > 0) {
				// key was loaded successfully! :)
				fclose(fd0);
				
				return RESULT;
			}
		}
		else {
			// block load
			fseek(fd0, XORENC_FILE_BLOCK_SIZE * block, SEEK_SET);
			
			RESULT.length = fread(RESULT.data, 1, XORENC_FILE_BLOCK_SIZE, fd0);
			
			if (RESULT.length == 0) {
				// could not load block from file
				fclose(fd0);
				
				free(RESULT.data);
				
				RESULT.data   = NULL;
				RESULT.length = 0;
				
				return RESULT;
			}
			else {
				// key was loaded successfully! :)
				fclose(fd0);
				
				return RESULT;
			}
		}
	}
	else if ((XORenc_key_is_byte_sequence(str) == true) && (block == 0)) {
		// it is byte sequence of type: 'XX XX XX...'; convert it to binary...
		uint8_t* key_data = RESULT.data;
		
		lpp0 = 0;
		lpp1 = 0;
		
		while (lpp0 < strlen(str)) {
			// high order digit
			if ((str[lpp0] >= 'A') && (str[lpp0] <= 'F')) {
				key_data[lpp1] = (str[lpp0] - 0x37) << 4;
			}
			else if ((str[lpp0] >= '0') && (str[lpp0] <= '9')) {
				key_data[lpp1] = (str[lpp0] - 0x30) << 4;
			}
			
			
			lpp0 += 1;
			
			
			// low order digit
			if ((str[lpp0] >= 'A') && (str[lpp0] <= 'F')) {
				key_data[lpp1] |= str[lpp0] - 0x37;
			}
			else if ((str[lpp0] >= '0') && (str[lpp0] <= '9')) {
				key_data[lpp1] |= str[lpp0] - 0x30;
			}
			
			
			lpp1 += 1;
			lpp0 += 2; // skip space
		}
		
		RESULT.length = lpp1;
	}
	else {
		// couldn't find a way to load key
		RESULT.length = 0;
		
		free(RESULT.data);
		
		return RESULT;
	}


	return RESULT;
}



/** ----------------------------------------------------------------------------------------

	XORenc_encrypt_xor:

		Perform encryption/decryption of input data.

		The key given must be of the same length as the input data.

		The input data is simply XOR'ed with the input key.

	Parameters:

		data     -> Pointer to data to be encrypted/decrypted.

		data_len -> Length of input data (in bytes).

		key      -> Pointer to key to be used for encryption.

		key_len  -> Length of input key (in bytes).

	---------------------------------------------------------------------------------------- */
void XORenc_encrypt_xor(uint8_t* data, const size_t data_len, const uint8_t* key, const size_t key_len) {

	size_t written_bytes;
	size_t remaining_bytes;
	size_t lpp0;
	
	if (data_len != key_len) {
		return;
	}
	
	written_bytes = 0;
	
	// xor data in qword blocks (faster than byte by byte)
	if (data_len >= 8) {
		uint64_t* data_q = ((uint64_t*) data);
		uint64_t* key_q  = ((uint64_t*) key);
		
		for (lpp0=0; lpp0 < (data_len / 8); lpp0++) {
			data_q[lpp0] ^= key_q[lpp0];
			
			written_bytes += 8;
		}
	}


	remaining_bytes = data_len - written_bytes;


	// write remaining bytes if total length is not multiple of 8
	if (remaining_bytes > 0) {
		for (lpp0=0; lpp0 < remaining_bytes; lpp0++) {
			data[lpp0+written_bytes] ^= key[lpp0+written_bytes];
		}
	}
}



/** ----------------------------------------------------------------------------------------

	XORenc_encrypt_derived_next:

		Encrypts second or higher block of data (using derived key from 'key').

	Parameters:

		last_md5 -> Pair of md5sum (normal:inverted) from previous block to be used as salt for generation of this block.

		data     -> Pointer to data to be encrypted.

		data_len -> Length of input 'data', in bytes (maximum=XORENC_FILE_BLOCK_SIZE).
				
		key      -> The key used to generate derived data (as string).
				
		key_len  -> The length of input 'key'.
        
		md5sum[] -> Where to store 'md5sum' normal and inverted of processed (XOR'ed with Argon2<->Scrypt) derived data (at least 33 bytes in length).

	Return value:

		Returns 0 or positive value on success.

	---------------------------------------------------------------------------------------- */
int XORenc_encrypt_derived_next(char* last_md5[], uint8_t* data, const size_t data_len, const char* key, const size_t key_len, char* md5sum[]) {

	size_t lpp0;
	
	if (data_len > XORENC_FILE_BLOCK_SIZE) {
		return -1;
	}
	
	// generate derived data (Argon2, Scrypt), from password+salt+md5sum_(1, 2)
	char* final_salt  = calloc(1, 256);
	char* final_salt2 = calloc(1, 256);
  			
	if ((strlen(XORENC_SALT) + strlen(last_md5[0])) < 256) {
		strcat(final_salt, XORENC_SALT);
		strcat(final_salt, last_md5[0]);
  				
		strcat(final_salt2, XORENC_SALT);
		strcat(final_salt2, last_md5[1]);
	}
	else {
		fprintf(stderr, "Warning: Buffer overflow! Consider decreasing XORENC_SALT length or increase buffer. (0xe041bd206b89ad10)");
	}

	TXORencHash dkey_1 = XORenc_hash_argon2(key, key_len, final_salt, strlen(final_salt));
	TXORencHash dkey_2 = XORenc_hash_scrypt(key, key_len, final_salt2, strlen(final_salt2));

	free(final_salt);
	free(final_salt2);
  
	if ((dkey_1.data == NULL) || (dkey_2.data == NULL)) {
		if (dkey_1.data != NULL) {
			free(dkey_1.data);
		}

		if (dkey_2.data != NULL) {
			free(dkey_2.data);
		}

		return -1;
	}

	// XOR derived data from Argon2 with Scrypt's
	XORenc_encrypt_xor(dkey_1.data, dkey_1.length, dkey_2.data, dkey_2.length);

	free(dkey_2.data);
  
  
	// #4 - Encrypt input data :)
	XORenc_encrypt_xor(data, data_len, dkey_1.data, data_len);
  
  
	// generate md5sum(s) of generated and processed derived data for this block
	if ((md5sum != NULL) && ((md5sum[0] != NULL) && (md5sum[1] != NULL))) {
		// generate 'md5sum' for processed (XOR'ed Argon2<->Scrypt) derived data
		uint8_t* md5_1b = calloc(1, 16);
		char*    md5_1s = calloc(1, (16*2)+1);
		char*    md5_2s = calloc(1, (16*2)+1);
		
		XORenc_md5(dkey_1.data, dkey_1.length, md5_1b, md5_1s);
		
		for (lpp0=0; lpp0 < 16; lpp0++) {
			// invert the bits
			md5_1b[lpp0] = ~md5_1b[lpp0];
		}
		
		// inverted md5sum to string
		for (lpp0=0; lpp0 < 16; lpp0++) {
			char* tmp = XORenc_int2hex(md5_1b[lpp0], 2, true);

			md5_2s[(lpp0*2)+0] = tmp[0];
			md5_2s[(lpp0*2)+1] = tmp[1];

			free(tmp);
		}
  	
		// copy final md5sum(s) to their respective destination
		memcpy(md5sum[0], md5_1s, strlen(md5_1s)+1); // copy everything, including null terminator
		memcpy(md5sum[1], md5_2s, strlen(md5_2s)+1); // copy everything, including null terminator

		// free used resources		
		free(md5_2s);
		free(md5_1s);
		free(md5_1b);
	}
  
  
	free(dkey_1.data);
  
	return 0;
}



/** ----------------------------------------------------------------------------------------

	XORenc_encrypt_derived_first:

		Encrypts first block of data (using derived key from 'key').

	Parameters:

		data     -> Pointer to data to be encrypted.

		data_len -> Length of input 'data', in bytes (maximum=XORENC_FILE_BLOCK_SIZE).

		key      -> Pointer to key string (used to generate derived data).

		key_len  -> Length of input 'key'.
        
		md5sum[] -> Where to store 'md5sum' normal and inverted of processed (XOR'ed with Argon2<->Scrypt) derived data (at least 33 bytes in length).

	Return value:

		Returns 0 or positive value on success.

	---------------------------------------------------------------------------------------- */
int XORenc_encrypt_derived_first(uint8_t* data, const size_t data_len, const char* key, const size_t key_len, char* md5sum[]) {

	size_t lpp0;
	
	if (data_len > XORENC_FILE_BLOCK_SIZE) {
		return -1;
	}

	// #1 :)
	uint8_t* key_md5_1b = calloc(1, 16);       // md5 digest as raw bytes
	char*    key_md5_1s = calloc(1, (16*2)+1); // md5 digest as string

	uint8_t* key_md5_2b = calloc(1, 16);       // md5 digest -> 2 <- as raw bytes
	char*    key_md5_2s = calloc(1, (16*2)+1); // md5 digest -> 2 <- as string

	XORenc_md5((uint8_t*)key, key_len, key_md5_1b, key_md5_1s);

	// generate inverted md5sum of original
	memcpy(key_md5_2b, key_md5_1b, 16);

	for (lpp0=0; lpp0 < 16; lpp0++) {
		// invert the bits
		key_md5_2b[lpp0] = ~key_md5_2b[lpp0];
	}

	// inverted md5sum to string
	for (lpp0=0; lpp0 < 16; lpp0++) {
		char* tmp = XORenc_int2hex(key_md5_2b[lpp0], 2, true);

		key_md5_2s[(lpp0*2)+0] = tmp[0];
		key_md5_2s[(lpp0*2)+1] = tmp[1];

		free(tmp);
	}

	// generate derived data (Argon2), from password+salt+md5sum_1s
	char* final_salt  = calloc(1, 256);
	char* final_salt2 = calloc(1, 256);
  			
	if ((strlen(XORENC_SALT) + strlen(key_md5_1s)) < 256) {
		strcat(final_salt, XORENC_SALT);
		strcat(final_salt, key_md5_1s);
  				
		strcat(final_salt2, XORENC_SALT);
		strcat(final_salt2, key_md5_2s);
	}
	else {
		fprintf(stderr, "Warning: Buffer overflow! Consider decreasing XORENC_SALT length or increase buffer. (0xa594f280c9e2ad60)");
	}

	TXORencHash dkey_1 = XORenc_hash_argon2(key, key_len, final_salt, strlen(final_salt));
	TXORencHash dkey_2 = XORenc_hash_scrypt(key, key_len, final_salt2, strlen(final_salt2));

	free(final_salt);
	free(final_salt2);

	free(key_md5_1b); free(key_md5_1s);
	free(key_md5_2b); free(key_md5_2s);

	if ((dkey_1.data == NULL) || (dkey_2.data == NULL)) {
		if (dkey_1.data != NULL) {
			free(dkey_1.data);
		}

		if (dkey_2.data != NULL) {
			free(dkey_2.data);
		}

		return -1;
	}

	// XOR derived data from Argon2 with Scrypt's
	XORenc_encrypt_xor(dkey_1.data, dkey_1.length, dkey_2.data, dkey_2.length);

	free(dkey_2.data);

	// #2 - Encrypt input data :)
	XORenc_encrypt_xor(data, data_len, dkey_1.data, data_len);

	// check if we were told to generate 'md5sum' of XOR'ed key data
	if ((md5sum != NULL) && ((md5sum[0] != NULL) && (md5sum[1] != NULL))) {
		// generate 'md5sum' for processed (XOR'ed Argon2<->Scrypt) derived data
		uint8_t* md5_1b = calloc(1, 16);
		char*    md5_1s = calloc(1, (16*2)+1);
		char*    md5_2s = calloc(1, (16*2)+1);
		
		XORenc_md5(dkey_1.data, dkey_1.length, md5_1b, md5_1s);
		
		for (lpp0=0; lpp0 < 16; lpp0++) {
			// invert the bits
			md5_1b[lpp0] = ~md5_1b[lpp0];
		}

		// inverted md5sum to string
		for (lpp0=0; lpp0 < 16; lpp0++) {
			char* tmp = XORenc_int2hex(md5_1b[lpp0], 2, true);

			md5_2s[(lpp0*2)+0] = tmp[0];
			md5_2s[(lpp0*2)+1] = tmp[1];

			free(tmp);
		}

		// copy final md5sum(s) to their respective destination
		memcpy(md5sum[0], md5_1s, strlen(md5_1s)+1); // copy everything, including null terminator
		memcpy(md5sum[1], md5_2s, strlen(md5_2s)+1); // copy everything, including null terminator

		// free used resources		
		free(md5_2s);
		free(md5_1s);
		free(md5_1b);
	}
	
	free(dkey_1.data);
  
	return 0;
}
