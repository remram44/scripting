#include "ScriptedObject.h"
#include "ScriptingContext.h"

int ScriptedObject::id = 0;

ScriptedObject::ScriptedObject(ScriptingContext *context)
 :  m_ID(id++), m_Context(context)
{
    context->addObject(m_ID, this);
}

ScriptedObject::~ScriptedObject()
{
    m_Context->removeObject(m_ID);
}
