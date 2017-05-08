/**
 * @file	util.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "util.hpp"
#include "../dag/IO.hpp"
#include "../dag/Diversity.hpp" // @


namespace map { namespace detail {

namespace { // anonymous namespace
	using std::string;
}

/***********
   string
 ***********/

string operator+(const string &str, int i) {
	return str + std::to_string(i);
}

string operator+(int i, const string &str) {
	return std::to_string(i) + str;
}

string& operator+=(string &str, int i) {
	return str += std::to_string(i);
}

/**********
   Header
 **********/

string kernel_sign(const string &signature) {
	size_t hash = std::hash<string>()(signature);
	return "kernel void krn" + std::to_string(hash);
}

string in_arg(const Node *node, bool extended) {
	if (extended)
		return in_arg_focal(node);
	else
		return in_arg_local(node);
}

string in_arg_local(const Node *node) {
	bool d0 = (node->numdim() == D0);
	string type = node->datatype().ctypeString();

	string str = "";
	if (d0) {
		str += type + " IN_" + node->id + "v,";
	} else { // !d0
		str += "global " + type + " *IN_" + node->id + ", ";
		str += type + " IN_" + node->id + "v, ";
		str += string("uchar IN_") + node->id + "f,";
	}

	return str;
}

string in_arg_focal(const Node *node) {
	string str = "TYPE_VAR_LIST(";
	str += node->datatype().ctypeString();
	str += string(",IN_") + node->id + "),";
	return str;
}

string out_arg(const Node *node) {
	bool d0 = (node->numdim() == D0);
	std::string type = node->datatype().ctypeString();

	string str = "global " + type + " *OUT_" + node->id + ",";
	if (d0)
		str += string(" int idx_") + node->id + ",";

	return str;
}

/*************
   Variables
 *************/

string in_var(const Node *node) {
	bool d0 = (node->numdim() == D0);
	string type = node->datatype().toString();

	string str = "";
	if (d0) {
		str += str + "IN_" + node->id + "v";
	} else { // !d0
		str += str + "LOAD_L_" + type + "(IN_" + node->id + ")";
	}

	return str;
}

string out_var(const Node *node) {
	assert(node->numdim() != D0);
	string str = string("OUT_") + node->id + "[" + global_proj(node->numdim().toInt()) + "]";
	return str;
}

string var_name(const Node *node, TypeMem mem, TypeVar var) {
	string str = node->datatype().toString();
	switch (mem) {
		case PRIVATE: break;
		case SHARED  : str += "S"; break;
		case LITERAL : str += "L"; break;
		default: assert(0);
	}
	switch (var) {
		case SCALAR : break;
		case ARRAY : str += "A"; break;
		case POINTER: str += "P"; break;
		default: assert(0);
	}
	return str + "_" + std::to_string(node->id);
}

string shared_decl(const Node *node, int size) {
	string str = string("local ") + node->datatype().ctypeString();
	return str + " " + var_name(node,SHARED) + "[" + std::to_string(size) + "];";
}

string scalar_decl(const std::vector<int> &id_vec, DataType dt) {
	string str = dt.ctypeString();
	for (auto id : id_vec) {
		str = str + " " + dt.toString() + "_" + id + ",";
	}
	str[str.size()-1] = ';';
	return str;
}
// Is this necessary?
string pointer_decl(const std::vector<Node*> &node_list, DataType dt) {
	string str = dt.ctypeString();
	for (auto node : node_list) {
		DataSize ds = node->datasize();
		str = str + " ";
		for (int j=0; j<ds.size(); j++)
			str = str + "*";
		str = str + dt.toString() + "P_" + node->id + ",";
	}
	str[str.size()-1] = ';';
	return str;
}

string mask_decl(const Mask &mask, int id) {
	DataSize ds = mask.datasize();
	DataType dt = mask.datatype();
	string str = dt.ctypeString() + " ";
	
	str += dt.toString() + "L_" + std::to_string(id);
	for (int i=0; i<ds.size(); i++) {
		str += "[" + std::to_string(ds[i]) + "]";
	}
	str += " = ";

	BlockSize idx(ds.size());
	for (int i=0; i<idx.size(); i++)
		idx[i] = 0;

	mask_helper(str,mask,idx,ds.size());
	return str + ";";
}

