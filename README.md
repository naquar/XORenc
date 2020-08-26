# XORenc
XORenc is a "XOR-based" data encryption tool.


**Usage:**

`xorenc <option(s)> <input_file>`


**Example(s):**

`xorenc --key 'BB 2A 33 C5 79 D4 3A' /tmp/input.file`


**Output file to 'stdout':**

`xorenc --stdout --key 'BB 2A 33 C5 79 D4 3A' /tmp/input.file`


**There are 3 ways to specify the key to be used:**

1. As a byte sequence:

	`--key 'AA BB CC DD'`

2. From file:

	`--key /path/to/key.file`

3. Common password (actual key is derived from it):

	`--key d2YqJUiaCawZzkq`


**For maximum security, make sure you follow the instructions below:**

1. Never use the same key/password to encrypt different files.

2. Always use random/unpredictable keys/passwords to encrypt files.

3. When using direct encryption mode, make sure the key is (at least) as long as the data being encrypted.


## **Possible questions:**

**Q.** *How do I decrypt a file previously encrypted?*

**A.** *Just run the program with the same commands (key) by giving the encrypted file as input. This should generate the decrypted/original file.*


**Known issues:**

1. Encryption is absurdly slow when using normal key.

	*This is normal because the key derivation function is purposefully slow.*


## Compilation instructions:

**Required software:**

* A C compiler (e.g. GCC).

* GNU Make.

* GNU Binutils.


**Required external libraries:**

* argon2

* scrypt

* avutil


**When all required software are installed go to the program's source directory and run:**

```
	make release
#	make install
```

*Note(s): Lines starting with _#_ means to run as root (administrator) is required.* :+1:

**Now you should be able to run the program `xorenc` from the command line.**


## General notes:

**This currently is beta (untested) software and may contain (very) dangerous bugs.**

If you have found bugs or have any suggestions, please send an e-mail to **bugs368@pm.me**.

Your message is appreciated! :)


## Release notes:

* **2020-05-03: v1.0.0-beta.2:**

	Fixed bug when specifying no key would lead program to use input data as key (resulting in zeroed output data).

* **2019-02-09: v1.0.0-beta.1:**

	First beta release!
