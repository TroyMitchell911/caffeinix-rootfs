/*
 * @Author: TroyMitchell
 * @Date: 2024-05-22
 * @LastEditors: TroyMitchell
 * @LastEditTime: 2024-05-30
 * @FilePath: /caffeinix/user/ls.c
 * @Description: 
 * Words are cheap so I do.
 * Copyright (c) 2024 by TroyMitchell, All Rights Reserved. 
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define MAXPATH                 128

static char* name_format(char *path)
{
        static char buf[DIRSIZ], *p;

        for(p = path + strlen(path); p >= path && *p != '/'; p--);
        p++;

        /* return directly */
        if(strlen(p) >= DIRSIZ)
                return p;

        memmove(buf, p, strlen(p));
        memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
        return buf;
}

void ls(char *path)
{
        int fd, ret;
        struct stat st;
        char buf[MAXPATH], *p;
        struct dirent de;

        fd = open(path, 0);
        if(fd < 0) {
                fprintf(stderr, "ls: open %s failed\n", path);
                return;
        }

        ret = fstat(fd, &st);
        if(ret == -1) {
                fprintf(stderr, "ls: fstat %s failed\n", path);
                goto r1;
        }
        
        switch(st.st_mode) {
                case _IFCHR:
                case _IFMT:
                        fprintf(stdout, "%s %d %d %ld\n", name_format(path), st.st_mode, st.st_ino, st.st_size);
                        break;
                case _IFDIR:
                        if(strlen(path) + DIRSIZ + 1 + 1 >= MAXPATH) {
                                fprintf(stderr, "ls: %s too long name\n", path);
                                goto r1;
                        }
                        strcpy(buf, path);
                        p = strlen(buf) + buf;
                        *p++ = '/';
                        while(read(fd, &de, sizeof(struct dirent)) == sizeof(struct dirent)) {
                                if(!de.inum)
                                        continue;
                                memmove(p, de.name, DIRSIZ);
                                if(stat(buf, &st) < 0) {
                                        fprintf(stderr, "ls: fstat %s failed\n", path);
                                        continue;
                                }
                                fprintf(stdout, "%s %d %d %ld\n", name_format(buf), st.st_mode, st.st_ino, st.st_size);
                        }
                        break;
        }
r1:
        close(fd);
}

int main(int argc, char *argv[])
{
        if(argc < 2)
                ls(".");
        else if(argc == 2)
                ls(argv[1]);
        return 0;
}
