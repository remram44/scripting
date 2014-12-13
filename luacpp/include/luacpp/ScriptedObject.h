#ifndef SCRIPTEDOBJECT_H
#define SCRIPTEDOBJECT_H

#include <string>
#include <set>

#include "lua.hpp"

struct lua_State;

/**
 * A scripted object, i.e. that exists in the script context.
 *
 * These object have a counterpart in the script context which can be used to
 * call the methods and access the properties of the native object.
 *
 * All interaction from the script context comes through the abstract methods
 * set_property, get_property and call_method.
 *
 * Due to limitations of the underlying scripting languages, from the script,
 * the properties must be prefixed with "p_", and the methods with "m_".
 */
class ScriptedObject {

public:
    class DestructionListener {

    public:
        virtual ~DestructionListener() {}
        virtual void objectDestroyed(ScriptedObject *object) = 0;

    };

private:
    static int id;
    /** The ID of this object, used to identify it from the script context. */
    unsigned int m_ID;

    /**
     * The ScriptingContext's this object is associated to.
     *
     * We'll have to notify them when we are destroyed.
     */
    std::set<DestructionListener*> m_Listeners;

public:
    /**
     * Constructor.
     *
     * Assigns an ID to this object and creates its counterpart in the script
     * context.
     */
    ScriptedObject();

    /**
     * Destructor.
     *
     * Renders this object inaccessible from the script context.
     */
    virtual ~ScriptedObject();

    void registerDestructionListener(DestructionListener *listener);

    /**
     * Abstract method used to set the value of a property from a script.
     *
     * @param prop The name of the property to set.
     * @param state A Lua state, whose stack only contains the value to be set.
     */
    virtual void set_property(const std::string &prop, lua_State *state) = 0;

    /**
     * Abstract method used to get the value of a property from a script.
     *
     * @param prop The name of the property to get.
     * @param state A Lua state, on which to push the value of the property.
     */
    virtual void get_property(const std::string &prop, lua_State *state) = 0;

    /**
     * Abstract method used to call a method from a script.
     *
     * @param method The name of the method to call.
     * @param state A Lua State, containing the parameters of the method, and
     * on which the return values should be left.
     * @return The number of values to be returned.
     */
    virtual int call_method(const std::string &method, lua_State *state) = 0;

    inline unsigned int getID() const
    { return m_ID; }

};

#endif