void mask_helper(string& str, const Mask &mask, BlockSize& idx, int n) {
	assert(n > 0);
	DataSize ds = mask.datasize();
	DataType dt = mask.datatype();
	n--;
	str += "{";
	for (idx[n]=0; idx[n]<ds[n]; idx[n]++) {
		if (n == 0) {
			str += mask.array[proj(idx,ds)].toString();
		} else {
			mask_helper(str,mask,idx,n);
		}
		str += ',';
	}
	str[str.size()-1] = '}';
}

string in_var_focal(const Node *node) {
	string type = node->datatype().toString();
	string str = "LOAD_F_" + type + "(IN_" + node->id + ")";
	return str;
}

string halo_sum(int n, std::vector<BlockSize> halo) {
	string plus, str = "";
	for (int h=0; h<halo.size(); h++) {
		plus = (h < halo.size()-1) ? " + " : "";
		str += std::to_string(halo[h][n]) + plus;
	}
	return str;
}

string in_var_spread(const Node *node) {
	string str;
	string var = string("OUT_") + node->id;
	string type = node->datatype().ctypeString();
	str = "LOAD_" + type + "_i(" + var + ")";
	return str;
}

string out_var_spread(const Node *node) {
	string str;
	string var = string("OUT_") + node->id;
	string type = node->datatype().ctypeString();
	str = "STORE_" + type + "(" + var + ")";
	return str;
}

string diver_decl(const Node *node, int n_arg, int size, DataType dt) {
	int N = node->numdim().toInt();
	string type = dt.ctypeString();
	string elemsa = string("elemSA_") + node->id;
	string cntsa = string("countSA_") + node->id;
	string str = "";
	str += "local " + type + " " + elemsa + "_group[" + size + "];" + "\n";
	str += "\tlocal int " + cntsa + "_group[" + size + "];" + "\n";
	str += "\tlocal " + type + "* " + elemsa + " = " + elemsa + "_group + " + n_arg + "*(" + local_proj(N) + ");" + "\n";
	str += "\tlocal int* " + cntsa + " = " + cntsa + "_group + " + n_arg + "*(" + local_proj(N) + ");" + "\n";
	return str;
}

/************
   Indexing
 ************/

string global_proj(int N) {
	// Recursively ((bc2)*BS1 + bc1)*BS0 + bc0
	string str = string("bc") + (N-1);
	for (int n=N-2; n>=0; n--) {
		str = string("(") + str + ")*BS"+n;
		str += string("+bc") + n;
	}
	return str;
}

string local_proj(int N) {
	if (N <= 0) return string("0");
	// Recursively ((gc2)*GS1 + gc1)*GS0 + gc0
	string str = string("gc") + (N-1);
	for (int n=N-2; n>=0; n--) {
		str = string("(") + str + ")*GS"+n;
		str += string("+gc") + n;
	}
	return str;
}

string total_proj(int N) {
	// @ not working, is missing NumBlock
	// Recursively BS1*BC1+BS0*BC0 + ((bc2)*BS1 + bc1)*BS0 + bc0
	string str = string("BS")+(N-1)+"*BC"+(N-1);
	for (int n=N-2; n>=0; n--) {
		str += string("+BS")+n+"*BC"+n;
	}
	str += "+" + global_proj(N);
	return str;
}

string group_size_prod(int N) {
	// GS0 * GS1 * GS2 ...
	string str = "GS0";
	for (int n=1; n<N; n++)
		str += string("*GS")+n;
	return str;
}

string global_cond(int N) {
	// bc0 < BS0 && bc1 < BS1 && bc2 < BS2
	string str = "";
	for (int n=0; n<N; n++) {
		if (n > 0) str += " && ";
		str += string("bc")+n+" < BS"+n;
	}
	return str;
}

string local_proj_focal(int N) {
	// Recursively ((gc2)*(GS1+H1*2) + gc1)*(GS0+H0*2) + gc0
	string str = string("gc")+(N-1);
	for (int n=N-2; n>=0; n--) {
		str = string("(") + str + ")*(GS"+n+"+H"+n+"*2)";
		str += string("+gc")+n;
	}
	return str;
}

string local_proj_focal_x(int N, string x) {
	// Recursively ((gc2+x2)*(GS1+H1*2) + gc1+x1)*(GS0+H0*2) + gc0+x0
	string str = string("gc")+(N-1) + "+"+x+(N-1);
	for (int n=N-2; n>=0; n--) {
		str = string("(") + str + ")*(GS"+n+"+H"+n+"*2)";
		str += string("+gc")+n + "+"+x+n;
	}
	return str;
}

