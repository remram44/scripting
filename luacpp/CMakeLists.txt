# Build the library
SET(LIBCPP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include"
    CACHE INTERNAL "libcpp include directory")
INCLUDE_DIRECTORIES("${LIBCPP_INCLUDE_DIR}")
ADD_LIBRARY(luacpp STATIC src/ScriptedObject.cpp src/ScriptingContext.cpp)
