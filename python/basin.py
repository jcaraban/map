from map import * ## "Parallel Map Algebra" package
import sys

setupDevices("",0x04,"") # 0x04 == GPU

## Arguments

assert len(sys.argv) > 3
in1 = sys.argv[1]
in2 = sys.argv[2]
out = sys.argv[3]

## Methods

nbh0 = [0,+1,+1, 0,-1,-1,-1, 0,+1]
nbh1 = [0, 0,+1,+1,+1, 0,-1,-1,-1]
dire = [0, 1, 2, 4, 8,16,32,64,128] # --> uchar(2**(i-1))

def outside(raster,ngb):
	ds = raster.datasize()
	idx0 = ngb[0] + index(raster,D0)
	idx1 = ngb[1] + index(raster,D1)
	out0 = (idx0 < 0) + (idx0 >= ds[0])
	out1 = (idx1 < 0) + (idx1 >= ds[1])
	return out0 + out1

def flowDir(dem,stream):
	assert( dem.datatype() == F32 )
	assert( stream.datatype() == S32 )

	maxval = zeros()
	maxpos = zeros({},U8)

	for i in Range(1,9):
		ngb = [nbh0[i],nbh1[i]]
		# flows to the steepest NeiGhBor
		zdif = dem - dem(ngb)
		dist = con(i%2==0, 1, 1.414213)
		drop = zdif / dist
		# borders always flow out
		drop = con(outside(dem,ngb), +inf, drop)
		# streams also attract the flow
		drop = con(stream, +inf, drop)
		# if using localMax, how to reduce 'i' together with drop?
		maxpos[drop > maxval] = i
		maxval[drop > maxval] = drop

	return pick(dire,maxpos) # 2**(i-1)
# flowDir

def flowDirFlat(flow):
	assert( flow.datatype() == U8 )
	# @ this loop can stop locally, once flow!=0
	while flow==0:
		for i in Range(1,9):
			ngb = [nbh0[i],nbh1[i]]
			cond = flow == 0 and flow(ngb) != 0
			flow[cond] = pick(dire,i)
	return flow
# flowDirFlat

def dir2coord(flow):
	i = con(flow==0, 0, log2(flow)+1)
	return (pick(nbh0,i),pick(nbh1,i))
	# @ could do nbh[i] overloading list.__setitem__

def catchAssign(flow,stream):
	assert( flow.datatype() == U8 )
	assert( stream.datatype() == S32 )
	catch = stream
	# @ this loop can stop locally, once catch!=0
	# @ because with D8 no other stream traces back here
	while catch==0:
		ngb = dir2coord(flow)
		cond = catch == 0 and catch(ngb) != 0
		catch[cond] = catch(ngb)
	return catch
# flowPath

def basinBorder(catch):
	border = zeros_like(catch,b8)
	for ngb in zip(nbh0,nbh1):
		cond = catch != catch(ngb)
		border[cond] = True
	return border
# basinBorder

## Computation

dem = read(in1)
stream = read(in2)

flow = flowDir(dem,stream)
flow = flowDirFlat(flow)
catch = catchAssign(flow,stream)
basin = basinDelin(catch)

write(basin,out)
