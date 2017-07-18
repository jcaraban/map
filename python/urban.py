from map import * ## "Parallel Map Algebra" package
import sys, os

setupDevices("",DEV_GPU,"")
#setNumRanks(1)

assert len(sys.argv) > 2
iwd = os.path.join(sys.argv[1],'') # input directory
ext = '.tif' # raster files extension

a  = 6.4640   # Constant coefficient
b1 = 43.5404  # Elevation coefficient
b2 = 1.9150   # Slope coefficient
b3 = 41.3441  # Distance to city centers coefficients
b4 = 12.5878  # Distance to transportations coefficient
b5 = [0.,0.,-9.865,-8.746,-9.268,-8.032,-9.169,-8.942,-9.45]
# {water,urban,barren,forest,shrub,woody,herb,crop,wetlad}
d  = full(5.0)     # dispersion parameter
q  = full(16000) # max cells to become urban per year

x1 = read(iwd+'dem'+ext)     # elevation layer
x2 = read(iwd+'slope'+ext)   # slope layer
x3 = read(iwd+'center'+ext)  # distance to centers layer
x4 = read(iwd+'transp'+ext)  # distance to transportations layer
x5 = read(iwd+'landuse'+ext) # land use layer
e  = read(iwd+'excl'+ext)    # exclusion layer (e.g. water bodies)
s  = read(iwd+'urban'+ext)   # initial state: urban / not-urban
N  = full(50,[],U8) # years of simulation i.e. time steps

#x1 = stats(x1)
#x2 = stats(x2)
#x3 = stats(x3)
#x4 = stats(x4)
#x5 = stats(x5)
#e  = stats(e)
#s  = stats(s)

#while N > 0:
for i in range(50):
	z  = a + b1*x1 + b2*x2 + b3*x3 + b4*x4 + pick(x5,b5)
	pg = exp(z) / (1 + exp(z))
	pc = pg * e.no() * s.no() * fsum(s) / (3*3-1)
	pd = pc * exp(-d * (1 - pc / zmax(pc)))
	ps = q * pd / zsum(pd)
	seed = s + x1 + i
	s  = s | (ps > rand(seed))
	N = N - 1

write(s,sys.argv[2])
