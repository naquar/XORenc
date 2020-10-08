// Warning: Best read if using a monospaced/fixed-width font and tab width of 4.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
//---
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//---
#include "vars.h" // compile time variables

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

#include "xorenc.c"
#include "xorenc_implementation.c"
#include "main_cmdline.c"

/***************************************************/
/* ---------------- Main function ---------------- */
/***************************************************/
// 'main' variables, constants and other data
enum CmdOptions
	{ Help=0, Version, License, StandardInput, StandardOutput, Key };

#define MAIN_OPTION_COUNT 6

char*          m_work_dir;
int            m_param_count;
TUserCmdLine*  m_user_cmd_line;
TCmdLine       m_cmd_line[MAIN_OPTION_COUNT] = {
                                                  {{ "--help",                   "-h",   "",        "Show help message.",                                              0, false }},
                                                  {{ "--version",                "-v",   "",        "Show version info.",                                              0, false }},
                                                  {{ "--license",                "-l",   "",        "Show license info.",                                              0, false }},
                                                  {{ "--stdin",                  "-in",  "",        "Input file from standard input (stdin).",                         0, false }},
                                                  {{ "--stdout",                 "-out", "",        "Output file to standard output (stdout).",                        0, false }},
                                                  {{ "--key",                    "-k",   " <text>", "Input key as bytes (39 4B 8A...), common password, or key file.", 0, false }}
                                               };
// xorenc vars
TXORencParams XORenc_params;

// loop exclusive variables
size_t lp0, lp1;

