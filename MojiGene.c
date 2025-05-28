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
static char Header[BUFSIZE] = "hr hr <bt>";
static char Footer[BUFSIZE] = "<ar>";
static char FileName[BUFSIZE] = "MojiGene.txt";

struct config {
	char *string;
	int (*function)(char *);
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

static struct config keywords[] = {
	/* need a space after keyword */
	{"WordLen ", set_wordlen},
	{"Chars ", set_chars},
	{"Words ", set_charsbywords},
	{"NumRatio ", set_numratio},
	{"SleepTime ", set_sleeptime},
	{"WordPerLine ", set_wordperline},
	{"Header ", set_header},
	{"Footer ", set_footer},
	{"FileName ", set_filename},
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

static char *skip_spaces(char *buf)
{
	char *p;

	for (p = buf; *p == ' '; p++);

	return p;
}

static int parse(char *buf)
{
	int i;
	char *p, *q;

	/* remove LF/CR */
	if ((p = strchr(buf, '\x0a')) != NULL) *p = '\0';
	if ((p = strchr(buf, '\x0d')) != NULL) *p = '\0';

	p = skip_spaces(buf);

	/* '=' separator required */
	if ((q = strchr(p, '=')) == NULL)
		return -1;

	*q = '\0';
	q = skip_spaces(q + 1);
	
	/* value required */
	if (*q == '\0')
		return -1;

	for (i = 0; i < sizeof(keywords) / sizeof(struct config); i++) {
		if (!strcmp(p, keywords[i].string))
			return (*keywords[i].function)(q);

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

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (*buf == '#') continue;
		parse(buf);
	}

	fclose(fp);

	return 0;
}

static int mojigene_ch(void)
{
	if ((rand() & 0x7f) < NumRatio)
		return '0' + (rand() % 10);
	else
		return 'a' + (rand() % 26);
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
	if (WordLen > 0 && WordPerLine > 0 && Chars > 0) mojigene(fp);
	if (strlen(Footer)) fprintf(fp, "%s" CRLF, Footer);

	fclose(fp);

	return 0;
}

int main(int argc, char *argv[])
{
	int ch;
	time_t t;
	bool debug = false;

	t = time(NULL);
	srand(t);

	do_config();

	/* override by command line */
	while ((ch = getopt(argc, argv, "W:c:w:n:s:L:H:F:o:d")) != -1) {
		switch (ch) {
		case 'W': set_wordlen(optarg); break;
		case 'c': set_chars(optarg); break;
		case 'w': set_charsbywords(optarg); break;
		case 'n': set_numratio(optarg); break;
		case 's': set_sleeptime(optarg); break;
		case 'L': set_wordperline(optarg); break;
		case 'H': set_header(optarg); break;
		case 'F': set_footer(optarg); break;
		case 'o': set_filename(optarg); break;
		case 'd': debug = true; break;
		}
	}

	if (debug) {
		fprintf(stderr, "WordLen = %d\n", WordLen);
		fprintf(stderr, "Chars = %d\n", Chars);
		fprintf(stderr, "NumRatio = %d\n", NumRatio);
		fprintf(stderr, "SleepTime = %d\n", SleepTime);
		fprintf(stderr, "WordPerLine = %d\n", WordPerLine);
		fprintf(stderr, "Header = \"%s\"\n", Header);
		fprintf(stderr, "Footer = \"%s\"\n", Footer);
		fprintf(stderr, "FileName = \"%s\"\n", FileName);
	}

#if defined(__LCC__)
	Sleep(SleepTime * 1000); /* Windows API */
#else
	sleep(SleepTime);
#endif

	do_main();

	return 0;
}
