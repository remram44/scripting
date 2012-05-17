CXX=g++ -g
LIBS=-llua5.1
RM=rm -f
CFLAGS=-Wall -W -Wall -Wextra -I"/usr/include/lua5.1"

.PHONY: all

all: test

# Linking
test: test.o ScriptedObject.o ScriptingContext.o
	$(CXX) $(CFLAGS) test.o ScriptedObject.o ScriptingContext.o -o $@ $(LIBS)

# Compile a .cpp into a .o
%.o: %.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

# Clean up object files
clean:
	$(RM) *.o


test.o: test.cpp ScriptedObject.h lua.hpp ScriptingContext.h
ScriptedObject.o: ScriptedObject.cpp ScriptedObject.h lua.hpp
ScriptingContext.o: ScriptingContext.cpp ScriptingContext.h lua.hpp \
 ScriptedObject.h