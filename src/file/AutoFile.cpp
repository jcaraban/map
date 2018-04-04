/* 
 * Jesús Carabaño Bravo - jcaraban@abo.fi
 *
 * fauto.cpp
 */

#include "fauto.hpp"
#include "gdal/gdal_priv.h"
#include <string>
#include <cassert>


namespace map { namespace detail {

/***********
   Support
 ***********/

#define SUPPORT template<> const bool IFormat<fauto>::Support

SUPPORT :: StreamDir :: IN  = 1;
SUPPORT :: StreamDir :: OUT = 1;
SUPPORT :: StreamDir :: IO  = 1;

SUPPORT :: DataType :: F32 = 1;
SUPPORT :: DataType :: F64 = 1;
SUPPORT :: DataType :: B8  = 1;
SUPPORT :: DataType :: U8  = 1;
SUPPORT :: DataType :: U16 = 1;
SUPPORT :: DataType :: U32 = 1;
SUPPORT :: DataType :: U64 = 1;
SUPPORT :: DataType :: S8  = 1;
SUPPORT :: DataType :: S16 = 1;
SUPPORT :: DataType :: S32 = 1;
SUPPORT :: DataType :: S64 = 1;

SUPPORT :: NumDim :: TIME = 0;
SUPPORT :: NumDim :: D0 = 0;
SUPPORT :: NumDim :: D1 = 0;
SUPPORT :: NumDim :: D2 = 1;
SUPPORT :: NumDim :: D3 = 0;

SUPPORT :: MemOrder :: BLK = 1;
SUPPORT :: MemOrder :: ROW = 1;
SUPPORT :: MemOrder :: COL = 0;
SUPPORT :: MemOrder :: SFC = 0;

SUPPORT :: Parallel :: PARAREAD = 1;
SUPPORT :: Parallel :: PARAWRITE = 0;

/***********
   Methods
 ***********/

fauto::fauto(MetaData& meta)
	: IFormat<fauto>( meta )
	, dataset(NULL)
	, band(NULL)
{
	GDALAllRegister();
}

fauto::~fauto() { }

Ferr fauto::open(std::string file_path, StreamDir stream_dir) {
	if (stream_dir == IN) 
	{
		dataset = (GDALDataset *) GDALOpen(file_path.c_str(), GA_ReadOnly);
		if (!dataset) {
			assert(!"Couldn't open <fauto> file for reading!");
		}

		band = dataset->GetRasterBand(1);
		if (!band) {
			assert(!"Couldn't open the 1st band of the <fauto> file for reading!");
		}

		// GDAL metadata
		type = band->GetRasterDataType();
		switch (type) {
			case GDT_Float32: meta.data_type = F32; break;
			case GDT_Float64: meta.data_type = F64; break;
			case GDT_Byte   : meta.data_type = B8 ; break;
			case GDT_UInt16 : meta.data_type = U16; break;
			case GDT_UInt32 : meta.data_type = U32; break;
			case GDT_Int16  : meta.data_type = S16; break;
			case GDT_Int32  : meta.data_type = S32; break;
			default: assert(0);
		}

		meta.num_dim = D2;
		meta.mem_order = static_cast<MemOrder>(ROW+BLK);
		meta.data_size[0] = dataset->GetRasterXSize();
		meta.data_size[1] = dataset->GetRasterYSize();
		band->GetBlockSize(&meta.block_size[0], &meta.block_size[1]);
	}
	else if (stream_dir == OUT) 
	{
		assert(!"NOT IMPLEMENTED YET");
		//dataset = (GDALDataset *) GDALOpen(file, GA_Update);
	} 
	else if (stream_dir == IO) 
	{
		assert(!"NOT IMPLEMENTED YET");
		//dataset = (GDALDataset *) GDALOpen(file, GA_Update);
	} 
	else 
	{
		assert(!"Unknown mode");
	}

	if (!dataset) {
		assert(!"Couldn't open <fauto> file!");
	}

	return 0;
}

Ferr fauto::close()
{
    GDALClose( (GDALDatasetH) dataset );
	return 0;
}

Ferr fauto::read(void* dst, const Coord& beg_coord, const Coord& end_coord) {
	Coord len = end_coord - beg_coord;
    band->RasterIO( GF_Read, beg_coord[0], beg_coord[1], len[0], len[1], dst, len[0], len[1], type, 0, 0 );
}

Ferr fauto::write(const void* src, const Coord& beg_coord, const Coord& end_coord) {
	assert(!"NOT IMPLEMENTED YET");
	//Coord len = end_coord - beg_coord;
    //band->RasterIO( GF_Write, beg_coord[0], beg_coord[1], len[0], len[1], const_cast<void*>(src), len[0], len[1], type, 0, 0 );
}

Ferr fauto::readBlock(void* dst, const Coord& block_coord) {
	band->ReadBlock(block_coord[0], block_coord[1], dst);
}

Ferr fauto::writeBlock(const void* src, const Coord& block_coord) {
	assert(!"NOT IMPLEMENTED YET");
	//band->WriteBlock(block_coord[0], block_coord[1], const_cast<void*>(src));
}

} } // namespace map::detail
