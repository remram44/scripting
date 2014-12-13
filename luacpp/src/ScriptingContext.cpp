#include "luacpp/ScriptingContext.h"
#include "luacpp/ScriptedObject.h"

static inline unsigned int proxy_table(unsigned int id)
{
    return id | 0x40000000;
}

ScriptingContext::ScriptingContext(lua_State *initial)
{
    m_State = initial;

    setup();
}

ScriptingContext::ScriptingContext()
{
    m_State = lua_open();
    luaL_openlibs(m_State);

    setup();
}

void ScriptingContext::setup()
{
    // registry["sc"] = lightuserdata(scriptingcontext)
    lua_pushstring(m_State, "sc");
    lua_pushlightuserdata(m_State, this);
    lua_settable(m_State, LUA_REGISTRYINDEX);

    // Create the metatable for all ScriptedObject's
    lua_pushstring(m_State, "mt");

    lua_createtable(m_State, 0, 2);

    lua_pushstring(m_State, "__index");
    lua_pushcfunction(m_State, ScriptingContext::l_index);
    lua_settable(m_State, -3);

    lua_pushstring(m_State, "__newindex");
    lua_pushcfunction(m_State, ScriptingContext::l_newindex);
    lua_settable(m_State, -3);

    // registry["mt"] = metatable
    lua_settable(m_State, LUA_REGISTRYINDEX);
}

ScriptingContext::~ScriptingContext()
{
    lua_close(m_State);
}

lua_State *ScriptingContext::getState() const
{
    return m_State;
}

void ScriptingContext::pushObject(ScriptedObject *object)
{
    unsigned int id = object->getID();

    // If we haven't yet created a wrapper for this object, do it
    if(!getObject(id))
    {
        m_Objects[id] = object;

        // registry[id] = userdata(id)
        lua_pushinteger(m_State, id);

            // Create the userdata
            int* ptr = (int*)
                    lua_newuserdata(m_State, sizeof(int));
            *ptr = id;

            // Set the metatable

                // Get the metatable from the registry
                lua_pushstring(m_State, "mt");
                lua_gettable(m_State, LUA_REGISTRYINDEX);

            lua_setmetatable(m_State, -2);

        lua_settable(m_State, LUA_REGISTRYINDEX);

        // registry[id | 0x40000000] = proxy_table
        lua_pushinteger(m_State, proxy_table(id));
        lua_newtable(m_State);
        lua_settable(m_State, LUA_REGISTRYINDEX);

        object->registerDestructionListener(this);
    }

    // Find the object in the registry and place it on the context's stack
    lua_pushinteger(m_State, id);
    lua_gettable(m_State, LUA_REGISTRYINDEX);
}

void ScriptingContext::objectDestroyed(ScriptedObject *object)
{
    unsigned int id = object->getID();

    // Forget the object
    lua_pushinteger(m_State, id);
    lua_pushnil(m_State);
    lua_settable(m_State, LUA_REGISTRYINDEX);

    // Forget the proxy table
    lua_pushinteger(m_State, proxy_table(id));
    lua_pushnil(m_State);
    lua_settable(m_State, LUA_REGISTRYINDEX);
}

ScriptedObject *ScriptingContext::getObject(int id) const
{
    std::map<int, ScriptedObject*>::const_iterator it =  m_Objects.find(id);
    if(it != m_Objects.end())
        return it->second;
    else
        return NULL;
}

int ScriptingContext::proxyGet(int id, lua_State *state)
{
    // Get the proxy table from the registry
    lua_pushinteger(state, proxy_table(id));
    lua_gettable(state, LUA_REGISTRYINDEX);

    // The key is assumed to be on top of the stack when proxyGet is called
    lua_pushvalue(state, -2);

    // Get the value we need
    lua_gettable(state, -2);

    return 1;
}

void ScriptingContext::proxySet(int id, lua_State *state)
{
    // Get the proxy table from the registry
    lua_pushinteger(state, proxy_table(id));
    lua_gettable(state, LUA_REGISTRYINDEX);

    // The key and the value are on top of the stack when proxySet is called
    lua_pushvalue(state, -3); // key
    lua_pushvalue(state, -3); // value

    // Set the value we want
    lua_settable(state, -3);
}

