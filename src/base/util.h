#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>

typedef struct file_buff file_buff_t;
struct file_buff {
    char* buff;
    size_t length;
};

file_buff_t read_full_file(const char* path);

#endif // UTIL_H_
