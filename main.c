#include "ramfs.h"
#include "shell.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "stdio.h"

int main() {
    init_ramfs();
    init_shell();
    printf("%s", getPrePathFromFullPath("///12"));
}
