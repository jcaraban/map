from map import * ## "Parallel Map Algebra" package
import sys

setupDevices("",0x04,"") # 0x04 == GPU

## Arguments

assert len(sys.argv) > 3
in1 = sys.argv[1]
in2 = sys.argv[2]
out = sys.argv[3]

def flowAccu(water,flow):
	assert( water.numdim() == D2 )
	assert( flow.numdim() == D2 )
	assert( flow.datatype() == U8 )

	acu = zeros_like(water)
	while water:
		ngb = dir2coord(flow)
		acu += water # registers the passing water
		water = spreadSum(water,ngb)

	return acu
# flowAccu

## Computation

water = read(in1)
flow = read(in2)

accu = flowAccu(water,flow)

write(accu,out)

##

acu = zeros_like(water)
delta water:
	ngb = dir2coord(flow)
	acu += water # registers the passing water
	water = spreadSum(water,ngb)

## TOPO op

acu = intrinsic_flowAccu(water,flow)