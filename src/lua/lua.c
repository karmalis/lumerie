#include "lua.h"

#include <lualib.h>
#include <lauxlib.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

lua_State* lua_init() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    return L;
}

void lua_shutdown(lua_State* L) {
    lua_close(L);
}

int32_t lua_load_file(lua_State* L, const char* script) {
    int result = 0;

    int status = luaL_loadfile(L, script);

    if (status) {
        fprintf(stderr, "Error loading lua script: %s\n", lua_tostring(L, -1));
        result = -1;
    }

    return result;
}

int32_t lua_exec_script(lua_State* L) {
    int result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result) {
        fprintf(stderr, "Error executing script: %s\n", lua_tostring(L, -1));
    }

    return result;
}


int32_t lua_call_va(lua_State* L, const char* func, const char* sig, ...) {
    va_list vl;
    int narg, nres;

    va_start(vl, sig);
    lua_getglobal(L, func);

    narg = 0;
    while (*sig) {
        switch(*sig++) {
            case 'd': /* double*/
                lua_pushnumber(L, va_arg(vl, double));
                break;
            case 'i':
                lua_pushnumber(L, va_arg(vl, int));
                break;
            case 's':
                lua_pushstring(L, va_arg(vl, char *));
                break;
            case '>':
                goto endwhile;
            default:
                fprintf(stderr, "Invalid option (%c)\n", *(sig - 1));
        }
        narg++;
        luaL_checkstack(L, 1, "too many arguments");
    } endwhile:

    nres = strlen(sig);
    if (lua_pcall(L, narg, nres, 0) != 0) {
        fprintf(stderr, "error running function %s: %s", func, lua_tostring(L, -1));
        return -1;
    }

    nres = -nres;
    while(*sig) {
        switch (*sig++) {
            case 'd':
                if(!lua_isnumber(L, nres)) {
                    fprintf(stderr, "wrong result type");
                    break;
                }
                *va_arg(vl, double *) = lua_tonumber(L, nres);
                break;
            case 'i':
                if (!lua_isnumber(L, nres)) {
                    fprintf(stderr, "wrong result type");
                    break;
                }
                *va_arg(vl, int *) = lua_tonumber(L, nres);
                break;
            case 's':
                if (!lua_isstring(L, nres)) {
                    fprintf(stderr, "wrong result type");
                    break;
                }
                *va_arg(vl, const char**) = lua_tostring(L, nres);
                break;
            default:
                fprintf(stderr, "ivalid option (%c)", *(sig - 1));
        }
        nres++;
    }
    va_end(vl);

    return -nres;
}


void lua_stack_dump(lua_State* L) {
    int i;
    int top = lua_gettop(L);
    for (i = 1; i <= top; i++) {
        int t = lua_type(L, i);
        switch (t) {
            case LUA_TSTRING:
                printf("%s", lua_tostring(L, i));
                break;
            case LUA_TBOOLEAN:
                printf(lua_toboolean(L, i) ? "true" : "fasle");
                break;
            case LUA_TNUMBER:
                printf("%g", lua_tonumber(L, i));
                break;
            default:
                printf("%s", lua_typename(L, t));
                break;
        }
        printf("\n");
    }
    printf("\n");
}
