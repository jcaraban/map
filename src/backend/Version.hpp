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
struct Version; // forward declaration
typedef std::vector<std::unique_ptr<Version>> OwnerVersionList;
typedef std::vector<Version*> VersionList;

/*
 *
 */
struct Verkey {
	const Task *task;
	cle::Device dev;
	GroupSize group;
	std::string detail;

	Verkey(Task *task);
	Verkey(Version *ver);
	bool operator==(const Verkey &) const;
	struct Hash {
		std::size_t operator()(const Verkey &k) const;
	};
};
typedef std::vector<Verkey> VerkeyList;


/*
 * Code Version
 * e.g. GPU version, CPU version
 */
struct Version {
  // constructor
	Version(Verkey content);

  // methods
	cle::Device device() const;
	DeviceType deviceType() const;
	const GroupSize& groupsize() const;
	const NumGroup& numgroup() const;
	std::string signature() const;

	void generateCode();
	void createProgram();
	void compileCode();
	void reuseCode(Version *ver);

  // vars
	const Task *task; //!< Aggregation

	cle::Device dev;
	DeviceType dev_type; //!< Device type {CPU,GPU,PHI}
	std::string detail; //!< Parameter to detail the class of version (e.g. radial NorthWest)
	std::string ver_sign; //!< Signature that uniquely represents the code version

	std::string code; //!< Kernel code
	cle::Task tsk; //!< cle::Task
	
	int shared_size; //!< Shared memory size
	GroupSize group_size; //!< Work-group size
	NumGroup num_group; //!< Work-group number
	
	std::vector<int> extra_arg; //!< @ Extra arguments needed by the skeleton
};

} } // namespace map::detail

#endif
