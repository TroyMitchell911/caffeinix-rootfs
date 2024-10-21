#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
        int ret;
        if(argc < 2) {
                fprintf(stderr, "mkdir: missing operand\n");
                return -1;
        }

        ret = mkdir(argv[1], S_IREAD | S_IWRITE);
        if(ret < 0) {
                fprintf(stderr, "mkdir: %s failed\n", argv[1]);
                return -1;
        }
        return 0;
}
