#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

/* ========== 魔数与版本 ========== */
#define PKT_MAGIC		0xF5ED0001u
#define PKT_VERSION		1

/* ========== 命令码 ========== */
typedef enum {
    /* 会话管理 */
    CMD_LOGIN       = 0x0001,   /* 登录（第一期：发用户名字符串）*/
    CMD_LOGOUT      = 0x0002,   /* 退出 */

    /* 短命令（目录操作）*/
    CMD_LS          = 0x0010,   /* 列出目录 */
    CMD_CD          = 0x0011,   /* 切换目录 */
    CMD_PWD         = 0x0012,   /* 显示当前目录 */
    CMD_MKDIR       = 0x0013,   /* 创建目录 */
    CMD_RM          = 0x0020,   /* 删除文件 */
    CMD_STAT        = 0x0014,   /* 查看文件信息 */

    /* 长命令（文件传输）*/
    CMD_PUTS_INIT   = 0x0030,   /* 上传：初始化 */
    CMD_PUTS_CHUNK  = 0x0031,   /* 上传：数据块 */
    CMD_PUTS_FINISH = 0x0032,   /* 上传：完成 */
    CMD_GETS_INIT   = 0x0040,   /* 下载：初始化 */
    CMD_GETS_CHUNK  = 0x0041,   /* 下载：数据块 */

    CMD_HEARTBEAT   = 0x00FF,   /* 心跳 */
} cmd_code_t;

/* ==================== 包头结构 ==================== */
/*
 * __attribute__((packed))：告诉编译器不要填充对齐字节
 * 保证这个结构体在内存中是连续的 24 字节，可以直接 send/recv
 *
 * 包头 24 字节布局：
 *   [0..3]   magic     4字节
 *   [4..5]   version   2字节
 *   [6..7]   cmd       2字节
 *   [8..11]  seq       4字节
 *   [12..15] body_len  4字节
 *   [16..19] status    4字节（响应包用，请求包填0）
 *   [20..21] flags     2字节
 *   [22..23] reserved  2字节
 */
typedef struct __attribute__((packed)) {
    uint32_t magic;       /* 魔数 PKT_MAGIC */
    uint16_t version;     /* 协议版本 */
    uint16_t cmd;         /* 命令码 */
    uint32_t seq;         /* 序列号（请求和响应的 seq 相同，方便匹配）*/
    uint32_t body_len;    /* Body 数据的字节数（不含包头）*/
    uint32_t status;      /* 状态码（响应包使用，请求包填 0）*/
    uint16_t flags;       /* 标志位（第一期暂时不用，填 0）*/
    uint16_t reserved;    /* 保留（填 0）*/
} pkt_header_t;

#define PKT_HEADER_SIZE  sizeof(pkt_header_t)   /* = 24 字节 */
#define PKT_MAX_BODY     (64 * 1024 * 1024)     /* Body 最大 64MB */

/* ==================== 完整数据包 ==================== */
typedef struct {
    pkt_header_t header/* PROTOCOL_H */;
    uint8_t     *body;    /* 指向 body 数据（动态分配）*/
} pkt_t;

/* ==================== 常量 ==================== */
#define MAX_USERNAME_LEN  64
#define MAX_PATH_LEN      512
#define MAX_FILENAME_LEN  256
#define CHUNK_SIZE        (4 * 1024 * 1024)   /* 文件分块大小 4MB */

#endif /* PROTOCOL_H */
