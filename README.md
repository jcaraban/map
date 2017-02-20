# Compiler + Parallel + Map Algebra
Map Algebra is a mathematical formalism for the processing and analysis of raster geographical data ("Geographic Information Systems and Cartographic Modelling," Tomlin 1990). Map Algebra becomes a powerful spatial modeling framework when embedded into a scripting language with branching, looping and functions.

Operations in Map Algebra only take and return *rasters*. They belong to one of several *classes*:
* **Local**: access single raster cells (*element-wise/ map* pattern)
* **Focal**: access bounded neighborhoods (*stencil/ convolution* pattern)
* **Zonal**: access any cell with associative order (*reduction* pattern)
* **Global**: access any cell following some particular topological order
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
e.g. viewshed, hydrological modeling, least cost analysis...

For my PhD I design a Parallel Map Algebra implementation that runs efficiently on OpenCL devices. Users write sequential single-source Python scripts and the framework generates and executes parallel code automatically. Compiler techniques are at the core of system, from dependency analysis to loop fusion. They key challenge is minimizing memory movements, since they pose the major bottleneck to performance.

## Sample script: *Hillshade*
The following Python script depicts a hillshade algorithm (Horn 1981). A extended explanion can be found in the [wiki](https://github.com/jcaraban/map/wiki/Hillshade). Hillshade computes the derivatives of a DEM and matches them to the azimuth and altitude angles of the sun in order to draw a self-shadowing effect that creates a sense of topographic relief.

```{.py}
	from map import * ## Parallel Map Algebra
	PI = 3.141593

	def hori(dem, dist):
		h = [ [1, 0, -1],
			  [2, 0, -2],
			  [1, 0, -1] ]
		return convolve(dem,h) / 8 / dist

	def vert(dem, dist):
		d = dem
		v = 1*d(-1,-1) +2*d(0,-1) +1*d(1,-1)
		   +0*d(-1,0)  +0*d(0,0)  +0*d(1,0)
		   -1*d(-1,1)  -2*d(0,1)  -1*d(1,1)
		return v / 8 / dist

	def slope(dem, dist=1):
		x = hori(dem,dist)
		y = vert(dem,dist)
		z = atan(sqrt(x*x + y*y))
		return z

	def aspect(dem, dist=1):
		x = hori(dem,dist)
		y = vert(dem,dist)
		z1 = (x!=0) * atan2(y,-x)
		z1 = z1 + (z1<0) * (PI*2)
		z0 = (x==0) * ((y>0)*(PI/2)
		   + (y<0)*(PI*2-PI/2))
		return z1 + z0

	def hillshade(dem, zenith, azimuth):
		zr = (90 - float(zenith)) / 180*PI
		ar = 360 - float(azimuth) + 90
		ar = ar - 360 if (ar > 360) else ar
		ar = ar / 180 * PI
		hs = cos(zr) * cos(slope(dem)) +
			 sin(zr) * sin(slope(dem)) *
			 cos(ar - aspect(dem))
		return hs

	dem = read('in_file_path')
	out = hillshade(dem,45,315)
	write(out,'out_file_path')
```

## Wiki
If you wish to know more about the , other sample scripts and extended information can be found in the wiki:
* Compiler approach to Parallel Map Algebra (<-- link)
* [Hillshade](github.com/jcaraban/map/wiki/Hillshade), [Statistics](github.com/jcaraban/map/wiki/Statistics), [Viewshed](github.com/jcaraban/map/wiki/Viewshed)
* Conway's Game of [Life](github.com/jcaraban/map/wiki/Life)
* Cellular Automata for [Urban Growth](github.com/jcaraban/map/wiki/Urban)
* ...

## Requirements
This project has been developed and tested with:

* Python 2.7, CPython implementation
* OpenCL 1.2, Intel and AMD implementations
* GCC C++ compiler, any version with c++11 support

Other compilers / OpenCl drivers are probably compatible, but have not been tested.

## Build
**Note:** this is a research project and the code is not intended to be used as a production software, but as a minimum viable prototype for testing our research hypothesis. If you still wish to continue: download the source, install the requirements and type on your terminal:
```
make library && cd python
python hill.py input-raster.tif output-raster.tif > log.txt
```
## Contact
Questions? Contact me through [mail](jcaraban@abo.fi)!

**Jesús Carabaño Bravo** <jcaraban@abo.fi> | PhD Student at Åbo Akademi, Finland  
