#include "uac.h"
#include "sla.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <MAFSA/automaton.h>

#define UAC_DEFINE_GROUP(name) { UAC_GROUP_ ## name, "g_" # name, 0 },
#define UAC_DEFINE_FLAG(name,_groups) { UAC_FLAG_ ## name, # name, _groups },

#define UAC_GROUP_MASK_ANY	((uint64_t)0xFFFFFFFFFFFFFFFF)
#define UAC_GROUP_MASK_other	((uint64_t)1 << UAC_GROUP_other)
#define UAC_GROUP_MASK_android	((uint64_t)1 << UAC_GROUP_android)
#define UAC_GROUP_MASK_windows	((uint64_t)1 << UAC_GROUP_windows)
#define UAC_GROUP_MASK_apple	((uint64_t)1 << UAC_GROUP_apple)
#define UAC_GROUP_MASK_unix	((uint64_t)1 << UAC_GROUP_unix)

uac_std_flag_title_t uac_std_flags[] = {
	UAC_DEFINE_GROUP(other)
	UAC_DEFINE_GROUP(android)
	UAC_DEFINE_GROUP(apple)
	UAC_DEFINE_GROUP(windows)
	UAC_DEFINE_GROUP(unix)

	UAC_DEFINE_FLAG(phone,	UAC_GROUP_MASK_ANY)
	UAC_DEFINE_FLAG(tablet,	UAC_GROUP_MASK_ANY)
	UAC_DEFINE_FLAG(tv,	UAC_GROUP_MASK_ANY)

	UAC_DEFINE_FLAG(64bit,	UAC_GROUP_MASK_ANY)
	UAC_DEFINE_FLAG(arm,	UAC_GROUP_MASK_ANY)

	UAC_DEFINE_FLAG(android,	UAC_GROUP_MASK_android)
	UAC_DEFINE_FLAG(android_old,	UAC_GROUP_MASK_android)
	UAC_DEFINE_FLAG(android_2_3_x,	UAC_GROUP_MASK_android)
	UAC_DEFINE_FLAG(android_3_x,	UAC_GROUP_MASK_android)
	UAC_DEFINE_FLAG(android_4,	UAC_GROUP_MASK_android)
	UAC_DEFINE_FLAG(android_api_16,	UAC_GROUP_MASK_android)
	UAC_DEFINE_FLAG(android_api_17,	UAC_GROUP_MASK_android)
	UAC_DEFINE_FLAG(android_api_18,	UAC_GROUP_MASK_android)
	UAC_DEFINE_FLAG(android_api_19,	UAC_GROUP_MASK_android)
	UAC_DEFINE_FLAG(android_api_20,	UAC_GROUP_MASK_android)
	UAC_DEFINE_FLAG(android_5,	UAC_GROUP_MASK_android)
	UAC_DEFINE_FLAG(android_api_21,	UAC_GROUP_MASK_android)

	UAC_DEFINE_FLAG(linux,	UAC_GROUP_MASK_unix)
	UAC_DEFINE_FLAG(bsd,	UAC_GROUP_MASK_unix)
	UAC_DEFINE_FLAG(cros,	UAC_GROUP_MASK_unix)

	UAC_DEFINE_FLAG(debian,	UAC_GROUP_MASK_unix)
	UAC_DEFINE_FLAG(ubuntu,	UAC_GROUP_MASK_unix)
	UAC_DEFINE_FLAG(gentoo,	UAC_GROUP_MASK_unix)

	UAC_DEFINE_FLAG(ipod,	UAC_GROUP_MASK_apple)
	UAC_DEFINE_FLAG(iphone,	UAC_GROUP_MASK_apple)
	UAC_DEFINE_FLAG(ipad,	UAC_GROUP_MASK_apple)
	UAC_DEFINE_FLAG(mac,	UAC_GROUP_MASK_apple)

	UAC_DEFINE_FLAG(win_old,	UAC_GROUP_MASK_windows)
	UAC_DEFINE_FLAG(win_xp,		UAC_GROUP_MASK_windows)
	UAC_DEFINE_FLAG(win_vista,	UAC_GROUP_MASK_windows)
	UAC_DEFINE_FLAG(win_7,		UAC_GROUP_MASK_windows)
	UAC_DEFINE_FLAG(win_8,		UAC_GROUP_MASK_windows)
	UAC_DEFINE_FLAG(win_10,		UAC_GROUP_MASK_windows)

	UAC_DEFINE_FLAG(symbian,	UAC_GROUP_MASK_other)
	UAC_DEFINE_FLAG(nokia,		UAC_GROUP_MASK_other + UAC_GROUP_MASK_windows)
	UAC_DEFINE_FLAG(blackberry,	UAC_GROUP_MASK_other)
	UAC_DEFINE_FLAG(samsung,	UAC_GROUP_MASK_other + UAC_GROUP_MASK_android)
	UAC_DEFINE_FLAG(nexus,		UAC_GROUP_MASK_other + UAC_GROUP_MASK_android)
	UAC_DEFINE_FLAG(huawei,		UAC_GROUP_MASK_other + UAC_GROUP_MASK_android)
	UAC_DEFINE_FLAG(htc,		UAC_GROUP_MASK_other + UAC_GROUP_MASK_android + UAC_GROUP_MASK_windows)
	UAC_DEFINE_FLAG(lenovo,		UAC_GROUP_MASK_other + UAC_GROUP_MASK_android + UAC_GROUP_MASK_windows)

	UAC_DEFINE_FLAG(chrome,		UAC_GROUP_MASK_ANY)
	UAC_DEFINE_FLAG(firefox,	UAC_GROUP_MASK_ANY)
	UAC_DEFINE_FLAG(safari,		UAC_GROUP_MASK_ANY)
	UAC_DEFINE_FLAG(ie,		UAC_GROUP_MASK_windows)
	UAC_DEFINE_FLAG(gnome,		UAC_GROUP_MASK_other + UAC_GROUP_MASK_unix)
	UAC_DEFINE_FLAG(opera,		UAC_GROUP_MASK_ANY)
	UAC_DEFINE_FLAG(yabrowser,	UAC_GROUP_MASK_ANY)
	UAC_DEFINE_FLAG(corom,		UAC_GROUP_MASK_ANY)
	UAC_DEFINE_FLAG(maxthon,	UAC_GROUP_MASK_ANY)
	UAC_DEFINE_FLAG(konqueror,	UAC_GROUP_MASK_other + UAC_GROUP_MASK_unix)
	UAC_DEFINE_FLAG(links,		UAC_GROUP_MASK_other + UAC_GROUP_MASK_unix)

	UAC_DEFINE_FLAG(ie_old,		UAC_GROUP_MASK_windows)
	UAC_DEFINE_FLAG(ie_latest,	UAC_GROUP_MASK_windows)
	UAC_DEFINE_FLAG(ie_11,		UAC_GROUP_MASK_windows)
	UAC_DEFINE_FLAG(ie_10,		UAC_GROUP_MASK_windows)

	UAC_DEFINE_FLAG(maxthon_cloud,	UAC_GROUP_MASK_ANY)
	{ 0, NULL, 0 }
};

