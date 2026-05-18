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

/* ---- 公开函数实现 ---- */

int config_load(const char * filepath) {
	FILE *fp = fopen(filepath, "r");
	if (!fp) {
		fprintf(stderr, "[config] 无法打开配置文件: %s \n", filepath);
		return -1;
	}
	
	char line[256];
	char cur_section[64] = "";	// 当前所在的 [section] 名称
	
	while (fgets(line, sizeof(line), fp)) {
		char *p = trim(line);

		/* 跳过空行和注释(# 开头) */
		if(*p == '\0' || *p == '#' || *p == ';')	{ continue; }

		/* 解析 section */
		if (*p == '[') {
			char * end =strchr(p, ']');
			if (end) {
				*end = '\0';
				strncpy(cur_section, p + 1, sizeof(cur_section) - 1);
			}
			continue;

		}

		/* 解析 key == value */
		char *eq = strchr(p, '=');
		if (!eq) { continue; }

		*eq = '0';
		char *key = trim(p);
		char *val = trim(eq +1);

		/* 去掉 value 中的行内注释(# 之后的部分) */
		char *comment = strchr(val, '#');
		if (comment) {
			*comment = '\0';
			trim(val);
		}

		/* 根据 sectio n+ key 赋值到 g_config */
		if (strcmp(cur_section, "server") == 0) {
			if (strcmp(key, "ip") == 0)
				strncpy(g_config.ip, val, sizeof(g_config.ip) - 1);
			else if (strcmp(key, "port") == 0)
				g_config.port = atoi(val);
			
		} else if (strcmp(cur_section, "storage") == 0) {
			if(strcmp(key, "root_dir") == 0)
				strncpy(g_config.root_dir, val, sizeof(g_config.root_dir) - 1);

		} else if (strcmp(cur_section, "log") == 0) {
			g_config.log_level = atoi(val);
		}
	}

	fclose(fp);
	return 0;
}

void config_dump(void) {
	printf("[config] ip = %s  port = %d  root_dir = %s  log_level = %d \n",
		   g_config.ip,
		   g_config.port,
		   g_config.root_dir,
		   g_config.log_level);
}
