#ifndef CONFIG_H
#define CONFIG_H

/* 存放从 server.ini 读取的配置的结构体 */

typedef struct {
	/* [server] */
	char ip[64];
	int port;

	/* [storage] */
	char root_dir[256];		// 文件存储的根目录
	
	/* [log] */
	int log_level;			// 0=DEBUG 1=INFO 2=WARN 3=ERROR
} server_config_t;

/* extern 声明：实际定义在 config.c 中 */
extern server_config_t g_config;

/**
 * 从 ini 文件中加载配置
 * @param filepath "config/server.ini"
 * @return 0 = 成功 -1 = 失败(文件不存在或格式错误)
 */
int config_load(const char * filepath);

/* 打印当前配置到标准输出(调试用) */
void config_dump(void);

#endif /* CONFIG_H */