string local_proj_focal_H(int N) {
	// Recursively ((gc2+H2)*(GS1+H1*2) + gc1+H1)*(GS0+H0*2) + gc0+H0
	return local_proj_focal_x(N,"H");
}

string local_proj_focal_i(int N) {
	// Recursively ((gc2+i2)*(GS1+H1*2) + gc1+i1)*(GS0+H0*2) + gc0+i0
	return local_proj_focal_x(N,"i");
}

string local_proj_focal_of(int N) {
	// Recursively ((gc2+of2)*(GS1+H1*2) + gc1+of1)*(GS0+H0*2) + gc0+of0
	return local_proj_focal_x(N,"of");
}

string local_proj_focal_Hi(int N, int id) {
	// Recursively ((gc2+H2+i2)*(GS1+H1*2) + gc1+H1+i1)*(GS0+H0*2) + gc0+H0+i0
	string str = string("gc")+(N-1) + "+H"+(N-1)+"_"+id+"+i"+(N-1);
	for (int n=N-2; n>=0; n--) {
		str = string("(") + str + ")*(GS"+n+"+H"+n+"_"+id+"*2)";
		str += string("+gc")+n + "+H"+n+"_"+id+"+i"+n;
	}
	return str;
}

string local_proj_focal_nbh(int N, Coord nbh) {
	// Recursively ((gc2+H2+nbh2)*(GS1+H1*2) + gc1+H1+nbh1)*(GS0+H0*2) + gc0+H0+nbh0
	string str = string("gc")+(N-1) + "+H"+(N-1)+"+"+nbh[N-1];
	for (int n=N-2; n>=0; n--) {
		str = string("(") + str + ")*(GS"+n+"+H"+n+"*2)";
		str += string("+gc")+n + "+H"+n+"+"+nbh[n];
	}
	return str;
}

string group_size_prod_x(int N, string x) {
	// (GS2+x2) * (GS1+x1) * (GS0+x0) ...
	string str = string("(GS")+(N-1) + "+"+x+(N-1)+")";
	for (int n=N-2; n>=0; n--)
		str += string("*(GS")+n +"+"+x+n+")";
	return str;
}

string group_size_prod_H(int N) {
	if (N == 0) return string("1");
	// (GS2+2*H2) * (GS1+2*H1) * (GS0+2*H0) ...
	return group_size_prod_x(N,"2*H");
}

string nbh_size(int N) {
	// H0*2+1 * H1*2+1 * H2*2+ ...
	string str = "H0*2+1";
	for (int n=1; n<N; n++)
		str = "(" + str + ")*H"+n+"*2+1";
	return str;
}

string pre_load_loop(int N) {
	// int i=0; i<gspH/gsp; i++
	string str = "int i=0; i<";
	str += "("+group_size_prod_H(N)+"-1)/("+group_size_prod(N)+")+1";
	str += "; i++";
	return str;
}

string local_proj_zonal(int N) {
	return local_proj(N);
}

string group_proj(int N) {
	// Recursively ((GC2)*GN1 + GC1)*GN0 + GC0
	string str = string("GC") + (N-1);
	for (int n=N-2; n>=0; n--) {
		str = string("(") + str + ")*GN"+n;
		str += string("+GC") + n;
	}
	return str;
}

/**************
   Conditions
 **************/

string global_cond_focal(int N) {
	// bc0 < BS0+H0 && bc1 < BS1+H1 && bc2 < BS2+H2
	string str = "";
	for (int n=0; n<N; n++) {
		if (n > 0) str += " && ";
		str += string("bc")+n+" < BS"+n+"+H"+n;
	}
	return str;
}

string local_cond_focal(int N) {
	// gc0 >= 0 && gc0 < GS0 && gc1 >= 0 && gc1 < GS1 && ...
	string str = "";
	for (int n=0; n<N; n++) {
		if (n > 0) str += " && ";
		str += string("gc")+n+" >= 0 && gc"+n+" < GS"+n;
	}
	return str;
}

string local_cond_zonal(int N) {
	// gc0 == 0 && gc1 == 0 && gc2 == 0 ...
	string str = "";
	for (int n=0; n<N; n++) {
		if (n > 0) str += " && ";
		str += string("gc")+n + " == 0";
	}
	return str;
}

