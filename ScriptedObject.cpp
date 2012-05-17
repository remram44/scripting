#include "ScriptedObject.h"

int ScriptedObject::id = 0;

ScriptedObject::ScriptedObject()
 :  m_ID(id++)
{
}

ScriptedObject::~ScriptedObject()
{
    std::set<ScriptedObject::DestructionListener*>::iterator it;
    it = m_Listeners.begin();
    for(; it != m_Listeners.end(); ++it)
        (*it)->objectDestroyed(this);
}

void ScriptedObject::registerDestructionListener(DestructionListener *listener)
{
    m_Listeners.insert(listener);
}
