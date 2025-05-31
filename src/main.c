#include "ptable/ptable.h"


#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char** argv) {

    PTable ptable = ptable_create("Hello world");
    ptable_insert(&ptable, 11, "!");
    ptable_insert(&ptable, 0, "- ");
    ptable_insert(&ptable, 7, ".");
    ptable_print_node_sequence(&ptable, 1);

    //ptable_delete(&ptable, 7, 1);
    //ptable_print_node_sequence(&ptable, 1);

    ptable_delete(&ptable, 0, 2);
    ptable_print_node_sequence(&ptable, 1);


    printf("\nLUA TESTS\n");

    int status, result, i;
    double sum;
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    status = luaL_loadfile(L, "script.lua");
    if (status) {
        fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
        return 1;
    }

    lua_newtable(L);
    /*
     * To put values into the table, we first push the index, then the
     * value, and then call lua_rawset() with the index of the table in the
     * stack. Let's see why it's -3: In Lua, the value -1 always refers to
     * the top of the stack. When you create the table with lua_newtable(),
     * the table gets pushed into the top of the stack. When you push the
     * index and then the cell value, the stack looks like:
     *
     * <- [stack bottom] -- table, index, value [top]
     *
     * So the -1 will refer to the cell value, thus -3 is used to refer to
     * the table itself. Note that lua_rawset() pops the two last elements
     * of the stack, so that after it has been called, the table is at the
     * top of the stack.
     */
    for (i = 1; i <=5; i++) {
        lua_pushnumber(L, i);
        lua_pushnumber(L, i*2);
    }

    return 0;
}
