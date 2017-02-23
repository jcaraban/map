/* 
 * @file	tiff.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "tiff.hpp"
#include <cassert>


namespace { // anonymous namespace, used for the additional TIFF TAGS

const unsigned int N_TAGS = 9;
const unsigned int TIFFTAG_STATS  = 65000;
const unsigned int TIFFTAG_MAX    = 65001;
const unsigned int TIFFTAG_MEAN   = 65002;
const unsigned int TIFFTAG_MIN    = 65003;
const unsigned int TIFFTAG_STD    = 65004;
const unsigned int TIFFTAG_MAX_B  = 65005;
const unsigned int TIFFTAG_MEAN_B = 65006;
const unsigned int TIFFTAG_MIN_B  = 65007;
const unsigned int TIFFTAG_STD_B  = 65008;

static const TIFFFieldInfo xtiffFieldInfo[] = {
	{ TIFFTAG_STATS , 1 , 1 , TIFF_BYTE  , FIELD_CUSTOM, 1, 0, (char*)"Stats" },
	{ TIFFTAG_MAX   , 1 , 1 , TIFF_DOUBLE, FIELD_CUSTOM, 1, 0, (char*)"Max"   },
	{ TIFFTAG_MEAN  , 1 , 1 , TIFF_DOUBLE, FIELD_CUSTOM, 1, 0, (char*)"Mean"  },
	{ TIFFTAG_MIN   , 1 , 1 , TIFF_DOUBLE, FIELD_CUSTOM, 1, 0, (char*)"Min"   },
	{ TIFFTAG_STD   , 1 , 1 , TIFF_DOUBLE, FIELD_CUSTOM, 1, 0, (char*)"Std"   },
	{ TIFFTAG_MAX_B , -1, -1, TIFF_DOUBLE, FIELD_CUSTOM, 1, 1 , (char*)"Max-b" },
	{ TIFFTAG_MEAN_B, -1, -1, TIFF_DOUBLE, FIELD_CUSTOM, 1, 1 , (char*)"Mean-b"},
	{ TIFFTAG_MIN_B , -1, -1, TIFF_DOUBLE, FIELD_CUSTOM, 1, 1 , (char*)"Min-b" },
	{ TIFFTAG_STD_B , -1, -1, TIFF_DOUBLE, FIELD_CUSTOM, 1, 1 , (char*)"Std-b" }
};

static TIFFExtendProc parent_extender = NULL;

static void registerCustomTIFFTags(TIFF *tif)
{
    TIFFMergeFieldInfo(tif, xtiffFieldInfo, N_TAGS);
    if (parent_extender) (*parent_extender)(tif);
}

static void augment_libtiff_with_custom_tags()
{
    static bool first_time = true;
    if (!first_time) return;
    first_time = false;
    parent_extender = TIFFSetTagExtender(registerCustomTIFFTags);
}

} // namespace anonymous, used for the additional TIFF TAGS

namespace map { namespace detail {

/***********
   Support
 ***********/

#define SUPPORT template<> const bool IFormat<tiff>::Support

SUPPORT :: StreamDir :: IN  = 1;
SUPPORT :: StreamDir :: OUT = 1;
SUPPORT :: StreamDir :: IO  = 0;

SUPPORT :: DataType :: F32 = 1;
SUPPORT :: DataType :: F64 = 0;
SUPPORT :: DataType :: B8  = 0;
SUPPORT :: DataType :: U8  = 1;
SUPPORT :: DataType :: U16 = 1;
SUPPORT :: DataType :: U32 = 1;
SUPPORT :: DataType :: U64 = 0;
SUPPORT :: DataType :: S8  = 1;
SUPPORT :: DataType :: S16 = 1;
SUPPORT :: DataType :: S32 = 1;
SUPPORT :: DataType :: S64 = 0;

SUPPORT :: NumDim :: TIME = 0;
SUPPORT :: NumDim :: D0 = 0;
SUPPORT :: NumDim :: D1 = 0;
SUPPORT :: NumDim :: D2 = 1;
SUPPORT :: NumDim :: D3 = 0;

SUPPORT :: MemOrder :: BLK = 1;
SUPPORT :: MemOrder :: ROW = 1;
SUPPORT :: MemOrder :: COL = 0;
SUPPORT :: MemOrder :: SFC = 0;

SUPPORT :: Parallel :: PARAREAD = 0;
SUPPORT :: Parallel :: PARAWRITE = 0;

