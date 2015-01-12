/*
 * Copyright (C) 2007, libturglem development team.
 *
 * This file is released under the LGPL.
*/

#include "sla.h"
#include <string.h>

static int sla_convert_array_initiated = 0;
static MAFSA_letter sla_convert_array[256] = { 0 };

static void sla_convert_array_init()
{
	int i;
	memset(sla_convert_array, 10, 256);

	for (i = 0; i <= 9; ++i)
	{
		sla_convert_array['0' + i] = i;
	}

	sla_convert_array['-'] = 11;
	sla_convert_array['|'] = MAX_LETTER_SLA;

	for (i = 0; i < 26; ++i)
	{
		sla_convert_array['a' + i] = i + 12;
		sla_convert_array['A' + i] = i + 12;
	}

	sla_convert_array_initiated = 1;
}

ssize_t conv_s2l_sla(const char *s, MAFSA_letter *l, size_t sz)
{
	if (!sla_convert_array_initiated)
	{
		sla_convert_array_init();
	}

	ssize_t pos = 0;
	const char *i = s;
	while (*i && (*i != '\n') && (pos < sz))
	{
		MAFSA_letter _n = sla_convert_array[(unsigned char)*i++];

		if (255 == _n) continue;

		if ((10 == _n) && pos)
		{
			if (10 == l[pos-1]) continue;
		}

		l[pos++] = _n;
	}

	return pos;
}

size_t conv_l2s_sla(const MAFSA_letter *l, size_t sz_l, char *s, size_t sz_s)
{
	size_t pos_w = 0;
	size_t pos_r = 0;

	while ((pos_w + 1 < sz_s) && (pos_r < sz_l))
	{
		if(l[pos_r] < 10)
		{
			s[pos_w++] = l[pos_r] + '0';
		}
		else if ((l[pos_r] < 38) && (l[pos_r] > 11))
		{
			s[pos_w++] = l[pos_r] - 12 + 'A';
		}
		else
		{
			switch (l[pos_r])
			{
				case 10:
					s[pos_w++] = '_';
					break;
				case 11:
					s[pos_w++] = '-';
					break;
				default:
					s[pos_w++] = '|';
					break;
			}
		}
		pos_r++;
	}
	s[pos_w] = 0;
	return pos_w;
}

ssize_t conv_b2l_sla(const char *s, size_t sz_s, MAFSA_letter *l, size_t sz_l)
{
	ssize_t pos = 0;
	const char *i = s;
	const char *e = s + sz_s;
	while ((i < e) && (*i != '\n') && (pos < sz_l))
	{
		if (i[0] >= '0' && i[0] <= '9')
		{
			l[pos++] = i[0] - '0';
			i++;
		}
		else if (strchr(" \t_@./+\\\"'=%$#@!?(){}[]", i[0]))
		{
			if (pos && (l[pos-1] != 10) && (l[pos-1] != 11)) l[pos++] = 10;
			i++;
		}
		else if (strchr("-!#$%^&*().\\", i[0]))
		{
			if (pos && (l[pos-1] != 10) && (l[pos-1] != 11)) l[pos++] = 11; /* '-' */
			i++;
		}
		else if (i[0] >= 'a' && i[0] <= 'z')
		{
			l[pos++] = i[0] - 'a' + 12;
			i++;
		}
		else if (i[0] >= 'A' && i[0] <= 'Z')
		{
			l[pos++] = i[0] - 'A' + 12;
			i++;
		}
		else if (i[0] == '|')
		{
			l[pos++] = MAX_LETTER_SLA;
			i++;
		}
		else i++;
	}
	return pos;
}

size_t conv_l2b_sla(const MAFSA_letter *l, size_t sz_l, char *s, size_t sz_s)
{
	size_t pos_w = 0;
	size_t pos_r = 0;

	while ((pos_w < sz_s) && (pos_r < sz_l))
	{
		if(l[pos_r] < 10)
		{
			s[pos_w++] = l[pos_r] + '0';
		}
		else if ((l[pos_r] < 38) && (l[pos_r] > 11))
		{
			s[pos_w++] = l[pos_r] - 12 + 'A';
		}
		else if (pos_w)
		{
			switch (l[pos_r])
			{
				case 10:
					s[pos_w++] = '_';
					break;
				case 11:
					s[pos_w++] = '-';
					break;
				default:
					s[pos_w++] = '|';
					break;
			}
		}
		pos_r++;
	}
	return pos_w;
}

