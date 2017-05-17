/**
 * @file    File.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "File.hpp"
#include "../file/binary.hpp"
#include "../file/tiff.hpp"
#include "../file/scalar.hpp"
#include "../runtime/dag/Node.hpp"
#include "../runtime/dag/ZonalReduc.hpp"
#include <algorithm>


namespace map { namespace detail {

/*****************
   IFile Methods
 *****************/

IFile* IFile::Factory(std::string file_path) {
	IFile *file = nullptr;

	if (file_path.compare("/dev/null") == 0) {
		file = new File<binary>;
		return file;
	}

	size_t pos = file_path.find_last_of('.') + 1;
	std::string::iterator beg = file_path.begin() + pos;
	std::string::iterator end = file_path.end();
	int dist = std::distance(beg,end);

	if (std::equal(beg,end,"bin")){
		file = new File<binary>;
	} else if (std::equal(beg,end,"tif")) {
		file = new File<tiff>;
	} else if (std::equal(beg,end,"tiff")) {
		file = new File<tiff>;
	} else if (std::equal(beg,end,"tmp")) {
		file = new File<scalar>;
	} else {
		// try with GDAL
	}

	if (file == nullptr) {
		assert(0);
	}

	return file;
}
// @ Deprecated
IFile* IFile::Factory(Node *node) {
	// Refactorize and try to eliminate the dynamic_cast

	if (node->numdim() == D0) // scalar file
	{
		File<scalar> *sca_file = new File<scalar>(node->metadata());
		if (node->pattern().is(ZONAL))
		{
			auto *red = dynamic_cast<ZonalReduc*>(node);
			sca_file->setReductionType(red->type);
		}
		else // regular D0, for e.g. ScalarTask
		{
			sca_file->setReductionType(NONE_REDUCTION);
		}
		return sca_file;
	}
	else // binary file
	{
		File<binary> *bin_file = new File<binary>(node->metadata());
		std::string file_path = std::string("tmp") + std::to_string(node->id);
		Ferr ferr = bin_file->create_temp_file(file_path);
		if (ferr) {
			assert(0);
		}	
		return bin_file;
	}
}

IFile::IFile()
	: is_open(false)
{ }

IFile::IFile(MetaData meta)
	: meta(meta)
	, is_open(false)
{ }

MetaData IFile::getMetaData() const {
	return meta;
}

StreamDir IFile::getStreamDir() const {
	/*if (!is_open) {
		assert(!"Error in get method, no file was open yet!");
	}*/
	return meta.getStreamDir();
}

DataType IFile::getDataType() const {
	/*if (!is_open) {
		assert(!"Error in get method, no file was open yet!");
	}*/
	return meta.getDataType();
}

NumDim IFile::getNumDim() const {
	/*if (!is_open) {
		assert(!"Error in get method, no file was open yet!");
	}*/
	return meta.getNumDim();
}

MemOrder IFile::getMemOrder() const {
	/*if (!is_open) {
		assert(!"Error in get method, no file was open yet!");
	}*/
	return meta.getMemOrder();
}

const DataSize& IFile::getDataSize() const {
	/*if (!is_open) {
		assert(!"Error in get method, no file was open yet!");
	}*/
	return meta.getDataSize();
}

const BlockSize& IFile::getBlockSize() const {
	/*if (!is_open) {
		assert(!"Error in get method, no file was open yet!");
	}*/
	return meta.getBlockSize();
}

const NumBlock& IFile::getNumBlock() const {
	/*if (!is_open) {
		assert(!"Error in get method, no file was open yet!");
	}*/
	return meta.getNumBlock();
}

const std::string& IFile::getFilePath() const {
	return file_path;
} 

bool IFile::isOpen() const {
	return is_open;
}

DataStats IFile::getDataStats() const {
	return stats;
}

bool IFile::hasStats() const {
	return stats.active;
}

Ferr IFile::setMetaData(MetaData meta, StreamDir stream_dir) {
	Ferr ferr = 0;
	
	//this->meta = meta; // delete

	if (stream_dir == NONE_STREAMDIR)
		stream_dir = meta.getStreamDir();
	if ((ferr = setStreamDir(stream_dir)) != 0)
		assert(0); //return ferr;
	if ((ferr = setDataType(meta.getDataType())) != 0)
		assert(0); //return ferr;
	if ((ferr = setNumDim(meta.getNumDim())) != 0)
		assert(0); //return ferr;
	if ((ferr = setMemOrder(meta.getMemOrder())) != 0)
		assert(0); //return ferr;
	if ((ferr = setDataSize(meta.getDataSize())) != 0)
		assert(0); //return ferr;
	if ((ferr = setBlockSize(meta.getBlockSize())) != 0)
		assert(0); //return ferr;

	return ferr;
}

Ferr IFile::setDataStats(DataStats stats) {
	this->stats = stats;
}

} } // namespace map::detail
