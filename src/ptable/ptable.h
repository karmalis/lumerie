#ifndef PTABLE_H_
#define PTABLE_H_

#include <stdlib.h>
#include <stdint.h>

/// Piece Table
/// -----------

typedef struct string_buffer {
    char* buffer;
    size_t offset;
    size_t size;
} PTableCBuffer;

typedef enum table_node_type {
ORIGINAL,
ADDITION
} PTableNodeType;

typedef enum table_node_step_direction {
FORWARD,
BACKWARD,
} PTableNodeStepDirection;

typedef struct table_node {
    PTableNodeType node_type;
    size_t start;
    size_t length;
} PTableNode;

typedef struct piece_table {
    const PTableCBuffer original;
    PTableCBuffer add;
    PTableNode* nodes;
    size_t node_count;
} PTable;

// PTable manipulation

PTable ptable_create(const char*);
void ptable_insert(PTable* table, size_t pos, const char* text);
char ptable_index(PTable* table, size_t at);
void ptable_delete(PTable* table, size_t at, size_t len);
void ptable_release(PTable* table);

// Helpers and utils
void ptable_print(PTable* table);
void ptable_print_node_sequence(PTable* table, uint8_t print_final_string);

#endif // PTABLE_H_
