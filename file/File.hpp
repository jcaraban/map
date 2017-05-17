/**
 * @file    File.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: get rid of all this template non-sense !!
 */

#ifndef MAP_FILE_HPP_
#define MAP_FILE_HPP_

#include "Format.hpp"
#include "../util/util.hpp"
#include "../runtime/Block.hpp"
#include <string>


namespace map { namespace detail {

struct Node; // forward declaration

typedef int Ferr; //!< Ferr should be a enum

/*
 * @class Interface IFile
 * Used to have a generic-base class which dispatch, through vtables, specific File<...> classes
 *
 * Note: some virtual method are implemented in File<...> since IFile know nothing about Formats, etc
 */
class IFile
{
  public:
	/****************************************
	   Constructors, Destructors, Operators
	 ****************************************/

	static IFile* Factory(std::string file_path);
	static IFile* Factory(Node *node); // @ Deprecated

	IFile();
	IFile(MetaData meta);
	virtual ~IFile() { };

	/***********
	   Getters
	 ***********/
	
	MetaData getMetaData() const;
	StreamDir getStreamDir() const;
	DataType getDataType() const;
	NumDim getNumDim() const;
	MemOrder getMemOrder() const;
	const DataSize& getDataSize() const;
	const BlockSize& getBlockSize() const;
	const NumBlock& getNumBlock() const;
	const std::string& getFilePath() const;
	bool isOpen() const;
	DataStats getDataStats() const;
	bool hasStats() const;

	/***********
	   Setters
	 ***********/

	Ferr setMetaData(MetaData meta, StreamDir stream_dir=NONE_STREAMDIR);
	virtual Ferr setStreamDir(StreamDir stream_dir) = 0;
	virtual Ferr setDataType(DataType data_type) = 0;
	virtual Ferr setNumDim(NumDim num_dim) = 0;
	virtual Ferr setMemOrder(MemOrder mem_order) = 0;
	virtual Ferr setDataSize(DataSize data_size) = 0;
	virtual Ferr setBlockSize(BlockSize block_size) = 0;
	Ferr setDataStats(DataStats stats);

	/***********
	   Methods
	 ***********/

	virtual Ferr open(const std::string file_path, StreamDir stream_dir=NONE_STREAMDIR) = 0;
	virtual Ferr close() = 0;

	virtual Ferr read(void* dst, const Coord& beg_coord, const Coord& end_coord) = 0;
	virtual Ferr write(const void* src, const Coord& beg_coord, const Coord& end_coord) = 0;
	
	virtual Ferr readBlock(Block &block) const = 0;
	virtual Ferr writeBlock(const Block &block) = 0;

  protected:
	/*************
	   Variables
	 *************/

	MetaData meta;
	DataStats stats;
	std::string file_path;
	bool is_open;
	mutable std::mutex access_mtx; //!< Protects the file from concurrent accesses (if needed)
};

#define FILE_TPL typename Format
#define FILE_DEC File<Format>

/*
 * @class Meta-class File<...>
 * Defines specific File<...> classes configured by template parameteres. The purpose is avoiding boilerplate code
 * Using policy pattern
 *
 * Note: File and File<> compose an IS-A relationship
 * Note: some virtual methods have to be implemented here, since IFile don't know about Formats, etc
 */
template <FILE_TPL>
class File : public IFile, public Format
{
	static_assert(std::is_base_of<IFormat<Format>,Format>::value, "1st template does not implement the interface IFormat");

  public:
	/****************************************
	   Constructors, Destructors, Operators
	 ****************************************/

	//virtual IFile* factory();
	File();
	virtual ~File();
	File(MetaData meta);
	//File(MetaData meta, Format format);
	File(const std::string file_path, StreamDir stream_dir);

	File(const File& other) = delete; // Files cannot be copied, only pointers to them
	File& operator=(const File& other) = delete; // Files cannot be copied, only pointers to them

	/***********
	   Setters
	 ***********/

	virtual Ferr setStreamDir(StreamDir stream_dir);
	virtual Ferr setDataType(DataType data_type);
	virtual Ferr setNumDim(NumDim num_dim);
	virtual Ferr setMemOrder(MemOrder mem_order);
	virtual Ferr setDataSize(DataSize data_size);
	virtual Ferr setBlockSize(BlockSize block_size);

	/***********
	   Methods
	 ***********/

	virtual Ferr open(const std::string file_path, StreamDir stream_dir=NONE_STREAMDIR);
	virtual Ferr close();

