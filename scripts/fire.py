from map import * ## "Map Algebra Compiler" package

f = read('fire') # initial fire
e = read('dem')  # digital elevation
l = read('fuel')   # fuel load
m = read('moisture') # water presence
v = read('vegetation') # distance to transportations layer
t = f * ones_like(f,F32) # fire temperature
wd = rand(f,f.datasize(),F32) # wind direction
ws = rand(f,f.datasize(),F32) # wind speed
N  = 50            # minutes of simulation i.e. time steps

for i in range(N) :
	
	Am I downwind from a burning fire?
	If the wind is strong, I should then look farther upwind for a burning fire.
	If I am on a steep slope and my aspect is facing into the wind, I should look even farther upwind for a burning fire.


write(f,'output')