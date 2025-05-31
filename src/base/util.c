#ifdef VOLT_USE_OPENGL
#include <glad/gl.h>
#endif

#include "util.h"
#include <stdlib.h>
#include <stdio.h>


file_buff_t read_full_file(const char* path) {
    char* buffer = NULL;
    long length = 0;

    FILE* f = fopen(path, "rb");

    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length);
        if (buffer) {
            fread(buffer, 1, length, f);
        }
        fclose(f);
    }

    file_buff_t file_buffer = {0};
    file_buffer.buff = buffer;
    file_buffer.length = length;

    return file_buffer;
}
