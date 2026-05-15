# 1. 编译器与选项
CC = gcc
CFLAGS = -Wall -Wextra -g -I./include -I/usr/include/mysql
# LDFLAGS = -lmysqlclient -lssl -lcrypto -ljwt -lpthread
LDFLAGS = -lmysqlclient -lssl -lcrypto -lpthread

# 2. 目录定义
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# 3. 目标文件定义
# 假设你的服务端主程序是 src/server_main.c，客户端是 src/client_main.c
SERVER_TARGET = $(BIN_DIR)/file_server
CLIENT_TARGET = $(BIN_DIR)/file_client

# 自动获取所有的 .c 文件，但排除掉 client 相关的
SERVER_SRCS = $(shell find $(SRC_DIR) -name "*.c" ! -name "client_*.c")
# 假设客户端代码都以 client_ 开头
CLIENT_SRCS = $(SRC_DIR)/client_main.c $(SRC_DIR)/protocol.c $(SRC_DIR)/utils.c

# 将 .c 文件路径替换为 .o 路径
SERVER_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRCS))
CLIENT_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLIENT_SRCS))

# 4. 伪目标
.PHONY: all clean setup

all: setup $(SERVER_TARGET) $(CLIENT_TARGET)

# 创建必要的目录
setup:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# 5. 链接服务端
$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(SERVER_OBJS) -o $@ $(LDFLAGS)
	@echo "Successfully built Server: $@"

# 6. 链接客户端
$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CLIENT_OBJS) -o $@ $(LDFLAGS)
	@echo "Successfully built Client: $@"

# 7. 编译源文件为目标文件 (.c -> .o)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# 8. 清理产物
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned up all build artifacts."
