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

/** ----------------------------------------------------------------

	These types are related to the program's own options.

	---------------------------------------------------------------- */
typedef struct {
	char* Long;   // long option name
	char* Short;  // short option name
	char* Params; // option's parameters
	char* Descr;  // option's description
	long  Pos;    // current option's position in command line (from 1 to parameters count)
	_Bool Given;  // was it given by user
} TCmdLineOptions;

typedef struct {
	TCmdLineOptions Options;
} TCmdLine;

/** ----------------------------------------------------------------

These types are related to the user's own input options.

	---------------------------------------------------------------- */
typedef struct {
	char** Options; // pointer to pointers to 'char(s)'
} TUserCmdLine;

/** ----------------------------------------------------------------------------------------

	m_FatalError:

		Generate a fatal error message and terminates the program.

	Parameters:

		msg -> The message to be displayed.

	---------------------------------------------------------------------------------------- */
void m_FatalError(const char* msg) {

	fprintf(stderr, "\n%s\n", msg);

	exit(-5);
}



/** ----------------------------------------------------------------------------------------

	m_ShowHeader:

		Show program's 'welcome' message. :)

	---------------------------------------------------------------------------------------- */
void m_ShowHeader() {

	fprintf(stderr, "%s\n", COMPILE_DATE);
	fprintf(stderr, "%s v%s - %s\n", PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DESCR);
}



/** ----------------------------------------------------------------------------------------

	m_ShowHelp:

		Show help message.

	---------------------------------------------------------------------------------------- */
