from map import * ## "Map Algebra Compiler" package
import sys, os

#setNumRanks(1)
setupDevices("",DEV_GPU,"")

assert len(sys.argv) > 2
iwd = os.path.join(sys.argv[1],'') # input directory
ext = '.tif' # raster files extension

a  = 6.4640   # Constant coefficient
b1 = 43.5404  # Elevation coefficient
b2 = 1.9150   # Slope coefficient
b3 = 41.3441  # Distance to city centers coefficients
b4 = 12.5878  # Distance to transportations coefficient
b5 = [0.,0.,0.,-9.865,-8.746,-9.268,-8.032,-9.169,-8.942,-9.45]
# {null,water,urban,barren,forest,shrub,woody,herb,crop,wetlad}
d  = full(5.0)     # dispersion parameter
q  = full(16000) # max cells to become urban per year

x1 = read(iwd+'dem'+ext)     # elevation layer
x2 = read(iwd+'slope'+ext)   # slope layer
x3 = read(iwd+'center'+ext)  # distance to centers layer
x4 = read(iwd+'transp'+ext)  # distance to transportations layer
x5 = read(iwd+'landuse'+ext) # land use layer
e  = read(iwd+'excl'+ext)    # exclusion layer (e.g. water bodies)
s  = read(iwd+'urban'+ext)   # initial state: urban / not-urban
N  = 50 # years of simulation i.e. time steps

for i in range(N):
	z  = a + x1*b1 + x2*b2 + x3*b3 + x4*b4 + pick(x5,b5)
	pg = 1 / (1 + exp(z));
	pc = pg * e.no() * s.no() * fsum(s) / (3*3-1)
	pd = pc * exp(-d * (1 - pc / zmax(pc)))
	ps = q * pd / zsum(pd)
	s  = s | (ps > rand(ps+i))

write(s,sys.argv[2])
