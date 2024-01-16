#include "ramfs.h"
#include "shell.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

int main() {
    init_ramfs();
    init_shell();

    assert(sls("/home") == 1);

    assert(smkdir("/home") == 0);
    assert(smkdir("/test/1") == 1);
    assert(smkdir("/home/1/1") == 1);


    close_shell();
    close_ramfs();
}