static MAFSA_automaton uac_ma[UAC_GROUP_MAX + 1] = { 0 };
static MAFSA_automaton uac_ma_base = 0;

inline static void fill_error(const char* fname, char* err_buf, unsigned err_buf_sz)
{
	if (err_buf)
	{
		snprintf(err_buf, err_buf_sz, "Can't load %s: (%d) %s", fname, errno, strerror(errno));
	}
}

int uac_init(const char *dict_dir, char* err_buf, unsigned err_buf_sz)
{
	char fname[1024];

	snprintf(fname, 1024, "%s/base.automaton", dict_dir);
	uac_ma_base = MAFSA_automaton_load_from_binary_file(fname, 0);

	if (!uac_ma_base)
	{
		fill_error(fname, err_buf, err_buf_sz);
		return -1;
	}

	snprintf(fname, 1024, "%s/g_other.automaton", dict_dir);
	uac_ma[UAC_GROUP_other] = MAFSA_automaton_load_from_binary_file(fname, 0);
	if (0)//!uac_ma[UAC_GROUP_MAX])
	{
		fill_error(fname, err_buf, err_buf_sz);
		return -1;
	}

	int i;
	for (i = 0; uac_std_flags[i].title; ++i)
	{
		if (uac_std_flags[i].groups) continue;

		snprintf(fname, 1024, "%s/%s.automaton", dict_dir, uac_std_flags[i].title);

		unsigned group_id = uac_std_flags[i].flag_id;
		uac_ma[group_id] = MAFSA_automaton_load_from_binary_file(fname, 0);

		if (0)//!uac_ma[group_id])
		{
			fill_error(fname, err_buf, err_buf_sz);

			uac_free();
			return -1;
		}
	}

	return 0;
}

void uac_free()
{
	MAFSA_automaton_close(uac_ma_base);
}

#define MAX_MAFSA_LEN 1024

struct uac_enum_working_s
{
	uint64_t flags_plus;
	uint64_t flags_minus;
};

