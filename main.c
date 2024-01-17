#include "ramfs.h"
#include "shell.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

const char *content = "export PATH=/usr/bin/\n";
const char *ct = "export PATH=/home:$PATH";

int main() {
    init_ramfs();
    for (int i = 0; i < 100000; ++i) {
        char str[50]; // 存储转换结果的字符数组，长度要足够大

        // 使用sprintf函数进行转换
        // %d表示转换为十进制整数，str是存储结果的字符数组
        sprintf(str, "%d", i);
        assert(rmkdir(str) == 0);
    }

    close_ramfs();
}
