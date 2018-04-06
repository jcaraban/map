
set(SOURCES
	dag.cpp
	Node.cpp
	Cluster.cpp
	util.cpp
)

set(SOURCES ${SOURCES}
	dag/Constant.cpp
	dag/Empty.cpp
	dag/Index.cpp
	dag/Identity.cpp
	dag/Rand.cpp
	dag/Cast.cpp
	dag/Unary.cpp
	dag/Binary.cpp
	dag/Conditional.cpp
	dag/Diversity.cpp
	dag/Neighbor.cpp
	dag/BoundedNeighbor.cpp
	dag/SpreadNeighbor.cpp
	dag/Convolution.cpp
	dag/FocalFunc.cpp
	dag/FocalPercent.cpp
	dag/ZonalReduc.cpp
	dag/RadialScan.cpp
	dag/SpreadScan.cpp
	dag/IO.cpp
	dag/Read.cpp
	dag/Write.cpp
	dag/Scalar.cpp
	dag/Temporal.cpp
	dag/Access.cpp
	dag/LhsAccess.cpp
	dag/Summary.cpp
	dag/DataSummary.cpp
	dag/BlockSummary.cpp
	dag/GroupSummary.cpp
	dag/Barrier.cpp
	dag/Checkpoint.cpp
	dag/LoopCond.cpp
	dag/LoopHead.cpp
	dag/LoopTail.cpp
	dag/Merge.cpp
	dag/Switch.cpp
)

add_library(map_inter OBJECT ${SOURCES})
