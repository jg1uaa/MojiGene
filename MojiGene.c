// SPDX-License-Identifier: WTFPL
// export INCLUDE=$WATCOM/h:$WATCOM/h/nt
// wcl386 -bcl=nt MojiGene.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#if defined(__LCC__)
#include <getopt.h>
#include <windows.h>
#endif
#include "rng.h"
#include "utf8.h"

/* BSD's <sys/param.h> has nitems(), but others not */
#define N_ITEMS(_x) (sizeof((_x)) / sizeof((_x)[0]))

#define CRLF "\x0d\x0a"
#define BUFSIZE 4096
static char ConfigFile[BUFSIZE] = "MojiGene.ini";

#define TRIM_NONE 0
#define TRIM_MODERATE 1
#define TRIM_STRICT 2

static int WordLen = 5;
static int MinWordLen = 0;
static int Chars = 300;
static int NumRatio = 0x20;
static int SleepTime = 0;
static int CharPerLine = 34;
static int TrimMethod = TRIM_MODERATE;
static int UseSJIS = 0;
static int CharGroup0Len;
static int CharGroup1Len;
static int Header[BUFSIZE] = {
	'h', 'r', ' ', 'h', 'r', ' ', '<', 'b', 't', '>', '\0',
};
static int Footer[BUFSIZE] = {
	'<', 'a', 'r', '>', '\0',
};
static char FileName[BUFSIZE] = "MojiGene.txt";
static int CharGroup0[BUFSIZE] = {
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
	'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
	'u', 'v', 'w', 'x', 'y', 'z', '\0',
};
static int CharGroup1[BUFSIZE] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '\0',
};

#define DELIMITER_DEFAULT ' '
static int Delimiter = DELIMITER_DEFAULT;

static int PageBreak[BUFSIZE] = {
	'\0',
};

struct config {
	char *string;
	int (*function)(char *);
	bool allow_empty;
};

static int set_configfile(char *);
static int set_wordlen(char *);
static int set_minwordlen(char *);
static int set_chars(char *);
static int set_numratio(char *);
static int set_sleeptime(char *);
static int set_charperline(char *);
static int set_trimmethod(char *);
static int set_usesjis(char *);
static int set_header(char *);
static int set_footer(char *);
static int set_filename(char *);
static int set_chargroup0(char *);
static int set_chargroup1(char *);
static int set_delimiter(char *);
static int set_pagebreak(char *);

static struct config keywords[] = {
	/* need a space after keyword */
	{"WordLen ", set_wordlen, false},
	{"MinWordLen ", set_minwordlen, false},
	{"Chars ", set_chars, false},
	{"NumRatio ", set_numratio, false},
	{"SleepTime ", set_sleeptime, false},
	{"CharPerLine ", set_charperline, false},
	{"TrimMethod", set_trimmethod, true},
	{"UseSJIS ", set_usesjis, false},
	{"Header ", set_header, true},
	{"Footer ", set_footer, true},
	{"FileName ", set_filename, false},
	{"CharGroup0 ", set_chargroup0, true},
	{"CharGroup1 ", set_chargroup1, true},
	{"Delimiter ", set_delimiter, true},
	{"PageBreak ", set_pagebreak, true},
};

static void decode_utf8(int *dst, int dstsize, char *src, bool ignore_space)
{
	int i, n, srcsize;

	for (i = 0, srcsize = strlen(src);
	     srcsize && i < dstsize - 1; srcsize -= n, src += n) {
		if (ignore_space && *src == ' ') {
			n = 1;
			continue;
		}

		if (utf8toucs_char(src, 0, NULL) > srcsize) {
			dst[i++] = 0xfffd; /* unknown character */
			break;
		}

		n = utf8toucs_char(src, srcsize, &dst[i++]);
	}

	dst[i] = '\0';
}

static int set_configfile(char *buf)
{
	snprintf(ConfigFile, sizeof(ConfigFile), "%s", buf);
	return 0;
}

static int set_wordlen(char *buf)
{
	WordLen = atoi(buf);
	return 0;
}

static int set_minwordlen(char *buf)
{
	MinWordLen = atoi(buf);
	return 0;
}

static int set_chars(char *buf)
{
	Chars = atoi(buf);
	return 0;
}

static int set_numratio(char *buf)
{
	NumRatio = atoi(buf) & 0xff;
	return 0;
}

static int set_sleeptime(char *buf)
{
	SleepTime = atoi(buf) & 0x3f;
	return 0;
}

static int set_charperline(char *buf)
{
	CharPerLine = atoi(buf);
	return 0;
}

static int set_trimmethod(char *buf)
{
	TrimMethod = atoi(buf);
	return 0;
}

static int set_usesjis(char *buf)
{
	UseSJIS = atoi(buf) ? 1 : 0;
	return 0;
}

static int set_header(char *buf)
{
	decode_utf8(Header, N_ITEMS(Header), buf, false);
	return 0;
}

