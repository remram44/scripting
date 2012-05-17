#include <cstdio>

#include "ScriptedObject.h"
#include "ScriptingContext.h"

class Test : public ScriptedObject {

private:
    int a;

public:
    Test(int p)
      : a(p)
    {
    }

protected:
    void set_property(const std::string &prop, lua_State *state)
    {
        if(prop == "a")
        {
            if(lua_isnumber(state, 1))
            {
                a = lua_tointeger(state, 1);
                printf("Test::set_property(): a set to %d\n", a);
                return ;
            }
        }
        else if(prop == "b")
        {
            if(lua_isstring(state, 1))
            {
                printf("Test::set_property(): b set to %s\n",
                        lua_tostring(state, 1));
                return ;
            }
        }
        else
            luaL_error(state, "Test::set_property(): unknown property %s\n",
                    prop.c_str());
        luaL_error(state, "Test::set_property(): invalid value for %s\n",
                prop.c_str());
    }

    void get_property(const std::string &prop, lua_State *state)
    {
        if(prop == "a")
        {
            printf("Test::get_property(): returning a = %d\n", a);
            lua_pushinteger(state, a);
        }
        else if(prop == "b")
        {
            printf("Test::get_property(): returning b = coucou\n");
            lua_pushstring(state, "coucou");
        }
        else
            luaL_error(state, "Test::get_property(): unknown property %s\n",
                    prop.c_str());
    }

    int call_method(const std::string &method, lua_State *state)
    {
        if(method == "print")
        {
            if(lua_gettop(state) == 0)
            {
                print();
                return 0;
            }
        }
        else
            luaL_error(state, "Test::call_method(): unknown method %s\n",
                    method.c_str());
        luaL_error(state, "Test::call_method(): invalid arguments for %s\n",
                method.c_str());
        return 0; // Won't be reached
    }

    void print()
    {
        printf("Test::print(): a = %d\n", a);
    }

};

ScriptingContext *script = NULL;
Test *test = NULL;

int l_test(lua_State*)
{
    script->pushObject(test);
    return 1;
}

int main()
{
    // Create a context
    script = new ScriptingContext();

    // Create an object
    test = new Test(1);

    // Add a global function in the context
    {
        lua_State *state = script->getState();
        lua_pushstring(state, "foo");
        lua_pushcfunction(state, l_test);
        lua_settable(state, LUA_GLOBALSINDEX);
    }

    // Execute some code
    {
        lua_State *state = script->getState();
        luaL_loadstring(
                state,
                "t = foo()\n"
                "print(t.p_a)\n"
                "--t.m_print()\n"
                "t.p_a = 12\n"
                "print(t.p_a)\n"
                "--t.m_print()\n"
                "t.p_b = 5\n"
                "print(t.p_b)\n"
                "t = foo()\n"
                "print(t.p_a)\n"
                "--t.m_print()\n");
        if(lua_isstring(state, -1))
            printf("str: %s\n", lua_tostring(state, -1));
        else
            lua_call(state, 0, 0);
    }

    delete test;
    delete script;

    return 0;
}
