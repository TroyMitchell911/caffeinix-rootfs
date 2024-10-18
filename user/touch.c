#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
        int fd;

        if(argc < 2) {
                fprintf(stderr, "touch: missing file operand\n");
                return -1;
        }

        fd = open(argv[1], O_CREAT);
        if(fd < 0) {
                fprintf(stderr, "touch: create %s failed\n", argv[1]);
                return -1;
        } else {
                close(fd);
        }

        return 0;
}
