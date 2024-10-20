/*
 * @Date: 2024-09-12
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-09-26
 * @FilePath: /caffeinix-rootfs/user/rm.c
 * @Description: 
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#define TAG                     "rm: "

static int recursive_flag = 0;

static void clean_flag(void)
{
        recursive_flag = 0;
}

static void append_file(char *path, char *file)
{
        int path_len, file_len;

        path_len = strlen(path);
        path[path_len++] = '/';
        file_len = strlen(file);

        memcpy(path + path_len, file , file_len);

        *(path + path_len + file_len) = '\0';
}

static int rm_file(char *path)
{
        if (unlink(path) < 0) {
                fprintf(stderr, TAG"cannot remove '%s': No such file or directory\n", path);
                return -1;
        }

        return 0;
}

static int rm_dir(char *path)
{
        int fd, n;
        struct dirent dirent;

        fd = open(path, O_RDONLY);
        if (!fd) {
                fprintf(stderr, TAG"cannot open directory '%s'\n", path);
                return -1;
        }

        while ((n = read(fd, &dirent, sizeof(struct dirent))) > 0) {
                char sub_path[128];

                if (n != sizeof(struct dirent))
                        goto r1;

                if (strcmp(dirent.name, ".") == 0 || strcmp(dirent.name, "..") == 0)
                        continue;

                strcpy(sub_path, path);
                append_file(sub_path, dirent.name);

                struct stat st;
                if (stat(sub_path, &st) < 0) {
                        fprintf(stderr, TAG"cannot stat '%s': No such file\n", sub_path);
                        continue; 
                }

                if (S_ISDIR(st.st_mode)) {
                        if (rm_dir(sub_path) < 0) {
                                goto r1;
                        }
                } else {
                        if (rm_file(sub_path) < 0) {
                                goto r1;
                        }
                }
        }
        
        close(fd);

        if (unlink(path) < 0) {
                fprintf(stderr, TAG"cannot remove directory '%s'\n", path);
                goto r1;;
        }

        return 0;

r1:
        close(fd);
        return -1;
}

int main(int argc, char **argv)
{
        int i, opt;
        struct stat st;

        if(argc < 2) {
                fprintf(stderr, TAG"missing operand\n");
                return -1;
        }

        while ((opt = getopt(argc, argv, "r")) != -1) {
                switch (opt) {
                case 'r':
                recursive_flag = 1;
                break;
                default:
                clean_flag();
                break;
                }
        }
        
        for (i = optind; i < argc; i++) {
                if (stat(argv[i], &st) < 0) {
                        fprintf(stderr, TAG"can't remove '%s':No such file or directory\n", argv[i]);
                        continue;
                }

                if (S_ISDIR(st.st_mode)) {
                        if (!recursive_flag) {
                                fprintf(stderr, TAG"can't remove '%s': Is a directory\n", argv[i]);
                        } else {
                                return rm_dir(argv[i]);
                        }
                } else {
                        return rm_file(argv[i]);
                }
        }
        return 0;
}
