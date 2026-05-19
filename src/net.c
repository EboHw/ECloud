#include "net.h"
#include "logger.h"
#include "command.h"
#include "protocol.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>

/* ---- 内部辅助, 可靠收发 ---- */

/**
 * 可靠接收：循环调用 recv 直到收够 n 字节，或对端关闭
 * 为什么需要这个？
 *   TCP 是流式协议，一次 recv 可能只收到部分数据
 *   必须循环调用直到收到足够字节，否则会丢数据
 */
static ssize_t recv_all(int fd, void *buf, size_t n) {
	size_t recv_len = 0;
	while (recv_len < n) {
		ssize_t ret_recv = recv(fd, (char *)buf + recv_len, n - recv_len, 0);
		if (ret_recv <= 0) {
			/* r==0, 对端正常关闭; r<0: 出错 */
			return ret_recv == 0 ? -1 : -1;
		}
		recv_len += ret_recv;
	}
	return (ssize_t)recv_len;
}

/**
 * 可靠发送: 循环调用 send 直到发完所有数据
 */
static ssize_t send_all(int fd, const void *buf, size_t n) {
	size_t send_len = 0;
	while (send_len < n) {
		ssize_t ret_send = send(fd, (const char *)buf + send_len, n - send_len, 0);
		if (ret_send < 0)	{ return -1; }
		send_len += ret_send;
	}
	return (ssize_t)send_len;
}

/* ---- 公开函数实现 ---- */

int net_recv_packet(int fd, pkt_t *pkt) {
	/* 1. 接收固定的 24byte 包头 */
	if (recv_all(fd, &pkt->header, PKT_HEADER_SIZE) < 0) {
		log_debug("fd=%d 接收包头失败, 连接断开", fd);
		return -1;
	}

	/* 2. 校验魔数 */
	if (pkt->header.magic != PKT_MAGIC) {
		log_warn("fd=%d 魔数不匹配: 0x%08X(期望,0x%08X)",
				 fd,
				 pkt->header.magic,
				 PKT_MAGIC);
		return -1;
	}

	/* 3. 根据 body_len 接收 body */
	uint32_t body_len = pkt->header.body_len;
	pkt->body = NULL;

	if (body_len > 0) {
		if (body_len > PKT_MAX_BODY) {
			log_warn("fd=%d body_len=%u 超过最大限制", fd, body_len);
			return -1;
		}				
		pkt->body = malloc(body_len + 1);
		if (!pkt->body) {
			log_error("malloc(%u)失败", body_len);
			return -1;
		}
		pkt->body[body_len] = '\0';

		if (recv_all(fd, pkt->body, body_len) <0 ) {
			log_debug("fd=%d 接收 body 失败", fd);
			free(pkt->body);
			pkt->body = NULL;
			return -1;
		}
	}

	log_debug("fd=%d 收到包: cmd=0x%04X seq=%u body_len=%u",
			  fd, pkt->header.cmd, pkt->header.seq, body_len);
	return 0;
}

int net_send_response(conn_t *conn, uint16_t cmd, uint32_t status, 
					  const void *body, uint32_t body_len) {
	pkt_header_t hdr = {
		.magic		= PKT_MAGIC,
		.version	= PKT_VERSION,
		.cmd		= cmd,
		.seq		= conn->next_seq++,
		.body_len	= body_len,
		.status		= status,
		.flags		= 0,
		.reserved	= 0
	};

	/* 先发包头, 再发 body(body 可以为 NULL) */
	if (send_all(conn->fd, &hdr, PKT_HEADER_SIZE) < 0) {
		log_error("fd=%d 发送包头失败: %s", conn->fd, strerror(errno));
		return -1;
	}

	if (body && body_len > 0) {
		if (send_all(conn->fd, body, body_len) < 0) {
			log_error("fd=%d 发送 body 失败: %s", conn->fd, strerror(errno));
			return -1;
		}
	}

	log_debug("fd=%d 发送响应: cmd=0x%04X status=%u body_len=%u",
			  conn->fd, cmd, status, body_len);
	return 0;
}

/* ---- 每个客户端的处理线程 ---- */

/**
 * 每个新连接都会在这个函数中运行，直到连接断开
 * 参数是 malloc 出来的 conn_t 指针, 本函数负责 free
 */
static void *client_thread(void *arg) {
	conn_t *conn = (conn_t *)arg;
	log_info("新连接处理线程启动: fd=%d", conn->fd);

	/* 初始化虚拟工作目录为根目录 */
	strncpy(conn->virt_cwd, "/", sizeof(conn->virt_cwd) - 1);
	conn->next_seq = 0;

	/* 命令循环：不断接收包，分发处理，发回响应 */
	pkt_t pkt;
	while (1) {
		memset(&pkt, 0, sizeof(pkt));

		/* 接收一个完整的数据包 */
		if (net_recv_packet(conn->fd, &pkt) < 0) {
			log_info("fd=%d 用户 %s 断开连接", conn->fd, conn->username);
			break;
		}

		/* 把数据包交给命令分层处理 */
		command_dispatch(conn, &pkt);

		/* 释放本次包的 body 内存 */
		if (pkt.body) {
			free(pkt.body);
			pkt.body = NULL;
		}	
	}
	/* 清理：关闭 fd，释放 conn */
	close(conn->fd);
	free(conn);
	log_debug("客户端线程退出");
	return NULL;
}

/* ---- 服务器初始化与主循环 ---- */

int net_server_init(const char *ip, int port) {
	/* 1. 创建 TCP Socket */
	int listen_fd =socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
        log_error("socket() 失败: %s", strerror(errno));
        return -1;
    }

	/** 2. 设置 SO_REUSEADDR
     *     作用：服务器重启后，即使 TIME_WAIT 状态的连接还没消失，也能立即重新绑定端口
     *     如果不设这个，重启服务器时会报 "Address already in use"
     */
	int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	/* 3. 绑定 IP 和端口 */
	struct sockaddr_in addr = {
		.sin_family			= AF_INET,
		.sin_port			= htons(port),
		.sin_addr.s_addr	= inet_addr(ip) 
	};

	if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		log_error("bind() %s:%d 失败: %s", ip, port, strerror(errno));
        close(listen_fd);
        return -1;
	}

	/** 4. 开始监听
     *     SOMAXCONN：系统允许的最大 backlog 数（通常是 128 或 4096）
     */
    if (listen(listen_fd, SOMAXCONN) < 0) {
        log_error("listen() 失败: %s", strerror(errno));
        close(listen_fd);
        return -1;
    }

    log_info("服务器启动成功，监听 %s:%d", ip, port);
    return listen_fd;
}

void net_server_run(int listen_fd) {
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);

	log_info("等待客户端连接...");

	while (1) {
		/* accept 会阻塞，直到有新客户端连接进来 */
		int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addr_len);

		if (client_fd < 0) {
			log_error("accept() 失败: %s", strerror(errno));
			continue;
		}

		/* 打印客户端的 IP 和 Port */
		char client_ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
		log_info("新客户端连接: %s:%d  fd=%d",
                 client_ip, ntohs(client_addr.sin_port), client_fd);

		/* 为这个连接分配 conn_t, 并创建线程 */
		conn_t *conn = calloc(1,sizeof(conn_t));
		if (!conn) {
			log_error("calloc conn_t 失败");
            close(client_fd);
            continue;
		}
		conn->fd = client_fd;

		pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, conn) != 0) {
            log_error("pthread_create 失败: %s", strerror(errno));
            close(client_fd);
            free(conn);
            continue;
        }

		/* detach：线程结束后自动回收资源，主线程不用 join */
        pthread_detach(tid);
	} 

}