string global_cond_radial(int N) {
	// bc0 >= 0 && bc0 < BS0 && bc1 >= 0 && bc1 < BS1 && ...
	string str = "";
	for (int n=0; n<N; n++) {
		if (n > 0) str += " && ";
		str += string("bc")+n+" >= 0 && bc"+n+" < BS"+n;
	}
	str += " && dif0 >= dif1 && dif1 >= 0";
	str += " && dif0 >= 0"; // Including start
	return str;
}

string equal_coord_cond(int N, Coord c) {
	// bc0 == c[0] && bc1 == c[1] && bc2 == c[2] && ...
	string str = "";
	for (int n=0; n<N; n++) {
		if (n > 0) str += " && ";
		str += string("bc")+n+" == "+c[n];
	}
	return str;
}

string zero_cond_spread(int N) {
	// i0 == 0 && i1 == 0 && i2 == 0 ...
	string str = "";
	for (int n=0; n<N; n++) {
		if (n > 0) str += " && ";
		str += string("i")+n + " == 0";
	}
	return str;
}

/*************
   Functions
 *************/

string defines_local() {
	string str =
	"""" "#define VAR(x) x, x ## v, x ## f"
	"\n" "#define TYPE_VAR(t,x) global t * x , t x ## v , uchar x ## f"
	"\n";
	return str;
}

string defines_local_type(DataType data_type) {
	string type = data_type.toString();
	string ctype = data_type.ctypeString();
	string str =
	"""" "#define LOAD_L_"+type+"(x) load_L_"+type+"(VAR(x),bc0,bc1,BS0,BS1)"
	"\n" +ctype+" load_L_"+type+"(TYPE_VAR("+ctype+",IN), int bc0, int bc1, int BS0, int BS1) { return (INf) ?  INv : IN["+global_proj(2)+"]; }"
	"\n";
	return str;
}

string defines_diver_type(DataType data_type) {
	string type = data_type.ctypeString();
	string str =
	"""" "void addElem_"+type+"("+type+" e, local "+type+" *elemSA, local int *countSA, int *num) {"
	"\n" "	int _num = *num;"
	"\n" "	for (int i=0; i<_num; i++) {"
	"\n" "		if (e == elemSA[i]) {"
	"\n" "			countSA[i]++;"
	"\n" "			return;"
	"\n" "		}"
	"\n" "	}"
	"\n" "	elemSA[_num] = e;"
	"\n" "	countSA[_num] = 1;"
	"\n" "	(*num)++;"
	"\n" "}"
	"\n"
	"\n" ""+type+" majorElem_"+type+"(local "+type+" *elemSA, local int *countSA, int num) {"
	"\n" "	int major = 0;"
	"\n" "	for (int i=1; i<num; i++) {"
	"\n" "		if (countSA[i] > countSA[major]) {"
	"\n" "			major = i;"
	"\n" "		}"
	"\n" "	}"
	"\n" "	return elemSA[major];"
	"\n" "}"
	"\n"
	"\n" ""+type+" minorElem_"+type+"(local "+type+" *elemSA, local int *countSA, int num) {"
	"\n" "	int minor = 0;"
	"\n" "	for (int i=1; i<num; i++) {"
	"\n" "		if (countSA[i] < countSA[minor]) {"
	"\n" "			minor = i;"
	"\n" "		}"
	"\n" "	}"
	"\n" "	return elemSA[minor];"
	"\n" "}"
	"\n"
	"\n" ""+type+" meanElem_"+type+"(local "+type+" *elemSA, local int *countSA, int num) {"
	"\n" "	"+type+" mean = 0;"
	"\n" "	for (int i=0; i<num; i++)"
	"\n" "		mean += elemSA[i] * countSA[i];"
	"\n" "	return mean / num;"
	"\n" "}"
	"\n";
	return str;
}

string defines_focal() {
	string str =
	"""" "#define VARn(x,n) x ## _ ## n, x ## v ## _ ## n, x ## f ## _ ## n"
	"\n" "#define TYPE_VARn(t,x,n) global t * x ## _ ## n , t x ## v ## _ ## n , uchar x ## f ## _ ## n"
	"\n" "#define VAR_LIST(x) VARn(x,00), VARn(x,01), VARn(x,02), VARn(x,10), VAR(x), VARn(x,12), VARn(x,20), VARn(x,21), VARn(x,22)"
	"\n" "#define TYPE_VAR_LIST(t,x) TYPE_VARn(t,x,00), TYPE_VARn(t,x,01), TYPE_VARn(t,x,02), TYPE_VARn(t,x,10), TYPE_VAR(t,x), TYPE_VARn(t,x,12), TYPE_VARn(t,x,20), TYPE_VARn(t,x,21), TYPE_VARn(t,x,22)"
	"\n"
	"\n" "int modulo(int i, int d) { return (i<0) ? i+d : (i>=d) ? i-d : i; }"
	"\n" "int invert(int i, int d) { return (i<0) ? -i-1 : (i>=d) ? d-(i-d+1) : i; }"
	"\n";
	return str;
}

