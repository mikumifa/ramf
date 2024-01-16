#include "ramfs.h"
#include "shell.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

int notin(int fd, int *fds, int n) {
    for (int i = 0; i < n; i++) {
        if (fds[i] == fd) return 0;
    }
    return 1;
}

int genfd(int *fds, int n) {
    for (int i = 0; i < 4096; i++) {
        if (notin(i, fds, n))
            return i;
    }
    return -1;
}

int main() {
    init_ramfs();
    // 在main函数中添加以下代码进行测试
    int fd_append;
    fd_append = ropen("/append_test", O_CREAT | O_WRONLY | O_APPEND);
    assert(fd_append >= 0);

// 进行写入测试
    assert(rwrite(fd_append, "hello", 5) == 5);
    assert(rwrite(fd_append, "world", 5) == 5);

// 关闭并重新打开文件以进行读取
    assert(rclose(fd_append) == 0);
    fd_append = ropen("/append_test", O_RDONLY);
    assert(fd_append >= 0);

// 读取并验证内容
    char read_buf[11];
    memset(read_buf, 0, sizeof(read_buf));
    assert(rread(fd_append, read_buf, 10) == 10);
    assert(memcmp(read_buf, "helloworld", 10) == 0);

// 关闭文件
    assert(rclose(fd_append) == 0);

    return 0;
}
