#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* 全局配置实例，初始化为默认值 */
server_config_t g_config = {
	.ip			= "0.0.0.0",
	.port		= 8888,
	.root_dir	= "./storage",
	.log_level	= 1
};

/* ---- 内部辅助函数 ----*/

/* 去掉字符串首尾的空白，返回处理后的指针(在原字符串上操作) */
static char *trim(char * s) {
	/* 去掉头部空格 */
	while (*s && isspace((unsigned char)* s)){ s++; }
	if (*s == '\0') return s;
	/* 去掉尾部空格 */
	char * end = s + strlen(s) - 1;
	while (end > s && isspace((unsigned char)* end)) { end--;  }
	*(end + 1) = '\0';
	return s;
}
