# TODO: move Task.xpp/ Block.xpp/ Skeleton.xpp /dag.xpp /visitor.xpp out of their folders?

# Compiler
CC = g++
# -O3 -march=native -mtune=native
CFLAGS = -std=c++11 -m64 -fpic -O0 -g
IDIR = -I/opt/AMDAPP/include/ -I/usr/local/cuda/include/
LDIR = 
LIBS = -ltiff -pthread
LDFLAGS = $(LDIR) $(LIBS)

# OS dependent stuff
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CFLAGS += -framework OpenCL -Wa,-q -lstdc++
	IDIR += -D RAND123="/Users/jesus/jesus/code/map/thirdparty/Random123-1.09/include/"
else ifeq ($(UNAME_S),Linux)
	LIBS += -lOpenCL
	IDIR += -D RAND123="/home/jcaraban/jesus/code/map/thirdparty/Random123-1.09/include/"
endif

# Sources
S_FRON = $(addprefix front/, Raster.cpp bindings.cpp)
S_UTIL = $(addprefix util/, StreamDir.cpp DataType.cpp NumDim.cpp MemOrder.cpp VariantType.cpp UnaryType.cpp BinaryType.cpp ReductionType.cpp DiversityType.cpp PercentType.cpp null.cpp common.cpp Mask.cpp ValFix.cpp)
S_RUNT = $(addprefix runtime/, Runtime.cpp Clock.cpp Program.cpp Cache.cpp Scheduler.cpp Worker.cpp Job.cpp Key.cpp Entry.cpp Pattern.cpp Version.cpp ThreadId.cpp LoopAssembler.cpp)
S_DAG  = $(addprefix runtime/dag/, dag.cpp util.cpp Node.cpp Cluster.cpp Constant.cpp Empty.cpp Index.cpp Identity.cpp Rand.cpp Cast.cpp Unary.cpp Binary.cpp Conditional.cpp Diversity.cpp Neighbor.cpp BoundedNeighbor.cpp SpreadNeighbor.cpp Convolution.cpp FocalFunc.cpp FocalPercent.cpp ZonalReduc.cpp RadialScan.cpp SpreadScan.cpp IO.cpp Read.cpp Write.cpp Scalar.cpp Temporal.cpp Access.cpp LhsAccess.cpp Summary.cpp DataSummary.cpp BlockSummary.cpp GroupSummary.cpp Barrier.cpp Checkpoint.cpp LoopCond.cpp LoopHead.cpp LoopTail.cpp Merge.cpp Switch.cpp)
S_VISI = $(addprefix runtime/visitor/, Visitor.cpp Simplifier.cpp Fusioner.cpp Exporter.cpp Lister.cpp Sorter.cpp Unlinker.cpp Cloner.cpp Predictor.cpp Partitioner.cpp)
S_TASK = $(addprefix runtime/task/, Task.cpp ScalarTask.cpp RadialTask.cpp LoopTask.cpp TailTask.cpp IdentityTask.cpp) # SpreadTask.cpp 
S_BLK  = $(addprefix runtime/block/, Block.cpp Block0.cpp Block1.cpp BlockN.cpp BlockList.cpp)
S_SKEL = $(addprefix runtime/skeleton/, util.cpp Skeleton.cpp RadialSkeleton.cpp LoopSkeleton.cpp)
S_FILE = $(addprefix file/, File.cpp Format.cpp MetaData.cpp DataStats.cpp tiff.cpp binary.cpp scalar.cpp)
S_OCL  = $(addprefix cle/, OclEnv.cpp)
S_ALL  = $(S_FRON) $(S_UTIL) $(S_RUNT) $(S_DAG) $(S_VISI) $(S_TASK) $(S_BLK) $(S_SKEL) $(S_FILE) $(S_OCL)

