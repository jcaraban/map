
set(SOURCES
	MetaData.cpp
	DataStats.cpp
	Format.cpp
	File.cpp
	BinaryFile.cpp
	TiffFile.cpp
	ScalarFile.cpp
	# AutoFile.cpp	
)

add_library(map_file OBJECT ${SOURCES})
