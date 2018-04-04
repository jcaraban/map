from map import * ## "Map Algebra Compiler" package
import sys

setupDevices("",0x04,"") # 0x04 == GPU

## Arguments

assert len(sys.argv) > 2
in_file_path = sys.argv[1]
out_file_path = sys.argv[2]

## Methods

nbh0 = [1,1,0,-1,-1,-1,0,1]
nbh1 = [0,1,1,1,0,-1,-1,-1]

def border(raster):
	ds = raster.datasize()
	idx0 = index(raster,D0)
	idx1 = index(raster,D1)
	brd0 = idx0 == 0 || idx0 == ds[0]-1
	brd1 = idx1 == 0 || idx1 == ds[1]-1
	return brd0 || brd1

def pitFill(orig, stream):
	acti = stream || border(orig)
	elev = con(acti, orig, +inf)

	while zsum(acti):
		elst = []*9
		alst = []*9
		for i in range(9):
			ngb = (nbh0[i],nbh1[i])
			alst[i] = acti(ngb) and elev(ngb) < elev and elev > orig
			elst[i] = con(alst[i], max(elev(ngb),orig), elev )
		elev = lmin(elst)
		acti = lor(alst)

	return elev
# pitFill

def pitFill(orig, stream):
	acti = stream || border(orig)
	elev = con(acti, orig, +inf)
	nbh  = neighborhood([3,3])

	while zsum(acti):
		clst = elev(nbh) < elev and elev(nbh) > orig
		elst = con(clst, elev(nbh), elev)
		elev = lmin(elst)
		acti = lor(clst)

	return elev
# pitFill

def pitFill(orig, stream):
	acti = stream || border(dem)
	elev = con(acti, dem, +inf)
	nbh  = neighborhood([3,3])

	while zsum(acti):
		newe = lmin( max( min( elev(nbh) , elev ) , orig ) )
		acti = elev != newe
		elev = newe

	return elev
# pitFill

## Computation

dem = read(in_file_path)
stream = zeros_like(dem)
out = pitFill(dem,stream)
write(out, out_file_path)
