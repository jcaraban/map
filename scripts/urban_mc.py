from map import * ## "Map Algebra Compiler" package
import sys

assert len(sys.argv) > 8
setupDevices("",DEV_GPU,"")

a  = 6.4640   # Constant coefficient
b1 = 43.5404  # Elevation coefficient
b2 = 1.9150   # Slope coefficient
b3 = 41.3441  # Distance to city centers coefficients
b4 = 12.5878  # Distance to transportations coefficient
b5 = [0.,0.,-9.865,-8.746,-9.268,-8.032,-9.169,-8.942,-9.45]
# {water,urban,barren,forest,shrub,woody,herb,crop,wetlad}
d  = 5        # dispersion parameter
q  = 16000    # max cells to become urban per year

x1 = read(sys.argv[1]) # elevation layer
x2 = read(sys.argv[2]) # slope layer
x3 = read(sys.argv[3]) # distance to centers layer
x4 = read(sys.argv[4]) # distance to transportations layer
x5 = read(sys.argv[5]) # land use layer
e  = read(sys.argv[6]) # exclusion layer (e.g. water bodies)
s  = read(sys.argv[7]) # initial state: urban / not-urban
N  = 50                # years of simulation i.e. time steps

for i in range(N) :
	z  = a + b1*x1 + b2*x2 + b3*x3 + b4*x4 + pick(x5,b5)
	pg = exp(z) / (1 + exp(z))
	pc = pg * e.no() * s.no() * fsum(s) / (3*3-1)
	pd = pc * exp(-d * (1 - pc / zmax(pc)))
	ps = q * pd / zsum(pd)
	seed = ones_like(s) * i
	s  = s | (ps > rand(seed))

write(s,sys.argv[8])
'''
********************************* Monte Carlo **********************************
import urban # imports 'urban()' function, containing Listing 1

prob = zeros() # urban probability map
M = 1000       # Monte Carlo iterations

for i in range(M) :     # Monte Carlo method
	prob = prob + urban() # urban() returns the urban layer 's'
prob = prob / M           # urban() e {0,1} ==> prob e [0,1]

write(prob,'output')

********************** Calibration with Genetic Algorithm **********************
import montecarlo # Assume montecarlo.py defines the function "montecarlo(d,q)"
import random

X = 13 # number of parameters per individual
P = 10 # number of individual in the population
G = 50 # number of generations of the Genetic Algorithm
minv = [0,0,0,0,0,1,-20,-20,-20,-20,-20,-20,-20]
maxv = [50,50,50,50,50,10,0,0,0,0,0,0,0]
expected = read('expected') # expected urban state after N iterations

def randu() :
	return random.uniform(0,1)

def randuv() :
	return random.uniform(0,1,X)

def cost(individual) :
	simulated = montecarlo(*individual) # simulated future urban state with the CA
	difference = (expected - simulated) ** 2
	return sqrt(mean(difference)) # Root Mean Squared Error e [0,1]

def init() : # vector of randoms between max and min bounds
	return (maxv - minv) * randuv() + minv

def crossover(a,b) : # mean of parameters
	return (a + b) / 2

def mutate(indv) : # random reinitialization
	return (randuv() < 0.9) ? indv : (maxv - minv) * randuv() + minv

def descend(indv,g) : # random decend
	return indv + 0.02*(G-g)/G * (randuv()<0.5 ? maxv-minv : minv-maxv)

def tournament(population,score,K=3) :
	best = randu() * P
	for k in range(1,K) :
		i = randu() * P
		best = score[i] < score[best] ? i : best
	return population[best]

def leader(population,score) :
	return population[best]

population = [init() for p in range(0,P)] # Initializes a list of P individuals
score = [cost(population[p]) for p in range(0,P)] # Computes their cost
offspring = []

for g in range(G) :
	offspring[0] = leader(population,score)
	for p in range(1,P) :
		a = tournament(population,score)
		b = tournament(population,score)
		c = crossover(a,b)
		d = mutate(c)
		e = descend(d,g)
		offspring[p] = e

	population = offspring # the offspring becomes next generation population
	score = [cost(population[p]) for p in range(0,P)]

best = 0
for p in range(1,P) :
	best = score[p] < score[best] ? p : best

(d,q) =  population[best]
print "best params, d: " , d , " q: " , q
'''