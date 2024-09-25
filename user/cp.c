/*
 * @Date: 2024-09-12
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-09-19
 * @FilePath: /caffeinix/user/cp.c
 * @Description: 
 */
#include "user.h"
#include "fcntl.h"
#include "stat.h"
#include "dirent.h"
#include "getopt.h"

#define TAG                     "cp: "

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

static int cp(char *src, char *dst)
{
        int sfd, dfd, n, ret = -1;
        char buf[1024];

        sfd = open(src, O_RDONLY);
        if(!sfd) {
                printf(TAG"cannot stat '%s': No such file or directory\n", src);
                return ret;
        }

        dfd = open(dst, O_WRONLY | O_CREATE | O_TRUNC);
        if(!dfd) {
                printf(TAG"cannot create '%s'\n", dst);
                goto r1;
        }

        while((n = read(sfd, buf, 1024)) > 0) {
                if((n = write(dfd, buf, 1024)) < 0) {
                        printf(TAG"write '%s' failed\n", dst);
                        goto r2;
                }
        }

        ret = 0;

r2:
        close(dfd);
r1:    
        close(sfd);
        return ret;
}

static int cp_dir(char *src, char *dst)
{
        int sfd, n, ret = -1;
        struct stat st;
        struct dirent dirent;

        if(mkdir(dst) < 0) {
                printf(TAG"cannot create dirctory %s: No such file or directory\n");
                return ret;
        }

        sfd = open(src, O_RDONLY);
        if(!sfd) {
                printf(TAG"cannot open directory '%s'\n", src);
                return ret;
        }

        while((n = read(sfd, &dirent, sizeof(struct dirent))) > 0) { 
                char src_path[128], dst_path[128];

                if(n != sizeof(struct dirent))
                        goto r1;

                if(dirent.name[0] == '.' || dirent.name[1] == '.')
                        continue;

                strcpy(src_path, src);
                strcpy(dst_path, dst);
                append_file(src_path, dirent.name);
                append_file(dst_path, dirent.name);

                if(stat(src_path, &st) < 0) {
                        printf(TAG"cannot stat '%s': No such file\n", src);
                        goto r1;
                }

                if(st.type == T_DIR)
                        ret = cp_dir(src_path, dst_path);
                else
                        ret = cp(src_path, dst_path);

                if(ret)
                        goto r1;
        }

        ret = 0;

r1:
        close(sfd);
        return ret;
        
}

int main(int argc, char **argv)
{
        struct stat st;
        int opt;
        int src_index, dst_index;

        if(argc < 3) {
                printf(TAG"missing operand\n");
                return -1;
        }

        dst_index = argc - 1;
        src_index = dst_index - 1;

        if(stat(argv[src_index], &st) < 0) {
                printf(TAG"cannot stat '%s': No such file or directory\n", argv[1]);
                return -1;
        }

        clean_flag();

        while ((opt = getopt(argc, argv, "rf")) != -1) {
                switch(opt) {
                        case 'r':
                        recursive_flag = 1;
                        break;
                        default:
                        break;
                }
        }

        if(st.type == T_DIR) {
                if(recursive_flag)
                        return cp_dir(argv[src_index], argv[dst_index]);
                printf(TAG"target '%s' is a directory, but the -r option is not specified\n", argv[1]);
                return -1;
        } else {
                return cp(argv[src_index], argv[dst_index]);
        }
}