string defines_focal_type(DataType data_type) {
	string type = data_type.toString();
	string ctype = data_type.ctypeString();
	string str =
	"""" "#define LOAD_F_"+type+"(x) load_F_"+type+"(VAR_LIST(x),bc0,bc1,BS0,BS1)"
	"\n" +ctype+" load_F_"+type+"(TYPE_VAR_LIST("+ctype+",IN), int bc0, int bc1, int BS0, int BS1) { global const "+ctype+" *p = IN; "+ctype+" v = INv; uchar f = INf; int i0 = (bc0<0) ? 0 : (bc0>=BS0) ? 2 : 1; int i1 = (bc1<0) ? 0 : (bc1>=BS1) ? 2 : 1; if (i0 != 1 || i1 != 1) { if (i1 == 0) { /**/ if (i0 == 0) { p = IN_00; v = INv_00; f = INf_00; } else if (i0 == 1) { p = IN_01; v = INv_01; f = INf_01; } else if (i0 == 2) { p = IN_02; v = INv_02; f = INf_02; } } else if (i1 == 1) { /**/ if (i0 == 0) { p = IN_10; v = INv_10; f = INf_10; } else if (i0 == 2) { p = IN_12; v = INv_12; f = INf_12; } } else if (i1 == 2) { /**/ if (i0 == 0) { p = IN_20; v = INv_20; f = INf_20; } else if (i0 == 1) { p = IN_21; v = INv_21; f = INf_21; } else if (i0 == 2) { p = IN_22; v = INv_22; f = INf_22; } } if (!f && !p) { p = IN; v = INv; f = INf; bc0 = invert(bc0,BS0); bc1 = invert(bc1,BS1); } else if (!f) { bc0 = modulo(bc0,BS0); bc1 = modulo(bc1,BS1); } } return (f) ?  v : p["+global_proj(2)+"]; }"
	"\n";
	return str;
}

string defines_focal_flow() {
	string str =
	"""" "typedef enum {EAST,SOUTHEAST,SOUTH,SOUTHWEST,WEST,NORTHWEST,NORTH,NORTEAST,N_DIR} Dir;"
	"\n" "__constant const int offset0[N_DIR] = {2,2,1,0,0,0,1,2};"
	"\n" "__constant const int offset1[N_DIR] = {1,2,2,2,1,0,0,0};"
	"\n" "__constant unsigned int flowcod[N_DIR] = {1,2,4,8,16,32,64,128};"
	"\n";
	return str;
}

string defines_zonal_reduc(ReductionType rt, DataType dt) {
	string ftype = dt.ctypeString();
	string itype =  ( dt.is64() ? DataType(S64) : DataType(S32) ).ctypeString();
	string atomic = "atomic";
	string new_val, str = "";

	if (rt.isOperator())
		new_val = "old.f " + rt.code() + " val";
	else if (rt.isFunction())
		new_val = rt.code() + "(old.f,val)";
	else 
		assert(0);

	if (dt.is64()) {
		atomic = "atom";
		str +=
		"#pragma OPENCL EXTENSION cl_khr_fp64: enable\n"
		"#pragma OPENCL EXTENSION cl_khr_int64_base_atomics: enable\n";
	}

	str += "void atomic"+rt.toString()+"(global "+ftype+" *ptr, "+ftype+" val) { union {"+ftype+" f; "+itype+" i; } old, new; do { old.i = *(global "+itype+"*)ptr; new.f = "+new_val+"; } while ("+atomic+"_cmpxchg ( (volatile global "+itype+"*)ptr, old.i, new.i) != old.i); }";
	str += "\n";
	return str;
}

