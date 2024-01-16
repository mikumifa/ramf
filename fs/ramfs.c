#include "ramfs.h"
#include <assert.h>
#include <ctype.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>

node *root = NULL;

#define NRFD 4096
FD fdesc[NRFD];
#define MAX_PATH_PARTS 100
#define FILENAME_MAX 100
char parts[MAX_PATH_PARTS][FILENAME_MAX];

void free_node(node *pre, node *n);

int context_extend(void **content, int size) {
    if (content == NULL || *content == NULL) {
        *content = malloc(size);
    } else {
        *content = realloc(*content, size);
    }
    if (*content == NULL) {
        return 0; // 失败
    }
    return 1; // 成功
}


int is_valid_char(char c) {
    return isalnum(c) || c == '.' || c == '/'; // 只允许字母、数字和点号
}

int is_vaild_str(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len; ++i) {
        if (!is_valid_char(str[i]))
            return 0;
    }
    return 1;
}

int find_unuse_fd() {
    for (int i = 0; i < NRFD; ++i) {
        if (fdesc[i].used == 0)
            return i;
    }
    return -1;
}

int findFirstLastPathPos(const char *pathname) {
    int len = strlen(pathname);
    int startCount = 0;
    for (int i = len - 1; i >= 0; --i) {
        if (pathname[i] == '/') {
            if (startCount) {
                return i + 1;
            } else {
                continue;
            }
        } else {
            startCount = 1;
        }
    }
    return -1;
}

node *getPrePath(const char *pathname) {
    int pos = findFirstLastPathPos(pathname);
    char *pre_path = (char *) malloc(sizeof(char) * (pos + 4));
    strncpy(pre_path, pathname, pos);
    node *pre_path_node = find(pre_path);
    free(pre_path);
    return pre_path_node;
}

//切割出来，有问题返回-1，
//不然返回的是切割的结果
//文件的末尾不能有’/‘
int split_pathname(const char *pathname) {
    if (pathname == NULL) {
        return -1;
    }
    // 检查是否以斜杠开头
    if (pathname[0] != '/') {
        return -1; // 非绝对路径
    }
    // 开始切分路径
    int part_count = 0;
    int path_len = strlen(pathname);
    int isInCount = 0;
    int countStartPos = 0;
    for (int i = 0; i < path_len; ++i) {
        if (pathname[i] == '/') {
            if (isInCount) {
                isInCount = 0;
                //加入
                strncpy(parts[part_count++], pathname + countStartPos, i - countStartPos);
            } else {
                continue;
            }
        } else {
            //如何count没有开始,设置一下开始的位置
            if (!isInCount) {
                countStartPos = i;
            }
            isInCount = 1;
            if (!is_valid_char(pathname[i])) {
                return -1;
            }
        }

    }
    if (isInCount) {
        strcpy(parts[part_count++], pathname + countStartPos);
    }
    return part_count;
}

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

//node *find_recursive(node *current, const char *path) {
//    if (current == NULL || path == NULL) {
//        return NULL;
//    }
//    //路径是空的
//    if (*path == '\0') {
//        return NULL;
//    }
//    //第一个必须是‘/’
//    if (*path != '/') {
//        return NULL;
//    }
//
//    // 下一个还是就到下一个
//    if (*(path + 1) == '/')
//        return find_recursive(current, path + 1);
//
//    char *nameHead = path + 1;
//    char *next = strchr(nameHead, '/');
//    //找到求，没找到就是到底
//    int nameLen = next ? (next - nameHead) : strlen(nameHead);
//    //当前是nameHead开始
//    // 对于每个子节点
//    for (int i = 0; i < current->dir_num; ++i) {
//        if (strncmp(current->dirs[i]->name, nameHead, nameLen) == 0 && current->dirs[i]->name[nameLen] == '\0') {
//            // 找到匹配的子节点，递归继续查找
//            return current->dirs[i];
//        }
//    }
//    return NULL; // 没找到
//}


