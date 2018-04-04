from os import sys, path
if __name__ == '__main__' and __package__ is None:    
    sys.path.append(path.dirname(path.dirname(path.abspath(__file__))))
# moves path to parent directory
from map import * ## "Map Algebra Compiler" package

#setNumRanks(1)
setupDevices("",DEV_CPU,"")

## Arguments

argv = sys.argv
argc = len(argv)
assert argc > 3

out_file_path = argv[1]
ds = [int(argv[2]),int(argv[3])]
bs = [512,512]
gs = [16,16]
N = 16

if (argc > 4):
	bs = [int(argv[4]),int(argv[4])]
if (argc > 5):
	N = int(argv[5])

## Computation

def life(dem):
	S = [[1,1,1],
		 [1,0,1],
		 [1,1,1]]
	return convolve(dem,S)

state = rand(N,ds,U8,ROW+BLK,bs,gs) > 128

for i in range(N):
	nbh = life(state)
	state = (nbh == 3) + (nbh == 2) * state

write(state,out_file_path)
