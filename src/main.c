#include "ptable/ptable.h"
#include "lua/lua.h"
#include "editor/terminal.h"

#include <stdlib.h>
#include <stdio.h>


int main(int argc, char** argv) {

    PTable* ptable = ptable_create("Hello world");
    ptable_insert(ptable, 11, "!");
    ptable_insert(ptable, 0, "- ");
    ptable_insert(ptable, 7, ".");
    ptable_print_node_sequence(ptable, 1);

    //ptable_delete(&ptable, 7, 1);
    //ptable_print_node_sequence(&ptable, 1);

    ptable_delete(ptable, 0, 2);
    ptable_print_node_sequence(ptable, 1);

    // Lua testing // possible init
    lua_State* L = lua_init();
    int32_t result = lua_load_file(L, "scripts/setup.lua");
    if (result) return result;

    result = lua_exec_script(L);
    if (result) return result;

    // Experimental loop for beginings
    return terminal_loop(L);
}
