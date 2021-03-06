#ifndef SCRIPTINGCONTEXT_H
#define SCRIPTINGCONTEXT_H

#include <map>

#include "lua.hpp"

#include "ScriptedObject.h"

/**
 * The only scripting context used with ScriptedObject (singleton).
 */
class ScriptingContext : private ScriptedObject::DestructionListener {

private:
    lua_State *m_State;
    std::map<int, ScriptedObject*> m_Objects;

private:
    /**
     * Lua closure used as proxy for a C++ method.
     *
     * Retrieve the object and method name from its upvalues and calls
     * ScriptedObject::call_method().
     */
    static int l_function(lua_State *state);

    /**
     * Lua __index metamethod, used to read the members of the object.
     */
    static int l_index(lua_State *state);

    /**
     * Lua __newindex metamethod, used to write the members of the object.
     */
    static int l_newindex(lua_State *state);

    ScriptedObject *getObject(int id) const;

    int proxyGet(int id, lua_State *state);
    void proxySet(int id, lua_State *state);

private:
    friend class ScriptedObject;

    /* accessed by ScriptedObject */
    void objectDestroyed(ScriptedObject *object);

    void setup();

public:
    /**
     * Constructor, creating the Lua context.
     */
    ScriptingContext();

    /**
      * Alternate constructor, getting a custom initial Lua context.
      */
    ScriptingContext(lua_State *initial);

    /**
     * Destructor, finalizing the Lua context.
     */
    ~ScriptingContext();

    lua_State *getState() const;

    /**
     * Push the given object on this context's stack.
     *
     * This might create a wrapper if one doesn't exist yet.
     */
    void pushObject(ScriptedObject *object);

};

#endif