#undef SUPPORT

/***********
   Methods
 ***********/

tiff::tiff(MetaData& meta, DataStats& stats)
	: IFormat<tiff>(meta,stats)
{
	augment_libtiff_with_custom_tags();
}

tiff::~tiff() { }

Ferr tiff::open(std::string file_path, StreamDir stream_dir) {
	Ferr ferr = 0;

	if (stream_dir == IN)
	{
		handler = TIFFOpen(file_path.c_str(), "r8"); // r=read, 8=bigtiff
		if (!handler) {
			std::cerr << "Couldn't open " << file_path << " file for reading!" << std::endl;
			assert(0);
		}

		ferr = getMeta();
		if (ferr != 0) {
			assert(0);
		}

		ferr = getStats();
		if (ferr != 0) {
			assert(0);
		}

		// Extra traits go here...
	}
	else if (stream_dir == OUT)
	{
		handler = TIFFOpen(file_path.c_str(), "w8"); // w=write, 8=bigtiff
		if (!handler) {
			assert(!"Couldn't open <tiff> file for writing!");
		}

		ferr = setMeta();
		if (ferr != 0) {
			assert(0);
		}

		ferr = setOthers();
		if (ferr != 0) {
			assert(0);
		}

		//// Extra traits go here...
	}
	else if (stream_dir == IO)
	{
		assert(!"IO mode not allowed for <tiff> files");
	}
	else
	{
		assert(!"Unknown mode");
	}

	return ferr;
}

Ferr tiff::close() {
	Ferr ferr = 0;

	if (meta.stream_dir == IN)
	{
		// Nothing to do
	}
	else if (meta.stream_dir == OUT)
	{
		ferr = setStats();
		if (ferr != 0) {
			assert(0);
		}
	}
	else
	{
		assert(!"Unknown or not allowed mode");
	}

	TIFFClose(handler);
	return ferr;
}

Ferr tiff::getMeta() {
	uint32 imageWidth, imageLength;
	uint32 tileWidth, tileLength;
	int16 tag_bitsPerSample, tag_samplesPerPixel, tag_sampleFormat;
	Ferr ferr = 0;
	int ret;

	// stream_dir
	meta.stream_dir = IN;

	// data_type
	ret = TIFFGetField(handler, TIFFTAG_SAMPLESPERPIXEL, &tag_samplesPerPixel);
	if (ret != 1) {
		assert(0);
	}
	if (tag_samplesPerPixel != 1) {
		assert(!"SamplesPerPixel must be 1");
	}
	ret = TIFFGetField(handler, TIFFTAG_SAMPLEFORMAT, &tag_sampleFormat);
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFGetField(handler, TIFFTAG_BITSPERSAMPLE, &tag_bitsPerSample);
	if (ret != 1) {
		assert(0);
	}
	switch (tag_sampleFormat) {
		case SAMPLEFORMAT_UINT:
			switch (tag_bitsPerSample) {
				case 8 : meta.data_type = U8 ; break;
				case 16: meta.data_type = U16; break;
				case 32: meta.data_type = U32; break;
				default: assert(0);
			}
			break;
		case SAMPLEFORMAT_INT:
			switch (tag_bitsPerSample) {
				case 8 : meta.data_type = S8 ; break;
				case 16: meta.data_type = S16; break;
				case 32: meta.data_type = S32; break;
				default: assert(0);
			}
			break;
		case SAMPLEFORMAT_IEEEFP:
			switch (tag_bitsPerSample) {
				case 32: meta.data_type = F32; break;
				case 64: meta.data_type = F64; break;
				default: assert(0);
			}
			break;
		default: assert(0);
	}

	// num_dim
	meta.num_dim = D2; // Although D3 is possible, currently other layers than the first are ignored
	meta.data_size = DataSize(2);
	meta.block_size = BlockSize(2);

	// data_size
	ret = TIFFGetField(handler, TIFFTAG_IMAGEWIDTH, &imageWidth);
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFGetField(handler, TIFFTAG_IMAGELENGTH, &imageLength);
	if (ret != 1) {
		assert(0);
	}
	if (imageWidth <= 0 || imageLength <= 0) {
		assert(0);
	}
	meta.data_size[0] = static_cast<int>(imageWidth);
	meta.data_size[1] = static_cast<int>(imageLength);

	// mem_order
	// @ how to avoid strip?
	//uint32 strip;
	//ret = TIFFGetField(handler, TIFFTAG_STRIPOFFSETS, &strip);
	//ret += TIFFGetField(handler, TIFFTAG_ROWSPERSTRIP, &strip);
	//ret += TIFFGetField(handler, TIFFTAG_STRIPBYTECOUNTS, &strip);
	//if (ret > 0) {
	//	assert(!"Strip mode not supported");
	//}
	meta.mem_order = ROW+BLK; // only TILE mode supported

	// block_size
	ret = TIFFGetField(handler, TIFFTAG_TILEWIDTH, &tileWidth);
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFGetField(handler, TIFFTAG_TILELENGTH, &tileLength);
	if (ret != 1) {
		assert(0);
	}
	if (tileWidth <= 0 || tileLength <= 0) {
		assert(0);
	}
	meta.block_size[0] = static_cast<int>(tileWidth);
	meta.block_size[1] = static_cast<int>(tileLength);

	return ferr;
}

