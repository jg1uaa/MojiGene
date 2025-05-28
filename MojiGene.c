// SPDX-License-Identifier: WTFPL
// export INCLUDE=$WATCOM/h:$WATCOM/h/nt
// wcl386 -bcl=nt MojiGene.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#if defined(__LCC__)
#include <getopt.h>
#include <windows.h>
#endif

#define CRLF "\x0d\x0a"
#define ConfigFile "MojiGene.ini"
#define BUFSIZE 256

static int WordLen = 5;
static int Chars = 300;
static int NumRatio = 0x20;
static int SleepTime = 0;
static int WordPerLine = 5;
static int CharGroup0Len;
static int CharGroup1Len;
static char Header[BUFSIZE] = "hr hr <bt>";
static char Footer[BUFSIZE] = "<ar>";
static char FileName[BUFSIZE] = "MojiGene.txt";
static char CharGroup0[BUFSIZE] = "abcdefghijklmnopqrstuvwxyz";
static char CharGroup1[BUFSIZE] = "0123456789";

struct config {
	char *string;
	int (*function)(char *);
	bool allow_empty;
};

static int set_wordlen(char *);
static int set_chars(char *);
static int set_charsbywords(char *);
static int set_numratio(char *);
static int set_sleeptime(char *);
static int set_wordperline(char *);
static int set_header(char *);
static int set_footer(char *);
static int set_filename(char *);
static int set_chargroup0(char *);
static int set_chargroup1(char *);

static struct config keywords[] = {
	/* need a space after keyword */
	{"WordLen ", set_wordlen, false},
	{"Chars ", set_chars, false},
	{"Words ", set_charsbywords, false},
	{"NumRatio ", set_numratio, false},
	{"SleepTime ", set_sleeptime, false},
	{"WordPerLine ", set_wordperline, false},
	{"Header ", set_header, true},
	{"Footer ", set_footer, true},
	{"FileName ", set_filename, false},
	{"CharGroup0 ", set_chargroup0, true},
	{"CharGroup1 ", set_chargroup1, true},
};

static int set_wordlen(char *buf)
{
	WordLen = atoi(buf);
	return 0;
}

static int set_chars(char *buf)
{
	Chars = atoi(buf);
	return 0;
}

static int set_charsbywords(char *buf)
{
	Chars = atoi(buf) * WordLen;
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

static int set_wordperline(char *buf)
{
	WordPerLine = atoi(buf);
	return 0;
}

static int set_header(char *buf)
{
	snprintf(Header, sizeof(Header), "%s", buf);
	return 0;
}

static int set_footer(char *buf)
{
	snprintf(Footer, sizeof(Footer), "%s", buf);
	return 0;
}

static int set_filename(char *buf)
{
	snprintf(FileName, sizeof(FileName), "%s", buf);
	return 0;
}

static void remove_space_copy(char *dst, int dstsize, char *src)
{
	int i;

	for (i = 0; *src && i < dstsize - 1; src++)
		if (*src != ' ') dst[i++] = *src;

	dst[i] = '\0';
}

static int set_chargroup0(char *buf)
{
	remove_space_copy(CharGroup0, sizeof(CharGroup0), buf);
	CharGroup1[0] = '\0';
	return 0;
}

static int set_chargroup1(char *buf)
{
	remove_space_copy(CharGroup1, sizeof(CharGroup1), buf);
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
	if (CharGroup1Len > 0 && (rand() & 0x7f) < NumRatio)
		return CharGroup1[rand() % CharGroup1Len];
	else
		return CharGroup0[rand() % CharGroup0Len];
}
			
static void mojigene(FILE *fp)
{
	int i, n;

	for (i = 0; i < Chars; i++) {
		fputc(mojigene_ch(), fp);

		n = i + 1;
		if (n < Chars) {
			if (!(n % (WordLen * WordPerLine))) fputs(CRLF, fp);
			else if (!(n % WordLen)) fputc(' ', fp);
		}
	}

	fputs(CRLF, fp);
}

static int do_main(void)
{
	FILE *fp;

	fp = strcmp("-", FileName) ? fopen(FileName, "wb") : stdout;
	if (fp == NULL)
		return -1;

	if (strlen(Header)) fprintf(fp, "%s" CRLF, Header);
	if (WordLen > 0 && WordPerLine > 0 &&
	    Chars > 0 && CharGroup0Len > 0) mojigene(fp);
	if (strlen(Footer)) fprintf(fp, "%s" CRLF, Footer);

	fclose(fp);

	return 0;
}

int main(int argc, char *argv[])
{
	int ch;
	time_t t;
	char *p;
	bool debug = false;

	t = time(NULL);
	srand(t);

	do_config();

	/* override by command line */
	while ((ch = getopt(argc, argv, "W:c:w:n:s:L:H:F:o:x:y:d")) != -1) {
		if ((p = optarg) != NULL) {
			p = skip_spaces(optarg);
			remove_trailing_spaces(p);
		}

		switch (ch) {
		case 'W': set_wordlen(p); break;
		case 'c': set_chars(p); break;
		case 'w': set_charsbywords(p); break;
		case 'n': set_numratio(p); break;
		case 's': set_sleeptime(p); break;
		case 'L': set_wordperline(p); break;
		case 'H': set_header(p); break;
		case 'F': set_footer(p); break;
		case 'o': set_filename(p); break;
		case 'x': set_chargroup0(p); break;
		case 'y': set_chargroup1(p); break;
		case 'd': debug = true; break;
		}
	}

	CharGroup0Len = strlen(CharGroup0);
	CharGroup1Len = strlen(CharGroup1);

	if (debug) {
		fprintf(stderr, "WordLen = %d\n", WordLen);
		fprintf(stderr, "Chars = %d\n", Chars);
		fprintf(stderr, "NumRatio = %d\n", NumRatio);
		fprintf(stderr, "SleepTime = %d\n", SleepTime);
		fprintf(stderr, "WordPerLine = %d\n", WordPerLine);
		fprintf(stderr, "Header = \"%s\"\n", Header);
		fprintf(stderr, "Footer = \"%s\"\n", Footer);
		fprintf(stderr, "FileName = \"%s\"\n", FileName);
		fprintf(stderr, "CharGroup0Len = %d\n", CharGroup0Len);
		fprintf(stderr, "CharGroup1Len = %d\n", CharGroup1Len);
		fprintf(stderr, "CharGroup0 = \"%s\"\n", CharGroup0);
		fprintf(stderr, "CharGroup1 = \"%s\"\n", CharGroup1);
	}

#if defined(__LCC__)
	Sleep(SleepTime * 1000); /* Windows API */
#else
	sleep(SleepTime);
#endif

	do_main();

	return 0;
}
