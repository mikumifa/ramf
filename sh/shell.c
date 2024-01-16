#include "ramfs.h"
#include "shell.h"

#ifndef ONLINE_JUDGE
#define print(...) printf("\033[31m");printf(__VA_ARGS__);printf("\033[0m");
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define PATH_LEN 2048
typedef struct PathNode {
    char *path;
    struct PathNode *next;
} PathNode;
PathNode *pathHead = NULL;

void addPathNode(PathNode **head, char *path, int atStart) {
    PathNode *newNode = malloc(sizeof(PathNode));
    newNode->path = strdup(path); // 复制路径字符串
    newNode->next = NULL;

    if (atStart || *head == NULL) {
        newNode->next = *head;
        *head = newNode;
    } else {
        PathNode *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

void clearPath(PathNode **head) {
    PathNode *current = *head;
    PathNode *next;

    while (current != NULL) {
        next = current->next;
        free(current->path);
        free(current);
        current = next;
    }

    *head = NULL;
}

int sls(const char *pathname) {
    print("ls %s\n", pathname);

}

int scat(const char *pathname) {
    print("cat %s\n", pathname);

}

int smkdir(const char *pathname) {
    print("mkdir %s\n", pathname);

}

int stouch(const char *pathname) {
    print("touch %s\n", pathname);

}

int secho(const char *content) {
    print("echo %s\n", content);

}

int swhich(const char *cmd) {
    print("which %s\n", cmd);

}

void processLine(char *line) {
    // 判断是否为 PATH 设置的行
    if (strncmp(line, "export PATH=", 12) == 0) {
        char *pathValue = line + 12; // 跳过 "export PATH="

        if (strstr(pathValue, ":$PATH") != NULL) {
            // 在 PATH 的尾部添加
            *strchr(pathValue, ':') = '\0'; // 移除 ":$PATH"
            addPathNode(&pathHead, pathValue, 0);
        } else if (strstr(pathValue, "$PATH:") == pathValue) {
            // 在 PATH 的首部添加
            pathValue += 6; // 跳过 "$PATH:"
            addPathNode(&pathHead, pathValue, 1);
        } else {
            // 设置新的 PATH 值
            clearPath(&pathHead);
            addPathNode(&pathHead, pathValue, 0);
        }
    }
}

void init_shell() {
    int fd = ropen("/home/ubuntu/.bashrc", O_RDONLY);
    char buf[1024];
    char line[1024] = {0};
    int bytesRead;
    int lineIndex = 0;

    PathNode *pathHead = NULL;

    while ((bytesRead = rread(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[bytesRead] = '\0'; // 确保字符串正确结束

        for (int i = 0; i < bytesRead; ++i) {
            if (buf[i] == '\n' || buf[i] == '\0') {
                line[lineIndex] = '\0'; // 完成一行
                processLine(line);
                lineIndex = 0; // 重置行索引为下一行
                if (buf[i] == '\0') break;
            } else {
                line[lineIndex++] = buf[i];
            }
        }
    }

    // 如果最后一行没有换行符
    if (lineIndex > 0) {
        line[lineIndex] = '\0';
        processLine(line);
    }
}

void close_shell() {
    clearPath(&pathHead);

}