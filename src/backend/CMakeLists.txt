
set(SOURCES
	Program.cpp
	Version.cpp
	Task.cpp
		task/IdentityTask.cpp
		task/LoopTask.cpp
		task/RadialTask.cpp
		task/ScalarTask.cpp
		task/TailTask.cpp
	Skeleton.cpp
		skeleton/LoopSkeleton.cpp
		skeleton/RadialSkeleton.cpp
		#skeleton/SpreadSkeleton.cpp
	Section.cpp
		skeleton/LoopSection.cpp
		skeleton/RadialSection.cpp
	util.cpp
)

add_library(map_back OBJECT ${SOURCES})