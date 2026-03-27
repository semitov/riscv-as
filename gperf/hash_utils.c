#include "hash.h"
/* ANSI-C code produced by gperf version 3.3 */
/* Command-line: gperf -t -C --output-file=hash_utils.c test.gperf  */
/* Computed positions: -k'1-4' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) && ('%' == 37) && ('&' == 38) && ('\'' == 39) &&        \
	  ('(' == 40) && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) && ('-' == 45) && ('.' == 46) &&         \
	  ('/' == 47) && ('0' == 48) && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) && ('5' == 53) &&         \
	  ('6' == 54) && ('7' == 55) && ('8' == 56) && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) &&         \
	  ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) && ('B' == 66) && ('C' == 67) && ('D' == 68) &&         \
	  ('E' == 69) && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) && ('J' == 74) && ('K' == 75) &&         \
	  ('L' == 76) && ('M' == 77) && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) && ('R' == 82) &&         \
	  ('S' == 83) && ('T' == 84) && ('U' == 85) && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) &&         \
	  ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) && ('^' == 94) && ('_' == 95) && ('a' == 97) &&        \
	  ('b' == 98) && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) && ('g' == 103) && ('h' == 104) &&    \
	  ('i' == 105) && ('j' == 106) && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) && ('o' == 111) &&  \
	  ('p' == 112) && ('q' == 113) && ('r' == 114) && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) &&  \
	  ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) && ('{' == 123) && ('|' == 124) && ('}' == 125) &&  \
	  ('~' == 126))
/* The character set is not based on ISO-646.  */
#error                                                                                                                 \
	"gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "test.gperf"
struct instruction;

