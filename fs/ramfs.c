#include "ramfs.h"
#include <assert.h>
#include <ctype.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>

node *root = NULL;

#define NRFD 4100
FD fdesc[NRFD];
char parts[MAX_PATH_PARTS][FILENAME_MAX];
int find_state;
int make_dir_state;
int pre_path_state;

void crash() {
    node *tmp = NULL;
    tmp->type = DIR_NODE;
}

node *create_node(int nodeType, const char *name) {
    node *newNode = (node *) malloc(sizeof(node));
    newNode->type = nodeType;
    newNode->dirs = NULL;
    newNode->dir_num = 0;
    newNode->content = strdup("");
    newNode->size = 0;
    newNode->name = strdup(name); // 复制名称
    return newNode;
}

int add_subdir(node *dir, node *subdir) {
    // 重新分配内存以容纳新的子目录
    node **newDirs = realloc(dir->dirs, (dir->dir_num + 1) * sizeof(node *));
    dir->dirs = newDirs;
    dir->dirs[dir->dir_num] = subdir;
    dir->dir_num++;
    return 0; // 成功
}

int remove_subdir(node *dir, node *subdir) {
    for (int i = 0; i < dir->dir_num; i++) {
        if (dir->dirs[i] == subdir) {
            free(subdir->name);
            free(subdir->content);
            free(subdir);
            // 将数组中的最后一个元素移动到当前位置
            dir->dirs[i] = dir->dirs[dir->dir_num - 1];
            dir->dir_num--;

            node **newDirs = realloc(dir->dirs, dir->dir_num * sizeof(node *));
            dir->dirs = newDirs;
            return 0; // 成功
        }
    }

    return -1; // 未找到子目录
}


int have_same_name(const char *name, node *dir) {
    for (int i = 0; i < dir->dir_num; ++i) {
        if (strcmp(dir->dirs[i]->name, name) == 0)
            return 1;
    }
    return 0;
}

int is_valid_char(char c) {
    return isalnum(c) || c == '.' || c == '/'; // 只允许字母、数字和点号
}

