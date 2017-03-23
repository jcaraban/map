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
#include <functional>


namespace map { namespace detail {

// Internal declarations

Rand::Key::Key(Rand *node) {
	seed = node->seed();
}

bool Rand::Key::operator==(const Key& k) const {
	return (seed==k.seed);
}

std::size_t Rand::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.seed);
}

// Factory

Node* Rand::Factory(Node *seed, DataType type, MemOrder order) {
	assert(seed != nullptr);

	DataSize ds = seed->datasize();
	DataType dt = (type != NONE_DATATYPE) ? type : seed->datatype();
	MemOrder mo = (order != NONE_MEMORDER) ? order : seed->memorder();
	BlockSize bs = seed->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new Rand(meta,seed);
}

Node* Rand::clone(NodeList new_prev_list, NodeList new_back_list) {
	return new Rand(this,new_prev_list,new_back_list);
}

// Constructors

Rand::Rand(const MetaData &meta, Node *seed)
	: Node(meta)
{
	prev_list.reserve(1);
	this->addPrev(seed); // [0]

	seed->addNext(this);
}

Rand::Rand(const Rand *other, NodeList new_prev_list, NodeList new_back_list)
	: Node(other,new_prev_list,new_back_list)
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
/*
Node*& Rand::seed() {
	return prev_list[0];
}
*/
Node* Rand::seed() const {
	return prev_list[0];
}

} } // namespace map::detail
