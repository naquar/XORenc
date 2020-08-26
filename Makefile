BINARY_NAME=xorenc
SOURCE_NAME=main.c
COMPILER_NAME=gcc
TEMP_FOLDER=tmp
TASK_SEPARATOR=---
COMPILE_DATE=$(shell date --utc +'%Y-%m-%d - %H:%M:%S %Z')
YEAR=2019
AUTHOR_NAME=Renan Souza da Motta
AUTHOR_EMAIL=renansouzadamotta@yahoo.com
BUG_REPORT_EMAIL=bugs368@pm.me
PROGRAM_NAME=XORenc
PROGRAM_VERSION=1.0.0-beta.2
PROGRAM_DESCR=A XOR-based data encryption tool.

SOURCE_FILES=COPYING LICENSE.txt README.md README.txt Makefile vars.sh xorenc.c xorenc_implementation.c $(SOURCE_NAME)

define LICENSE_INFO
The MIT License (MIT)\n\nCopyright (c) $(YEAR) $(AUTHOR_NAME) <$(AUTHOR_EMAIL)>\n\nPermission is hereby granted, free of charge, to any person obtaining a copy of\nthis software and associated documentation files (the "Software"), to deal in\nthe Software without restriction, including without limitation the rights to\nuse, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of\nthe Software, and to permit persons to whom the Software is furnished to do so,\nsubject to the following conditions:\n\nThe above copyright notice and this permission notice shall be included in all\ncopies or substantial portions of the Software.\n\nTHE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\nIMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS\nFOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR\nCOPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER\nIN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN\nCONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
endef

export YEAR
export BUG_REPORT_EMAIL
export AUTHOR_EMAIL
export AUTHOR_NAME
export COMPILE_DATE
export PROGRAM_NAME
export PROGRAM_VERSION
export PROGRAM_DESCR
export LICENSE_INFO

LINKER_FLAGS=-lm -largon2 -lscrypt -lavutil
COMPILER_FLAGS_RELEASE_1=-std=c99 -Wall -Wno-unused-variable -O3 $(LINKER_FLAGS)
COMPILER_FLAGS_DEBUG_1=-std=c99 -Wall -D DEBUG -Wno-unused-variable -O0 -g $(LINKER_FLAGS)

debug: # compile debug or under development version
	@./vars.sh vars
	@rm -r -f ./$(TEMP_FOLDER) && mkdir -p ./$(TEMP_FOLDER)
	@$(COMPILER_NAME) $(COMPILER_FLAGS_DEBUG_1) -o ./$(BINARY_NAME) $(SOURCE_NAME)
	@echo $(TASK_SEPARATOR)
	@echo -- Compiled DEBUG version \(compiler=$(COMPILER_NAME)\) --
	@echo -e 'You can now run it in the following way:\n\n\tARGS="param1 param2..." make run'

release: # compile release version
	@./vars.sh vars
	@rm -r -f ./$(TEMP_FOLDER) && mkdir -p ./$(TEMP_FOLDER)
	@$(COMPILER_NAME) $(COMPILER_FLAGS_RELEASE_1) -o ./$(BINARY_NAME) $(SOURCE_NAME) && strip -g --strip-unneeded ./$(BINARY_NAME)
	@echo $(TASK_SEPARATOR)
	@echo -- Compiled RELEASE version \(compiler=$(COMPILER_NAME)\) --
	@echo -e 'You can now run it in the following way:\n\n\tARGS="param1 param2..." make run'

run: # execute compiled program with given optional commands
	@./$(BINARY_NAME) $(ARGS)

license: # generate program's license information
	@echo $$LICENSE_INFO | tee > COPYING LICENSE.txt

source: # archive source file for distribution with binaries
	@mkdir -p $(PROGRAM_NAME)/src
	@cp $(SOURCE_FILES) $(PROGRAM_NAME)/src
	@tar -c $(PROGRAM_NAME) --to-stdout | bzip2 --compress --best > $(PROGRAM_NAME)-v$(PROGRAM_VERSION).tar.bz2
	@echo -e 'Done! :)\n\nSource code archived and saved as:\n\t$(PROGRAM_NAME)-v$(PROGRAM_VERSION).tar.bz2'

LOCAL_DIR=/usr/local
BIN_DIR=$(LOCAL_DIR)/bin
SRC_DIR=$(LOCAL_DIR)/src/$(BINARY_NAME)

install: # install program to system
	@cp $(BINARY_NAME) $(BIN_DIR)/$(BINARY_NAME)
	@mkdir -p $(SRC_DIR)
	@cp $(SOURCE_FILES) $(SRC_DIR)
	@echo -e 'Done! :)\n\nYou can now run the installed software by running:\n\t$(BINARY_NAME)'

clean: # remove unnecessary files from folder
	# clean 'release'
	@rm -r -f ./$(TEMP_FOLDER) ./$(BINARY_NAME)
