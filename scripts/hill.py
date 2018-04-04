from os import sys, path
if __name__ == '__main__' and __package__ is None:    
    sys.path.append(path.dirname(path.dirname(path.abspath(__file__))))
# moves path to parent directory
from map import * ## "Map Algebra Compiler" package


#setNumRanks(1)
setupDevices("",DEV_GPU,"")

## Arguments

assert len(sys.argv) > 2
in_file_path = sys.argv[1]
out_file_path = sys.argv[2]
PI = 3.141593

## Methods

def hori(dem, dist):
	h = [ [-1, 0, 1],
		  [-2, 0, 2],
		  [-1, 0, 1] ]
	return convolve(dem,h) / (8 * dist)

def vert(dem, dist) :
	v = [ [-1, -2, -1],
		  [ 0,  0,  0],
		  [ 1,  2,  1] ]
	return convolve(dem,v) / (8 * dist)

def slope(dem, zf=1, dist=1):
	x = hori(dem,dist)
	y = vert(dem,dist)
	z = atan(zf * sqrt(x*x + y*y))
	return z

def aspect(dem, dist=1):
	x = hori(dem,dist)
	y = vert(dem,dist)
	z1 = (x!=0) * atan2(y,-x)
	z1 = z1 + (z1<0) * (PI*2)
	z0 = (x==0) * ((y>0)*(PI/2) + (y<0)*(PI*2-PI/2))
	return z1 + z0

def hillshade(dem, zenith, azimuth, zfactor=1):
	zr = (90 - float(zenith)) / 180 * PI
	ar = 360 - float(azimuth) + 90
	ar = ar - 360 if (ar > 360) else ar
	ar = ar / 180 * PI
	hs = ( cos(zr) * cos(slope(dem)) +
		   sin(zr) * sin(slope(dem)) *
		   cos(ar - aspect(dem)) )
	return hs

## Computation

dem = read(in_file_path)
out = hillshade(dem,45,315)
write(out, out_file_path)
