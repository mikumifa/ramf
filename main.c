#include "ramfs.h"
#include "shell.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

const char *content = "export PATH=/usr/bin/\n";
const char *ct = "export PATH=/home:$PATH";

int main() {
    init_ramfs();

    assert(rmkdir("//") == 0);
    assert(rmkdir("//") == 0);
    assert(rmkdir("/test/test") == 0);
    assert(rmkdir("/test/test/test") == 0);
    assert(rmkdir("/test/test/test/") == 0);
    assert(rmkdir("/test/test/test") == 0);
    assert(rmkdir("/usr") == 0);
    assert(rmkdir("/usr/bin") == 0);
    close_ramfs();
}