string defines_radial() {
	string str =
	"""" "#define VARn(x,n) x ## _ ## n, x ## v ## _ ## n, x ## f ## _ ## n"
	"\n" "#define TYPE_VARn(t,x,n) global t * x ## _ ## n , t x ## v ## _ ## n , uchar x ## f ## _ ## n"
	"\n" "#define VAR_LIST(x) VARn(x,10), VARn(x,01), VARn(x,00), x"
	"\n" "#define TYPE_VAR_LIST(t,x) TYPE_VARn(t,x,10), TYPE_VARn(t,x,01), TYPE_VARn(t,x,00), global t * x"
	"\n"
	"\n" "int iceil(int n, int d) { return (n + d - 1) / d; }"
	"\n" "int modulo(int i, int d) { return (i<0) ? i+d : (i>=d) ? i-d : i; }"
	"\n";
	return str;
}

string defines_radial_type(DataType data_type) {
	string type = data_type.toString();
	string ctype = data_type.ctypeString();
	string str =
	"""" "#define LOAD_R_"+type+"(x) load_R_"+type+"(VAR_LIST(x),bc0,bc1,BS0,BS1)"
	"\n" "#define LIT_"+type+"(a,b) linear_interpolation_"+type+"(a,b,dif0,dif1)"
	"\n"
	"\n" +ctype+" load_R_"+type+"(TYPE_VAR_LIST("+ctype+",IN), int bc0, int bc1, int BS0, int BS1) { global const "+ctype+" *p = IN; "+ctype+" v; uchar f = false; int i0 = (bc0<0) ? 0 : (bc0>=BS0) ? 0 : 1; int i1 = (bc1<0) ? 0 : (bc1>=BS1) ? 0 : 1; if (i0 != 1 || i1 != 1) { /**/ if (i0 == 0 && i1 == 1) { p = IN_10; v = INv_10; f = INf_10; } else if (i0 == 1 && i1 == 0) { p = IN_01; v = INv_01; f = INf_01; } else if (i0 == 0 && i1 == 0) { p = IN_00; v = INv_00; f = INf_00; } if (f) return v; if (!p) return NAN; bc0 = modulo(bc0,BS0), bc1 = modulo(bc1,BS1); } return p["+global_proj(2)+"]; }"
	"\n" +ctype+" linear_interpolation_"+type+"("+ctype+" a, "+ctype+" b, int dif0, int dif1) { if (dif1 == 0) return a; if (dif0 == dif1) return b; "+ctype+" w = dif1 / ("+ctype+") dif0 * (dif0 - 1.0f) - (dif1 - 1.0f); return a * w + b * (1.0f-w); }"
	"\n";
	return str;
}

string defines_radial_const(Direction fst, Direction snd) {
	string str = string() +
	"""" "	const bool fst_north = " + fst.north2str() + ";"
	"\n" "	const bool snd_north = " + snd.north2str() + ";"
	"\n" "	const bool fst_east = " + fst.east2str() + ";"
	"\n" "	const bool snd_east = " + snd.east2str() + ";"
	"\n" "	const bool fst_south = " + fst.south2str() + ";"
	"\n" "	const bool snd_south = " + snd.south2str() + ";"
	"\n" "	const bool fst_west = " + fst.west2str() + ";"
	"\n" "	const bool snd_west = " + snd.west2str() + ";"
	"\n"
	"\n" "	const int fst_dim0  = fst_east  || fst_west;"
	"\n" "	const int fst_dim1  = fst_south || fst_north;"
	"\n" "	const int snd_dim0  = snd_east  || snd_west;"
	"\n" "	const int snd_dim1  = snd_south || snd_north;"
	"\n" "	const int fst_unit0 = fst_east  -  fst_west;"
	"\n" "	const int fst_unit1 = fst_south -  fst_north;"
	"\n" "	const int snd_unit0 = snd_east  -  snd_west;"
	"\n" "	const int snd_unit1 = snd_south -  snd_north;"
	"\n";
	return str;
}

string defines_radial_idx() {
	string str =
	"""" "	const int limit0 = BS0 * fst_dim0 + BS1 * fst_dim1;"
	"\n" "	const int limit1 = iceil(BS0,GS) * snd_dim0 + iceil(BS1,GS) * snd_dim1;"
	"\n"
	"\n" "	for (int i1=0; i1<limit1; i1++) { // 2nd dim FOR"
	"\n" "		for (int i0=0; i0<limit0; i0++) // 1st dim FOR"
	"\n" "		{"
	"\n" "			const int gc = get_local_id(0);"
	"\n"
	"\n" "			const int bc0 = (gc+GS*i1) * snd_east + (BS0-1-gc-GS*i1) * snd_west +"
	"\n" "							(i0) * fst_east + (BS0-1-i0) * fst_west;"
	"\n" "			const int bc1 = (gc+GS*i1) * snd_south + (BS1-1-gc-GS*i1) * snd_north +"
	"\n" "							(i0) * fst_south + (BS1-1-i0) * fst_north;"
	"\n"
	"\n" "			const int dc0 = BC0*BS0 + bc0;"
	"\n" "			const int dc1 = BC1*BS1 + bc1;"
	"\n"
	"\n" "			const int dif0 = (dc0 - sc0) * fst_unit0 + (dc1 - sc1) * fst_unit1;"
	"\n" "			const int dif1 = (dc1 - sc1) * snd_unit1 + (dc0 - sc0) * snd_unit0;"
	"\n"
	"\n" "			const bool cond = " + global_cond_radial(2) + ";"
	"\n";
	return str;
}

