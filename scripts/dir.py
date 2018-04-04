from map import * ## "Map Algebra Compiler" package
import sys

setupDevices("",0x04,"") # 0x04 == GPU

## Arguments

assert len(sys.argv) > 3
in1 = sys.argv[1]
in2 = sys.argv[2]
out = sys.argv[3]

## D8

def flowD8(dem):
	nbh0 = [0,+1,+1, 0,-1,-1,-1, 0,+1]
	nbh1 = [0, 0,+1,+1,+1, 0,-1,-1,-1]
	dire = [0, 1, 2, 4, 8,16,32,64,128] # --> uchar(2**(i-1))
	dlst = []

	for ngb in zip(nbh0,nbh1):
		zdif = dem - dem(ngb)
		dist = hypot(ngb[0],ngb[1])
		drop = zdif / dist
		dlst.append(drop)

	val,pos = lmax(dlst)
	return pick(dire,pos)

## FD8

def flowFD8(dem):
	nbh0 = [0,+1,+1, 0,-1,-1,-1, 0,+1]
	nbh1 = [0, 0,+1,+1,+1, 0,-1,-1,-1]
	dire = [0, 1, 2, 4, 8,16,32,64,128] # --> uchar(2**(i-1))
	dlst = []

	for ngb in zip(nbh0,nbh1):
		zdif = dem - dem(ngb)
		dist = hypot(ngb[0],ngb[1])
		drop = zdif / dist
		posi = max(drop,0)
		lst.append(posi)

	fdir = dire * lst>0
	fmag = lst / lsum(lst)
	return fdir, fmag

## D-Inf / Ref: Tarboton97 / Ref: Steve Eddins, Mathworks

def facetFlow(e0,e1,e2,d1=1,d2=1):
	s1 = (e0 - e1) / d1
	s2 = (e1 - e2) / d2
	r = atan2(s2, s1) # flow direction in radians
	s = hypot(s1, s2) # magnitude of the slope
	angl = atan2(d2, d1)
	dist = hypot(d1, d2)
	s[r<0] = s1
	s[r > angl] = (e0 - e2) / dist
	r = min(max(r,0),angl)
	return r, s

def pixelFlow(dem,d1=1,d2=1)
	x1 = [ 1  0  0 -1 -1  0  0  1]
	y1 = [ 0 -1 -1  0  0  1  1  0]
	x2 = [ 1  1 -1 -1 -1 -1  1  1]
	y2 = [-1 -1 -1 -1  1  1  1  1]
	ac = [ 0  1  1  2  2  3  3  4]
	af = [ 1 -1  1 -1  1 -1  1 -1]
	rl = []*8
	sl = []*8

	for i in range(8):
		ngb1 = dem([ x1[i], y1[i] ])
		ngb2 = dem([ x2[i], y2[i] ])
	    rl[i], sl[i] = facetFlow(dem,ngb1,ngb2,d1,d2)

	s,k = lmax(sl)
	r = pick(af,k) * rl[k] + pick(ac,k) * pi / 2
	s[s==0] = -1
	return r, s

def flowDinf(dem):
	assert( dem.datatype() == F32 )
	# TODO: return the 2 neighbors and % of flow going to each
	return pixelFlow(dem)

##

def flowDirFlat(flow):
	assert( flow.datatype() == U8 )

	cond = flow == 0 and flow(ngb) != 0
	while cond:
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

## Computation

dem = read(in1)

flow = flowDir(dem)
flow = flowDirFlat(flow)

write(bord,out)