Ferr tiff::setMeta() {
	uint32 imageWidth, imageLength;
	uint32 tileWidth, tileLength;
	int16 tag_bitsPerSample, tag_samplesPerPixel, tag_sampleFormat;
	Ferr ferr = 0;
	int ret;

	// stream_dir
	meta.stream_dir = OUT;
	
	// data_type
	switch (meta.data_type.get()) {
		case U8 : tag_sampleFormat = SAMPLEFORMAT_UINT; tag_bitsPerSample = 8; break;
		case U16: tag_sampleFormat = SAMPLEFORMAT_UINT; tag_bitsPerSample = 16; break;
		case U32: tag_sampleFormat = SAMPLEFORMAT_UINT; tag_bitsPerSample = 32; break;
		case S8 : tag_sampleFormat = SAMPLEFORMAT_INT; tag_bitsPerSample = 8; break;
		case S16: tag_sampleFormat = SAMPLEFORMAT_INT; tag_bitsPerSample = 16; break;
		case S32: tag_sampleFormat = SAMPLEFORMAT_INT; tag_bitsPerSample = 32; break;
		case F32: tag_sampleFormat = SAMPLEFORMAT_IEEEFP; tag_bitsPerSample = 32; break;
		case F64: tag_sampleFormat = SAMPLEFORMAT_IEEEFP; tag_bitsPerSample = 64; break;
		default: assert(0);
	}

	tag_samplesPerPixel = 1;
	ret = TIFFSetField(handler, TIFFTAG_SAMPLESPERPIXEL, tag_samplesPerPixel);
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFSetField(handler, TIFFTAG_SAMPLEFORMAT, tag_sampleFormat);
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFSetField(handler, TIFFTAG_BITSPERSAMPLE, tag_bitsPerSample);
	if (ret != 1) {
		assert(0);
	}

	// num_dim
	// --> it's configured by setting the data_size

	// data_size
	imageWidth = static_cast<uint32>(meta.data_size[0]);
	imageLength = static_cast<uint32>(meta.data_size[1]);
	if (imageWidth <= 0 || imageLength <= 0) {
		assert(0);
	}
	ret = TIFFSetField(handler, TIFFTAG_IMAGEWIDTH, imageWidth);
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFSetField(handler, TIFFTAG_IMAGELENGTH, imageLength);
	if (ret != 1) {
		assert(0);
	}

	// mem_order
	// it's configured by setting the block_size (only TILE mode supported)

	// block_size
	tileWidth = static_cast<uint32>(meta.block_size[0]);
	tileLength = static_cast<uint32>(meta.block_size[1]);
	if (tileWidth <= 0 || tileLength <= 0) {
		assert(0);
	}
	ret = TIFFSetField(handler, TIFFTAG_TILEWIDTH, tileWidth);
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFSetField(handler, TIFFTAG_TILELENGTH, tileLength);
	if (ret != 1) {
		assert(0);
	}

	return ferr;
}

