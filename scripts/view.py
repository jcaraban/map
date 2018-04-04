from os import sys, path
if __name__ == '__main__' and __package__ is None:    
    sys.path.append(path.dirname(path.dirname(path.abspath(__file__))))
# moves path to parent directory
from map import * ## "Map Algebra Compiler" package

#setNumRanks(1)
setupDevices("",DEV_GPU,"")

## Arguments

assert len(sys.argv) > 5

in_file_path = sys.argv[1]
out_file_path = sys.argv[2]
obs_x = int(sys.argv[3])
obs_y = int(sys.argv[4])
obs_h = float(sys.argv[5])

## Methods

def distance(raster,coord):
	x = coord[0] - index(raster,D1);
	y = coord[1] - index(raster,D2);
	return sqrt(x*x + y*y + 0.0);

def viewshed(dem,obs,h):
	#dem[obs] += h
	#dem = __setitem__(dem,obs,__getitem__(dem,obs)+h)
	shift = dem - h
	dist = distance(dem,obs)
	slope = shift / dist
	maxr = rmax(slope,obs)
	view = (maxr * dist + h) - dem
	#view = (maxr * dist + dem[obs]) - dem
	return view

## Computation

f = [ [1.0/16, 1.0/8, 1.0/16], [1.0/8, 1.0/4, 1.0/8], [1.0/16, 1.0/8, 1.0/16] ]

dem  = read(in_file_path)
norm = (dem - zmin(dem)) / (zmax(dem) - zmin(dem))
view = viewshed(norm, [obs_x,obs_y], obs_h)
soft = convolve(view, f)
out = con(soft < 0.1, dem, zeros_like(dem))
write(out, out_file_path)
