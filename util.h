
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <ctype.h>

#ifndef UTIL_H
#define UTIL_H

#define MIN_KERNEL_VERSION      5
#define MIN_MAJOR_VERSION       5

// 模块化单元，提供一些辅助函数


void fatal_error(const char *syscall) {
    perror(syscall);
    exit(1);
}




int check_kernel_version() {
    struct utsname buffer;
    char *p;
    long ver[16];
    int i=0;

    if (uname(&buffer) != 0) {
        perror("uname");
        exit(EXIT_FAILURE);
    }

    p = buffer.release;

    while (*p) {
        if (isdigit(*p)) {
            ver[i] = strtol(p, &p, 10);
            i++;
        } else {
            p++;
        }
    }
    printf("Minimum kernel version required is: %d.%d\n",
            MIN_KERNEL_VERSION, MIN_MAJOR_VERSION);
    if (ver[0] >= MIN_KERNEL_VERSION && ver[1] >= MIN_MAJOR_VERSION ) {
        printf("Your kernel version is: %ld.%ld\n", ver[0], ver[1]);
        return 0;
    }
    fprintf(stderr, "Error: your kernel version is: %ld.%ld\n",
                    ver[0], ver[1]);
    return 1;
}


// 小写转换

void strtolower(char *str) {
    for (; *str; ++str)
        *str = (char)tolower(*str);
}




void *zh_malloc(size_t size) {
    void *buf = malloc(size);
    if (!buf) {
        fprintf(stderr, "Fatal error: unable to allocate memory.\n");
        exit(1);
    }
    return buf;
}


int get_line(const char *src, char *dest, int dest_sz) {
    bool is_vaild = false;
    int i = 0;
    for (; i < dest_sz; i++) {
        is_vaild = true;
        dest[i] = src[i];
        if (src[i] == '\r' && src[i+1] == '\n') {
            dest[i] = '\0';
        }
    }

    if(i<dest_sz){
        dest[i] = '\0';
    }

    if(is_vaild){
        return 0;
    }
    return 1;
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "";
    return dot + 1;
}


#endif UTIL_H
