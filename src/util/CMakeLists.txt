
set(SOURCES
	StreamDir.cpp
	DataType.cpp
	NumDim.cpp
	MemOrder.cpp
	VariantType.cpp
	UnaryType.cpp
	BinaryType.cpp
	ReductionType.cpp
	DiversityType.cpp
	PercentType.cpp
	null.cpp
	common.cpp
	Mask.cpp
	ValFix.cpp
)

add_library(map_util OBJECT ${SOURCES})