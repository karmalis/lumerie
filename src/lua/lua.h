#ifndef LUA_H_
#define LUA_H_

#include <lua.h>
#include <stdint.h>

lua_State* lua_init();
void lua_shutdown(lua_State* L);

int32_t lua_load_file(lua_State* L, const char* file);
int32_t lua_exec_script(lua_State* L);

/**
 * Generic call lua function method
 *
 * Usage
 *
 * char * res;
 * double d_var;
 * int i_var;
 *
 * lua_call_va(L, "foo", "di>s", d_var, i_var, &res);
 * */
int32_t lua_call_va(lua_State* L, const char* func, const char* sig, ...);

void lua_stack_dump(lua_State* L);


#endif // LUA_H_
