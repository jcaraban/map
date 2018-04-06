# TODO: move Task.xpp/ Block.xpp/ Skeleton.xpp /dag.xpp /visitor.xpp out of their folders?

# Compiler
CC = g++
# -O3 -march=native -mtune=native
CFLAGS = -std=c++14 -m64 -fpic -Og -g
IDIR = -I/opt/intel/opencl/include/ -I/opt/AMDAPP/include/ -I/usr/local/cuda/include/
LDIR = -L/opt/amdgpu-pro/lib/x86_64-linux-gnu/
LIBS = -ltiff -pthread
LDFLAGS = $(LDIR) $(LIBS)

# OS dependent stuff
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LIBS += -framework OpenCL -Wa,-q -lstdc++
	RAND123 = "/Users/jesus/jesus/code/map/thirdparty/Random123-1.09/include/"
else ifeq ($(UNAME_S),Linux)
	LIBS += -lOpenCL
	RAND123 = "/home/jcaraban/jesus/code/map/thirdparty/Random123-1.09/include/"
endif

# Directories
SRC = src
INC = src
OBJ = obj

GENERAL  = Config Clock Pattern ThreadId Key Runtime
FRONTEND = $(addprefix frontend/, Raster bindings LoopAssembler)
INTERMED = $(addprefix intermediate/, dag Node Cluster util)
DAG      = $(addprefix intermediate/dag/, Constant Empty Index Identity Rand Cast Unary Binary Conditional Diversity Neighbor BoundedNeighbor SpreadNeighbor Convolution FocalFunc FocalPercent ZonalReduc RadialScan SpreadScan IO Read Write Scalar Temporal Access LhsAccess Summary DataSummary BlockSummary GroupSummary Barrier Checkpoint LoopCond LoopHead LoopTail Merge Switch)
VISITOR  = $(addprefix visitor/, Visitor Simplifier Fusioner Exporter Lister Sorter Unlinker Cloner Predictor Partitioner)
BACKEND  = $(addprefix backend/, Program Version Skeleton Section util Task)
SKELETON = $(addprefix backend/skeleton/, RadialSkeleton LoopSkeleton)
SECTION  = $(addprefix backend/skeleton/, RadialSection LoopSection)
TASK     = $(addprefix backend/task/, ScalarTask RadialTask LoopTask TailTask IdentityTask) # SpreadTask
RUNTIME  = $(addprefix runtime/, Cache Scheduler Worker Job Entry Block)
BLOCK    = $(addprefix runtime/block/, Block0 Block1 BlockN BlockList)
UTIL     = $(addprefix util/, StreamDir DataType NumDim MemOrder VariantType UnaryType BinaryType ReductionType DiversityType PercentType null common Mask ValFix)
FILE     = $(addprefix file/, File Format MetaData DataStats TiffFile BinaryFile ScalarFile)
OCLENV   = $(addprefix cle/, OclEnv)
ALL = $(GENERAL) $(FRONTEND) $(INTERMED) $(DAG) $(VISITOR) $(BACKEND) $(SKELETON) $(SECTION) $(TASK) $(RUNTIME) $(BLOCK) $(UTIL) $(FILE) $(OCLENV)

SOURCES  = $(addprefix $(SRC)/, $(addsuffix .cpp, $(ALL)))
HEADERS  = $(addprefix $(INC)/, $(addsuffix .hpp, $(ALL)))
OBJECTS  = $(addprefix $(OBJ)/, $(addsuffix .o, $(ALL)))

# Binaries pattern rule
bin/%: examples/%.cpp lib/libmap.so
	$(CC) $(CFLAGS) $< -Llib/ -lmap -o $@

# Shared library
lib/libmap.so: $(OBJECTS)
	$(CC) $(CFLAGS) $(IDIR) $(OBJECTS) -shared $(LDFLAGS) -o lib/libmap.so

# Object files pattern rule
$(OBJ)/%.o: $(SRC)/%.cpp
	mkdir -p $(dir $@) # create dir if needed
	$(CC) $(CFLAGS) -c $< -o $@

# Object files special cases
obj/cle/OclEnv.o: src/cle/OclEnv.cpp
	mkdir -p obj/cle/
	$(CC) $(CFLAGS) $(IDIR) -c $< -o $@

obj/backend/Version.o: src/backend/Version.cpp
	mkdir -p obj/backend/
	$(CC) $(CFLAGS) -D RAND123=$(RAND123) -c $< -o $@


.PHONY: clean
clean:
	rm $(OBJECTS) bin/* lib/libmap.so
