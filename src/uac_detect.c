#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "uac.h"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("usage: uac_detect /path/to/dict/dir\n");
		return -1;
	}

	char err_buf[1024];
	int r = uac_init(argv[1], err_buf, 1024);
	if (r)
	{
		printf("Can't load uac dictionary automatons: %s\n", err_buf);
		return -1;
	}

	while (!feof(stdin))
	{
		char buf[1024];
		if (!fgets(buf, 1024, stdin)) continue;

		char* nlrl = strpbrk(buf, "\r\n");
		if (nlrl) *nlrl = 0;

		uac_result_t ret;
		uac_classify(&ret, buf);		

		char buf_res[1024];
		uac_hr_cl_result(&ret, buf_res, 1024);

		printf("G-%u\t%016" PRIX64 "\t%s\t%s\n", ret.group_id, ret.flags, buf_res, buf);
	}


	uac_free();

	return 0;
}

