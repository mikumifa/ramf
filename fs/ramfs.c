#include "ramfs.h"
#include <assert.h>
#include <ctype.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>

node *root = NULL;

#define NRFD 4096
FD fdesc[NRFD];
//tools
char *strdup(const char *s) {
    if (s == NULL) {
        return NULL;
    }
    char *duplicate = malloc(strlen(s) + 1);
    if (duplicate == NULL) {
        return NULL;
    }
    strcpy(duplicate, s);
    return duplicate;
}

node *find(const char *pathname) {
    // NULL
    if (pathname == NULL || root == NULL) {
        return NULL;
    }

    // len==0
    if (strlen(pathname) == 0) {
        return NULL;
    }

    // Root
    if (strcmp(pathname, "/") == 0) {
        return root;
    }
    // 第一个一定是根目录
    if (pathname[0] != '/') {
        return NULL;
    }

    // 路径
    char *path = strdup(pathname);

    //开始读取
    char *token = strtok(path, "/");
    node *current = root;
    while (token != NULL) {
        bool found = false;
        for (int i = 0; i < current->dir_num && current->type == DIR_NODE; ++i) {
            if (strcmp(current->dirs[i]->name, token) == 0) {
                current = current->dirs[i];
                found = true;
                break;
            }
        }

        if (!found) {
            free(path);
            return NULL; // Node not found in path
        }

        token = strtok(NULL, "/");
    }

    free(path);
    return current; // Node found
}

int ropen(const char *pathname, int flags) {

}

int rclose(int fd) {

}

ssize_t rwrite(int fd, const void *buf, size_t count) {

}

ssize_t rread(int fd, void *buf, size_t count) {

}

off_t rseek(int fd, off_t offset, int whence) {

}

int rmkdir(const char *pathname) {

}

int rrmdir(const char *pathname) {

}

int runlink(const char *pathname) {

}

void init_ramfs() {
    // 分配内存
    root = (node *) malloc(sizeof(node));
    root->type = DIR_NODE;
    // D
    root->dirs = NULL;
    root->dir_num = 0;

    // F
    root->content = NULL;
    root->size = 0;
    // 设置目录名称为 "/"
    root->name = strdup("/");
}

//递归的函数
void free_node(node *n) {
    if (n == NULL) {
        return;
    }
    // 目录节点，释放子节点
    if (n->type == DIR_NODE) {
        for (int i = 0; i < n->dir_num; ++i) {
            free_node(n->dirs[i]);
        }
        // 释放子节点数组
        free(n->dirs);
    } else {
        // 文件节点，释放内容
        free(n->content);
    }
    // 名称
    free(n->name);
    //本身
    free(n);
}

void close_ramfs() {
    free_node(root);
    root=NULL;
}

