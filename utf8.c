// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 SASANO Takayoshi <uaa@uaa.org.uk>

#include <string.h>
#include "utf8.h"

extern unsigned short ucs2tosjis[65536];

int utf8toucs_char(unsigned char *s, int len, int *ucs)
{
	int i, m, n, l, h, u = -1;

	if (*s >= 0x00 && *s <= 0x7f) {
		m = 0x7f; n = 1; l = 0x00000000; h = 0x0000007f;
	} else if (*s >= 0xc0 && *s <= 0xdf) {
		m = 0x1f; n = 2; l = 0x00000080; h = 0x000007ff;
	} else if (*s >= 0xe0 && *s <= 0xef) {
		m = 0x0f; n = 3; l = 0x00000800; h = 0x0000ffff;
	} else if (*s >= 0xf0 && *s <= 0xf7) {
		m = 0x07; n = 4; l = 0x00010000; h = 0x001fffff;
	} else if (*s >= 0xf8 && *s <= 0xfb) {
		m = 0x03; n = 5; l = 0x00200000; h = 0x03ffffff;
	} else if (*s >= 0xfc && *s <= 0xfd) {
		m = 0x01; n = 6; l = 0x04000000; h = 0x7fffffff;
	} else {
		m = 0x00; n = 1; l = 0x00000001; h = 0x00000001;
	}

	if (ucs == NULL) return n;
	if (len < n) {
		n = len;
		goto fin;
	}

	u = *s++ & m;
	for (i = 1; i < n; i++) {
		u <<= 6;
		if ((*s & 0xc0) != 0x80) goto fin;
		u |= *s++ & 0x3f;
	}
	u = (u >= l && u <= h) ? u : -1;
fin:
	*ucs = u;
	return n;
}

int ucstoutf8_char(unsigned char *s, int len, int ucs)
{
	int i, m, n, v;

	if (ucs >= 0x00000000 && ucs <= 0x0000007f) {
		m = 0x7f; n = 1; v = 0x00;
	} else if (ucs >= 0x00000080 && ucs <= 0x000007ff) {
		m = 0x1f; n = 2; v = 0xc0;
	} else if (ucs >= 0x00000800 && ucs <= 0x0000ffff) {
		m = 0x0f; n = 3; v = 0xe0;
	} else if (ucs >= 0x00010000 && ucs <= 0x001fffff) {
		m = 0x07; n = 4; v = 0xf0;
	} else if (ucs >= 0x00200000 && ucs <= 0x03ffffff) {
		m = 0x03; n = 5; v = 0xf8;
	} else if (ucs >= 0x04000000 && ucs <= 0x7fffffff) {
		m = 0x01; n = 6; v = 0xfc;
	}

	if (s == NULL) goto fin;
	if (len < n) return 0;

	*s++ = v | ((ucs >> ((n - 1) * 6)) & m);
	for (i = 1; i < n; i++)
		*s++ = 0x80 | ((ucs >> ((n - i - 1) * 6)) & 0x3f);
fin:
	return n;
}

int ucstosjis_char(unsigned char *s, int len, int ucs)
{
	int n, sjis = 0;

	if (ucs < sizeof(ucs2tosjis) / sizeof(unsigned short))
		sjis = ucs2tosjis[ucs];

	if (ucs && !sjis)
		sjis = 0x81ac;

	n = (sjis < 0x8000) ? 1 : 2;

	if (s == NULL) goto fin;
	if (len < n) return 0;

	switch (n) {
	case 2:
		*s++ = sjis >> 8;
		/* FALLTHROUGH */
	case 1:
		*s = sjis;
		break;
	}
	
fin:
	return n;
}
