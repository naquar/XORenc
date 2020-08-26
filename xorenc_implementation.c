// Warning: Best read if using a monospaced/fixed-width font and tab width of 4.
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

void XORenc_show_tips() {

	fprintf(stderr, "\nFor maximum security, make sure you follow the instructions below:\n\n");
	
	fprintf(stderr, "\t1.Never use the same key/password to encrypt different files.\n");
	fprintf(stderr, "\t2.Always use random/unpredictable keys/passwords to encrypt files.\n");
	fprintf(stderr, "\t3.When using direct encryption mode, make sure the key is (at least) as long as the data being encrypted.\n");
}



/** ----------------------------------------------------------------------------------------

	XORenc_write_to_file:

		Write buffer to file named 'filename'.

	Parameters:

		filename  -> Write file with this name. If filename is '-' then write to standard output (stdout).
        
		extension -> Extension of filename to be written (e.g. bmp, jpg, txt...).

		buf       -> Pointer to data that will be written to file.
        
		buf_len   -> Length of input data (in bytes) to be written to file.
        
		overwrite -> Overwrite file if it exists?!
        
		std_out   -> Write file to standard output?
        
	Return value:
    
		Returns positive value or 0 if successful.

	---------------------------------------------------------------------------------------- */
int XORenc_write_to_file(const char* filename, const char* extension, const uint8_t* buf, const size_t buf_len, const bool overwrite, const bool std_out) {

	FILE* fd1 = NULL;

	if (std_out == true) {
		// write to stdout
		fwrite(buf, 1, buf_len, stdout);
	}
	else {
		// write to filename
		char* out_filename = calloc(1, 4096);
		
		strcat(out_filename, filename);
		
		if (extension != NULL) {
			strcat(out_filename, extension);
		}
		
		if ((access(out_filename, R_OK) == 0) && (overwrite == false)) {
			// file exists not overwriting it...
			free(out_filename);
			
			return -1;
		}
		
		// write data to file
		fd1 = fopen(out_filename, "a");
		
		// seek to the end of file
		fseek(fd1, 0, SEEK_END);
		
		if (fd1 != NULL) {
			size_t bytes_written = fwrite(buf, 1, buf_len, fd1);
			
			int close = fclose(fd1);
			
			if ((bytes_written != buf_len) || (close != 0)) {
				free(out_filename);
				
				return -1;
			}
		}
		else {
			// failure while opening/creating file
			free(out_filename);
			
			return -1;
		}
	}
	
	return 0;
}



/** ----------------------------------------------------------------------------------------

	XORenc_encrypt:

		Perform encryption/decryption of input data from file.

	Parameters:

		filename     -> Path to file to be encrypted. Must be NULL if input is to be read from standard input (stdin).

		key_filename -> Path to key file to be used for encryption, must not be NULL if encryption mode is direct and key is file.
        
		key_str      -> Input key as string, must be NULL if 'key_filename' is specified.
        
		params       -> The parameters to be applied.
        
		std_out      -> Output file to standard output (stdout)?
        
	Return value:
    
		Returns positive value or 0 if successful.

	---------------------------------------------------------------------------------------- */