#define TOTAL_KEYWORDS 33
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 5
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 89
/* maximum key range = 88, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
	static unsigned int hash(register const char *str, register size_t len) {
	static const unsigned char asso_values[] = {
		90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
		90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
		90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
		90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 15, 20, 90, 20, 20, 90, 45,
		0,	20, 90, 90, 0,	90, 25, 0,	90, 30, 25, 10, 5,	0,	90, 5,	15, 90, 90, 90, 90, 90, 90, 90, 90, 90,
		90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
		90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
		90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
		90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
		90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90};
	register unsigned int hval = len;

	switch (hval) {
		default:
			hval += asso_values[(unsigned char)str[3]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ &&                 \
														__clang_major__ + (__clang_minor__ >= 9) > 3))) ||             \
	(defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L &&                                                        \
	 ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
			[[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
			__attribute__((__fallthrough__));
#endif
		/*FALLTHROUGH*/
		case 3:
			hval += asso_values[(unsigned char)str[2]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ &&                 \
														__clang_major__ + (__clang_minor__ >= 9) > 3))) ||             \
	(defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L &&                                                        \
	 ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
			[[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
			__attribute__((__fallthrough__));
#endif
		/*FALLTHROUGH*/
		case 2:
			hval += asso_values[(unsigned char)str[1]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ &&                 \
														__clang_major__ + (__clang_minor__ >= 9) > 3))) ||             \
	(defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L &&                                                        \
	 ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
			[[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
			__attribute__((__fallthrough__));
#endif
		/*FALLTHROUGH*/
		case 1:
			hval += asso_values[(unsigned char)str[0]];
			break;
	}
	return hval;
}

const struct instruction *in_word_set(register const char *str, register size_t len) {
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
	static const struct instruction wordlist[] = {{""},
												  {""},
#line 23 "test.gperf"
												  {"lh", 0000011, 0x1, 0xFF, 'I'},
#line 26 "test.gperf"
												  {"lhu", 0000011, 0x5, 0xFF, 'I'},
												  {""},
												  {""},
												  {""},
#line 24 "test.gperf"
												  {"lw", 0000011, 0x2, 0xFF, 'I'},
												  {""},
												  {""},
												  {""},
												  {""},
#line 28 "test.gperf"
												  {"sh", 0100011, 0x1, 0xFF, 'S'},
#line 8 "test.gperf"
												  {"sll", 0110011, 0x1, 0x00, 'R'},
												  {""},
												  {""},
												  {""},
#line 29 "test.gperf"
												  {"sw", 0100011, 0x2, 0xFF, 'S'},
#line 11 "test.gperf"
												  {"slt", 0110011, 0x2, 0x00, 'R'},
#line 12 "test.gperf"
												  {"sltu", 0110011, 0x3, 0x00, 'R'},
												  {""},
												  {""},
#line 22 "test.gperf"
												  {"lb", 0000011, 0x0, 0xFF, 'I'},
#line 25 "test.gperf"
												  {"lbu", 0000011, 0x4, 0xFF, 'I'},
												  {""},
												  {""},
												  {""},
#line 6 "test.gperf"
												  {"or", 0110011, 0x6, 0x00, 'R'},
#line 32 "test.gperf"
												  {"blt", 1100011, 0x4, 0xFF, 'B'},
#line 34 "test.gperf"
												  {"bltu", 1100011, 0x6, 0xFF, 'B'},
												  {""},
												  {""},
#line 27 "test.gperf"
												  {"sb", 0100011, 0x0, 0xFF, 'S'},
#line 4 "test.gperf"
												  {"sub", 0110011, 0x0, 0x20, 'R'},
#line 17 "test.gperf"
												  {"slli", 0010011, 0x1, 0xFF, 'I'},
												  {""},
												  {""},
												  {""},
#line 9 "test.gperf"
												  {"srl", 0110011, 0x5, 0x00, 'R'},
#line 20 "test.gperf"
												  {"slti", 0010011, 0x2, 0xFF, 'I'},
#line 21 "test.gperf"
												  {"sltiu", 0010011, 0x3, 0xFF, 'I'},
												  {""},
												  {""},
#line 5 "test.gperf"
												  {"xor", 0110011, 0x4, 0x00, 'R'},
												  {""},
												  {""},
												  {""},
												  {""},
#line 15 "test.gperf"
												  {"ori", 0010011, 0x6, 0xFF, 'I'},
												  {""},
												  {""},
												  {""},
												  {""},
#line 10 "test.gperf"
												  {"sra", 0110011, 0x5, 0x20, 'R'},
												  {""},
												  {""},
												  {""},
												  {""},
#line 3 "test.gperf"
												  {"add", 0110011, 0x0, 0x00, 'R'},
#line 18 "test.gperf"
												  {"srli", 0010011, 0x5, 0xFF, 'I'},
												  {""},
												  {""},
												  {""},
#line 7 "test.gperf"
												  {"and", 0110011, 0x7, 0x00, 'R'},
#line 14 "test.gperf"
												  {"xori", 0010011, 0x4, 0xFF, 'I'},
												  {""},
												  {""},
												  {""},
#line 31 "test.gperf"
												  {"bne", 1100011, 0x1, 0xFF, 'B'},
												  {""},
												  {""},
												  {""},
												  {""},
#line 30 "test.gperf"
												  {"beq", 1100011, 0x0, 0xFF, 'B'},
#line 19 "test.gperf"
												  {"srai", 0010011, 0x5, 0xFF, 'I'},
												  {""},
												  {""},
												  {""},
												  {""},
#line 13 "test.gperf"
												  {"addi", 0010011, 0x0, 0xFF, 'I'},
												  {""},
												  {""},
												  {""},
												  {""},
#line 16 "test.gperf"
												  {"andi", 0010011, 0x7, 0xFF, 'I'},
												  {""},
												  {""},
												  {""},
#line 33 "test.gperf"
												  {"bge", 1100011, 0x5, 0xFF, 'B'},
#line 35 "test.gperf"
												  {"bgeu", 1100011, 0x7, 0xFF, 'B'}};
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic pop
#endif

	if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH) {
		register unsigned int key = hash(str, len);

		if (key <= MAX_HASH_VALUE) {
			register const char *s = wordlist[key].name;

			if (*str == *s && !strcmp(str + 1, s + 1)) {
				return &wordlist[key];
			}
		}
	}
	return (struct instruction *)0;
}
