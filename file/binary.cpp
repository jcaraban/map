/* 
 * @file	binary.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: maybe check erroneous file by comparing its length (seek_end-seek_beg) with data_size
 * TODO: consider spliting the StreamDir IO into InOut and OutIn, to model the case where we want to write
 *       but also to have a temporal file backing the data (to be reused later). OI would cover this case
 * TODO: move c_func into anonymous namespace?
 */

#include "binary.hpp"
#include <cstring>
#include <iostream>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>


#define PAGE_SIZE 4096


namespace map { namespace detail {

/*********
   Utils
 *********/

// C Posix wrappers to avoid name collisions with internal open() / read() / etc
int c_open(const char *a, int b) { return open(a,b); }
int c_open(const char *a, int b, mode_t c) { return open(a,b,c); }
ssize_t c_read(int a, void *b, size_t c) { return read(a,b,c); }
ssize_t c_write(int a, const void *b, size_t c) { return write(a,b,c); }
ssize_t c_pread(int a, void *b, size_t c, off_t d) { return pread(a,b,c,d); }
ssize_t c_pwrite(int a, const void *b, size_t c, off_t d) { return pwrite(a,b,c,d); }
int c_close(int a) { return close(a); }

/***********
   Support
 ***********/

#define SUPPORT template<> const bool IFormat<binary>::Support

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
SUPPORT :: NumDim :: D1 = 1;
SUPPORT :: NumDim :: D2 = 1;
SUPPORT :: NumDim :: D3 = 0;

SUPPORT :: MemOrder :: BLK = 1;
SUPPORT :: MemOrder :: ROW = 1;
SUPPORT :: MemOrder :: COL = 0;
SUPPORT :: MemOrder :: SFC = 0;

SUPPORT :: Parallel :: PARAREAD = 1;
SUPPORT :: Parallel :: PARAWRITE = 1;

#undef SUPPORT

/***********
   Methods
 ***********/

binary::binary(MetaData& meta, DataStats& stats)
	: IFormat<binary>(meta,stats)
	, fd(0)
	, initial_offset(0)
	, total_data_size(0)
	, total_block_size(0)
{ }

binary::~binary() { }

Ferr binary::open(std::string file_path, StreamDir stream_dir) {
	Ferr ferr = 0;

	if (stream_dir == IN || stream_dir == IO)
	{
		if (stream_dir == IN)
		{
			if ((fd = c_open(file_path.c_str(), O_RDONLY)) == -1) {
				assert(!"Couldn't open <binary> file for reading!");
			}
		}
		else if (stream_dir == IO)
		{
			// This case only includes when the file already contains data. We first read and maybe write later
			// For an empty file where we write and them read (eg file-backed data) see create_temp_file()
			if ((fd = c_open(file_path.c_str(), O_RDWR)) == -1) {
				assert(!"Couldn't open <binary> file for reading / writting!");
			}
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

		// Cached variables
		initial_offset = PAGE_SIZE;
		total_data_size = meta.getTotalDataSize();
		total_block_size = meta.getTotalBlockSize();

		// TODO: maybe check erroneous file by comparing its length (seek_end-seek_beg) with total_data_size
	}
	else if (stream_dir == OUT) 
	{
		// Always creates and truncates the file
		if ((fd = c_open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, (mode_t)0600)) == -1) {
				std::cout << strerror(errno) << std::endl;
				assert(!"Couldn't create <binary> file for writing!");
		}

		// Cached variables
		initial_offset = PAGE_SIZE;
		total_data_size = meta.getTotalDataSize();
		total_block_size = meta.getTotalBlockSize();

		ferr = setMeta();
		if (ferr != 0) {
			assert(0);
		}

		// Extra traits go here...
	}
	else
	{
		assert(!"Unknown mode");
	}
	
	return ferr;
}

Ferr binary::create_temp_file() {
	Ferr ferr = 0;
	char name[] = "mapXXXXXX";

	// Creates temporal file that will be automatically removed after closed
	if ((fd = mkstemp(name)) == -1) {
			std::cout << strerror(errno) << std::endl;
			assert(!"Couldn't create temporary <binary> file for reading / writing!");
	}
    unlink(name);

	// Cached variables
	initial_offset = PAGE_SIZE;
	total_data_size = meta.getTotalDataSize();
	total_block_size = meta.getTotalBlockSize();

	// Truncate file to the desired length
    if (ftruncate(fd, initial_offset+total_data_size) == -1) {
    	std::cout << strerror(errno) << std::endl;
		assert(!"Couldn't truncate the file for reading / writing!");
	}

	ferr = setMeta();
	if (ferr != 0) {
		assert(0);
	}
		
	// Extra traits go here...

	return ferr;
}

Ferr binary::close() {
	Ferr ferr;

	if (meta.stream_dir == IN || meta.stream_dir == IO)
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

	// Closing file
	ferr = c_close(fd);
	if (ferr) {
		assert(0);
	}

	return ferr;
}

Ferr binary::getMeta() {
	Ferr ferr = 0;
	size_t off = 0;

	// data_type
	c_pread(fd, (char*)&meta.data_type, sizeof(meta.data_type), off);
	off += sizeof(meta.data_type);

	// num_dim
	c_pread(fd, (char*)&meta.num_dim, sizeof(meta.num_dim), off);
	off += sizeof(meta.num_dim);
	const int num = meta.num_dim.toInt();
	meta.data_size = DataSize(num);
	meta.block_size = BlockSize(num);

	// data_size
	for (int i=0; i<num; i++) {
		c_pread(fd, (char*)&meta.data_size[i], sizeof(meta.data_size[i]), off);
		off += sizeof(meta.data_size[i]);
	}

	// mem_order
	c_pread(fd, (char*)&meta.mem_order, sizeof(meta.mem_order), off);
	off += sizeof(meta.mem_order);

	// block_size
	for (int i=0; i<num; i++) {
		c_pread(fd, (char*)&meta.block_size[i], sizeof(meta.block_size[i]), off);
		off += sizeof(meta.block_size[i]);
	}

	assert(off <= PAGE_SIZE);
	return ferr;
}

Ferr binary::setMeta() {
	Ferr ferr = 0;
	size_t off = 0;

	// data_type
	c_pwrite(fd, (char*)&meta.data_type, sizeof(meta.data_type), off);
	off += sizeof(meta.data_type);

	// num_dim
	c_pwrite(fd, (char*)&meta.num_dim, sizeof(meta.num_dim), off);
	off += sizeof(meta.num_dim);

	// data_size
	for (int i=0; i<meta.num_dim.toInt(); i++) {
		c_pwrite(fd, (char*)&meta.data_size[i], sizeof(meta.data_size[i]), off);
		off += sizeof(meta.data_size[i]);
	}

	// mem_order
	c_pwrite(fd, (char*)&meta.mem_order, sizeof(meta.mem_order), off);
	off += sizeof(meta.mem_order);

	// block_size
	for (int i=0; i<meta.num_dim.toInt(); i++) {
		c_pwrite(fd, (char*)&meta.block_size[i], sizeof(meta.block_size[i]), off);
		off += sizeof(meta.block_size[i]);
	}

	assert(off <= PAGE_SIZE);
	return ferr;
}

Ferr binary::getStats() {
	Ferr ferr = 0;

	return ferr;
}

Ferr binary::setStats() {
	Ferr ferr = 0;

	return ferr;
}

Ferr binary::read(void* dst, const Coord& beg_coord, const Coord& end_coord) {
	if (meta.mem_order != ROW || meta.num_dim != D2) {
		assert(0);
	}

	size_t offset = initial_offset + (beg_coord[1]*meta.data_size[0] + beg_coord[0]) * meta.data_type.sizeOf();
	Coord len = end_coord - beg_coord;
	Ferr ferr = 0;

	for (int y=beg_coord[1]; y<end_coord[1]; y++) {
		ferr = (pread(fd, dst, len[1], offset) == -1);
		if (ferr) {
			std::cout << strerror(errno) << std::endl;
			assert(0);
		}
		offset += meta.data_size[0];
		dst = static_cast<char*>(dst) + meta.data_size[0];
	}

	return ferr;
}

Ferr binary::write(const void* src, const Coord& beg_coord, const Coord& end_coord) {
	if (meta.mem_order != ROW || meta.num_dim != D2) {
		assert(0);
	}

	size_t offset = initial_offset + (beg_coord[1]*meta.data_size[0] + beg_coord[0]) * meta.data_type.sizeOf();
	Coord len = end_coord - beg_coord;
	Ferr ferr = 0;

	for (int y=beg_coord[1]; y<end_coord[1]; y++) {
		ferr = (pwrite(fd, src, len[1], offset) == -1);
		if (ferr) {
			std::cout << strerror(errno) << std::endl;
			assert(0);
		}
		offset += meta.data_size[0];
		src = static_cast<const char*>(src) + meta.data_size[0];
	}

	return ferr;
}

Ferr binary::readBlock(Block &block) const {
	if (meta.mem_order != ROW+BLK) {
		assert(0);
	}

	size_t offset = proj(block.key.coord,meta.num_block) * total_block_size;
	Ferr ferr = 0;
	size_t ret, len = 0;
	char *mem = (char*)block.entry->host_mem;

	offset += initial_offset;
	while (len < total_block_size) {
		ret = pread(fd, mem+len, total_block_size-len, offset+len);
		ferr = (ret < 0);
		if (ferr) {
			std::cout << strerror(errno) << std::endl;
			assert(0);
		}
		len += ret;
	}

	return ferr;
}

Ferr binary::writeBlock(const Block &block) {
	if (meta.mem_order != ROW+BLK) {
		assert(0);
	}

	size_t offset = proj(block.key.coord,meta.num_block) * total_block_size;
	Ferr ferr = 0;
	size_t ret, len = 0;
	char *mem = (char*)block.entry->host_mem;

	offset += initial_offset;
	while (len < total_block_size) {
		ret = pwrite(fd, mem+len, total_block_size-len, offset+len);
		ferr = (ret < 0);
		if (ferr) {
			std::cout << strerror(errno) << std::endl;
			assert(0);
		}
		len += ret;
	}

	return ferr;
}

#define FALLOC_FL_KEEP_SIZE    0x01
#define FALLOC_FL_PUNCH_HOLE   0x02

Ferr binary::discard(const Block &block) {
	Ferr ferr = 0;
	size_t offset = proj(block.key.coord,meta.num_block) * total_block_size;

	//ferr = fallocate(fd, FALLOC_FL_ZERO_RANGE, offset+initial_offset, total_block_size);
	ferr = fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, offset+initial_offset, total_block_size);
	if (ferr) {
		std::cout << strerror(errno) << std::endl;
		assert(0);
	}

	return ferr;
}

} } // namespace map::detail