int XORenc_encrypt(const char* filename, const char* key_filename, const char* key_str, const TXORencParams params, const bool std_out) {

	FILE* fd0; // regular input file

	uint8_t*       buf;         // file data buffer
	size_t         buf_len;     // length of 'buf'
	TXORencKey     key;         // key used to encrypt
	const uint8_t* key_b;       // pointer to array of byte
	size_t         file_size;   // size of input file (in bytes)
	bool           EoF = false; // End of File reached?
	char*          md5sum[2];   // Used for derived encryption in blocks
	size_t         block = 0;   // number of block being encrypted
	
	// loop vars
	size_t lpp0;
	
	/* ******* --- XORenc_encrypt --- ******* */
	
	if (filename != NULL) {
		if (access(filename, R_OK) != 0) {
			// specified input file does not exist or could not be accessed
			return -500;
		}
	}
	
	if ((access(key_filename, R_OK) != 0) && (key_str == NULL) && (! XORenc_key_is_byte_sequence(key_filename))) {
		// specified key file does not exist or could not be accessed
		return -450;
	}
	
	// open file
	if (filename != NULL) {
		fd0 = fopen(filename, "rb");
	
		if (fd0 == NULL) {
			// could not open file
			return -400;
		}
	}
	// *** FREE: fd0


	// allocate memory for file data buffer
	buf = malloc(XORENC_FILE_BLOCK_SIZE);

	if ((buf == NULL) && (filename != NULL)) {
		fclose(fd0);

		return -300;
	}
	// *** FREE: fd0, buf


	switch(params.key_type) {
		case Direct:
			// read buffer; from regular file or standard input (stdin)?
			if (filename != NULL) {
				// read from regular file
				buf_len = fread(buf, 1, XORENC_FILE_BLOCK_SIZE, fd0);
			}
			else {
				// read from standard input (stdin)
				buf_len = fread(buf, 1, XORENC_FILE_BLOCK_SIZE, stdin);
			}
			
			// process whole file at once or in blocks?
			if (buf_len < XORENC_FILE_BLOCK_SIZE) {
				// whole file can be processed at once (TEST OK!)
				key   = XORenc_key_load(key_filename, 0);
				key_b = key.data;
				
				XORenc_encrypt_xor(buf, buf_len, key_b, buf_len);
				
				// write encrypted buffer to file
				if (XORenc_write_to_file(filename, ".xen", buf, buf_len, false, std_out) < 0) {
					// failed writing to file
					free(key.data);
					
					if (filename != NULL) {
						fclose(fd0);
					}
					
					free(buf);
					
					return -250;
				}
				
				// free used resources
				free(key.data);
			}
			else while (EoF == false) {
				// process file in blocks of 'XORENC_FILE_BLOCK_SIZE'
				if ((block == 0) && (buf_len > 0)) {
					// encrypt first block
					key   = XORenc_key_load(key_filename, block);
					key_b = key.data;
					
					XORenc_encrypt_xor(buf, buf_len, key_b, buf_len);
					
					// write encrypted buffer to file
					if (XORenc_write_to_file(filename, ".xen", buf, buf_len, false, std_out) < 0) {
						// failed writing to file
						free(key.data);
						
						if (filename != NULL) {
							fclose(fd0);
						}
						
						free(buf);
						
						return -200;
					}
					
					// go to next block
					block += 1;
					
					// free used resources
					free(key.data);
				}
				else if ((block > 0) && (buf_len > 0)) {
					// encrypt second block onwards...
					key   = XORenc_key_load(key_filename, block);
					key_b = key.data;
					
					XORenc_encrypt_xor(buf, buf_len, key_b, buf_len);
					
					// write encrypted buffer to file
					XORenc_write_to_file(filename, ".xen", buf, buf_len, true, std_out);
					
					// go to next block
					block += 1;
					
					// free used resources
					free(key.data);
				}
				
				
				if (filename != NULL) {
					// read from regular file
					buf_len = fread(buf, 1, XORENC_FILE_BLOCK_SIZE, fd0);
				}
				else {
					// read from standard input (stdin)
					buf_len = fread(buf, 1, XORENC_FILE_BLOCK_SIZE, stdin);
				}
				
				
				if (buf_len == 0) {
					EoF = true;
				}
			}
		break;


		case Derived:
			if ((key_str == NULL) || (strlen(key_str) == 0)) {
				// invalid key was given
				if (filename != NULL) {
					fclose(fd0);
				}
				
				free(buf);
				
				return -150;
			}
			
			
			// read buffer; from regular file or standard input (stdin)?
			if (filename != NULL) {
				// read from regular file
				buf_len = fread(buf, 1, XORENC_FILE_BLOCK_SIZE, fd0);
			}
			else {
				// read from standard input (stdin)
				buf_len = fread(buf, 1, XORENC_FILE_BLOCK_SIZE, stdin);
			}


			if (buf_len < XORENC_FILE_BLOCK_SIZE) {
				// process whole file at once...
				// encrypt first block
				XORenc_encrypt_derived_first(buf, buf_len, key_str, strlen(key_str), NULL);
				
				// write encrypted data to file...
				if (XORenc_write_to_file(filename, ".xen", buf, buf_len, false, std_out) < 0) {
					// failed writing to file
					if (filename != NULL) {
						fclose(fd0);
					}
  			  
					free(buf);
  			  
					return -100;
				}
			}
			else while (EoF == false) {
				// process in blocks until end of file...
				if ((block == 0) && (buf_len > 0)) {
					// first block
					md5sum[0] = calloc(1, (16*2)+1);
					md5sum[1] = calloc(1, (16*2)+1);

					// encrypt first block
					XORenc_encrypt_derived_first(buf, buf_len, key_str, strlen(key_str), md5sum);
					
					// write encrypted block to file
					if (XORenc_write_to_file(filename, ".xen", buf, buf_len, false, std_out) < 0) {
						free(md5sum[0]);
						free(md5sum[1]);

						if (filename != NULL) {
							fclose(fd0);
						}
						
						free(buf);
						
						return -50;
					}
					
					block += 1;
				}
				else if ((block > 0) && (buf_len > 0)) {
					// second block onwards; encrypt this block...
					XORenc_encrypt_derived_next(md5sum, buf, buf_len, key_str, strlen(key_str), md5sum);

					// write to file
					XORenc_write_to_file(filename, ".xen", buf, buf_len, true, std_out);
					
					block += 1;
				}
				
				
				if (EoF == true) {
					free(md5sum[0]);
					free(md5sum[1]);
				}
				
				
				// read next block (if any)
				if (filename != NULL) {
					// read from regular file
					buf_len = fread(buf, 1, XORENC_FILE_BLOCK_SIZE, fd0);
				}
				else {
					// read from standard input (stdin)
					buf_len = fread(buf, 1, XORENC_FILE_BLOCK_SIZE, stdin);
				}
				
				
				if (buf_len == 0) {
					EoF = true;
				}
			}
		break;


		default:
			// free resources used
			if (filename != NULL) {
				fclose(fd0);
			}
			
			free(buf);
		break;
	}


	if (filename != NULL) {
		fclose(fd0);
	}
	
	free(buf);
	
	return 0;
}



