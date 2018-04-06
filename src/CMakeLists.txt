
add_subdirectory(frontend)
add_subdirectory(intermediate)
add_subdirectory(backend)
add_subdirectory(runtime)

add_subdirectory(file)
add_subdirectory(visitor)
add_subdirectory(util)

set(SOURCES
	Clock.cpp
	Config.cpp
	Key.cpp
	Pattern.cpp
	Runtime.cpp
	ThreadId.cpp
	cle/OclEnv.cpp
)

add_library(map_core OBJECT ${SOURCES})
