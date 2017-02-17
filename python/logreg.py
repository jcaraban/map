from map import * ## "Parallel Map Algebra" package

x1 = read('x1')
x2 = read('x2')
x3 = read('x3')
x4 = read('x4')
x5 = focalMean(x1) # derived spatial feature
y  = read('y')
I = 100 # gradient decend iterations
S = prod(y.datasize())

x = [x1,x2,x3,x4,x5] # vector of layers
t = [0] * len(x)     # vector of thetas
a = 0.1              # descend speed

for i in range(I) :
	loss = localSum(x*t) - y
	grad = zonalSum(x*loss)
	t = t - a * grad / S

print t
