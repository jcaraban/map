
set(SOURCES
	Visitor.cpp
	Simplifier.cpp
	Fusioner.cpp
	Exporter.cpp
	Lister.cpp
	Sorter.cpp
	Unlinker.cpp
	Cloner.cpp
	Predictor.cpp
	Partitioner.cpp
)

add_library(map_visit OBJECT ${SOURCES})
