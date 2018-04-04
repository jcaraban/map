from os import sys, path, getcwd, system
if __name__ == '__main__' and __package__ is None:    
    sys.path.append(path.dirname(path.dirname(path.abspath(__file__))))
# moves path to parent directory
import map as ma ## "Map Algebra Compiler" package
import numpy as np
from scipy import signal
from osgeo import gdal


in_path = getcwd() + '/tests/data/fjord.tif'
out_path = getcwd() + '/tests/data/hill_fjord.tif'

##

def hori(dem, dist):
    h = [ [-1, 0, 1],
          [-2, 0, 2],
          [-1, 0, 1] ]
    return ma.convolve(dem,h) / (8 * dist)

def vert(dem, dist) :
    v = [ [-1, -2, -1],
          [ 0,  0,  0],
          [ 1,  2,  1] ]
    return ma.convolve(dem,v) / (8 * dist)

def slope(dem, zf=1, dist=1):
    x = hori(dem,dist)
    y = vert(dem,dist)
    z = ma.atan(zf * ma.sqrt(x*x + y*y))
    return z

def aspect(dem, dist=1):
    x = hori(dem,dist)
    y = vert(dem,dist)
    z1 = (x!=0) * ma.atan2(y,-x)
    z1 = z1 + (z1<0) * (ma.pi*2)
    z0 = (x==0) * ((y>0)*(ma.pi/2)
        + (y<0)*(ma.pi*2-ma.pi/2))
    return z1 + z0

def hillshade(dem, zenith, azimuth, zfactor=1):
    zr = (90 - float(zenith)) / 180 * ma.pi
    ar = 360 - float(azimuth) + 90
    ar = ar - 360 if (ar > 360) else ar
    ar = ar / 180 * ma.pi
    hs = ( ma.cos(zr) * ma.cos(slope(dem)) +
           ma.sin(zr) * ma.sin(slope(dem)) *
           ma.cos(ar - aspect(dem)) )
    return hs

def map_hillshade():
    dem = ma.read(in_path)
    hill = hillshade(dem,45,315)
    ma.write(hill,out_path)
    ds = gdal.Open(out_path)
    return np.array(ds.GetRasterBand(1).ReadAsArray())

##

def numpy_hillshade():
    ds = gdal.Open(in_path)
    dem = np.array(ds.GetRasterBand(1).ReadAsArray())

    zenith = 45
    azimuth = 315
    dist = 1
    zf = 1

    zr = (90 - float(zenith)) / 180 * np.pi
    ar = 360 - float(azimuth) + 90
    ar = ar - 360 if (ar > 360) else ar
    ar = ar / 180 * np.pi

    h = np.array([[+1, 0,-1],[+2, 0,-2],[+1, 0,-1]])
    hori_dem = signal.convolve2d(dem,h,boundary='symm',mode='same') / (8 * dist)

    v = np.array([[+1,+2,+1],[ 0, 0, 0],[-1,-2,-1]])
    vert_dem = signal.convolve2d(dem,v,boundary='symm',mode='same') / (8 * dist)

    x = hori_dem
    y = vert_dem
    z = np.arctan(zf * np.sqrt(x*x + y*y))
    slope_dem = z

    x = hori_dem
    y = vert_dem
    z1 = (x!=0) * np.arctan2(y,-x)
    z1 = z1 + (z1<0) * (np.pi*2)
    z0 = (x==0) * ((y>0)*(np.pi/2)
        + (y<0)*(np.pi*2-np.pi/2))
    aspect_dem = z1 + z0

    hs = ( np.cos(zr) * np.cos(slope_dem) +
           np.sin(zr) * np.sin(slope_dem) *
           np.cos(ar - aspect_dem) )
    
    return hs

##

def test_hillshade():
    ma_hill = map_hillshade()
    np_hill = numpy_hillshade()
    assert np.allclose(ma_hill,np_hill,rtol=1e-05,atol=1e-06)
