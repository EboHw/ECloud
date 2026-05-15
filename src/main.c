#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

int main() {
	printf("--- E-Cloud Server Starting ---\n");

    // 1. 验证协议头大小（确保对齐正确）
    printf("[DEBUG] Protocol Header Size: %zu bytes\n", sizeof(ecloud_pkt_header_t));

    if (sizeof(ecloud_pkt_header_t) != 12) {
        fprintf(stderr, "[ERROR] Protocol alignment error! Expected 12 bytes.\n");
        return EXIT_FAILURE;
    }

    // 2. 模拟读取配置（下一节我们会写真正的 INI 解析）
    printf("[INFO] Loading config from config/server.ini...\n");

    // 3. 打印魔数验证（测试引用是否成功）
    printf("[INFO] Expected Magic: 0x%X\n", ECLOUD_MAGIC);

    printf("--- Server Ready ---\n");
    return 0;
}
