/**
 * @file lua.hpp This header includes Lua's headers, in an extern "C" block,
 * since <lua.hpp> is only distributed by Debian.
 */

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
