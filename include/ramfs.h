#pragma once

#include <stdint.h>
#include <stdbool.h>

#define O_APPEND 02000
#define O_CREAT 0100
#define O_TRUNC 01000
#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

//在内存文件系统中，你需要一个数据结构来存储一个文件的信息（我们沿用了Linux中“Everything is a file”的设计），
// 包括这个文件的类型（是普通文件还是目录），名称；如果它是一个普通文件，则它的文件内容、文件大小也值得我们关注；
// 如果它是一个目录，则它有哪些子节点也值得我们关注。
// 这里我们给出一个可供参考的数据结构，它对应了上文中我们关注的文件信息，
// 你可以试着自己理解这些字段的含义，也可以根据自己的需要设计自己的数据结构：
typedef struct node {
    enum {
        FILE_NODE, DIR_NODE
    } type;
    struct node **dirs; // if DTYPE
    int dir_num;
    void *content;
    char *name;
    int size;
} node;

typedef struct FD {
    bool used; // 对应的文件
    int offset; //偏移量
    int flags; //读写性质（支持对文件进行的操作，例如只读、只写等）
    node *f;
} FD;

typedef intptr_t ssize_t; //有符号指针
typedef uintptr_t size_t; //无符号指针
typedef long off_t; //偏移量的类型，

int ropen(const char *pathname, int flags);

int rclose(int fd);

ssize_t rwrite(int fd, const void *buf, size_t count);

ssize_t rread(int fd, void *buf, size_t count);

off_t rseek(int fd, off_t offset, int whence);

int rmkdir(const char *pathname);

int rrmdir(const char *pathname);

int runlink(const char *pathname);

void init_ramfs();

void close_ramfs();

node *find(const char *pathname);

char *strdup(const char *);

void free_node(node *pre, node *n);

node *getPrePath(const char *pathname);


int split_pathname(const char *pathname);

int is_valid_char(char c);

int is_vaild_str(const char *str);

int findFirstLastPathPos(const char *);