void m_ShowHelp(const int exit_code) {

	fprintf(stderr, "\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "\txorenc <option(s)> <input_file>\n\n");

	fprintf(stderr, "Example(s):\n");
	fprintf(stderr, "\txorenc --key 'BB 2A 33 C5 79 D4 3A' /tmp/input.file\n\n");
  
	fprintf(stderr, "Output file to 'stdout':\n");
	fprintf(stderr, "\txorenc --stdout --key 'BB 2A 33 C5 79 D4 3A' /tmp/input.file\n\n");
  
	fprintf(stderr, "Input file from 'stdin' (outputs automatically to 'stdout'):\n");
	fprintf(stderr, "\txorenc --stdin --key 'BB 2A 33 C5 79 D4 3A'\n\n");
  

	fprintf(stderr, "For more information and instructions start with '--help' or '-h' :)\n");

	exit(exit_code);
}



/** ----------------------------------------------------------------------------------------

	m_ShowVersionInfo:

		Show version information. (Option #2: --version, -v)

	---------------------------------------------------------------------------------------- */
void m_ShowVersionInfo() {

	fprintf(stderr, "\n");
	fprintf(stderr, "Version information:\n");
	fprintf(stderr, "--------------------\n");
	fprintf(stderr, "This is an untested (BETA) release and may contain (very) dangerous bugs.\n");
	fprintf(stderr, "Please, use with caution and report bugs to: \"%s\" :)\n", BUG_REPORT_EMAIL);
	fprintf(stderr, "\n");
	fprintf(stderr, "For license information start with: '--license' or '-l'\n");
}



/** ----------------------------------------------------------------------------------------

	m_ShowLicenseInfo:

		Show license information. (Option #3: --license, -l)

	---------------------------------------------------------------------------------------- */
void m_ShowLicenseInfo() {

	fprintf(stderr, "\n");
	fprintf(stderr, "License information:\n");
	fprintf(stderr, "--------------------\n");
	fprintf(stderr, "The MIT License (MIT)\n\
\n\
Copyright (c) %s %s <%s>\n\
\n\
Permission is hereby granted, free of charge, to any person obtaining a copy of\n\
this software and associated documentation files (the \"Software\"), to deal in\n\
the Software without restriction, including without limitation the rights to\n\
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of\n\
the Software, and to permit persons to whom the Software is furnished to do so,\n\
subject to the following conditions:\n\
\n\
The above copyright notice and this permission notice shall be included in all\n\
copies or substantial portions of the Software.\n\
\n\
THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n\
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS\n\
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR\n\
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER\n\
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN\n\
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n", YEAR, AUTHOR_NAME, AUTHOR_EMAIL);
}



/** ----------------------------------------------------------------------------------------

	m_ShowOptions_Bigger:

		Returns the length of the bigger/longer combination of...
		"CmdLine.Options.Long/Short + CmdLine.Options.Params" string

	Parameters:

		cmd_line[] -> Refers to an array of 'CmdLine' structure.

		len        -> The number of items in the array 'cmd_line[]'.

		SHORT      -> Check for "short options + params" instead of "long + params"?

---------------------------------------------------------------------------------------- */
unsigned int m_ShowOptions_Bigger(const TCmdLine cmd_line[], const int len, const _Bool SHORT) {

	unsigned int RESULT = 0;

	int pl0 = 0;


	while (pl0 < len) {
		if (! SHORT) {
			// get length of "long options + params"
			if (strlen(cmd_line[pl0].Options.Long) + strlen(cmd_line[pl0].Options.Params) > RESULT) {
				RESULT = strlen(cmd_line[pl0].Options.Long) + strlen(cmd_line[pl0].Options.Params);
			}
		}
		else {
			// get length of "short options + params"
			if (strlen(cmd_line[pl0].Options.Short) > RESULT) {
				RESULT = strlen(cmd_line[pl0].Options.Short);
			}
		}

		pl0++;
	}


	return RESULT;
}



/** ----------------------------------------------------------------------------------------

	m_ShowOptions_Spaces:

		Returns a string with repeated 'count' of space char.

	Parameters:

		count -> The number of spaces to be returned as null terminated string.

	---------------------------------------------------------------------------------------- */
char* m_ShowOptions_Spaces(const unsigned int count) {

	char* RESULT = calloc(1, count);

	if (RESULT == NULL) {
		m_FatalError("Could not allocate memory. (0x92f0b63c88f29810)");
	}


	if (count <= 0) {
		return RESULT;
	}
	else {
		unsigned int pl0 = 0;

		while (pl0 < count) {
			strcat(RESULT, " ");

			pl0++;
		}
	}


	return RESULT;
}



/** ----------------------------------------------------------------------------------------

	m_SetOptionsPos:

		Set options position in user given command line.

	---------------------------------------------------------------------------------------- */
void m_SetOptionsPos(const TUserCmdLine user_cmd_line, TCmdLine cmd_line[], const unsigned int user_cmd_line_len, const unsigned int cmd_line_len) {

	unsigned int pl0, pl1;

	if ((user_cmd_line_len > 0) && (cmd_line_len > 0)) {

		for (pl1=0; pl1 < user_cmd_line_len; pl1++) {

			for (pl0=0; pl0 < cmd_line_len; pl0++) {

				if ( strcmp(user_cmd_line.Options[pl1], cmd_line[pl0].Options.Long)  == 0 ||
					 strcmp(user_cmd_line.Options[pl1], cmd_line[pl0].Options.Short) == 0 ) {
					cmd_line[pl0].Options.Pos = pl1+1;
				}
			}
		}
	}
}



/** ----------------------------------------------------------------------------------------

	m_ShowOptions:

		Show all command line options available. (Option #1: --help, -h)

	Parameters:

		cmd_line -> Refers to an array of 'CmdLine' structure.

		len      -> The total number of items present in the array given.

---------------------------------------------------------------------------------------- */
void m_ShowOptions(const TCmdLine cmd_line[], const unsigned int len) {

	const char* PREFIX = "\t";

	unsigned int biggerL, biggerS, pl0;

	if (len > 0) {
		fprintf(stderr, "\n");
		fprintf(stderr, "All options:\n");
		fprintf(stderr, "------------\n");

		// write all options in array...
		pl0 = 0;

		biggerL = m_ShowOptions_Bigger(cmd_line, len, 0);
		biggerS = m_ShowOptions_Bigger(cmd_line, len, 1);

		while (pl0 < len) {
			fprintf(stderr, "%s%s%s,%s%s %s: %s\n",
					PREFIX,
					cmd_line[pl0].Options.Long,
					cmd_line[pl0].Options.Params,
					m_ShowOptions_Spaces(biggerL - ( strlen(cmd_line[pl0].Options.Long) + strlen(cmd_line[pl0].Options.Params) )),
					m_ShowOptions_Spaces(biggerS - strlen(cmd_line[pl0].Options.Short)),
					cmd_line[pl0].Options.Short,
					cmd_line[pl0].Options.Descr
			);

			pl0++;
		}
	}
}



/** ----------------------------------------------------------------------------------------

	m_CheckOptions:

		Checks all program's command line options and generates a fatal error...
		if conflicting options are found (i.e. it cannot have two options with the same name).

	Parameters:

		cmd_line -> Refers to an array of CmdLine structure.

		len      -> The length of the array 'cmd_line'.

---------------------------------------------------------------------------------------- */
void m_CheckOptions(const TCmdLine cmd_line[], const unsigned int len) {

	unsigned int count;

	unsigned int pl0, pl1;


	pl1 = 0;

	while (pl1 < len) {
		count = 0;

		// check current option against all options (including itself)
		pl0 = 0;

		while (pl0 < len) {
			if ( (strcmp(cmd_line[pl0].Options.Long,  cmd_line[pl1].Options.Long)  == 0) ||
				 (strcmp(cmd_line[pl0].Options.Short, cmd_line[pl1].Options.Short) == 0) ) {
				count++;
			}

			if ( (strcmp(cmd_line[pl0].Options.Long,  cmd_line[pl1].Options.Short) == 0) ||
				 (strcmp(cmd_line[pl0].Options.Short, cmd_line[pl1].Options.Long)  == 0) ) {
				count++;
			}

			if (count >= 2) {
				char* err_msg = calloc(1, 384);

				if (err_msg == NULL) {
					m_FatalError("Could not allocate memory. (0x8732fb9621b29f20)");
				}

				strcat(err_msg, "Duplicated option found: '");
				strcat(err_msg, cmd_line[pl0].Options.Long);
				strcat(err_msg, "' '");
				strcat(err_msg, cmd_line[pl0].Options.Short);
				strcat(err_msg, "'. (0x2f78ebb77ccf51a0)");

				m_FatalError(err_msg);
			}

			pl0++;
		}

		pl1++;
	}
}



/** ----------------------------------------------------------------------------------------

	m_GetUserOptions:

		Get user's given command line options.

	Parameters:

		param_count -> The total count of parameters given by the user.

		argv        -> Pointer to user's given options string list.

	---------------------------------------------------------------------------------------- */
TUserCmdLine* m_GetUserOptions(const unsigned int param_count, char* argv[]) {

	TUserCmdLine* output = NULL;

	if (param_count < 1) {
		return output;
	}
	else {
		output = malloc(sizeof(TUserCmdLine));

		if (output == NULL) {
			m_FatalError("Could not allocate memory. (0x8fe9a546e3689430)");
		}


		output->Options = malloc(sizeof(char*) * param_count);

		if (output->Options == NULL) {
			m_FatalError("Could not allocate memory. (0x626e29f515fa6c30)");
		}


		const unsigned int BUFFER_SIZE = 4096;

		unsigned int pl0 = 0;

		while (pl0 < param_count) {
			output->Options[pl0] = calloc(1, BUFFER_SIZE);

			if (output->Options[pl0] == NULL) {
				m_FatalError("Could not allocate memory. (0x3dd852c0c52e85e0)");
			}


			if (strlen(argv[pl0+1]) < BUFFER_SIZE) { // BUFFER_SIZE + 1 NULL char.
				strcpy(output->Options[pl0], argv[pl0+1]);
			}
			else {
				m_FatalError("String is too large. Consider increasing buffer size. (0xc57820f02e24e5d0)");
			}

			pl0++;
		}
	}

	return output;
}



#include "xorenc.c"
#include "xorenc_implementation.c"



/***************************************************/
/* ---------------- Main function ---------------- */
/***************************************************/
enum CmdOptions
	{ Help=0, Version, License, StandardInput, StandardOutput, Key };

#define MAIN_OPTION_COUNT 6

// main variables
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

