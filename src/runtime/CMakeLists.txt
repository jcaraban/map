
set(SOURCES
	Cache.cpp
	Scheduler.cpp
	Worker.cpp
	Job.cpp
	Entry.cpp
	Block.cpp
		block/Block0.cpp
		block/Block1.cpp
		block/BlockN.cpp
		block/BlockList.cpp
)

add_library(map_runt OBJECT ${SOURCES})