int main(int argc, char* argv[]) {
	
	m_ShowHeader();


	// get command line arguments count
	if (argc > 0) {
		m_param_count = argc-1;
	}
	else {
		m_FatalError("Not fit for environment. (0x72d9e9a082e501d0)");
	}


	// get current working directory
	m_work_dir = malloc(256);

	if (m_work_dir == NULL) {
		m_FatalError("Could not allocate memory. (0xd999a65603ef84c0)");
	}

	getcwd(m_work_dir, 256);

	// check if options are arranged correctly
	m_CheckOptions(m_cmd_line, MAIN_OPTION_COUNT);

	// get user given command line options
	m_user_cmd_line = m_GetUserOptions(m_param_count, argv);

	// set option's position in user given command line
	if (m_user_cmd_line != NULL) {
		m_SetOptionsPos(*m_user_cmd_line, m_cmd_line, m_param_count, MAIN_OPTION_COUNT);
	}

	// if no options given or something else went wrong show help
	if (m_user_cmd_line == NULL) {
		m_ShowHelp(0);
	}


	// check user given options and mark their existence if they are present
	lp1 = 0;

	while (lp1 < m_param_count) {
		lp0 = 0;

		while (lp0 < MAIN_OPTION_COUNT) {
			// check if current user given command exists in program
			if ( (strcmp(m_user_cmd_line->Options[lp1], m_cmd_line[lp0].Options.Long)  == 0) ||
				 (strcmp(m_user_cmd_line->Options[lp1], m_cmd_line[lp0].Options.Short) == 0) ) {
				m_cmd_line[lp0].Options.Given = 1;
			}

			lp0++;
		}

		lp1++;
	}


	// check for option #1
	if (m_cmd_line[Help].Options.Given) {
		m_ShowOptions(m_cmd_line, MAIN_OPTION_COUNT);

		return 0;
	}

	// check for option #2
	if (m_cmd_line[Version].Options.Given) {
		m_ShowVersionInfo();

		return 0;
	}

	// check for option #3
	if (m_cmd_line[License].Options.Given) {
		m_ShowLicenseInfo();

		return 0;
	}
  
	// check for option #4
	if (m_cmd_line[Key].Options.Given) {
		// read option parameter, it must exist
		if (m_cmd_line[Key].Options.Pos < m_param_count) {
			// check if next param is path to a key file
			if (access(argv[m_cmd_line[Key].Options.Pos+1], R_OK) == 0) {
				// key file exists, key corresponds to path to a key file; and a file to encrypt was given as well; use direct key mode
				XORenc_params.key_type = Direct;
  			
  			
				if ((m_cmd_line[StandardOutput].Options.Given == true) && (m_cmd_line[StandardInput].Options.Given == false)) {
					// from regular file, to standard output
					if (m_cmd_line[Key].Options.Pos+1 < m_param_count) {
						// file is not from standard input and file was specified
						XORenc_process_file(argv[m_param_count], argv[m_cmd_line[Key].Options.Pos+1], m_cmd_line[StandardOutput].Options.Given, XORenc_params);
					}
					else {
						// input file is missing from command line
						m_FatalError("Error: Input file not given or is not accessible.");
					}
				}
				else if (m_cmd_line[StandardInput].Options.Given == true) {
					// from standard input, to standard output
					XORenc_process_file(NULL, argv[m_cmd_line[Key].Options.Pos+1], true, XORenc_params);
				}
				else if ((m_cmd_line[StandardOutput].Options.Given == false) && (m_cmd_line[StandardInput].Options.Given == false)) {
					// from regular file, to regular file
					if (m_cmd_line[Key].Options.Pos+1 < m_param_count) {
						XORenc_process_file(argv[m_param_count], argv[m_cmd_line[Key].Options.Pos+1], m_cmd_line[StandardOutput].Options.Given, XORenc_params);
					}
					else {
						// input file is missing from command line
						m_FatalError("Error: Input file not given or is not accessible.");
					}
				}
				else {
					m_ShowHelp(-1);
				}
			}
			else if (XORenc_key_is_byte_sequence(argv[m_cmd_line[Key].Options.Pos+1]) == true) {
				// key corresponds to a byte sequence in the form: 'XX XX XX...'; use direct key mode
				XORenc_params.key_type = Direct;

				if ((m_cmd_line[StandardOutput].Options.Given == true) && (m_cmd_line[StandardInput].Options.Given == false)) {
					// from regular file, to standard output
					if (m_cmd_line[Key].Options.Pos+1 < m_param_count) {
						XORenc_process_file(argv[m_param_count], argv[m_cmd_line[Key].Options.Pos+1], m_cmd_line[StandardOutput].Options.Given, XORenc_params);
					}
					else {
						// input file is missing from command line
						m_FatalError("Error: Input file not given or is not accessible.");
					}
				}
				else if (m_cmd_line[StandardInput].Options.Given == true) {
					// from standard input, to standard output
					XORenc_process_file(NULL, argv[m_cmd_line[Key].Options.Pos+1], true, XORenc_params);
				}
				else if ((m_cmd_line[StandardOutput].Options.Given == false) && (m_cmd_line[StandardInput].Options.Given == false)) {
					// from regular file, to regular file
					if (m_cmd_line[Key].Options.Pos+1 < m_param_count) {
						XORenc_process_file(argv[m_param_count], argv[m_cmd_line[Key].Options.Pos+1], m_cmd_line[StandardOutput].Options.Given, XORenc_params);
					}
					else {
						// input file is missing from command line
						m_FatalError("Error: Input file not given or is not accessible.");
					}
				}
				else {
					m_ShowHelp(-1);
				}
			}
			else {
				// it corresponds to a common string as key; used derived key mode
				XORenc_params.key_type = Derived;
  			
  			
				if (strlen(argv[m_cmd_line[Key].Options.Pos+1]) < XORENC_MIN_PASSWORD_LENGTH) {
					// password length must be at least 8 characters
					m_FatalError("Error: Password is too short, minimum is 8.");
				}
  			
  			
				if ((m_cmd_line[StandardOutput].Options.Given == true) && (m_cmd_line[StandardInput].Options.Given == false)) {
					// from regular file, to standard output
					if (m_cmd_line[Key].Options.Pos+1 < m_param_count) {
						XORenc_process_file(argv[m_param_count], argv[m_cmd_line[Key].Options.Pos+1], m_cmd_line[StandardOutput].Options.Given, XORenc_params);
					}
					else {
						// input file is missing from command line
						m_FatalError("Error: Input file not given or is not accessible.");
					}
				}
				else if (m_cmd_line[StandardInput].Options.Given == true) {
					// from standard input, to standard output
					XORenc_process_file(NULL, argv[m_cmd_line[Key].Options.Pos+1], true, XORenc_params);
				}
				else if ((m_cmd_line[StandardOutput].Options.Given == false) && (m_cmd_line[StandardInput].Options.Given == false)) {
					// from regular file, to regular file
					if (m_cmd_line[Key].Options.Pos+1 < m_param_count) {
						XORenc_process_file(argv[m_param_count], argv[m_cmd_line[Key].Options.Pos+1], m_cmd_line[StandardOutput].Options.Given, XORenc_params);
					}
					else {
						// input file is missing from command line
						m_FatalError("Error: Input file not given or is not accessible.");
					}
				}
				else {
					m_ShowHelp(-1);
				}
			}
		}

		return 0;
	}

	return 0;
}