	virtual Ferr read(void* dst, const Coord& beg_coord, const Coord& end_coord);
	virtual Ferr write(const void* src, const Coord& beg_coord, const Coord& end_coord);
	
	virtual Ferr readBlock(Block &block) const;
	virtual Ferr writeBlock(const Block &block);
	
	Ferr create_temp_file(std::string file_path); // @
	Ferr discard(const Block &block);
	Ferr setReductionType(const ReductionType &type); // @
	VariantType value(); // @
};

#undef FILE_TPL
#undef FILE_DEC

} } // namespace map::detail

#endif

//-------------------------------------------------------------------------------------------------------------------//

#ifndef MAP_FILE_TPL_
#define MAP_FILE_TPL_

#include <cassert>


namespace map { namespace detail {

#define FILE_TPL typename Format
#define FILE_DEC File<Format>

/******************
   File<> Methods
 ******************/

//template <FILE_TPL>
//IFile* FILE_DEC::factory() {
//	return new FILE_DEC;
//}

template <FILE_TPL>
FILE_DEC::File()
	: IFile()
	, Format(IFile::meta,IFile::stats)
{ }

template <FILE_TPL>
FILE_DEC::~File() {
	if (is_open) {
		close();
	}
}

template <FILE_TPL>
FILE_DEC::File(MetaData meta)
	//IFile(meta),
	: Format(IFile::meta,IFile::stats)
{
	Ferr ferr = setMetaData(meta);
	assert(ferr == 0);
}

template <FILE_TPL>
FILE_DEC::File(std::string file_path, StreamDir stream_dir)
	: IFile()
	, Format(IFile::meta,IFile::stats)
{
	open(file_path,stream_dir);
}

template <FILE_TPL>
Ferr FILE_DEC::setStreamDir(StreamDir stream_dir) {
	Ferr ferr = 0;
	
	if (is_open) {
		assert(!"Error in set method, file was open already!");
	}
	switch (stream_dir.get()) {
		case IN  : if (Format::Support::StreamDir::IN  == 0) assert(0); break;
		case OUT : if (Format::Support::StreamDir::OUT == 0) assert(0); break;
		case IO  : if (Format::Support::StreamDir::IO  == 0) assert(0); break;
		default  : assert(0);
	}
	meta.setStreamDir(stream_dir);

	return ferr;
}

template <FILE_TPL>
Ferr FILE_DEC::setDataType(DataType data_type) {
	Ferr ferr = 0;
	
	if (is_open) {
		assert(!"Error in set method, file was open already!");
	}
	switch (data_type.get()) {
		case F32 : if (Format::Support::DataType::F32 == 0) assert(0); break;
		case F64 : if (Format::Support::DataType::F64 == 0) assert(0); break;
		case B8  : if (Format::Support::DataType::B8  == 0) assert(0); break;
		case U8  : if (Format::Support::DataType::U8  == 0) assert(0); break;
		case U16 : if (Format::Support::DataType::U16 == 0) assert(0); break;
		case U32 : if (Format::Support::DataType::U32 == 0) assert(0); break;
		case U64 : if (Format::Support::DataType::U64 == 0) assert(0); break;
		case S8  : if (Format::Support::DataType::S8  == 0) assert(0); break;
		case S16 : if (Format::Support::DataType::S16 == 0) assert(0); break;
		case S32 : if (Format::Support::DataType::S32 == 0) assert(0); break;
		case S64 : if (Format::Support::DataType::S64 == 0) assert(0); break;
		default  : assert(0);
	}
	meta.setDataType(data_type);

	return ferr;
}

template <FILE_TPL>
Ferr FILE_DEC::setNumDim(NumDim num_dim) {
	Ferr ferr = 0;
	
	if (is_open) {
		assert(!"Error in set method, file was open already!");
	}
	switch (num_dim.get() & ~TIME) {
		case D0 : if (Format::Support::NumDim::D0 == 0) assert(0); break;
		case D1 : if (Format::Support::NumDim::D1 == 0) assert(0); break;
		case D2 : if (Format::Support::NumDim::D2 == 0) assert(0); break;
		case D3 : if (Format::Support::NumDim::D3 == 0) assert(0); break;
		default : assert(0);
	}
	if (num_dim.is(TIME) && Format::Support::NumDim::TIME == 0) {    
		assert(0);
	}
	meta.setNumDim(num_dim);

	return ferr;
}

template <FILE_TPL>
Ferr FILE_DEC::setMemOrder(MemOrder mem_order) {
	Ferr ferr = 0;
	
	if (is_open) {
		assert(!"Error in set method, file was open already!");
	}
	switch (mem_order.get() & ~BLK) {
		case ROW : if (Format::Support::MemOrder::ROW == 0) assert(0); break;
		case COL : if (Format::Support::MemOrder::COL == 0) assert(0); break;
		case SFC : if (Format::Support::MemOrder::SFC == 0) assert(0); break;
		default : assert(0);
	}
	if (mem_order.is(BLK) && Format::Support::MemOrder::BLK == 0) {    
		assert(0);
	}
	meta.setMemOrder(mem_order);

	return ferr;
}

template <FILE_TPL>
Ferr FILE_DEC::setDataSize(DataSize data_size) {
	Ferr ferr = 0;

	if (is_open) {
		assert(!"Error in set method, file was open already!");
	}
	meta.setDataSize(data_size);

	return ferr;
}

template <FILE_TPL>
Ferr FILE_DEC::setBlockSize(BlockSize block_size) {
	Ferr ferr = 0;

	if (is_open) {
		assert(!"Error in set method, file was open already!");
	}
	meta.setBlockSize(block_size);

	return ferr;
}

template <FILE_TPL>
Ferr FILE_DEC::open(std::string file_path, StreamDir stream_dir) {
	Ferr ferr;

	if (is_open) {
		assert(!"Error opening, file was open already!");
	}
	if (stream_dir == NONE_STREAMDIR) { 
		if (meta.getStreamDir() == NONE_STREAMDIR) { // Checks that at least one StreamDir was given
			assert(!"One stream_dir must be given, either before or by argument");
		} else { // Always uses the last given
			stream_dir = meta.stream_dir;
		}
	}
	if (stream_dir == OUT) {
		if (meta.getDataType() == NONE_DATATYPE || meta.getNumDim() == NONE_NUMDIM || meta.getMemOrder() == NONE_MEMORDER) {
			assert(!"Format must be configured before opening an OUT file");
		}
	}

	ferr = Format::open(file_path,stream_dir);
	if (ferr) {
		assert(!"Error opening, the specific Format::open() method returned an error");
	}

	// These vars are assigned in any case
	this->file_path = file_path;
	meta.stream_dir = stream_dir;
	is_open = true;

	if (stream_dir == IN) {
		// MetaData vars should be assigned here, not in Format::open()::IN
		// this has to do with the current task division, where Format has more responsabilities than it should
		// Format defines how the data is stored and encoded, but has nothing to do with e.g. opening closing the file
		// @@ Temporary {data_type,num_dim,mem_order,data_size,block_size} are asigned in Formats
		meta.setGroupSize({16,16}); // @@
	}

	return ferr;
}

template <FILE_TPL>
Ferr FILE_DEC::close() {
	Ferr ferr;

	if (!is_open) {
		assert(!"Error closing, no file was open yet!");
	}

	ferr = Format::close();
	if (ferr) {
		assert(!"Error closing, the specific Format::close() method returned an error");
	}

	is_open = false;

	// TODO: now it keeps the previous config, shall the config be reset to NONE ?

	return ferr;
}

template <FILE_TPL>
Ferr FILE_DEC::read(void* dst, const Coord& beg_coord, const Coord& end_coord) {
	Ferr ferr;

	if (!is_open) {
		assert(!"Error reading, no file was open yet!");
	}
	if (beg_coord.size() != end_coord.size() || beg_coord.size() != meta.num_dim.toInt()) {
		assert(0);
	}
	if (any(beg_coord >= end_coord)) {
		assert(0);
	}
	if (any(!in_range(beg_coord,meta.getDataSize()))) {
		assert(0);
	}

	if (Format::Support::Parallel::PARAREAD == 0)
		access_mtx.lock();

	ferr = Format::read(dst,beg_coord,end_coord);
	if (ferr) {
		assert(!"Error reading, the specific Format::read() method returned an error");
	}

	if (Format::Support::Parallel::PARAREAD == 0)
		access_mtx.unlock();
	
	return ferr;
}

template <FILE_TPL>
Ferr FILE_DEC::write(const void* src, const Coord& beg_coord, const Coord& end_coord) {
	Ferr ferr;

	if (!is_open) {
		assert(!"Error reading, no file was open yet!");
	}
	if (beg_coord.size() != end_coord.size() || beg_coord.size() != meta.num_dim.toInt()) {
		assert(0);
	}
	if (any(beg_coord >= end_coord)) {
		assert(0);
	}
	if (any(!in_range(beg_coord,meta.getDataSize()))) {
		assert(0);
	}

	if (Format::Support::Parallel::PARAWRITE == 0)
		access_mtx.lock();

	ferr = Format::write(src,beg_coord,end_coord);
	if (ferr) {
		assert(!"Error writing, the specific Format::write() method returned an error");
	}

	if (Format::Support::Parallel::PARAWRITE == 0)
		access_mtx.unlock();

	return ferr;
}

template <FILE_TPL>
Ferr FILE_DEC::readBlock(Block &block) const {
	Ferr ferr;

	if (!is_open) {
		assert(!"Error reading, no file was open yet!");
	}
	if (meta.num_dim.get() != D0) {
		if (block.key.coord.size() != meta.num_dim.toInt()) {
			assert(0);
		}
		if (any(!in_range(block.key.coord,meta.getNumBlock()))) { // @
			assert(!"Error reading, coord is out of range");
		}
	}

	if (stats.active) {
		int idx = proj(block.key.coord,getNumBlock());
		block.stats.max = stats.maxb[idx];
		block.stats.mean = stats.meanb[idx];
		block.stats.min = stats.minb[idx];
		block.stats.std = stats.stdb[idx];
		block.stats.active = true;
		if (block.stats.max.f64 == block.stats.min.f64) { // @
			block.fixValue( VariantType(block.stats.max,getDataType()) );
			return 0; // @ No need to read when the value is fixed
		}
	}

	if (not Format::Support::Parallel::PARAREAD)
		access_mtx.lock();

	ferr = Format::readBlock(block);
	if (ferr) {
		assert(!"Error reading block, the specific Format::readBlock() method returned an error");
	}

	if (not Format::Support::Parallel::PARAREAD)
		access_mtx.unlock();

	return ferr;
}

template <FILE_TPL>
Ferr FILE_DEC::writeBlock(const Block &block) {
	Ferr ferr;

	if (!is_open) {
		assert(!"Error writing, no file was open yet!");
	}
	if (meta.num_dim.get() != D0) {
		if (block.key.coord.size() != meta.num_dim.toInt()) {
			assert(0);
		}
		if (any(!in_range(block.key.coord,meta.getNumBlock()))) { // @
			assert(!"Error writing, coord is out of range");
		}
	}

	if (Format::Support::Parallel::PARAWRITE == 0)
		access_mtx.lock();

	ferr = Format::writeBlock(block);
	if (ferr) {
		assert(!"Error writing block, the specific Format::writeBlock() method returned an error");
	}

	if (Format::Support::Parallel::PARAWRITE == 0)
		access_mtx.unlock();

	if (stats.active) {
		assert(block.stats.active);
		int idx = proj(block.key.coord,getNumBlock());
		stats.maxb[idx] = block.stats.max;
		stats.meanb[idx] = block.stats.mean;
		stats.minb[idx] = block.stats.min;
		stats.stdb[idx] = block.stats.std;
	}

	return ferr;
}

// @ This function are only for specific formats, but the actual models requires them to be declared in File<...>
// @ Calling them from another File<format> will cause compiling errors

template <FILE_TPL>
Ferr FILE_DEC::create_temp_file(std::string file_path) { // @
	Ferr ferr;

	if (is_open) {
		assert(!"Error, file was open already!");
	}
	
	if (meta.data_type == NONE_DATATYPE || meta.num_dim == NONE_NUMDIM || meta.mem_order == NONE_MEMORDER) {
		assert(!"Format must be configured before creating a temporal file");
	}

	ferr = Format::create_temp_file();
	if (ferr) {
		assert(!"Error creating temporal file");
	}

	// These vars are assigned in any case
	this->file_path = file_path;
	meta.stream_dir = IO;
	is_open = true;

	return ferr;
}

template <FILE_TPL>
Ferr FILE_DEC::discard(const Block &block) { // @
	Ferr ferr;
	
	if (getStreamDir() == IN) {
		assert(!"discard not supported in read-only");
	}
	
	ferr = Format::discard(block);

	return ferr;
}

template <FILE_TPL>
Ferr FILE_DEC::setReductionType(const ReductionType &type) { // @
	Format::setReductionType(type);
	is_open = true;
	return 0;
}

template <FILE_TPL>
VariantType FILE_DEC::value() {
	return Format::value();
}

#undef FILE_TPL
#undef FILE_DEC

} } // namespace map::detail

#endif
