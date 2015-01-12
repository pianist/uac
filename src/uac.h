#ifndef __UAC_H__USER_AGENT_CLASSIFIER__
#define __UAC_H__USER_AGENT_CLASSIFIER__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UAC_GROUP_other		0
#define UAC_GROUP_android	1
#define UAC_GROUP_windows	2
#define UAC_GROUP_apple		3
#define UAC_GROUP_unix		4

#define UAC_GROUP_unknown	64
#define UAC_GROUP_MAX		UAC_GROUP_unknown

/* common flags */
#define UAC_FLAG_phone		0
#define UAC_FLAG_tablet		1
#define UAC_FLAG_tv		2

#define UAC_MASK_device		((1 << UAC_FLAG_phone) + (1 << UAC_FLAG_tablet) + (1 << UAC_FLAG_tv))

#define UAC_FLAG_64bit		4
#define UAC_FLAG_arm		5

/* windows versions */
#define UAC_FLAG_win_old	8
#define UAC_FLAG_win_xp		9
#define UAC_FLAG_win_vista	10
#define UAC_FLAG_win_7		11
#define UAC_FLAG_win_8		12
#define UAC_FLAG_win_10		13
#define UAC_MASK_win_os_ver	(((uint64_t)1 << UAC_FLAG_win_old) + ((uint64_t)1 << UAC_FLAG_win_xp) + ((uint64_t)1 << UAC_FLAG_win_vista) + ((uint64_t)1 << UAC_FLAG_win_7) + ((uint64_t)1 << UAC_FLAG_win_8) + ((uint64_t)1 << UAC_FLAG_win_10))

/* unix oses (x11 or console based) */
#define UAC_FLAG_linux		8
#define UAC_FLAG_bsd		9
#define UAC_FLAG_cros		10

/* linux distros */
#define UAC_FLAG_debian		16
#define UAC_FLAG_ubuntu		17
#define UAC_FLAG_gentoo		18

/* Apple */
#define UAC_FLAG_ipod		8
#define UAC_FLAG_iphone		9
#define UAC_FLAG_ipad		10
#define UAC_FLAG_mac		11

/* mobiles */
#define UAC_FLAG_symbian	24
#define UAC_FLAG_nokia		25
#define UAC_FLAG_blackberry	26
#define UAC_FLAG_samsung	27

/* browsers */
#define UAC_FLAG_chrome		40
#define UAC_FLAG_firefox	41
#define UAC_FLAG_safari		42
/* IE and Gnome (Galeon,Epiphany,Web...) have the same flag id, because IE is win browser, and Gnome browsers are X11 */
#define UAC_FLAG_ie		43
#define UAC_FLAG_gnome		43
#define UAC_FLAG_opera		44
#define UAC_FLAG_yabrowser	45
#define UAC_FLAG_corom		46
#define UAC_FLAG_maxthon	47
#define UAC_FLAG_konqueror	48
#define UAC_FLAG_links		49
#define UAC_MASK_browsers	(((uint64_t)1 << UAC_FLAG_chrome) + ((uint64_t)1 << UAC_FLAG_firefox) + ((uint64_t)1 << UAC_FLAG_safari) + ((uint64_t)1 << UAC_FLAG_ie) + ((uint64_t)1 << UAC_FLAG_opera) + ((uint64_t)1 << UAC_FLAG_yabrowser) + ((uint64_t)1 << UAC_FLAG_corom) + ((uint64_t)1 << UAC_FLAG_maxthon) + ((uint64_t)1 << UAC_FLAG_konqueror) + ((uint64_t)1 << UAC_FLAG_links))

/* ie flags */
#define UAC_FLAG_ie_latest	56
#define UAC_FLAG_ie_old		57
#define UAC_FLAG_ie_11		58
#define UAC_FLAG_ie_10		59

/* Maxthon browser flags */
#define UAC_FLAG_maxthon_cloud	56



int uac_init(const char *dict_dir, char* err_buf, unsigned err_buf_sz);
void uac_free();

//???#define MAX_UAC_RESULT_STRINGS 64
struct uac_result_s
{
	uint32_t group_id;
	uint64_t flags;
//???	const char* txt_flags[MAX_UAC_RESULT_STRINGS];
};

typedef struct uac_result_s uac_result_t;

int uac_classify(uac_result_t* ret, const char* s);
int uac_classifyi_binary(uac_result_t* ret, const char* s, size_t sz);

void uac_hr_cl_result(uac_result_t* ret, char* buf, size_t sz);


/* ----------------------------------- */

struct uac_std_flag_title_s
{
	uint32_t flag;
	const char* title;
	uint32_t is_group;
};

typedef struct uac_std_flag_title_s uac_std_flag_title_t;
extern uac_std_flag_title_t uac_std_flags[];

#ifdef __cplusplus
}
#endif

#endif /* __UAC_H__USER_AGENT_CLASSIFIER__ */