# Headers
H_FRON = $(addprefix front/, Raster.hpp bindings.hpp)
H_UTIL = $(addprefix util/, util.hpp StreamDir.hpp DataType.hpp NumDim.hpp MemOrder.hpp Array.hpp Array4.hpp VariantType.hpp UnaryType.hpp BinaryType.hpp ReductionType.hpp DiversityType.hpp PercentType.hpp null.hpp common.hpp Mask.hpp Direction.hpp ValFix.hpp)
H_RUNT = $(addprefix runtime/, Runtime.hpp Config.hpp Clock.hpp Program.hpp Cache.hpp Scheduler.hpp Worker.hpp Job.hpp Key.hpp Entry.hpp Pattern.hpp Version.hpp ThreadId.hpp LoopAssembler.hpp)
H_DAG  = $(addprefix runtime/dag/, dag.hpp util.hpp Node.hpp Cluster.hpp Constant.hpp Empty.hpp Index.hpp Identity.hpp Rand.hpp Cast.hpp Unary.hpp Binary.hpp Conditional.hpp Diversity.hpp Neighbor.hpp BoundedNeighbor.hpp SpreadNeighbor.hpp Convolution.hpp FocalFunc.hpp FocalPercent.hpp ZonalReduc.hpp RadialScan.hpp SpreadScan.hpp IO.hpp Read.hpp Write.hpp Scalar.hpp Temporal.hpp Access.hpp LhsAccess.hpp Summary.hpp DataSummary.cpp BlockSummary.cpp GroupSummary.cpp Barrier.hpp Checkpoint.hpp LoopCond.hpp LoopHead.hpp LoopTail.hpp Merge.hpp Switch.hpp)
H_VISI = $(addprefix runtime/visitor/, Visitor.hpp Simplifier.hpp Fusioner.hpp Exporter.hpp Lister.hpp Sorter.hpp Unlinker.hpp Cloner.hpp Predictor.hpp Partitioner.hpp)
H_TASK = $(addprefix runtime/task/, Task.hpp ScalarTask.hpp RadialTask.hpp LoopTask.hpp TailTask.hpp IdentityTask.hpp) # SpreadTask.hpp
H_BLK  = $(addprefix runtime/block/, Block.hpp Block0.hpp Block1.hpp BlockN.hpp)
H_SKEL = $(addprefix runtime/skeleton/, util.hpp Skeleton.hpp RadialSkeleton.hpp LoopSkeleton.hpp)
H_FILE = $(addprefix file/, File.hpp Format.hpp MetaData.hpp DataStats.hpp tiff.hpp binary.hpp scalar.hpp)
H_OCL  = $(addprefix cle/, cle.hpp OclEnv.hpp)
H_ALL  = $(H_FRON) $(H_UTIL) $(H_RUNT) $(H_DAG) $(H_VISI) $(H_TASK) $(H_BLK) $(H_SKEL) $(H_FILE) $(H_OCL)

# Objects
O_ALL  = $(S_ALL:.cpp=.o)

# Dependencies
DEP = $(O_ALL)

# Rules
.cpp.o: $(O_ALL)
	$(CC) $(CFLAGS) $(IDIR) -c $< -o $@

# TODO: try static pattern rule to eliminate the code-bloat below

main: $(DEP) bin/main.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/main.cpp $(LDFLAGS) -o $@

wsum: $(DEP) bin/wsum.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/wsum.cpp $(LDFLAGS) -o $@

diver: $(DEP) bin/diver.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/diver.cpp $(LDFLAGS) -o $@

stats: $(DEP) bin/stats.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/stats.cpp $(LDFLAGS) -o $@

hill: $(DEP) bin/hill.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/hill.cpp $(LDFLAGS) -o $@

focal: $(DEP) bin/focal.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/focal.cpp $(LDFLAGS) -o $@

life: $(DEP) bin/life.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/life.cpp $(LDFLAGS) -o $@

seism: $(DEP) bin/seism.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/seism.cpp $(LDFLAGS) -o $@

view: $(DEP) bin/view.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/view.cpp $(LDFLAGS) -o $@

flow: $(DEP) bin/flow.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/flow.cpp $(LDFLAGS) -o $@

urban: $(DEP) bin/urban.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/urban.cpp $(LDFLAGS) -o $@

urban_mc: $(DEP) bin/urban_mc.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/urban_mc.cpp $(LDFLAGS) -o $@

urban_ga: $(DEP) bin/urban_ga.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/urban_ga.cpp $(LDFLAGS) -o $@

water: $(DEP) bin/water.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/water.cpp $(LDFLAGS) -o $@

olive: $(DEP) bin/olive.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/olive.cpp $(LDFLAGS) -o $@

file_translate: $(DEP) bin/file_translate.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/file_translate.cpp $(LDFLAGS) -o $@

gen_data: $(DEP) bin/gen_data.cpp
	$(CC) $(CFLAGS) $(IDIR) $(DEP) bin/gen_data.cpp $(LDFLAGS) -o $@

library: $(DEP)
	$(CC) $(CFLAGS) $(IDIR) $(DEP) -shared $(LDFLAGS) -o libmap.so

FILTER_OUT = $(foreach v,$(2),$(if $(findstring $(1),$(v)),,$(v)))
NON_O = $(strip $(call FILTER_OUT,.o,$(O_ALL)))

clean:
ifneq (,$(NON_O))
$(info Attempted to delete non .o files: $(NON_O))
else
	rm $(O_ALL)
endif
