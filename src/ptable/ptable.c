 #include "ptable.h"

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define PTABLE_INIT_ADD_SIZE 1024
#define PTABLE_INIT_NODE_SIZE 128
#define FORWARD 1
#define BACKWARD 0



PTable ptable_create(const char* buff) {
    size_t len = strlen(buff);
    const PTableCBuffer original = { .buffer = (char* ) buff, .size = len, .offset = len - 1 };

    char* a_buff = malloc(sizeof(char) * PTABLE_INIT_ADD_SIZE);
    PTableCBuffer addition = { .buffer = a_buff, .size = PTABLE_INIT_ADD_SIZE, .offset = 0 };

    PTableNode first = {.node_type = ORIGINAL, .length = len, .start = 0};

    PTableNode* nodes = malloc(sizeof(PTableNode) * PTABLE_INIT_NODE_SIZE);
    nodes[0] = first;

    PTable table = { .nodes = nodes, .node_count = 1, .original = original, .add = addition};
    return table;
}

void ptable_insert(PTable* table, size_t pos, const char* text) {
    size_t text_len = strlen(text);

    if (table->add.offset + text_len > table->add.size) {
        // Reallocate
        table->add.size = (table->add.offset + text_len + 1) * 2;
        char* new_add_buffer = (char*) realloc(table->add.buffer, table->add.size);
        if (!new_add_buffer) {
            perror("Failed to realloc add buffer size");
            return;
        }
        table->add.buffer = new_add_buffer;
    }
    strcpy(table->add.buffer + table->add.offset, text);
    size_t add_start = table->add.offset;
    table->add.offset += text_len;

    PTableNode addition = {.node_type = ADDITION, .length = text_len, .start = add_start};
    size_t node_offset_pos = 0;

    for (size_t i = 0; i < table->node_count; i++) {
        PTableNode* cursor = &table->nodes[i];
        size_t c_start = node_offset_pos;
        size_t c_end = node_offset_pos + cursor->length;

        if (pos >= c_start && pos <= c_end) {
            size_t offset = pos - c_start;
            if (offset == 0) {
                // Insert before node
                memmove(&table->nodes[i + 1], &table->nodes[i], (table->node_count - i) * sizeof(PTableNode));
                table->nodes[i] = addition;
                table->node_count++;
            } else if (offset == cursor->length) {
                // Insert after node
                memmove(&table->nodes[i + 2], &table->nodes[i + 1], (table->node_count - (i + 1)) * sizeof(PTableNode));
                table->nodes[i+1] = addition;
                table->node_count++;
            }  else {
                // Split node
                // Move next nodes as if inserting after
                memmove(&table->nodes[i + 3], &table->nodes[i + 1], (table->node_count - (i + 1)) * sizeof(PTableNode));
                table->nodes[i+1] = addition;
                table->node_count++;

                // Adjust the "current node"
                PTableNode node_a = *cursor;
                node_a.length = offset;

                PTableNode node_b = *cursor;
                node_b.start = node_a.start + node_a.length;
                //node_b.length = c_end - node_b.start;
                node_b.length = cursor->length - node_a.length;

                table->nodes[i] = node_a;
                table->nodes[i+2] = node_b;
                table->node_count++;
            }

            return;
        }

        node_offset_pos += cursor->length;
    }

    // End of table
    if (pos == node_offset_pos) {
        // TODO: Ensure capacity
        table->nodes[table->node_count] = addition;
        table->node_count++;
    } else {
        // TODO: Ensure bounds
        fprintf(stderr, "Insertion pos %zu out of bounds (doc length %zu).\n", pos, node_offset_pos);
    }
}

char ptable_index(PTable* table, size_t at) {
    size_t logical_pos = 0;
    size_t to_find_idx = at;

    for (size_t i = 0; i < table->node_count; i++) {
        PTableNode* cursor = &table->nodes[i];
        if (logical_pos < cursor->length) {
            const char* source_buffer_ptr = NULL;
            switch (cursor->node_type) {
                case ORIGINAL: {
                    source_buffer_ptr = table->original.buffer;
                } break;
                case ADDITION: {
                    source_buffer_ptr = table->add.buffer;
                } break;
            }

            // TODO Possible bound check placemenet

            size_t actual_buffer_offset = cursor->start + to_find_idx;
            return source_buffer_ptr[actual_buffer_offset];
        } else {
            to_find_idx -= cursor->length;
        }
    }

    return '\0';
}

