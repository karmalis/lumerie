#ifndef MEM_H_
#define MEM_H_

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2*sizeof(void*))
#endif

#include <stdint.h>
#include <stdlib.h>



typedef struct Arena Arena;
struct Arena {
    uint8_t* buf;
    size_t buf_len;
    size_t prev_offset;
    size_t curr_offset;
};

void* arena_alloc_align(Arena* a, size_t size, size_t align);
void* arena_alloc(Arena* a, size_t size);

void arena_init(Arena* a, void* backing_buffer, size_t backing_buffer_length);
void arena_free(Arena* a, void* ptr);

void* arena_resize_align(Arena* a, void* old_memory, size_t old_size, size_t new_size, size_t align);
void* arena_resize(Arena* a, void* old_memory, size_t old_size, size_t new_size);

void arena_free_all(Arena* a);

#endif // MEM_H_
