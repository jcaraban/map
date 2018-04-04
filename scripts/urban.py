from map import * ## "Map Algebra Compiler" package

a  = 6.4640   # Constant coefficient
b1 = 43.5404  # Elevation coefficient
b2 = 1.9150   # Slope coefficient
b3 = 41.3441  # Distance to centers coefficients
b4 = 12.5878  # Distance to roads coefficient
b5 = [0.,0.,0.,-9.865,-8.746,-9.268,-8.032,-9.169,-8.942,-9.45]
# {null,water,urban,barren,forest,shrub,woody,herb,crop,wetlad}
d  = 5.0      # dispersion parameter
q  = 16000    # max cells to become urban per year

x1 = read('dem.tif')     # elevation layer
x2 = read('slope.tif')   # slope layer
x3 = read('center.tif')  # distance to centers layer
x4 = read('roads.tif')   # distance to roads layer
x5 = read('landuse.tif') # land use layer
e  = read('excl.tif')    # exclusion layer (e.g. water)
s  = read('urban.tif')   # initial state: urban / not-urban

def iteration(seed):
	z  = a + x1*b1 + x2*b2 + x3*b3 + x4*b4 + pick(x5,b5)
	pg = 1 / (1 + exp(z));
	pc = pg * e.no() * s.no() * fsum(s) / (3*3-1)
	pd = pc * exp(-d * (1 - pc / zmax(pc)))
	ps = q * pd / zsum(pd)
	s  = s or ps > rand(ps+seed)

## 1) Cellular automata urban development simulation

N  = 50 # years of simulation i.e. time steps

for i in range(N):
	iteration(i)

write(s,'output.tif') # write final urban state to disk

## 2) Number of years for MARIN to develop 5000 new cells?

init = read('urban.tif')   # initial urban state
zone = read('county.tif')  # zonal county later
MARIN = 30   # zone id of the Marin state of California
cells = 5000 # number of new desired urban cells

s = init
n = 0

while zsum(s - init, zone == MARIN) < cells:
	iteration(n)
	n = n + 1

print n, " years to develop "+str(cells)+" urban cells" # 26

## 3) Value of 'q' for 5000 cells to develop in just 20 years?

years = 20
s = init
q = 16000

while zsum(s - init, zone == MARIN) < cells:
	s = init
	q = q + 1000

	for i in range(years):
		iteration(i)

print q, " urbanizable cells/year are needed" # q = 21000
