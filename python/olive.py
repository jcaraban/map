from map import * ## "Parallel Map Algebra" package

f = read('flies')   # outbreak of olive fruit flies
o = read('olives')  # olive fields 
t = read('temper')  # avg. annual temperature
r = read('rain')    # avg. annual rain
w = read('rivers')  # rivers network
N = 50              # number of time steps

tT = [0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0] # temp table
rT = [0,.05,0.1,.15,0.2,.25,0.3,.35,0.4,.45,0.5] # rain table

def spread(f) :
	lst = [ 2, 3, 3, 3, 2,  #                   ▒ █ █ █ ▒
		    3, 2, 2, 2, 3,  # spreading         █ ▒ ▒ ▒ █
		    3, 2, 1, 2, 3,  # density      -->  █ ▒ · ▒ █
		    3, 2, 2, 2, 3,  # distribution      █ ▒ ▒ ▒ █
		    2, 3, 3, 3, 2 ] #                   ▒ █ █ █ ▒
	msk = Mask([5,5],lst) # 5x5 mask of weights
	return convolve(f,msk) / 61 # normalization

for i in range(N) :
	s = 0.5*w + pick(r,rT)  # suitability of cells
	f = spread(f)           # migration of flies
	b = o * s * (1-f)*f*4   # birth rate
	f = f + rand() * 0.1*b  # stochastic reproduction
	d = pick(t,tT) * f      # death rate
	f = f - rand() * 0.1*d  # stochastic extinction

write(f,'output')
