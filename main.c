#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ramfs.h"
#include "ramfs.h"

int main() {
    init_ramfs(); // 初始化你的文件系统

    // 测试目录和文件的创建、打开和关闭
    int fd[10];
    char buf[1024];
    memset(buf, 0, sizeof(buf));

    // 创建目录
    assert(rmkdir("/dir") == 0);
    assert(rmkdir("/dir/subdir") == 0);

    // 在目录中创建文件
    assert((fd[0] = ropen("/dir/file0", O_CREAT | O_WRONLY)) >= 0);
    assert((fd[1] = ropen("/dir/subdir/file1", O_CREAT | O_WRONLY)) >= 0);

    // 随机写入数据
    for (int i = 0; i < 100; i++) {
        sprintf(buf, "RandomData%d", rand());
        assert(rwrite(fd[i % 2], buf, strlen(buf)) == strlen(buf));
    }

    // 关闭文件
    assert(rclose(fd[0]) == 0);
    assert(rclose(fd[1]) == 0);

    // 重新打开文件进行读取测试
    assert((fd[0] = ropen("/dir/file0", O_RDONLY)) >= 0);
    assert((fd[1] = ropen("/dir/subdir/file1", O_RDONLY)) >= 0);

   

    // 关闭文件
    assert(rclose(fd[0]) == 0);
    assert(rclose(fd[1]) == 0);

    // 尝试打开不存在的文件
    assert(ropen("/dir/nonexistent", O_RDONLY) == -1);

    // 尝试在非法路径下创建文件
    assert(ropen("/nonexistent_dir/file", O_CREAT | O_WRONLY) == -1);

    // 尝试非法操作，比如在文件上执行目录操作
    assert(rmkdir("/dir/file0") == -1);

    // 测试边界条件，如空文件的读写
    assert((fd[2] = ropen("/dir/emptyfile", O_CREAT | O_WRONLY)) >= 0);
    assert(rwrite(fd[2], "", 0) == 0);
    assert(rclose(fd[2]) == 0);

    assert((fd[2] = ropen("/dir/emptyfile", O_RDONLY)) >= 0);
    assert(rread(fd[2], read_buf, sizeof(read_buf)) == 0);
    assert(rclose(fd[2]) == 0);

// 测试大量写入和读取
    assert((fd[3] = ropen("/dir/largefile", O_CREAT | O_WRONLY)) >= 0);
    for (int i = 0; i < 10000; i++) {
        assert(rwrite(fd[3], "LargeFileData", 13) == 13);
    }
    assert(rclose(fd[3]) == 0);

    assert((fd[3] = ropen("/dir/largefile", O_RDONLY)) >= 0);
    for (int i = 0; i < 10000; i++) {
        assert(rread(fd[3], read_buf, 13) == 13);
        assert(memcmp(read_buf, "LargeFileData", 13) == 0);
    }
    assert(rclose(fd[3]) == 0);

// 测试文件的截断和扩展
    assert((fd[4] = ropen("/dir/modfile", O_CREAT | O_WRONLY)) >= 0);
    assert(rwrite(fd[4], "InitialContent", 14) == 14);
    assert(rclose(fd[4]) == 0);

// 重新打开文件进行截断
    assert((fd[4] = ropen("/dir/modfile", O_WRONLY)) >= 0);
    assert(rwrite(fd[4], "Short", 5) == 5);
    assert(rclose(fd[4]) == 0);

// 验证内容被正确截断
    assert((fd[4] = ropen("/dir/modfile", O_RDONLY)) >= 0);
    memset(read_buf, 0, sizeof(read_buf));
    assert(rread(fd[4], read_buf, sizeof(read_buf)) == 5);
    assert(memcmp(read_buf, "Short", 5) == 0);
    assert(rclose(fd[4]) == 0);


    return 0;
}