string defines_spread() {
	string str =
	"""" "#define VAR(x,n) x ## _ ## n"
	"\n" "#define TYPE(t) global t *"
	"\n" "#define VAR_LIST(x) VAR(x,00), VAR(x,01), VAR(x,02), VAR(x,10), VAR(x,11), VAR(x,12), VAR(x,20), VAR(x,21), VAR(x,22)"
	"\n" "#define TYPE_VAR_LIST(x,t) TYPE(t) VAR(x,00), TYPE(t) VAR(x,01), TYPE(t) VAR(x,02), TYPE(t) VAR(x,10), TYPE(t) VAR(x,11), TYPE(t) VAR(x,12), TYPE(t) VAR(x,20), TYPE(t) VAR(x,21), TYPE(t) VAR(x,22)"
	"\n"
	"\n" "int modulo(int i, int d) { return (i<0) ? i+d : (i>=d) ? i-d : i; }"
	"\n" "int invert(int i, int d) { return (i<0) ? -i-1 : (i>=d) ? d-(i-d+1) : i; }"
	"\n"
	"\n" "uchar dircod(int i0, int i1) { uchar dir[3][3] = {{2,4,8},{1,0,16},{128,64,32}}; return dir[i1+1][i0+1]; };"
	"\n" "#define DIRCOD() dircod(i0,i1)"
	"\n";
	return str;
}

string defines_spread_type(DataType data_type) {
	string type = data_type.ctypeString();
	string str =
	"""" "#define LOAD_"+type+"(x) *access_"+type+"(VAR_LIST(x),bc0,bc1,BS0,BS1)"
	"\n" "#define LOAD_"+type+"_i(x) *access_"+type+"(VAR_LIST(x),bc0+i0,bc1+i1,BS0,BS1)"
	"\n" "#define STORE_"+type+"(x) *access_"+type+"(VAR_LIST(x),bc0,bc1,BS0,BS1)"
	"\n" "global "+type+"* access_"+type+"(TYPE_VAR_LIST(IN,"+type+"), int bc0, int bc1, int BS0, int BS1) { global "+type+" *p = IN_11; int i0 = (bc0<0) ? 0 : (bc0>=BS0) ? 2 : 1; int i1 = (bc1<0) ? 0 : (bc1>=BS1) ? 2 : 1; if (i0 != 1 || i1 != 1) { if (i1 == 0) { if (i0 == 0) p = IN_00; else if (i0 == 1) p = IN_01; else if (i0 == 2) p = IN_02; } else if (i1 == 1) { if (i0 == 0) p = IN_10; else if (i0 == 2) p = IN_12; } else if (i1 == 2) { if (i0 == 0) p = IN_20; else if (i0 == 1) p = IN_21; else if (i0 == 2) p = IN_22; } if (p == 0x0) { p = IN_11; bc0 = invert(bc0,BS0); bc1 = invert(bc1,BS1); } else { bc0 = modulo(bc0,BS0); bc1 = modulo(bc1,BS1); } } return p + ("+global_proj(2)+"); }"
	"\n";
	return str;
}

} } // namespace map::detail
