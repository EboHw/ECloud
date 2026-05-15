#ifndef ECLOUD_PROTOCOL_H
#define ECLOUD_PROTOCOL_H

#include <cstdint>
#include <iterator>
#include <stdint.h>

// Magic Number
#define ECLOUD_MAGIC 0x45434C44

#define ECLOUD_VERSION 1

typedef enum {
	CMD_LOGIN	= 1,	// Login
	CMD_LS		= 2,	// ls
	CMD_CD		= 3,	// cd
	CMD_PWD		= 4,	// pwd
	CMD_PUTS	= 5,	// 上传文件(长任务)
	CMD_GETS	= 6,	// 下载文件(长任务)
	CMD_RM		= 7,	// rm
	CMD_MKDIR	= 8,	// mkdir
	CMD_ERROR	= 99	// error_num
} ecloud_cmd_t;

/**
 * 协议包头结构体
 * 采用 __attribute__((packed)) 强制字节对齐为 1
 */
typedef struct __attribute__((packed)) {
	uint32_t magic;
	uint16_t version;
	uint16_t cmd_type;
	uint32_t body_len;
} ecloud_pkt_header_t;

#endif
