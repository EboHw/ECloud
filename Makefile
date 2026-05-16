CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -g -D_GNU_SOURCE -I include
LDFLAGS = -lpthread

# 定义存放中间文件和最终可执行文件的目录
OBJ_DIR = obj
BIN_DIR = bin

SERVER_SRCS = src/main.c \
              src/config.c \
              src/logger.c \
              src/net.c \
              src/command.c \
              src/storage.c \
              src/utils/str_utils.c

CLIENT_SRCS = client/main.c \
              client/cmd_parser.c \
              src/config.c \
              src/logger.c \
              src/utils/str_utils.c

SERVER_OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SERVER_SRCS))
CLIENT_OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(CLIENT_SRCS))

# 定义最终生成的目标程序路径
SERVER_TARGET = $(BIN_DIR)/file_server
CLIENT_TARGET = $(BIN_DIR)/file_client

# 默认编译所有目标
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# 编译服务端，加入创建 bin 目录的逻辑
$(SERVER_TARGET): $(SERVER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 编译客户端，加入创建 bin 目录的逻辑
$(CLIENT_TARGET): $(CLIENT_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# 编译 .o 文件的通用规则
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# 清理时同时删掉 obj 和 bin 目录
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# 运行规则需要同步更新路径
run-server: $(SERVER_TARGET)
	./$(SERVER_TARGET) -c config/server.ini

run-client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET) -c config/server.ini

.PHONY: all clean run-server run-client
