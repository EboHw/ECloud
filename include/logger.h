#ifndef LOGGER_H
#define LOGGER_H

#include <syslog.h>

/* 日志级别枚举(于 syslog 优先级对齐)*/
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO  = 1,
    LOG_LEVEL_WARN  = 2,
    LOG_LEVEL_ERROR = 3
} log_level_t;

/**
 * 初始化日志系统
 * @param level  最低打印级别, 低于此级别的日志直接丢弃
 *               例: level=INFO, DEBUG 日志不打印 
 */
void logger_init(log_level_t level);

/**
 * 关闭日志系统(程序退出时调用)
 */
void logger_close(void);

/**
 * 日志宏 -- 调用防守与 printf 相同: 
 * log_info("用户 %s 连接, fd = %d", username, fd);
 * 
 * __FILE__ / __LINE__ / __func__: C语言内置宏, 自动展开为当前文件名,行号,函数名
 * 这让每条日志都可以定位到具体的代码位置
 */
#define log_debug(fmt, ...) \
    _log_write(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define log_info(fmt, ...) \
    _log_write(LOG_LEVEL_INFO,  __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define log_warn(fmt, ...) \
    _log_write(LOG_LEVEL_WARN,  __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define log_error(fmt, ...) \
    _log_write(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

/* 内部函数, 不要直接调用, 通过上面的宏使用*/
void _log_write(log_level_t, const char *file, int line,
                const char *func, const char *fmt, ...);

#endif /* LOGGER_H */