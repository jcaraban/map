from map import * ## "Map Algebra Compiler" package
import sys

setupDevices("",0x04,"") # 0x04 == GPU

## Arguments

assert len(sys.argv) > 3
in_file_path = sys.argv[1]
out_file_path = sys.argv[2]
coord_x = int(sys.argv[3])
coord_y = int(sys.argv[4])

## Computation

inf  = float("inf")
nbh0 = [+1,+1, 0,-1,-1,-1, 0,+1]
nbh1 = [ 0,+1,+1,+1, 0,-1,-1,-1]

surf = read(in_file_path)
cost = full_like(surf,+inf)
acti = zeros_like(surf,B8)
start = [coord_x,coord_y]
cost[start] = 0
acti[start] = 1
cell[start] = 37 # cell id

assert value(zand(surf >= 0))

while zor(acti):
	cost_ = []*9
	acti_ = []*9
	for i in range(9):
		ngb = (nbh0[i],nbh1[i])
		acti_[i] = acti(ngb) and cost(ngb)+surf < cost
		cost_[i] = con(acti_[i], cost(ngb)+surf, cost)
		# cell ?
	cost = lmin(cost_)
	acti = lor(acti_)

write(bord, out_file_path)

##

while delta(cost):
	for i in range(9):
		ngb = (nbh0[i],nbh1[i])
		cost = min( cost(ngb)+surf , cost)

## preferred, only cost

nbh  = neighborhood([3,3])
while delta(cost):
	cost = lmin( min( cost(nbh)+surf , cost) )

## with cell, no possible with Sym Loop

while delta(cost):
	for i in range(9):
		ngb = (nbh0[i],nbh1[i])
		cond = cost(ngb) + surf < cost
		cost[ cond ] = cost(ngb) + surf
		cell[ cond ] = cell(ngb)

##

while delta(cost): # needs (cost,cell) tuple
	nbh  = neighborhood([3,3])
	cond = cost(nbh) + surf < cost
	cost = lmin( con(cond, cost(ngb) + surf , cost) )
	# cell = con(cond, cell(ngb) , cell)
