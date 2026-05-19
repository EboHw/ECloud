#ifndef NET_H
#define NET_H

#include "protocol.h"

/* 表示客户端连接的所有状态 */
typedef struct {
	int			fd;
	char		username[MAX_USERNAME_LEN];
	char		virt_cwd[MAX_PATH_LEN];
	uint32_t	next_seq;
} conn_t;

/**
 * 初始化服务器：创建 socket、绑定、监听
 * @return 监听 socket fd，失败返回 -1
 */
int net_server_init(const char *ip, int port);

/**
 * 服务器主循环：不断 accept 新连接，每个连接创建新线程处理
 * @param listen_fd  由 net_server_init 返回的监听 fd
 */
void net_server_run(int listen_fd);

/**
 * 向客户端发送一个完整的响应包
 * @param conn    连接对象
 * @param cmd     命令码（与请求相同）
 * @param status  状态码
 * @param body    响应体数据（可以为 NULL）
 * @param body_len 响应体长度
 * @return 0=成功  -1=失败
 */
int net_send_response(conn_t *conn, uint16_t cmd, uint32_t status,
					  const void *body, uint32_t body_len);

/**
 * 从客户端接收一个完整的请求包
 * 内部处理粘包：先接收 24 字节包头，再根据 body_len 接收 body
 * @param fd   客户端 fd
 * @param pkt  输出参数，接收到的数据包（body 由本函数 malloc，调用方负责 free）
 * @return 0=成功  -1=对端关闭或出错
 */

int net_recv_packet(int fd, pkt_t *pkt);

#endif /* NET_H */
