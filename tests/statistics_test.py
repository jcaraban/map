from os import sys, path, getcwd, system
if __name__ == '__main__' and __package__ is None:    
    sys.path.append(path.dirname(path.dirname(path.abspath(__file__))))
# moves path to parent directory
import map as ma ## "Map Algebra Compiler" package
import numpy as np
from scipy import signal
from osgeo import gdal


in_path = getcwd() + '/tests/data/fjord.tif'
#in_path = getcwd() + '/tests/data/s32x32_b32x32_g32x32.tif'

##

def map_statistics():
    raster = ma.read(in_path)
    N = ma.prod(raster.datasize())

    maxv = ma.zmax(raster)
    minv = ma.zmin(raster)
    rang = maxv - minv

    mean = ma.zsum(raster) / N
    geom = ma.zprod(raster) ** (1.0/N)
    harm = N / ma.zsum(1.0/raster)
    quam = ma.sqrt(ma.zsum(raster**2)/N)
    cubm = ma.cbrt(ma.zsum(raster**3)/N)

    dev = (raster - mean) ** 2
    var = ma.zsum(dev) / N
    std = ma.sqrt(var)

    norm = (dev - minv) / rang
    #norm = ((raster - mean - minv)/rang)**2
    nstd  = ma.sqrt(ma.zsum(norm) / N)

    ma.eval(maxv,minv,rang,mean,geom,harm,quam,cubm,std,nstd)
    return [maxv,minv,rang,mean,geom,harm,quam,cubm,std,nstd]
    
##

def numpy_statistics():
    ds = gdal.Open(in_path)
    raster = np.array(ds.GetRasterBand(1).ReadAsArray())
    N = np.prod(raster.shape)

    maxv = np.max(raster)
    minv = np.min(raster)
    rang = maxv - minv

    mean = np.sum(raster) / N
    geom = np.prod(raster) ** (1.0/N)
    harm = N / np.sum(1.0/raster)
    quam = np.sqrt(np.sum(raster**2)/N)
    cubm = np.cbrt(np.sum(raster**3)/N)

    dev = (raster - mean) ** 2
    var = np.sum(dev) / N
    std = np.sqrt(var)

    norm = (dev - minv) / rang
    #norm = ((raster - mean - minv)/rang)**2
    nstd  = np.sqrt(np.sum(norm) / N)

    return [maxv,minv,rang,mean,geom,harm,quam,cubm,std,nstd]
    
##

def test_statistics():
    ma_stats = map_statistics()
    np_stats = numpy_statistics()
    assert len(ma_stats) == len(np_stats)
    ma_stats = np.array([ma.value(x) for x in ma_stats])
    np_stats = np.array(np_stats)
    assert np.allclose(ma_stats,np_stats,rtol=1e-05,atol=1e-08,equal_nan=True)

map_statistics()