static int set_footer(char *buf)
{
	decode_utf8(Footer, N_ITEMS(Footer), buf, false);
	return 0;
}

static int set_filename(char *buf)
{
	snprintf(FileName, N_ITEMS(FileName), "%s", buf);
	return 0;
}

static int set_chargroup0(char *buf)
{
	decode_utf8(CharGroup0, N_ITEMS(CharGroup0), buf, true);
	CharGroup1[0] = '\0';
	return 0;
}

static int set_chargroup1(char *buf)
{
	decode_utf8(CharGroup1, N_ITEMS(CharGroup1), buf, true);
	return 0;
}

static int set_delimiter(char *buf)
{
	int tmp[2];

	decode_utf8(tmp, N_ITEMS(tmp), buf, true);
	if (tmp[0]) Delimiter = tmp[0];
	return 0;
}

static int set_pagebreak(char *buf)
{
	decode_utf8(PageBreak, N_ITEMS(PageBreak), buf, false);
	return 0;
}

static int *u_strchr(int *u_str, int u_chr)
{
	for (; *u_str; u_str++)
		if (*u_str == u_chr) return u_str;

	return NULL;
}

static int u_strlen(int *u_str)
{
	int i;

	for (i = 0; u_str[i]; i++);

	return i;
}

static int u_fputc(int uc, FILE *fp)
{
	char buf[8];

	buf[UseSJIS ?
	    ucstosjis_char(buf, sizeof(buf), uc) :
	    ucstoutf8_char(buf, sizeof(buf), uc)] = '\0';

	return fputs(buf, fp);
}

static int u_fputs(int *u_str, FILE *fp)
{
	while (*u_str) u_fputc(*u_str++, fp);
	return 0;
}

static char *skip_spaces(char *buf)
{
	char *p;

	for (p = buf; *p == ' '; p++);

	return p;
}

static void remove_trailing_spaces(char *buf)
{
	char *p;

	for (p = buf + strlen(buf) - 1; p >= buf && *p == ' '; *p-- = '\0');
}

static int parse(char *buf)
{
	int i;
	char *p, *q;

	/* remove LF/CR */
	if ((p = strchr(buf, '\x0a')) != NULL) *p = '\0';
	if ((p = strchr(buf, '\x0d')) != NULL) *p = '\0';

	/* '#' as comment line */
	if (*(p = skip_spaces(buf)) == '#')
		return 0;

	/* '=' separator required */
	if ((q = strchr(p, '=')) == NULL)
		return -1;

	*q = '\0';
	q = skip_spaces(q + 1);
	remove_trailing_spaces(q);
	
	for (i = 0; i < sizeof(keywords) / sizeof(struct config); i++) {
		if (!strcmp(p, keywords[i].string))
			return (keywords[i].allow_empty || *q) ?
				(*keywords[i].function)(q) : -1;
	}

	return -1;
}

static int do_config(void)
{
	FILE *fp;
	char buf[BUFSIZE];

	fp = fopen(ConfigFile, "rb");
	if (fp == NULL)
		return -1;

	while (fgets(buf, sizeof(buf), fp) != NULL)
		parse(buf);

	fclose(fp);

	return 0;
}

static int mojigene_ch(void)
{
	if (CharGroup1Len > 0 && random_value(0, 0x7f) < NumRatio)
		return CharGroup1[random_value(0, CharGroup1Len - 1)];
	else
		return CharGroup0[random_value(0, CharGroup0Len - 1)];
}

static int mojigene_count_char_and_chop(int *buf, int limit)
{
	int i, d, p, n, x, w;

	/* chop at the nearest size */
	for (i = p = n = 0; buf[i]; i++) {
		if (buf[i] == Delimiter) {
			if (n >= limit) {
				buf[i] = '\0';
				break;
			}
			p = i + 1; /* last word position */
		} else {
			n++;
		}
	}

	/* trim: minimize exceed size */
	if (TrimMethod && n > limit) {
		x = n - limit; /* exceed size */
		w = i - p; /* last word size */
		if (TrimMethod & TRIM_STRICT)
			d = x;
		else
			d = ((w - x) < MinWordLen) ? (w - MinWordLen) : x;
		buf[i - d] = '\0';
		n -= d;
	}

	return n;
}

static void mojigene_one_word(int *buf, int size)
{
	int i;

	for (i = 0; i < size - 1; i++)
		buf[i] = mojigene_ch();

	buf[i] = Delimiter;
}

static void mojigene_make_words(int *buf, int size)
{
	int i, n;

	if (size < MinWordLen + 1)
		return;

	for (i = 0; ; ) {
		n = random_value(MinWordLen, WordLen) + 1;

		if ((size - i) < n) {
			buf[(i > 0) ? (i - 1) : 0] = '\0';
			break;
		}

		mojigene_one_word(&buf[i], n);
		i += n;
	}
}

