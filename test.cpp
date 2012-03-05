#include "ScriptedObject.h"
#include "ScriptingContext.h"

class Test : public ScriptedObject {

private:
    int a;

public:
    Test(ScriptingContext *context, int p)
      : ScriptedObject(context), a(p)
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
            }
        }
        else if(prop == "b")
        {
            if(lua_isstring(state, 1))
                printf("Test::set_property(): b set to %s\n",
                        lua_tostring(state, 1));
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

int main()
{
    ScriptingContext script;
    Test t(&script, 1);

    return 0;
}
