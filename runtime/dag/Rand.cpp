/**
 * @file	Rand.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 *****************************************************************************
 * 			 RANDOM123 LICENSE AGREEMENT
 * Copyright 2010-2011, D. E. Shaw Research. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions, and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions, and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of D. E. Shaw Research nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************
 */

#include "Rand.hpp"
#include "../visitor/Visitor.hpp"
#include "../../thirdparty/Random123-1.09/include/Random123/philox.h"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Rand::Content::Content(Rand *node) {
	seed = node->seed();
}

bool Rand::Content::operator==(const Content& k) const {
	return (seed==k.seed);
}

std::size_t Rand::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.seed);
}

// Factory

Node* Rand::Factory(Node *seed, DataType type, MemOrder order) {
	assert(seed != nullptr);

	DataSize ds = seed->datasize();
	DataType dt = (type != NONE_DATATYPE) ? type : seed->datatype();
	MemOrder mo = (order != NONE_MEMORDER) ? order : seed->memorder();
	BlockSize bs = seed->blocksize();
	GroupSize gs = seed->groupsize();
	MetaData meta(ds,dt,mo,bs,gs);

	return new Rand(meta,seed);
}

Node* Rand::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Rand(this,other_to_this);
}

// Constructors

Rand::Rand(const MetaData &meta, Node *seed)
	: Node(meta)
{
	prev_list.reserve(1);
	this->addPrev(seed); // [0]
	seed->addNext(this);

	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

Rand::Rand(const Rand *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{ }

// Methods

void Rand::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Rand::getName() const {
	return "Rand";
}

std::string Rand::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += numdim().toString();
	sign += datatype().toString();
	return sign;
}

Node* Rand::seed() const {
	return prev_list[0];
}

// Compute

void Rand::computeScalar(std::unordered_map<Node*,VariantType> &hash) {
	typedef r123::Philox2x32 rng32;
	typedef r123::Philox2x64 rng64;
	typedef unsigned char uchar;
	typedef unsigned short ushort;
	typedef unsigned int uint;
	typedef unsigned long ulong;
	assert(numdim() == D0);

	auto seed_value = hash.find(seed())->second;

	if (datatype().is64()) // 64-bits
	{
		rng64::ctr_type ctr = {{seed_value.convert(U64).get<U64>()}};
		rng64::key_type key = {{0}};
		rng64::ctr_type res = rng64()(ctr,key);

		if (datatype().isFloating()) // floating-point
		{
			value.ref<F64>() = *(double*)& res;
		}
		else if (datatype().isSigned()) // signed integer
		{
			value.ref<S64>() = *(long*)& res;
		}
		else if (datatype().isUnsigned()) // unsigned integer
		{
			value.ref<U64>() = *(ulong*)& res;	
		}
		else {
			assert(0);
		}
	}
	else if (datatype().is32()) // 32-bits
	{
		rng32::ctr_type ctr = {{seed_value.convert(U32).get<U32>()}};
		rng32::key_type key = {{0}};
		rng32::ctr_type res = rng32()(ctr,key);

		if (datatype().isFloating()) // floating-point
		{
			value.ref<F32>() = *(float*)& res;
		}
		else if (datatype().isSigned()) // signed integer
		{
			value.ref<S32>() = *(int*)& res;
		}
		else if (datatype().isUnsigned()) // unsigned integer
		{
			value.ref<U32>() = *(uint*)& res;	
		}
		else {
			assert(0);
		}
	}
	else if (datatype().is16()) // 16-bits, using rng32
	{
		rng32::ctr_type ctr = {{seed_value.convert(U32).get<U32>()}};
		rng32::key_type key = {{0}};
		rng32::ctr_type res = rng32()(ctr,key);

		if (datatype().isFloating()) // floating-point
		{
			assert(!"F16 not supported");
		}
		else if (datatype().isSigned()) // signed integer
		{
			value.ref<S16>() = *(short*)& res;
		}
		else if (datatype().isUnsigned()) // unsigned integer
		{
			value.ref<U16>() = *(ushort*)& res;	
		}
		else {
			assert(0);
		}
	}
	else if (datatype().is8()) // 8-bits, using rng32
	{
		rng32::ctr_type ctr = {{seed_value.convert(U32).get<U32>()}};
		rng32::key_type key = {{0}};
		rng32::ctr_type res = rng32()(ctr,key);

		if (datatype().isFloating()) // floating-point
		{
			assert(!"F8 not supported");
		}
		else if (datatype().isSigned()) // signed integer
		{
			value.ref<S8>() = *(char*)& res;
		}
		else if (datatype().isUnsigned()) // unsigned integer
		{
			value.ref<U16>() = *(uchar*)& res;	
		}
		else {
			assert(0);
		}
	}
	else {
		assert(0);
	}
}

void Rand::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	hash[{this,coord}] = ValFix(); // @ something more? max / min ?
}

} } // namespace map::detail