Ferr tiff::getStats() {
	uint8 active = 0;
	Ferr ferr = 0;
	int ret;

	ret = TIFFGetField(handler, TIFFTAG_STATS, &active);
	if (ret != 1 || active == 0) {
		stats = DataStats();
		return ferr;
	}
	stats.active = static_cast<bool>(active);

	ret = TIFFGetField(handler, TIFFTAG_MAX, &stats.max);
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFGetField(handler, TIFFTAG_MEAN, &stats.mean);
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFGetField(handler, TIFFTAG_MIN, &stats.min);
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFGetField(handler, TIFFTAG_STD, &stats.std);
	if (ret != 1) {
		assert(0);
	}

	DataStats::type *ptr;
	uint32 n = 0;
	
	ret = TIFFGetField(handler, TIFFTAG_MAX_B, &n, &ptr);
	if (ret != 1) {
		assert(0);
	}
	stats.maxb = decltype(stats.maxb)(ptr,ptr+n);

	ret = TIFFGetField(handler, TIFFTAG_MEAN_B, &n, &ptr);
	if (ret != 1) {
		assert(0);
	}
	stats.meanb = decltype(stats.meanb)(ptr,ptr+n);

	ret = TIFFGetField(handler, TIFFTAG_MIN_B, &n, &ptr);
	if (ret != 1) {
		assert(0);
	}
	stats.minb = decltype(stats.minb)(ptr,ptr+n);

	ret = TIFFGetField(handler, TIFFTAG_STD_B, &n, &ptr);
	if (ret != 1) {
		assert(0);
	}
	stats.stdb = decltype(stats.stdb)(ptr,ptr+n);

	assert(stats.maxb.size() == stats.meanb.size());
	assert(stats.meanb.size() == stats.minb.size());
	assert(stats.minb.size() == stats.stdb.size());

	return ferr;
}

Ferr tiff::setStats() {
	uint8 active = 0;
	Ferr ferr = 0;
	int ret;

	if (!stats.active)
		return ferr;

	active = static_cast<uint8>(stats.active);
	ret = TIFFSetField(handler, TIFFTAG_STATS, active);
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFSetField(handler, TIFFTAG_MAX, stats.max.f64); // @
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFSetField(handler, TIFFTAG_MEAN, stats.mean);
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFSetField(handler, TIFFTAG_MIN, stats.min);
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFSetField(handler, TIFFTAG_STD, stats.std);
	if (ret != 1) {
		assert(0);
	}

	assert(stats.maxb.size() == stats.meanb.size());
	assert(stats.meanb.size() == stats.minb.size());
	assert(stats.minb.size() == stats.stdb.size());

	ret = TIFFSetField(handler, TIFFTAG_MAX_B, stats.maxb.size(), stats.maxb.data());
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFSetField(handler, TIFFTAG_MEAN_B, stats.meanb.size(), stats.meanb.data());
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFSetField(handler, TIFFTAG_MIN_B, stats.minb.size(), stats.minb.data());
	if (ret != 1) {
		assert(0);
	}
	ret = TIFFSetField(handler, TIFFTAG_STD_B, stats.stdb.size(), stats.stdb.data());
	if (ret != 1) {
		assert(0);
	}

	return ferr;
}

Ferr tiff::setOthers() {
	Ferr ferr = 0;
	int ret;

	// Other tags...
	ret = TIFFSetField(handler, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	if (ret != 1) {
		assert(0);
	}
	TIFFSetField(handler, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	if (ret != 1) {
		assert(0);
	}
	TIFFSetField(handler, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	if (ret != 1) {
		assert(0);
	}
	TIFFSetField(handler, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	if (ret != 1) {
		assert(0);
	}

	return ferr;
}

Ferr tiff::read(void* dst, const Coord& beg_coord, const Coord& end_coord) {
	assert(!"NOT IMPLEMENTED");
}
	
Ferr tiff::write(const void* src, const Coord& beg_coord, const Coord& end_coord) {
	assert(!"NOT IMPLEMENTED");
}

Ferr tiff::readBlock(Block &block) const {
	uint32 x_pos_abs = block.key.coord[0] * meta.block_size[0]; // libTIFF takes an absolute index
	uint32 y_pos_abs = block.key.coord[1] * meta.block_size[1]; // see TIFF reference page for more info

	tsize_t ret = TIFFReadTile(handler, block.entry->host_mem, x_pos_abs, y_pos_abs, 0, 0);

	return (ret < 0) ? ret : 0;
}

Ferr tiff::writeBlock(const Block &block) {
	uint32 x_pos_abs = block.key.coord[0] * meta.block_size[0]; // libTIFF takes an absolute index
	uint32 y_pos_abs = block.key.coord[1] * meta.block_size[1]; // see TIFF reference page for more info

	tsize_t ret = TIFFWriteTile(handler, const_cast<void*>(block.entry->host_mem), x_pos_abs, y_pos_abs, 0, 0);

	return (ret < 0) ? ret : 0;
}

} } // namespace map::detail
