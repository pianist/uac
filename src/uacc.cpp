#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <map>
#include <MAFSA/daciuk.hpp>
#include "sla.h"
#include "uac.h"

typedef std::map<std::string, uint32_t> uac_std_flag_title_rev_t;
typedef MAFSA::daciuk<MAX_LETTER_SLA + 1> dict_compiler_t;

uac_std_flag_title_rev_t flag_title_rev;

void process_input_file(dict_compiler_t& dc, const char* fname)
{
	FILE* f = fopen(fname, "r");

	if (!f)
	{
		fprintf(stderr, "Can't open %s for reading: %s\n", fname, strerror(errno));
		exit(-1);
	}

	unsigned line_no = 1;
	while (!feof(f))
	{
		char buf[1024];
		fgets(buf, 1024, f);

		if ('#' == buf[0]) continue;

		char* delim = strpbrk(buf, " \t:");
		if (!delim) continue;
		
		*delim++ = 0;
		//while (strchr(delimiters, delim)) delim++;

		char* nlrl = strpbrk(delim, "\r\n|");
		if (nlrl)
		{
			line_no++;
			*nlrl = 0;
		}

		MAFSA_letter l[1024];
		ssize_t ssz = conv_s2l_sla(delim, l, 1024);
		if (ssz > 0)
		{
			l[ssz++] = MAX_LETTER_SLA;

			char* tk = buf;
			while (tk)
			{
				char* ntk = strpbrk(tk, ",;");
				if (ntk) *ntk++ = 0;

				ssize_t len = ssz;
#ifdef _DEBUG
ssize_t save_len = len;
#endif

				const char* to_search = tk;
				if ('-' == *to_search) to_search++;

				uac_std_flag_title_rev_t::const_iterator it = flag_title_rev.find(to_search);
				if (it != flag_title_rev.end())
				{
					uint32_t id = it->second;
					do
					{
						l[len++] = id % MAX_LETTER_SLA;
						id /= MAX_LETTER_SLA;
					}
					while (id);
				}
				else
				{
					fprintf(stderr, "Unknown flag '%s', at line %u in %s can't continue...\n", to_search, line_no, fname);
					exit(-1);
				}

				l[len++] = MAX_LETTER_SLA;

				len += conv_s2l_sla(tk, l + len, 1024 - len);
				dc.insert(l, len);

#ifdef _DEBUG
char x[1024] = {0};
conv_l2s_sla(l, len, x, 1024);
printf("%s -- %s\n", x, buf);
uint32_t id = 0;
uint32_t mul = 1;
while (l[save_len] != MAX_LETTER_SLA) { id += mul * l[save_len++]; mul *= MAX_LETTER_SLA;}
if (mul == 1) { printf("\tno id\n"); } else { printf("\tid %u\n", id); }
#endif

				tk = ntk;
			}
		}
	}

	fclose(f);
}

int main(int argc, char **argv)
{
#ifdef _DEBUG
printf("UAC_FLAG_mobile: %u\n", UAC_FLAG_apple);
printf("UAC_MASK_mobile: %08lX\n", UAC_MASK_apple);
printf("UAC_TXT_mobile: %s\n", UAC_TXT_apple);
#endif

	for (int i = 0; uac_std_flags[i].title; ++i)
	{
		//printf("%u\t%s\n", uac_std_flags[i].flag, uac_std_flags[i].title);
		flag_title_rev[uac_std_flags[i].title] = uac_std_flags[i].flag_id;
	}

        if (argc < 3)
        {
                printf("usage: uacc output.automaton dict_file.txt\n");
                return -1;
        }

	dict_compiler_t dc;

	for (int i = 2; i < argc; ++i)
	{
		process_input_file(dc, argv[i]);
	}

	dc.save_to_file(argv[1]);
	return 0;
}

