#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <syslog.h>

/* 当前全局日志级别(低于此级别的日志直接返回) */
static log_level_t g_log_level = LOG_LEVEL_INFO;

static const char *LEVEL_NAMES[] = { "DEBUG", "INFO", "WARN", "ERROR" };

static const int SYSLOG_PRIORITY[] = {
    LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERR
};

void logger_init(log_level_t level) {
    g_log_level = level;
    /*
     * openlog 参数说明：
     *   "file_server" - 日志前缀标识，syslog 中能搜到这个前缀
     *   LOG_PID       - 每条日志附上进程ID，方便区分多进程
     *   LOG_CONS      - syslog 不可用时输出到控制台
     *   LOG_LOCAL0    - 使用 local0 设施（配合 rsyslog 分文件记录）
     */
    openlog("file_server", LOG_PID | LOG_CONS, LOG_LOCAL0);
    log_info("日志系统初始化完成, 级别: %s", LEVEL_NAMES[level]);
}

void logger_close(void) {
    closelog();
}

void _log_write(log_level_t level, const char *file, int line,
                const char *func, const char *fmt, ...) {
    /* 级别过滤：低于设定级别的直接丢弃 */
    if (level < g_log_level)    { return;}
    
    /* 1. 获取当前时间字符串 */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    /* 2. 格式化用户消息 */
    char msg_buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
    va_end(args);

    /*
     * 3. 提取文件名（去掉路径前缀）
     * 例：__FILE__ = "src/storage.c" → 只取 "storage.c"
     */
    const char *filename = strrchr(file, '/');
    filename = filename ? filename + 1 : file;

    /* 4. 同时输出到：stderr（实时看）+ syslog（持久化）*/
    char full_msg[1200];
    snprintf(full_msg, sizeof(full_msg), "[%s] [%s] [%s:%d %s()] %s",
             time_buf, LEVEL_NAMES[level], filename, line, func, msg_buf);

    fprintf(stderr, "%s\n", full_msg);                       /* 终端实时输出 */
    syslog(SYSLOG_PRIORITY[level], "%s", full_msg);          /* 系统日志持久化 */
}