static void MAFSACALL uac_determine_callback(void* user_data, const MAFSA_letter* l, size_t sz)
{
	if (!user_data) return;

	struct uac_enum_working_s* ws = (struct uac_enum_working_s*)user_data;

	size_t i = 0;
	while (i < sz)
	{
		if (MAX_LETTER_SLA == l[i++]) break;
	}

	uint32_t group_id = 0;
	uint32_t mul = 1;
	while ((MAX_LETTER_SLA != l[i]) && (i < sz))
	{
		group_id += mul * l[i++];
		mul *= MAX_LETTER_SLA;
	}

	if (MAX_LETTER_SLA != l[i++])
	{
		// no text string at the end? strange...
		return;
	}

	if (1 != mul)
	{
		if (LETTER_MINUS == l[i])
		{
			ws->flags_minus |= ((uint64_t)1 << group_id);
		}
		else
		{
			ws->flags_plus |= ((uint64_t)1 << group_id);
		}
	}
}

static int uac_do_classify(uac_result_t* ret, const MAFSA_letter* l, ssize_t ssz)
{
	MAFSA_letter tmp[MAX_MAFSA_LEN];

	struct uac_enum_working_s ws;
	memset(&ws, 0, sizeof(struct uac_enum_working_s));

	MAFSA_automaton_search_enumerate(uac_ma_base, l, ssz, tmp, MAX_MAFSA_LEN, &ws, MAX_LETTER_SLA, uac_determine_callback);

	uint64_t base_flags = (ws.flags_plus & ~ws.flags_minus);

	uint32_t group_id = 0;
	uint64_t it_flag = 1;
	while (it_flag)
	{
		if ((base_flags & it_flag) == base_flags) break;

		group_id++;
		it_flag <<= 1;
	}

	ret->group_id = group_id;
	ret->flags = 0;

	if (!uac_ma[group_id]) return -1;

	struct uac_enum_working_s ws2;
	memset(&ws2, 0, sizeof(struct uac_enum_working_s));

	MAFSA_automaton_search_enumerate(uac_ma[group_id], l, ssz, tmp, MAX_MAFSA_LEN, &ws2, MAX_LETTER_SLA, uac_determine_callback);

	uint64_t group_flags = (ws2.flags_plus & ~ws2.flags_minus);
	ret->flags = group_flags;

	return 0;
}

int uac_classify(uac_result_t* ret, const char* s)
{
	MAFSA_letter l[MAX_MAFSA_LEN];
	ssize_t ssz = conv_s2l_sla(s, l, 1024);

	if (ssz <= 0) return -1;

	return uac_do_classify(ret, l, ssz);
}

int uac_classifyi_binary(uac_result_t* ret, const char* s, size_t sz)
{
	MAFSA_letter l[MAX_MAFSA_LEN];
	ssize_t ssz = conv_b2l_sla(s, sz, l, 1024);

	if (ssz <= 0) return -1;

	return uac_do_classify(ret, l, ssz);
}

static int uac_bit_text_initiated = 0;

#define UAC_GROUP_COUNT			(UAC_GROUP_unix + 1)
#define UAC_FLAG_BITS_COUNT		64
#define UAC_MAX_FLAG_TITLE_STRLEN	32
static const char* uac_bit_texts[UAC_GROUP_COUNT][UAC_FLAG_BITS_COUNT] = { { 0 } };
static const char* uac_group_titles[UAC_GROUP_COUNT] = { 0 };

static void uac_bit_text_init()
{
	int i;
	int j;

	for (i = 0; uac_std_flags[i].title; ++i)
	{
		if (!uac_std_flags[i].groups)
		{
			uac_group_titles[uac_std_flags[i].flag_id] = uac_std_flags[i].title;
		}
		else
		{
			for (j = 0; j < UAC_GROUP_COUNT; ++j)
			{
				if ((uac_std_flags[i].groups & ((uint64_t)1 << j)))
				{
					uac_bit_texts[j][uac_std_flags[i].flag_id] = uac_std_flags[i].title;
				}
			}
		}
	}

	uac_bit_text_initiated = 1;
}

#define WRITE_STR(_s) \
{ \
	*ptr = 0; \
	if (_s) \
	{ \
		if (strlen(_s) + 1 > (sz - (ptr - buf))) return; \
		ptr = stpcpy(ptr,_s); \
	} \
}

void uac_hr_cl_result(uac_result_t* ret, char* buf, size_t sz)
{
	if (!uac_bit_text_initiated)
	{
		uac_bit_text_init();
	}

	if (!sz) return;

	char* ptr = buf;
	if (ret->group_id >= UAC_GROUP_COUNT)
	{
		WRITE_STR("unknown");
		return;
	}
	WRITE_STR(uac_group_titles[ret->group_id]);
	WRITE_STR(":");

	int ft = 1;
	int i;
	for (i = 0; i < UAC_FLAG_BITS_COUNT; ++i)
	{
		if ((ret->flags & ((uint64_t)1 << i)))
		{
			if (!ft) WRITE_STR(",");
			ft = 0;

			WRITE_STR(uac_bit_texts[ret->group_id][i]);
		}
	}
}

