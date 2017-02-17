from map import * ## "Parallel Map Algebra" package

w = read('water')   # water depth layer
h = read('dem') - w # digital elevation layer
i = read('inflow')  # inlets inflow layer
o = read('outflow') # outlets outflow layer
T = 1000            # number of time steps

def swap(x,i,j):
	x[i], x[j] = min(x[i],x[j]), max(x[i],x[j])

def netsort5(x):
	swap(x,0,1); swap(x,2,3); swap(x,0,2)
	swap(x,3,4); swap(x,0,3); swap(x,1,3)
	swap(x,2,4); swap(x,1,4); swap(x,1,2)

def avglevel(w,h,x):                 #   Minimization   #
	netsort5(x) # ascending order    #  of differences, #
	s = w+x[0]  # sum variable       # Di Gregorio 1999 #
	n = 1       # count variable     #                  #
	for i in range(1,5):             # Central cell:    #
		b = (s >= x[i]*i)            # 7 h + 11 w       #
		s += b*x[i]                  #                  #
		n += b                       # Von Neumann NBH: #
	return s / n                     #      | 30 |      #
                                     # | 13 |  7 |  6 | #
def gather(w,h):                     #      |  3 |      #
	x = [0]*5 # neigborhood (NBH)    #                  #
	x[0] = h  # central cell         # Sorting:         #
	x[1] = h(0,-1) + w(0,-1)         # |3|6|7|13|30|    #
	x[2] = h(-1,0) + w(-1,0)         #                  #
	x[3] = h(+1,0) + w(+1,0)         # Exclusion:       #
	x[4] = h(0,+1) + w(0,+1)         # |3|6|7|*|*|      #
	return avglevel(w,h,x)           #                  #
                                     # Gathered avg.    #
def distri(w,h,l):                   # water level: 9   #
	wh = w+h # prev water level      #                  #
	c  = max(0, l(0,-1) − wh)        # Water distrib:   #
	c += max(0, l(-1,0) − wh)        # |3+6|6+3|7+2|*|* #
	c += max(0, l(+1,0) − wh)        #                  #
	c += max(0, l(0,+1) - wh)        # Final chw level: #
	c += max(h, l) − wh              #      | 30 |      #
	cwh = max(c + wh, h)             # | 13 |  9 |  9 | #
	return cwh - h                   #      |  9 |      #

for t in range(T):
	w = w + i         # fill inlets with water
	l = gather(w,h)   # gather avg. water level
	w = distri(w,h,l) # distribute water to NBH
	w = max(w-o,0)    # drain water from outlets

write(w,'output')