/** ----------------------------------------------------------------------------------------

	XORenc_process_file:

		Process input file according to given parameters.

	Parameters:

		filename -> Path to file to be encrypted. Must be NULL if input is to be read from standard input (stdin).

		key      -> Corresponds to key in one of the three available formats: path to file, byte sequence, or common string.
        
		std_out  -> Output file to standard output (stdout)?
        
		params   -> The parameters to be considered.
        
	Return value:

		Returns positive value or 0 if successful.

	---------------------------------------------------------------------------------------- */
int XORenc_process_file(const char* filename, const char* key, const bool std_out, const TXORencParams params) {

	int r = -1;


	if ((access(key, R_OK) == 0) && (params.key_type == Direct)) {
		// file exists, key is path to key file
		r = XORenc_encrypt(filename, key, NULL, params, std_out);
	}
	else if ((XORenc_key_is_byte_sequence(key) == true) && (params.key_type == Direct)) {
		// key refers to a sequence of bytes as string, in the form: 'XX XX XX...'
		r = XORenc_encrypt(filename, key, NULL, params, std_out);
	}
	else if (params.key_type == Derived) {
		// key is a common password
		r = XORenc_encrypt(filename, NULL, key, params, std_out);
	}


	if ((r >= 0) && (filename != NULL)) {
		// input was from regular file
		fprintf(stderr, "\nFile: \"%s\" en/de-crypted successfully! :)\n", filename);
		
		XORenc_show_tips();
	}
	else if ((r >= 0) && (filename == NULL)) {
		// input was from standard input (stdin)
		fprintf(stderr, "\nFile en/de-crypted successfully! :)\n");
		
		XORenc_show_tips();
	}
	else if (r < 0) {
		fprintf(stderr, "\nError (%d) occurred while processing file. :(\n", r);
	}
	
	
	return r;
}
