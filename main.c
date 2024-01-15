#include "ramfs.h"
#include "shell.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

const char *content = "export PATH=/usr/bin/\n";
const char *ct = "export PATH=/home:$PATH";

int main() {
    init_ramfs();

    assert(rmkdir("/home") == 0);
    assert(rmkdir("//home") == -1);
    assert(rmkdir("/test/1") == -1);
    assert(rmkdir("/home/ubuntu") == 0);
    assert(rmkdir("/usr") == 0);
    assert(rmkdir("/usr/bin") == 0);
    assert(ropen("/usr/bin/12", O_APPEND) == -1);
    assert(ropen("/usr/bin/12", O_CREAT) == 0);
    assert(ropen("/usr/bin/12", O_APPEND) == 0);

}
