#include "ramfs.h"
#include "shell.h"
#include <assert.h>
#include <string.h>

extern node *root;
const char *content = "export PATH=$PATH:/usr/bin/\n";
const char *ct = "export PATH=/home:$PATH";

int main() {
    init_ramfs();

    assert(smkdir("/home") == 0);
    assert(smkdir("/home/ubuntu") == 0);
    assert(smkdir("/usr") == 0);
    assert(smkdir("/usr/bin") == 0);
    assert(stouch("/home/ubuntu/.bashrc") == 0);
    rwrite(ropen("/home/ubuntu/.bashrc", O_WRONLY), content, strlen(content));
    rwrite(ropen("/home/ubuntu/.bashrc", O_WRONLY | O_APPEND), ct, strlen(ct));
    scat("/home/ubuntu/.bashrc");

    init_shell();
    swhich("ls");
    stouch("/usr/bin/ls");
    swhich("ls");
    stouch("/home/ls");
    swhich("ls");
    secho("hello world");
    secho("The Environment Variable PATH is:\\$PATH");
    secho("The Environment Variable PATH is:\\$PATH");
    secho("The Environment Variable PATH is:\\$PATH");

    close_ramfs();
    close_shell();
    assert(root == NULL);
}
