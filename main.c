#include "ramfs.h"
#include "shell.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

int main() {
    init_ramfs();
    init_shell();

    assert(sls("/home") == 1);

    assert(smkdir("/1") == 0);
    assert(stouch("/2.txt") == 0);
    assert(smkdir("/2.txt/1") == 1);
    assert(smkdir("/2.txt/1") == 1);
    assert(sls("/2.txt") == 0);

    assert(smkdir("/1/1") == 0);
    assert(sls("/1") == 0);
    assert(smkdir("/2") == 0);
    assert(smkdir("/3") == 0);
    assert(sls("/") == 0);
    assert(sls("") == 0);
    assert(sls("/2") == 0);
    assert(smkdir("/2/2") == 0);
    assert(smkdir("/2/1") == 0);
    assert(sls("/2") == 0);
    assert(smkdir("/2/1/1") == 0);
    assert(sls("/2//1//") == 0);


    close_shell();
    close_ramfs();
}