//node *find(const char *pathname) {
//    return find_recursive(root, pathname);
//}
node *find(const char *pathname) {
    int len = split_pathname(pathname);
    if (len == -1)
        return NULL;
    node *now_dir = root;
    for (int i = 0; i < len; ++i) {
        if (now_dir->type == FILE_NODE)
            return NULL;
        int canFind = 0;
        for (int k = 0; k < now_dir->dir_num; ++k) {
            if (strcmp(now_dir->dirs[k]->name, parts[i]) == 0) {
                // 找到匹配的子节点，递归继续查找
                now_dir = now_dir->dirs[k];
                canFind = 1;
                break;
            }
        }
        if (canFind == 0)
            return NULL;

    }
    int path_len = strlen(pathname);
    if (pathname[path_len - 1] == '/') {
        //如果最后一个是’/‘,找到的FILE不能算
        if (now_dir->type == FILE_NODE)
            return NULL;
    }
    return now_dir;
}

int ropen(const char *pathname, int flags) {
    if (!is_vaild_str(pathname))
        return -1;

    if (flags & O_APPEND) {
//追加
        node *file = find(pathname);
        if (file == NULL)
            return -1;
        if (file->type == DIR_NODE)
            return -1;
        int fd_top = find_unuse_fd();
        fdesc[fd_top].flags = flags;
        fdesc[fd_top].f = file;
        fdesc[fd_top].offset = file->size;
        fdesc[fd_top].used = 1;//这个属性有用吗？
        return fd_top;
    } else if (flags & O_CREAT) {
// 创建


// 读模式
        node *file = find(pathname);
        if (file == NULL) {
            node *pre_path_node = getPrePath(pathname);
            if (pre_path_node == NULL)
                return -1;
            if (pre_path_node->type == FILE_NODE)
                return -1;
            //找到最后一个然后添加最后一个
            int len = split_pathname(pathname);
            char *dir_name = parts[len - 1];
            //添加一个
            node **temp = (node **) malloc(sizeof(node *) * pre_path_node->dir_num + 1);
            int top = 0;
            for (int i = 0; i < pre_path_node->dir_num; ++i) {
                temp[top++] = pre_path_node->dirs[i];
            }
            temp[top] = (node *) malloc(sizeof(node));
            temp[top]->type = FILE_NODE;
            temp[top]->dir_num = 0;
            temp[top]->name = strdup(dir_name);
            temp[top]->content = strdup("");
            temp[top]->size = 0;
            temp[top]->dirs = NULL;

            free(pre_path_node->dirs);
            pre_path_node->dirs = temp;
            pre_path_node->dir_num++;
            file = temp[top];
        }
        int fd_top = find_unuse_fd();

        fdesc[fd_top].flags = flags;
        fdesc[fd_top].f = file;
        fdesc[fd_top].offset = 0;
        fdesc[fd_top].used = 1;//这个属性有用吗？
        return fd_top;

    } else if (flags & O_TRUNC) {
// 清空
        node *file = find(pathname);
        if (file == NULL)
            return -1;
        if (file->type == DIR_NODE)
            return -1;
        if (flags & O_RDWR || flags & O_WRONLY) {
            free(file->content);
            file->content = strdup("");
            file->size = 0;
        }
        int fd_top = find_unuse_fd();
        fdesc[fd_top].flags = flags;
        fdesc[fd_top].f = file;
        fdesc[fd_top].offset = 0;
        fdesc[fd_top].used = 1;//这个属性有用吗？
        return fd_top;
    } else {
        node *file = find(pathname);
        if (file == NULL)
            return -1;
        if (file->type == DIR_NODE)
            return -1;
        int fd_top = find_unuse_fd();

        fdesc[fd_top].flags = flags;
        fdesc[fd_top].f = file;
        fdesc[fd_top].offset = 0;
        fdesc[fd_top].used = 1;//这个属性有用吗？
        return fd_top;

    }
}

int rclose(int fd) {
    if (fd >= NRFD || fdesc[fd].used == 0)

        return -1;
    fdesc[fd].used = 0;
    return 0;
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
    if (fd >= NRFD || fdesc[fd].used == 0)
        return -1;
    if (fdesc[fd].f->type == DIR_NODE)
        return -1;

    int offset = fdesc[fd].offset;
    if (!context_extend(&(fdesc[fd].f->content), offset + count))
        return -1; // 扩展失败

    char *char_content = (char *) fdesc[fd].f->content;
    memcpy(char_content + offset, buf, count);
    fdesc[fd].offset += count;
    fdesc[fd].f->size = offset + count;
    return count; // 返回写入的字节数
}