void ptable_delete(PTable* table, size_t pos, size_t len) {
    // Basically shift offset to the left and add entry
    //ptable_insert(table, at, NULL, -len);


    size_t pos_end = pos + len;
    size_t cursor_doc_offset = 0;


    // New order nodes
    PTableNode* no_nodes = (PTableNode*) malloc((table->node_count + 1) * sizeof(PTableNode));
    size_t no_nodes_count = 0;


    // Find begin - end nodes
    for (size_t i = 0; i < table->node_count; i++) {
        PTableNode cursor = table->nodes[i];

        size_t c_start = cursor_doc_offset;
        size_t c_end = cursor_doc_offset + cursor.length;

        //printf("pos: %zu pos_end: %zu c_start: %zu c_end: %zu\n", pos, pos_end, c_start, c_end);
        if (pos_end <= c_start) {
            /// Include
            // N: |----|
            // D:        |----|
            no_nodes[no_nodes_count++] = cursor;
        } else if (pos >= c_end) {
            /// Include
            // N:        |----|
            // D: |----|
            no_nodes[no_nodes_count++] = cursor;
        } else if (pos >= c_start && pos_end <= c_end) {
             /// transform
            // N: |-----|
            // D:  |---|
            // ----OR----
            // N: |-----|
            // D: |-----|
            PTableNode part_a = cursor;
            part_a.length  = pos - c_start;
            if (part_a.length > 0) {
                no_nodes[no_nodes_count++] = part_a;
            }

            PTableNode part_b = cursor;
            part_b.start = cursor.start + (pos_end - c_start);
            part_b.length = c_end - pos_end;
            if (part_b.length > 0) {
                no_nodes[no_nodes_count++] = part_b;
            }
        }else if (pos <= c_start && pos_end >= c_end) {
            /// completely ignore
            // N:  |---|
            // D: |-----|
        } else if (pos > c_start && pos < c_end && pos_end >= c_end) {
             /// Include / transform
            // N: |----|
            // D:    |----|
            PTableNode node = cursor;
            node.length = pos - c_start;
            if (node.length > 0) {
                no_nodes[no_nodes_count++] = node;
            }
        } else if (pos_end < c_end && pos_end > c_start && pos <= c_start) {
            /// Include / transform
            // N:   |----|
            // D: |----|
            PTableNode node = cursor;
            node.start = cursor.start + (pos_end - c_start);
            node.length = cursor.length - (pos_end - c_start);
            if (node.length > 0) {
                no_nodes[no_nodes_count++] = node;
            }
        }

        cursor_doc_offset += cursor.length;
    }

    free(table->nodes);
    table->nodes = (PTableNode*) malloc(PTABLE_INIT_NODE_SIZE * sizeof(PTableNode));
    memcpy(table->nodes, no_nodes, no_nodes_count * sizeof(PTableNode));
    table->node_count = no_nodes_count;


}

void ptable_release(PTable* table) {
    free(table->nodes);
    free(table->original.buffer);
    free(table->add.buffer);
    free(table);
}

size_t ptable_get_length(PTable* table) {

    size_t result = 0;
    for (size_t i = 0; i < table->node_count; i++) {
        result += table->nodes[i].length;
    }

    return result;
}


void ptable_print(PTable* table) {
    for (size_t i = 0; i < table->node_count; i++) {
        PTableNode* cursor = &table->nodes[i];

        char* buffer = NULL;
        switch (cursor->node_type) {
            case ORIGINAL: {
                buffer = table->original.buffer;
            } break;
            case ADDITION: {
                buffer = table->add.buffer;
            } break;
        }

        fwrite(buffer + cursor->start, 1, cursor->length, stdout);
    }
    printf("\n");
}

void ptable_print_node_sequence(PTable* table, uint8_t print_final_string) {
    printf("Original:\n");
    printf("---------\n");
    printf("%s\n", table->original.buffer);
    printf("---------\n");
    for (size_t i = 0; i < table->node_count; i++) {
        PTableNode* cursor = &table->nodes[i];
        printf("Type: ");
        switch(cursor->node_type) {
            case ORIGINAL: {
                printf("ORIGINAL ");
            } break;
            case ADDITION: {
                printf("ADDITION ");
            } break;
        }
        printf("Offset: %zu ", cursor->start);
        printf("Length: %zu\n", cursor->length);

    }

    if (print_final_string) {
        ptable_print(table);
    }
    printf("--------------------\n");
}