static void mojigene(FILE *fp)
{
	int i, n;
	int *buf = calloc(CharPerLine + 1, sizeof(int));

	for (i = 0; i < Chars; ) {
		mojigene_make_words(buf, CharPerLine + 1);
		i += mojigene_count_char_and_chop(buf, Chars - i);

		for (n = 0; buf[n]; n++)
			u_fputc(buf[n], fp);
		if (i < Chars)
			u_fputs(PageBreak, fp);
		fputs(CRLF, fp);
	}

	free(buf);
}

static int do_main(void)
{
	FILE *fp;

	fp = strcmp("-", FileName) ? fopen(FileName, "wb") : stdout;
	if (fp == NULL)
		return -1;

	if (u_strlen(Header)) {
		u_fputs(Header, fp);
		fputs(CRLF, fp);
	}
	if (WordLen > 0 && CharPerLine > 0 &&
	    Chars > 0 && CharGroup0Len > 0) mojigene(fp);
	if (u_strlen(Footer)) {
		u_fputs(Footer, fp);
		fputs(CRLF, fp);
	}
	fclose(fp);

	return 0;
}

int main(int argc, char *argv[])
{
#define OPT_ARG "C:W:w:c:n:s:L:T:SUH:F:o:x:y:D:P:d"

	int ch;
	char *p;
	bool debug = false;

	initialize_random_generator();

	/* load configuration file */
	while ((ch = getopt(argc, argv, OPT_ARG)) != -1) {
		if ((p = optarg) != NULL) {
			p = skip_spaces(optarg);
			remove_trailing_spaces(p);
		}

		switch (ch) {
		case 'C': set_configfile(p); break;
		}
	}
	do_config();

	/* override by command line */
	optind = 1; /* rescan */
	while ((ch = getopt(argc, argv, OPT_ARG)) != -1) {
		if ((p = optarg) != NULL) {
			p = skip_spaces(optarg);
			remove_trailing_spaces(p);
		}

		switch (ch) {
		case 'W': set_wordlen(p); break;
		case 'w': set_minwordlen(p); break;
		case 'c': set_chars(p); break;
		case 'n': set_numratio(p); break;
		case 's': set_sleeptime(p); break;
		case 'L': set_charperline(p); break;
		case 'T': set_trimmethod(p); break;
		case 'S': set_usesjis("1"); break;
		case 'U': set_usesjis("0"); break;
		case 'H': set_header(p); break;
		case 'F': set_footer(p); break;
		case 'o': set_filename(p); break;
		case 'x': set_chargroup0(p); break;
		case 'y': set_chargroup1(p); break;
		case 'D': set_delimiter(p); break;
		case 'P': set_pagebreak(p); break;
		case 'd': debug = true; break;
		}
	}

	CharGroup0Len = u_strlen(CharGroup0);
	CharGroup1Len = u_strlen(CharGroup1);

	if (WordLen < 0)
		WordLen = 0;
	else if (WordLen > CharPerLine)
		WordLen = CharPerLine;
	if (MinWordLen <= 0 || MinWordLen > WordLen)
		MinWordLen = WordLen;

	if (u_strchr(CharGroup0, Delimiter) != NULL ||
	    u_strchr(CharGroup1, Delimiter) != NULL)
		Delimiter = DELIMITER_DEFAULT;

	if (debug) {
		fprintf(stderr, "ConfigFile = %s\n", ConfigFile);
		fprintf(stderr, "WordLen = %d\n", WordLen);
		fprintf(stderr, "MinWordLen = %d\n", MinWordLen);
		fprintf(stderr, "Chars = %d\n", Chars);
		fprintf(stderr, "NumRatio = %d\n", NumRatio);
		fprintf(stderr, "SleepTime = %d\n", SleepTime);
		fprintf(stderr, "CharPerLine = %d\n", CharPerLine);
		fprintf(stderr, "TrimMethod = %d\n", TrimMethod);
		fprintf(stderr, "UseSJIS = %d\n", UseSJIS);
		fputs("Header = \"", stderr);
		u_fputs(Header, stderr);
		fputs("\"\n", stderr);
		fputs("Footer = \"", stderr);
		u_fputs(Footer, stderr);
		fputs("\"\n", stderr);
		fprintf(stderr, "FileName = \"%s\"\n", FileName);
		fprintf(stderr, "CharGroup0Len = %d\n", CharGroup0Len);
		fprintf(stderr, "CharGroup1Len = %d\n", CharGroup1Len);
		fputs("CharGroup0 = \"", stderr);
		u_fputs(CharGroup0, stderr);
		fputs("\"\n", stderr);
		fputs("CharGroup1 = \"", stderr);
		u_fputs(CharGroup1, stderr);
		fputs("\"\n", stderr);
		fprintf(stderr, "Delimiter = %#x\n", Delimiter);
		fputs("PageBreak = \"", stderr);
		u_fputs(PageBreak, stderr);
		fputs("\"\n", stderr);
	}

#if defined(__LCC__)
	Sleep(SleepTime * 1000); /* Windows API */
#else
	sleep(SleepTime);
#endif

	do_main();

	return 0;
}