int ScriptingContext::l_function(lua_State *state)
{
    // ID of the object
    unsigned int id = lua_tointeger(state, lua_upvalueindex(1));

    // Context
    lua_pushstring(state, "sc");
    lua_gettable(state, LUA_REGISTRYINDEX);
    ScriptingContext *context = (ScriptingContext*)lua_touserdata(state, -1);
    lua_pop(state, 1);

    // Retrieve the object
    ScriptedObject *object = context->getObject(id);
    if(object == NULL)
        // This is actually weird, because it would have been deleted between
        // the method was read and its actual call
        // It means the script store the method somewhere and called it later
        luaL_error(state, "calling method on already destroyed C++ object");

    // Check the first parameter, which should be the object
    if(lua_gettop(state) < 1)
        luaL_error(state,
                "calling method with no argument - did you use '.' instead of "
                "':'?");
    int *arg0 = (int*)lua_touserdata(state, 1);
    if(arg0 == NULL)
        luaL_error(state, "calling method on unknown object");
    if(*arg0 != id)
        luaL_error(state, "calling method on different object");

    // Retrieve the method name
    const char *method = lua_tostring(state, lua_upvalueindex(2));

    // Prepare a different state on which to pass the argument and retrieve
    // the results
    // We don't want call_method() to get the userdata...
    int argc = lua_gettop(state) - 1;
    lua_State *state2 = lua_newthread(state);
    lua_xmove(state, state2, argc);

    // Actual call
    int nret = object->call_method(method, state2);

    // Move the return values back on the state and return
    if(lua_gettop(state2) < nret)
        luaL_error(
                state,
                "error in C++ code: %d values to be returned but only %d on "
                "the stack",
                nret, lua_gettop(state2));
    lua_xmove(state2, state, nret);
    return nret;
}

int ScriptingContext::l_index(lua_State *state)
{
    lua_pushstring(state, "sc");
    lua_gettable(state, LUA_REGISTRYINDEX);
    ScriptingContext *context = (ScriptingContext*)lua_touserdata(state, -1);
    lua_pop(state, 1);

    // __index(userdata, key)
    int *ptr = (int*)lua_touserdata(state, 1);
    ScriptedObject *object = context->getObject(*ptr);

    if(object == NULL)
        luaL_error(state, "accessed an already destroyed C++ object");

    if(lua_isstring(state, 2))
    {
        // Warning: we can't use longjmp() (and thus lua_error) if we have C++
        // objects on the stack!
        // We use a pointer to manually deallocate it before jumping
        std::string *key = new std::string(lua_tostring(state, 2));

        if(key->substr(0, 2) == "p_") // Property
        {
            // Create another empty state on which to write the value to be returned
            lua_State *state2 = lua_newthread(state);
            object->get_property(key->substr(2), state2);
            if(lua_gettop(state2) != 1)
            {
                lua_pushfstring(
                        state,
                        "error in C++ code: %d values returned by the object "
                        "for property %s",
                        lua_gettop(state2), key->substr(2).c_str()
                        );
                delete key;
                lua_error(state);
            }
            lua_xmove(state2, state, 1);
            delete key;
            return 1;
        }
        else if(key->substr(0, 2) == "m_") // Method
        {
            lua_pushinteger(state, object->getID());
            lua_pushstring(state, key->substr(2).c_str());
            lua_pushcclosure(state, ScriptingContext::l_function, 2);
            delete key;
            return 1;
        }

        delete key;
    }

    // Does not concern us, retrieve from the proxy table
    return context->proxyGet(*ptr, state);
}

int ScriptingContext::l_newindex(lua_State *state)
{
    lua_pushstring(state, "sc");
    lua_gettable(state, LUA_REGISTRYINDEX);
    ScriptingContext *context = (ScriptingContext*)lua_touserdata(state, -1);
    lua_pop(state, 1);

    // __newindex(userdata, key, value)
    int *ptr = (int*)lua_touserdata(state, 1);
    ScriptedObject *object = context->getObject(*ptr);

    if(object == NULL)
        luaL_error(state, "accessed an already destroyed C++ object");

    if(lua_isstring(state, 2))
    {
        std::string key = lua_tostring(state, 2);

        if(key.substr(0, 2) == "p_") // Property
        {
            // Create another state on which there is only the value to be assigned
            lua_State *state2 = lua_newthread(state);
            lua_pushvalue(state, -2);
            lua_xmove(state, state2, 1);
            object->set_property(key.substr(2), state2);
        }
        else if(key.substr(0, 2) == "m_") // Method, can't assign!
            luaL_error(state, "Attempted to assign a method!");
    }

    // Does not concern us, set in proxy table
    context->proxySet(*ptr, state);
    return 0;
}
