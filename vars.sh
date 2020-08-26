#! /bin/bash
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
# --- ***** RUN THIS AT YOUR OWN RISK *****
# --- About: Generate compile time variables and write them to file.
# --- Usage: script.sh <output>
# --- Tools needed: cat

# if insufficient number of commands were given show help message
if [ $# -lt 1 ]; then
  echo 'Usage: script.sh <output>'
  echo '  Example: script.sh /tmp/file.inc'
  exit 2
fi

cat << MYqxgW93 > ./"$1".h
// These are compile time variables - This file is auto generated, do not edit it!
const char YEAR[]             = "$YEAR";
const char BUG_REPORT_EMAIL[] = "$BUG_REPORT_EMAIL";
const char AUTHOR_EMAIL[]     = "$AUTHOR_EMAIL";
const char AUTHOR_NAME[]      = "$AUTHOR_NAME";
const char COMPILE_DATE[]     = "$COMPILE_DATE";
const char PROGRAM_NAME[]     = "$PROGRAM_NAME";
const char PROGRAM_VERSION[]  = "$PROGRAM_VERSION";
const char PROGRAM_DESCR[]    = "$PROGRAM_DESCR";
MYqxgW93
