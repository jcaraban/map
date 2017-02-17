/**
 * @file	util.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Utilities for the skeleton classes
 */

#ifndef MAP_RUNTIME_SKELETON_UTIL_HPP_
#define MAP_RUNTIME_SKELETON_UTIL_HPP_

#include "../../util/util.hpp"
#include "../Direction.hpp"
#include <string>
#include <algorithm>


namespace map { namespace detail {

struct Node; // forward delcaration

/*********
   Utils
 *********/

template <typename T>
void sort_unique(T &container) {	
	std::sort(container.begin(),container.end());
	auto it = std::unique(container.begin(),container.end());
	container.resize( std::distance(container.begin(),it) );
}

template <typename T, typename L, typename E>
void sort_unique(T &container, const L &less, const E &equal) {	
	std::sort(container.begin(),container.end(),less);
	auto it = std::unique(container.begin(),container.end(),equal);
	container.resize( std::distance(container.begin(),it) );
}

/***********
   string
 ***********/

std::string operator+(const std::string &str, int i);

std::string operator+(int i, const std::string &str);

std::string& operator+=(std::string &str, int i);

/**********
   Header
 **********/

std::string kernel_sign(const std::string &signature);
std::string in_arg(const Node *in);
std::string out_arg(const Node *out);

/*************
   Variables
 *************/

std::string in_var(const Node *in);
std::string out_var(const Node *out);
std::string var_name(const Node *node, TypeMem mem=PRIVATE, TypeVar var=SCALAR);
std::string shared_decl(const Node *node, int size);
std::string scalar_decl(const std::vector<int> &id, DataType dt);
std::string pointer_decl(const std::vector<Node*> &node_list, DataType dt);
std::string mask_decl(const Mask &mask, int id);
void mask_helper(std::string& str, const Mask &mask, BlockSize& idx, int n);
std::string in_var_focal(const Node *node);
std::string halo_sum(int n, std::vector<BlockSize> halo);
std::string in_var_spread(const Node *node);
std::string out_var_spread(const Node *node);

/************
   Indexing
 ************/

std::string global_proj(int N);
std::string local_proj(int N);
std::string total_proj(int N);
std::string group_size_prod(int N);
std::string local_proj_focal(int N);
std::string local_proj_focal_x(int N, std::string x);
std::string local_proj_focal_H(int N);
std::string local_proj_focal_i(int N);
std::string local_proj_focal_of(int N);
std::string local_proj_focal_nbh(int N, Coord nbh);
std::string local_proj_focal_Hi(int N);
std::string group_size_prod_x(int N, std::string x);
std::string group_size_prod_H(int N);
std::string nbh_size(int N);
std::string pre_load_loop(int N);
std::string local_proj_zonal(int N);
std::string group_proj(int N);

/**************
   Conditions
 **************/

std::string global_cond(int N);
std::string global_cond_focal(int N);
std::string local_cond_focal(int N);
std::string global_cond_zonal(int N);
std::string local_cond_zonal(int N);
std::string global_cond_radial(int N);
std::string equal_coord_cond(int N, Coord c);
std::string zero_cond_spread(int N);

/*************
   Functions
 *************/

std::string defines_local();
std::string defines_local_type(DataType data_type);
std::string defines_local_diver(DataType dt);
std::string defines_focal();
std::string defines_focal_type(DataType data_type);
std::string defines_focal_flow();
std::string defines_zonal_reduc(ReductionType rt, DataType dt);
std::string defines_radial();
std::string defines_radial_type(DataType data_type);
std::string defines_radial_const(Direction fst, Direction snd);
std::string defines_radial_idx();
std::string defines_spread();
std::string defines_spread_type(DataType data_type);

} } // namespace map::detail

#endif
