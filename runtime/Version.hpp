/**
 * @file    Version.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Version struct
 */

#ifndef MAP_RUNTIME_VERSION_HPP_
#define MAP_RUNTIME_VERSION_HPP_

#include "../cle/cle.hpp"
#include "../util/Array.hpp"
#include <string>


namespace map { namespace detail {

struct Task; // forward declaration

/*
 * Code Version
 * e.g. GPU version, CPU version
 */
struct Version {
  // constructor
	Version(Task *task, cle::Device dev, std::string detail);

  // methods
	cle::Device device() const;
	DeviceType deviceType() const;
	const BlockSize& groupsize() const;
	const NumBlock& numgroup() const;
	std::string signature() const;
	void createProgram();
	void compileProgram();

	void copyParams(Version *ver);

  // vars
	Task *task;
	cle::Device dev;
	DeviceType dev_type; //!< Device type {CPU,GPU,PHI}
	std::string detail; //!< Parameter to detail the class of version (e.g. radiating NorthWest)
	std::string ver_sign; //!< Signature that uniquely represents the code version

	std::string code; //!< Kernel code
	cle::Task tsk; //!< cle::Task
	
	int shared_size; //!< Shared memory size
	BlockSize group_size; //!< Work group size
	NumBlock num_group; //!< Work group number
	
	std::vector<int> extra_arg; //!< @ Extra arguments needed by the skeleton
};

typedef std::vector<std::unique_ptr<Version>> OwnerVersionList;
typedef std::vector<Version*> VersionList;

} } // namespace map::detail

#endif