int is_vaild_str(const char *str) {
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

//成功状态0
//调用前保证当前目录不能是跟目录
//先前的path有文件或者当前是文件 1
//先前的path不存在 2
node *getPrePath(const char *pathname) {
    int pos = findFirstLastPathPos(pathname);
    char *pre_path = (char *) malloc(sizeof(char) * (pos + 4));
    strncpy(pre_path, pathname, pos);
    pre_path[pos] = '\0';
    node *pre_path_node = find(pre_path);
    free(pre_path);

    //判断前面情况，
    if (find_state == 0) {
        //最后还要保证,不能是文件
        if (pre_path_node->type == FILE_NODE) {
            pre_path_state = 1;
            return NULL;
        }
        pre_path_state = 0;
        return pre_path_node;
    } else if (find_state == 1) {
        pre_path_state = 1;
        return NULL;
    } else if (find_state == 2) {
        pre_path_state = 2;
        return NULL;
    }
}

//切割出来，有问题返回-1，
//不然返回的是切割的结果
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
                strncpy(parts[part_count], pathname + countStartPos, i - countStartPos);
                parts[part_count][i - countStartPos] = '\0';
                part_count++;
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

//如果最后一个带了‘/‘，但是还是
node *find(const char *pathname) {
    int len = split_pathname(pathname);
    if (len == -1) {
        //路径有问题
        find_state = 2;
        return NULL;
    }
    node *now_dir = root;
    for (int i = 0; i < len; ++i) {
        if (now_dir->type == FILE_NODE) {
            find_state = 1;
            return NULL;
        }
        int canFind = 0;
        for (int k = 0; k < now_dir->dir_num; ++k) {
            if (strcmp(now_dir->dirs[k]->name, parts[i]) == 0) {
                // 找到匹配的子节点，递归继续查找
                now_dir = now_dir->dirs[k];
                canFind = 1;
                break;
            }
        }
        if (canFind == 0) {
            find_state = 2;
            return NULL;
        }

    }
    find_state = 0;
    return now_dir;
}

int ropen(const char *pathname, int flags) {
    if (!is_vaild_str((char *) pathname))
        return -1;
    if (flags & O_APPEND) {
        node *file = find(pathname);
        if (file == NULL)
            return -1;
        int fd_top = find_unuse_fd();
        fdesc[fd_top].flags = flags;
        fdesc[fd_top].f = file;
        fdesc[fd_top].offset = file->size;
        fdesc[fd_top].used = 1;//这个属性有用吗？
        return fd_top;
    } else if (flags & O_CREAT) {
        node *file = find(pathname);
        if (file == NULL) {
            node *pre_path_node = getPrePath(pathname);
            if (pre_path_node == NULL) {
                return -1;
            }
            int len = split_pathname(pathname);
            char *dir_name = parts[len - 1];
            if (strlen(dir_name) > 32) {
                return -1;
            }
            if (have_same_name(dir_name, pre_path_node)) {
                return -1; // 存在
            }
            node *new_node = create_node(FILE_NODE, dir_name);
            add_subdir(pre_path_node, new_node);
            file = new_node;
        }
        int fd_top = find_unuse_fd();
        fdesc[fd_top].flags = flags;
        fdesc[fd_top].f = file;
        fdesc[fd_top].offset = 0;
        fdesc[fd_top].used = 1;//这个属性有用吗？
        return fd_top;
    } else if (flags & O_TRUNC) {
        node *file = find(pathname);
        if (file == NULL)
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
        int fd_top = find_unuse_fd();
        fdesc[fd_top].flags = flags;
        fdesc[fd_top].f = file;
        fdesc[fd_top].offset = 0;
        fdesc[fd_top].used = 1;//这个属性有用吗？
        return fd_top;
    }
}

int rclose(int fd) {
    if (fd < 0 || fd >= NRFD || fdesc[fd].used == 0)
        return -1;

    fdesc[fd].used = 0;
    return 0;
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
    //偏移量指向的事下一个的位置
    if (fd < 0 || fd >= NRFD || fdesc[fd].used == 0)
        return -1;
    if (fdesc[fd].f->type == DIR_NODE)
        return -1;
    if (!(fdesc[fd].flags & O_WRONLY || fdesc[fd].flags & O_RDWR))
        return -1;

    off_t offset = fdesc[fd].offset;
    //先扩容到偏移量刚刚不满足，size和offect一样，如果size不够的话
    if (offset > fdesc[fd].f->size) {
        fdesc[fd].f->content = realloc(fdesc[fd].f->content, offset);
        char *char_content = (char *) fdesc[fd].f->content;
        for (int i = fdesc[fd].f->size; i < offset; ++i) {
            char_content[i] = 0;
        }
        fdesc[fd].f->size = offset;
    }

    //在扩容到可以存下count
    if (offset + count >= fdesc[fd].f->size) {
        fdesc[fd].f->content = realloc(fdesc[fd].f->content, offset + count);
        fdesc[fd].f->size = offset + count;
    }
    char *char_content = (char *) fdesc[fd].f->content;
    memcpy(char_content + offset, buf, count);
    fdesc[fd].offset += count;
    return count; // 返回写入的字节数
}


ssize_t rread(int fd, void *buf, size_t count) {
    if (fd < 0 || fd >= NRFD || fdesc[fd].used == 0)
        return -1;
    if (fdesc[fd].f->type == DIR_NODE)
        return -1;
    if (fdesc[fd].flags & O_WRONLY) {
        return -1;
    }
    off_t offset = fdesc[fd].offset;
    int left_len = fdesc[fd].f->size - offset;
    int read_len = left_len > count ? count : left_len;
    char *char_content = (char *) fdesc[fd].f->content;
    if (read_len <= 0)
        return 0;
    memcpy(buf, char_content + offset, read_len);
    fdesc[fd].offset += read_len;
    return read_len;
}


off_t rseek(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= NRFD || fdesc[fd].used == 0)
        return -1;
    if (fdesc[fd].f->type == DIR_NODE)
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

    if (new_offset < 0)
        return -1;

    fdesc[fd].offset = new_offset;
    return new_offset;

}


//make_dir_state
//正常 0
//这个绝对地址中包含了一个文件而非目录 1
//这个绝对地址中包含了不存在的文件目录 2
//最终指向的文件/目录已经存在 3
int rmkdir(const char *pathname) {
    if (!is_vaild_str(pathname)) {
        make_dir_state = 2;
        return -1;
    }
    if (*pathname == '\0')
        return -1;
    if (*pathname != '/')
        return -1;
    node *existing = find(pathname);
    if (existing != NULL) {
        make_dir_state = 3;
        return -1; // 存在
    }
    node *pre_path_node = getPrePath(pathname);
    if (pre_path_state == 1) {
        make_dir_state = 1;
        return -1; // 存在
    } else if (pre_path_state == 2) {
        make_dir_state = 2;
        return -1; // 存在
    }
    int len = split_pathname(pathname);
    if (len == -1)
        return -1;
    char *dir_name = parts[len - 1];
    for (int i = 0; i < len; ++i) {
        if (strlen(parts[i]) > 32) {
            return -1;
        }
    }
    if (have_same_name(dir_name, pre_path_node)) {
        make_dir_state = 3;
        return -1; // 存在
    }
    //添加一个
    make_dir_state = 0;
    node *new_node = create_node(DIR_NODE, dir_name);
    add_subdir(pre_path_node, new_node);
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
    //这一部肯定找到，只要能找到当前，说明前面的prePath没问题，并且不存在更目录情况
    node *pre_path_node = getPrePath(pathname);
    remove_subdir(pre_path_node, target);
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
    remove_subdir(pre_path_node, file);
    return 0;
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
    root = NULL;
}