ssize_t rread(int fd, void *buf, size_t count) {
    if (fd >= NRFD || fdesc[fd].used == 0)
        return -1;
    if (fdesc[fd].f->type == DIR_NODE)
        return -1;
    int offset = fdesc[fd].offset;
    if (!context_extend(&(fdesc[fd].f->content), offset + count))
        return -1; // 扩展失败

    char *char_content = (char *) fdesc[fd].f->content;
    memcpy(buf, char_content + offset, count);
    int buf_len = strlen(buf);
    int ret_len=buf_len > count ? count : strlen(buf); // 返回实际读取的字节数
    fdesc[fd].offset += ret_len;
    return  ret_len;
}


off_t rseek(int fd, off_t offset, int whence) {
    if (fd >= NRFD || fdesc[fd].used == 0)
        return -1;

    int new_offset;
    switch (whence) {
        case SEEK_SET:
            new_offset = offset;
            break;
        case SEEK_CUR:
            new_offset = fdesc[fd].offset + offset;
            break;
        case SEEK_END:
            new_offset = fdesc[fd].f->size + offset;
            break;
        default:
            return -1;
    }

    if (new_offset < 0 || new_offset > fdesc[fd].f->size)
        return -1; // 超出文件范围

    fdesc[fd].offset = new_offset;
    return new_offset;
}


int rmkdir(const char *pathname) {
    if (!is_vaild_str(pathname))
        return -1;
    node *existing = find(pathname);
    if (existing != NULL) {
        return -1; // 存在
    }
    //如何能找到前面的文件夹


    node *pre_path_node = getPrePath(pathname);
    if (pre_path_node == NULL)
        return -1;
    if (pre_path_node->type == FILE_NODE)
        return -1;

    //找到最后一个然后添加最后一个
    int len = split_pathname(pathname);
    char *dir_name = parts[len - 1];

    //添加一个
    node **temp = (node **) malloc(sizeof(node *) * pre_path_node->dir_num + 1);
    int top = 0;
    for (int i = 0; i < pre_path_node->dir_num; ++i) {
        temp[top++] = pre_path_node->dirs[i];
    }
    temp[top] = (node *) malloc(sizeof(node));
    temp[top]->type = DIR_NODE;
    temp[top]->dir_num = 0;
    temp[top]->name = strdup(dir_name);
    temp[top]->content = NULL;
    temp[top]->size = 0;
    temp[top]->dirs = NULL;

    free(pre_path_node->dirs);
    pre_path_node->dirs = temp;
    pre_path_node->dir_num++;
    return 0;
}

int rrmdir(const char *pathname) {
    node *target = find(pathname);

    if (target == NULL) {
        return -1; // 目录不存在
    }
    // 检查是否是根目录
    if (split_pathname(pathname) == 0) {
        return -1; // 尝试删除根目录，没有权限
    }

    if (target->type != DIR_NODE) {
        return -1; // 指定路径不是一个目录
    }

    if (target->dir_num > 0) {
        return -1; // 目录不为空
    }
    node *pre_path_node = getPrePath(pathname);
    free_node(pre_path_node, target);
    return 0; // 成功
}


//其实就是删除功能
int runlink(const char *pathname) {
    node *file = find(pathname);
    if (file == NULL)
        return -1;
    if (file->type == DIR_NODE)
        return -1;
    node *pre_path_node = getPrePath(pathname);
    free_node(pre_path_node, file);
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
void free_node(node *pre, node *n) {
    if (n == NULL) {
        return;
    }
    // 目录节点，释放子节点
    if (n->type == DIR_NODE) {
        for (int i = 0; i < n->dir_num; ++i) {
            free_node(n, n->dirs[i]);
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

    if (pre == NULL) {
        return;
    }
    //删除一个
    node **temp = (node **) malloc(sizeof(node *) * pre->dir_num - 1);
    int top = 0;
    for (int i = 0; i < pre->dir_num; ++i) {
        if (pre->dirs[i] != n) {
            temp[top++] = pre->dirs[i];
        }
    }
    free(pre->dirs);
    pre->dirs = temp;
    root->dir_num--;
}

void close_ramfs() {
    free_node(NULL, root);
    root = NULL;
}

