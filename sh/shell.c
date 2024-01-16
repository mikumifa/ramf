#include "ramfs.h"
#include "shell.h"

#ifndef ONLINE_JUDGE
#define print(...) printf("\033[31m");printf(__VA_ARGS__);printf("\033[0m");
#else
#define print(...)
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
    node *dir = NULL;
    if (*pathname == '\0') {
        dir = find("/");
    } else {
        dir = find(pathname);
    }

    if (find_state == 1) {
        printf("ls: cannot access '%s': Not a directory\n", pathname);
        return 1;
    } else if (find_state == 2) {
        printf("ls: cannot access '%s': No such file or directory\n", pathname);
        return 1;
    }
    if (dir == NULL) {
        printf("ls: cannot access '%s': No such file or directory\n", pathname);
        return 1;
    }
    if (dir->type == FILE_NODE) {
        printf("%s\n", pathname);
        return 0;
    } else {
        int len = dir->dir_num;
        for (int i = 0; i < len; ++i) {
            if (i != len - 1) {
                printf("%s ", (char *) dir->dirs[i]->name);
            } else {
                printf("%s\n", (char *) dir->dirs[i]->name);
            }
        }
        return 0;
    }

}

int scat(const char *pathname) {
    print("cat %s\n", pathname);

    node *file = find(pathname);
    if (find_state == 1) {
        printf("cat: %s: Not a directory\n", pathname);
        return 1;
    } else if (find_state == 2) {
        printf("cat: %s: No such file or directory\n", pathname);
        return 1;
    }

    if (file->type == DIR_NODE) {
        printf("cat: %s: Is a directory\n", pathname);
        return 1;
    } else {
        char *char_content = (char *) file->content;
        for (int i = 0; i < file->size; ++i) {
            putchar(char_content[i]);
        }
        puts("");
        return 0;
    }
}

int smkdir(const char *pathname) {
    print("mkdir %s\n", pathname);

    rmkdir(pathname);
    if (make_dir_state == 1) {
        printf("mkdir: cannot create directory '%s': Not a directory\n", pathname);
        return 1;
    } else if (make_dir_state == 2) {
        printf("mkdir: cannot create directory '%s': No such file or directory\n", pathname);
        return 1;
    } else if (make_dir_state == 3) {
        printf("mkdir: cannot create directory '%s': File exists\n", pathname);
        return 1; // 存在
    } else {
        return 0;
    }
}

int stouch(const char *pathname) {
    print("touch %s\n", pathname);
    node *existing = find(pathname);
    if (existing != NULL) {
        //如果存在，啥也不用做
        return 0;

    } else {
        //创建一个文件
        //这个时候肯定不是根，如果是跟，肯定可以找到
        node *prePath = getPrePath(pathname);
        if (pre_path_state == 1) {
            printf("touch: cannot touch '%s': Not a directory\n", pathname);
            return 1;
        } else if (find_state == 2) {
            printf("touch: cannot touch '%s': No such file or directory\n", pathname);
            return 1;
        }
        int fd = ropen(pathname, O_CREAT);
        rclose(fd);
        return 0;
    }
}

char *strndup(const char *s, size_t n) {
    int s_len = strlen(s);
    size_t len = s_len > n ? n : s_len;  // 计算字符串长度，但不超过n
    char *newStr = malloc(len + 1);  // 为新字符串分配内存，包括空字符

    if (newStr == NULL) {
        return NULL;  // 内存分配失败
    }

    memcpy(newStr, s, len);  // 复制字符串
    newStr[len] = '\0';      // 确保新字符串以空字符结束

    return newStr;
}

void print_env(char *env_str) {
    if (strcmp(env_str, "PATH") == 0) {
        PathNode *current = pathHead;
        PathNode *next;

        while (current != NULL) {
            printf("%s", current->path);
            if (current->next != NULL) {
                printf(":");
            }
            current = current->next;
        }
    } else {

    }
}

int secho(const char *content) {
    print("echo %s\n", content);
    const char *p = content;
    while (*p) {
        if (*p == '\\') {
            p++; // 跳过反斜杠
            switch (*p) {
                case '\\':
                    printf("\\");
                    break;
                case '$':
                    printf("$");
                    break;

                default:
                    printf("%c", *p);
            }
        } else if (*p == '$') {
            const char *start = ++p;
            while ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9')) {
                p++;
            }
            char *varName = strndup(start, p - start);
            print_env(varName);
            free(varName);
            p--;
        } else {
            printf("%c", *p);
        }
        p++;
    }
    printf("\n");
    return 0;
}


int swhich(const char *cmd) {
    print("which %s\n", cmd);
    PathNode *current = pathHead;
    while (current != NULL) {
        node *dir = find(current->path);
        if (dir == NULL) {

        } else {
            for (int i = 0; i < dir->dir_num; ++i) {
                if (dir->dirs[i]->type == FILE_NODE && strcmp(dir->dirs[i]->name, cmd) == 0) {
                    int len = split_pathname(current->path);
                    for (int j = 0; j < len; ++j) {
                        printf("/%s", parts[j]);
                    }
                    printf("/%s\n", cmd);
                    return 0;
                }
            }
        }
        current = current->next;
    }
    return 1;
}

void processLine(char *line) {
    // 判断是否为 PATH 设置的行
    if (strncmp(line, "export PATH=", 12) == 0) {
        char *pathValue = line + 12; // 跳过 "export PATH="

        if (strstr(pathValue, ":$PATH") != NULL) {
            // 在 PATH 的尾部添加
            *strchr(pathValue, ':') = '\0'; // 移除 ":$PATH"
            addPathNode(&pathHead, pathValue, 1);
        } else if (strstr(pathValue, "$PATH:") == pathValue) {
            // 在 PATH 的首部添加
            pathValue += 6; // 跳过 "$PATH:"
            addPathNode(&pathHead, pathValue, 0);
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
    rclose(fd);
}

void close_shell() {
    clearPath(&pathHead);
}