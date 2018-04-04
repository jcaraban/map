##
# @file		map.py 
# @author	Jesus Carabano Bravo <jcaraban@abo.fi>
#
# Python interface/wrapper to Map Algebra Compiler _lib using ctypes
#
# Note: Variant/Array are passed by value, but they are not POD in C++
#
# local  local_sum
# focal  f_or
#     gather gsum
#     spread ssum
# zonal  zsum
# radial r_and
# spread ssum
# time   tsum
# directional dsum
#
# TODO: move the 'C API' / 'compund functions' / 'loop assembler' to independent files
##

import ast
import inspect

import __builtin__
import operator

import math
import textwrap

import ctypes as ct

##

def load_libmap():
	try:
		return ct.CDLL("lib/libmap.so")
	except OSError:
		pass
	try:
		return ct.CDLL("../lib/libmap.so")
	except OSError:
		pass
	try:
		return ct.CDLL("/usr/local/lib/libmap.so")
	except:
		raise OSError("Failed to load shared library libmap")

_lib = load_libmap()

## Constants

pi = math.pi
inf = float('inf')

## Enumerates

DataTypeId = [ 'NONE_DATATYPE','F32','F64','B8','U8','U16','U32','U64','S8','S16','S32','S64','N_DATATYPE' ]
DataTypeVal = range(len(DataTypeId))

NumDimId = [ 'NONE_NUMDIM','TIME','D0','D1','D2','D3','N_NUMDIM' ]
NumDimVal = [ 0x00,	0x01, 0x02, 0x04, 0x06, 0x08, 0x0A]

MemOrderId = [ 'NONE_MEMORDER','BLK','ROW','COL','SFC','N_MEMORDER' ]
MemOrderVal = [ 0x00, 0x01, 0x02, 0x04, 0x06, 0x08 ]

DeviceTypeId = [ 'DEV_DEF','DEV_CPU','DEV_GPU','DEV_ACC','DEV_CUS','DEV_ALL' ]
DeviceTypeVal = [ 0x01, 0x02, 0x04, 0x08, 0x16, 0xFFFFFFFF ]

UnaryTypeId = [
	'NONE_UNARY','POS','NEG','NOT','bNOT','MARK_UNARY',
	'SIN','COS','TAN','ASIN','ACOS','ATAN',
	'SINH','COSH','TANH','ASINH','ACOSH','ATANH',
	'EXP','EXP2','EXP10','LOG','LOG2','LOG10',
	'SQRT','CBRT','ABS',
	'CEIL','FLOOR','TRUNC','ROUND','N_UNARY'
]
UnaryTypeSym = [
	'','__pos__','__neg__','__not__','__invert__','',
	'sin','cos','tan','asin','acos','atan',
	'sinh','cosh','tanh','asinh','acosh','atanh',
	'exp','exp2','exp10','log','log2','log10',
	'sqrt','cbrt','abs',
	'ceil','floor','trunc','round',''
]
UnaryTypeVal = range(len(UnaryTypeId))

BinaryTypeId = [
	'NONE_BINARY','ADD','SUB','MUL','DIV','MOD',
	'EQ','NE','LT','GT','LE','GE','AND','OR',
	'bAND','bOR','bXOR','SHL','SHR','MARK_BINARY',
	'MAX2','MIN2','ATAN2','POW','HYPOT','FMOD','N_BINARY'
]
BinaryTypeSym = [
	'','__add__','__sub__','__mul__','__div__','__mod__',
	'__eq__','__ne__','__lt__','__gt__','__le__','__ge__','','',
	'__and__','__or__','__xor__','__lshift__','__rshift__','',
	'max','min','atan2','pow','hypot','fmod',''
]
BinaryTypeVal = range(len(BinaryTypeId))

ReductionTypeId = [ 'NONE_REDUCTION','SUM','PROD','rAND','rOR','MARK_REDUCTION','MAX','MIN','N_REDUCTION' ]
ReductionTypeVal = range(len(ReductionTypeId))

EnumIds = [ DataTypeId, NumDimId, MemOrderId, DeviceTypeId, UnaryTypeId, BinaryTypeId, ReductionTypeId ]
EnumVals = [ DataTypeVal, NumDimVal, MemOrderVal, DeviceTypeVal, UnaryTypeVal, BinaryTypeVal, ReductionTypeVal ]

for ids, vals in zip(EnumIds,EnumVals):
	for i,v in zip(ids,vals):
		globals()[i] = v

## Wrapper classes

class Variant(ct.Structure):
	class Union(ct.Union):
		_fields_ = [
			('b8' ,ct.c_bool),
			('s8' ,ct.c_int8),
			('s16',ct.c_int16),
			('s32',ct.c_int32),
			('s64',ct.c_int64),
			('u8' ,ct.c_uint8),
			('u16',ct.c_uint16),
			('u32',ct.c_uint32),
			('u64',ct.c_uint64),
			('f16',ct.c_float),
			('f32',ct.c_float),
			('f64',ct.c_double)
		]
	# class Union
	_fields_ = [('union',Union),
				('dtype',ct.c_int)]

	def __init__(self,val):
		union = self.Union()
		dtype = ct.c_int()
		if isinstance(val,float):
			union.f32 = ct.c_float(val)
			dtype = F32
		elif isinstance(val,bool):
			union.b8 = ct.c_bool(val)
			dtype = B8
		elif isinstance(val,int):
			union.s32 = ct.c_int32(val)
			dtype = S32
		elif isinstance(val,ct.c_float):
			union.f32,dtype = val,F32
		elif isinstance(val,ct.c_double):
			union.f64,dtype = val,F64
		elif isinstance(val,ct.c_bool):
			union.b8,dtype = val,B8
		elif isinstance(val,ct.c_uint8):
			union.u8,dtype = val,U8
		elif isinstance(val,ct.c_uint16):
			union.u16,dtype = val,U16
		elif isinstance(val,ct.c_uint32):
			union.u32,dtype = val,U32
		elif isinstance(val,ct.c_uint64):
			union.u64,dtype = val,U64
		elif isinstance(val,ct.c_int8):
			union.s8,dtype = val,S8
		elif isinstance(val,ct.c_int16):
			union.s16,dtype = val,S16
		elif isinstance(val,ct.c_int32):
			union.s32,dtype = val,S32
		elif isinstance(val,ct.c_int64):
			union.s64,dtype = val,S64
		else:
		    assert(0)
		super(Variant,self).__init__(union,dtype)

	def datatype(self):
		return self.dtype

	def value(self):
		if self.dtype is F32:
			return self.union.f32
		if self.dtype is F64:
			return self.union.f64
		elif self.dtype is B8:
			return self.union.b8
		elif self.dtype is U8:
			return self.union.u8
		elif self.dtype is U16:
			return self.union.u16
		elif self.dtype is U32:
			return self.union.u32
		elif self.dtype is U64:
			return self.union.u64
		elif self.dtype is S8:
			return self.union.s8
		elif self.dtype is S16:
			return self.union.s16
		elif self.dtype is S32:
			return self.union.s32
		elif self.dtype is S64:
			return self.union.s64
		else:
			assert(0)

	def convert(self,new_dt):
		if new_dt is F32:
			return Variant( ct.c_float(self.value()) )
		if new_dt is F64:
			return Variant( ct.c_double(self.value()) )
		elif new_dt is B8:
			return Variant( ct.c_bool(self.value()) )
		elif new_dt is U8:
			return Variant( ct.c_uint8(self.value()) )
		elif new_dt is U16:
			return Variant( ct.c_uint16(self.value()) )
		elif new_dt is U32:
			return Variant( ct.c_uint32(self.value()) )
		elif new_dt is U64:
			return Variant( ct.c_uint64(self.value()) )
		elif new_dt is S8:
			return Variant( ct.c_int8(self.value()) )
		elif new_dt is S16:
			return Variant( ct.c_int16(self.value()) )
		elif new_dt is S32:
			return Variant( ct.c_int32(self.value()) )
		elif new_dt is S64:
			return Variant( ct.c_int64(self.value()) )
		else:
			assert(0)
# class Variant

class Array(ct.Structure):
	N = 2
	T = ct.c_int * N
	_fields_ = [('arr',T),
				('n',ct.c_int)]

	def __init__(self,arg=[]):
		if not arg: # if list is empty
			super(Array,self).__init__(self.T(-1,-1),0)
		elif isinstance(arg,(list,tuple)):
			assert(len(arg) == self.N)
			super(Array,self).__init__(self.T(*arg),self.N)
		elif isinstance(arg,Array):
			super(Array,self).__init__(arg.arr,arg.n)
		else:
			assert(0)

	def isNone(self):
		return all(e==-1 for e in self.arr)

	def toList(self):
		return [x for x in self.arr if x != -1]
# class Array

class Node(ct.c_void_p): # empty class to make ctypes
	pass				 # return 'void*' instead of 'int'
# class Node

class Raster:
	#def __new__(cls,node):
		# could new be used to simplify nodes within python?

	def __init__(self,node=None):
		if isinstance(node,Node):
			self._node = node
			_lib.ma_increaseRef(self)
		elif node is None:
			self._node = Node(None)
		else:
			assert(0)

	def __del__(self):
		_lib.ma_decreaseRef(self)

	@classmethod
	def from_param(cls,ins): # Lets ctypes extract ._node from
		return ins._node     # Rasters when given as arguments

	def __getitem__(self,key):
		if isinstance(key,(list,tuple)): # coord rhs indexing
			return Raster( _lib.ma_access(self,Array(key)) )
		elif isinstance(key,Raster): # boolean rhs indexing
			return Raster( _lib.ma_conditional(key,self,empty_like(self)) )
		assert(0)

	def __setitem__(self,key,other):
		other = _const_to_raster(other).astype(self.datatype())
		if isinstance(key,(list,tuple)): # coord lhs indexing
			_lib.ma_decreaseRef(self)
			self._node = _lib.ma_lhsAccess(self,other,Array(key))
			_lib.ma_increaseRef(self)
		elif isinstance(key,Raster): # boolean lhs indexing
			_lib.ma_decreaseRef(self)
			self._node = _lib.ma_conditional(key,other,self)
			_lib.ma_increaseRef(self)
		else:
			assert(0)

	def __call__(self,ngb):
		if isinstance(ngb,list):
			return Raster( _lib.ma_neighbor(self,Array(ngb)) )
		elif isinstance(ngb,tuple):
			assert(len(ngb) == 2 and isinstance(ngb[0],Raster))
			return Raster( _lib.ma_boundedNbh(self,ngb[0],ngb[1]) )
		assert(0)
	## how to, left neighbor? __call__ with RedType? spreadSum? __setattr__?

	def __nonzero__(self):
		tree = self._symloop_extraction() # Find while, turn into AST
		if tree is None:
			return
		self._symloop_execution(tree) # call loop start/end, execute body
		return False # hackish way to hook into the Python internal system

	def _symloop_extraction(self):
		# Note: this code requires 'inspect' and probably only work with CPython
		(_, filename, lineno, _, _, _) = inspect.stack()[2]
		if filename == '<stdin>':
			print("Symbolic loops are not supported in interactive interpretation mode!")
			return None
		with open(filename) as f:
			whole = f.read()
		first = lineno - 1
		half = whole.split('\n')[first:]
		indent = self._indent_len(half[0])
		last = 1
		while self._within_loop(half,last,indent):
			last += 1
		exact = half[:last] # exact source with the loop
		source = "\n".join(exact)
		source = textwrap.dedent(source)
		tree = ast.parse(source)
		wile = ast.walk(tree).next().body[0]
		if not isinstance(wile,ast.While):
			print("__nonzero__ was called outside a while loop header!")
			return None
		if True:
			print source, '\n'
			print ast.dump(tree), '\n'
		return tree

	def _symloop_execution(self,tree):
		# Note: calling eval() twice is not re-entrant, recursion not possible!
		(frame, filename, lineno, _, _, _) = inspect.stack()[2]
		wile = ast.walk(tree).next().body[0] # while node
		test = wile.test # while's condition node
		body = wile.body # while's body (list of nodes)
		wrap_test = ast.Expression(test)
		wrap_body = ast.Module(body)
		self._fill_missing_lineno(wrap_test,lineno)
		self._fill_missing_lineno(wrap_body,lineno)
		code_test = compile(wrap_test,filename,'eval')
		code_body = compile(wrap_body,filename,'exec')
		global_scope = frame.f_globals
		local_scope  = frame.f_locals
		_lib.ma_loopStart()
		_lib.ma_loopCond(self)
		_lib.ma_loopBody()
		eval(code_body,global_scope,local_scope)
		expr = eval(code_test,global_scope,local_scope)
		_lib.ma_loopAgain()
		_lib.ma_loopCond(expr)
		eval(code_body,global_scope,local_scope)
		eval(code_test,global_scope,local_scope)
		loop = Raster( _lib.ma_loopAssemble() )
		self._update_variables(local_scope,wile,loop)
		_lib.ma_loopEnd()
		ct.pythonapi.PyFrame_LocalsToFast(ct.py_object(frame),ct.c_int(0))

	def _indent_len(self,line):
		return len(line) - len(line.lstrip())

	def _within_loop(self,half,last,indent):
		return (self._indent_len(half[last]) > indent or
				not half[last]) # i.e. line is empty

	def _fill_missing_lineno(self,tree,lineno):
		for node in ast.walk(tree):
			if hasattr(node,'lineno'):
				node.lineno += lineno-1

	def _update_variables(self,local_scope,wile,loop):
		oldpy = ct.POINTER(Node)()
		newpy = ct.POINTER(Node)()
		num = ct.c_int()
		_lib.ma_loopUpdateVars(loop,ct.byref(oldpy),ct.byref(newpy),ct.byref(num))
		# collects python variables holding rasters
		var_lst = []
		for node in ast.walk(wile):
			if isinstance(node,ast.Assign):
				assert(len(node.targets) == 1)
				target = node.targets[0]
				if isinstance(target,ast.Subscript):
					target = target.value
				assert(isinstance(target,ast.Name))
				var = target.id
				if var in local_scope:
					value = local_scope[var]
					if isinstance(value,Raster):
						var_lst.append(var)
		# update the python variables to hold the right nodes
		for var in var_lst:
			for i in range(num.value):
				if local_scope[var]._node.value == oldpy[i].value:
					local_scope[var] = Raster( newpy[i] )

	## Public methods

	def nodeid(self):
		return _lib.ma_nodeid(self)

	def nodename(self):
		return _lib.ma_longname(self)

	def streamdir(self):
		return _lib.ma_streamdir(self)

	def datatype(self):
		return _lib.ma_datatype(self)

	def numdim(self):
		return _lib.ma_numdim(self)

	def memorder(self):
		return _lib.ma_memorder(self)

	def datasize(self):
		return _lib.ma_datasize(self).toList()

	def blocksize(self):
		return _lib.ma_blocksize(self).toList()

	def groupsize(self):
		return _lib.ma_groupsize(self).toList()

	#def numblock(self):
	#	return _lib.ma_numblock(self).toList()

	def __pow__(self,other): # exceptional operator in python
		rhs = _const_to_raster(other) + 0.0
		return Raster( _lib.ma_binary(self,rhs,POW) )

	def __rpow__(self,other): # which is not an operator in C
		lhs = _const_to_raster(other) + 0.0
		return Raster( _lib.ma_binary(lhs,self,POW) )

	def no(self):
		return Raster( _lib.ma_unary(self,NOT) )

	def isinf():
		True

	def isnan():
		True

	def isnull():
		False # no_value ?

	def astype(self,dt):
		return astype(self,dt)

	def zsum(self,cond=None):
		return zsum(self,cond)
# class Raster

## Helper functions

def _const_to_raster(arg):
	if isinstance(arg,Raster):
		return arg
	elif isinstance(arg,(bool,int,float)):
		var = Variant(arg)
		ds  = Array()
		dt  = Variant(arg).datatype()
		mo  = ROW+BLK
		bs  = Array()
		gs  = Array()
		return Raster( _lib.ma_constant(var,ds,dt,mo,bs,gs) )
	else:
		assert(0)

## Unary functions

def _def_unary_op(symbol,utype):
	# e.g. __neg__
	def uop(self):
		return Raster( _lib.ma_unary(self,utype) )
	setattr(Raster,symbol,uop) # adds operator to Raster

def _def_unary_fun(symbol,utype):
	# e.g. sqrt()
	def ufun(raster):
		if isinstance(raster,Raster):
			return Raster( _lib.ma_unary(raster,utype) )
		else: # if not a raster, calls math.func()
			return getattr(math,symbol)(raster)
	globals()[symbol] = ufun # adds function to module

for sym,ut in zip(UnaryTypeSym,UnaryTypeVal):
	if sym and ut < MARK_UNARY:
		_def_unary_op(sym,ut)
	elif sym and ut > MARK_UNARY:
		_def_unary_fun(sym,ut)

## Binary functions

def _def_binary_op(symbol,bitype):
	# e.g. __add__
	def biop(self,rhs):
		rhs = _const_to_raster(rhs)
		return Raster( _lib.ma_binary(self,rhs,bitype) )
	setattr(Raster,symbol,biop) # adds operator to Raster
	# e.g. __radd__
	def rbiop(self,lhs):
		lhs = _const_to_raster(lhs)
		return Raster( _lib.ma_binary(lhs,self,bitype) )
	rsymbol = symbol[:2]+'r'+symbol[2:]
	setattr(Raster,rsymbol,rbiop) # adds right operator

def _def_binary_fun(symbol,bitype):
	# e.g. atan2()
	def bifun(lhs,rhs):
		if not isinstance(lhs,Raster) and not isinstance(rhs,Raster):
			return getattr(math,symbol)(lhs,rhs)
		else: # if not a raster, calls math.func()
			lhs = _const_to_raster(lhs)
			rhs = _const_to_raster(rhs)
			return Raster( _lib.ma_binary(lhs,rhs,bitype) )
	globals()[symbol] = bifun # adds function to module

for sym,bt in zip(BinaryTypeSym,BinaryTypeVal):
	if sym and bt < MARK_BINARY:
		_def_binary_op(sym,bt)
	elif sym and bt > MARK_BINARY:
		_def_binary_fun(sym,bt)

## Standalone functions

def setupDevices(plat_name,dev_type,dev_name):
	_lib.ma_setupDevices(plat_name,dev_type,dev_name)

def setNumRanks(ranks):
	_lib.ma_setNumRanks(ranks)

def eval(*args):
	## Note: shadowing built-in functions is considered herecy
	cond = [isinstance(a,Raster) for a in args]
	if len(args) and not all(cond):
		if not any(cond):
			return __builtin__.eval(*args) ## calls built-in eval
		assert(0) ## a partial number of Rasters is not accepted
	# if all arguments are a Raster, forward them to _lib.ma_
	num = len(args)
	NodeArr = Node * num # C array of Nodes
	nodearr = NodeArr(*[r._node for r in args])
	nodeptr = ct.cast(nodearr, ct.POINTER(Node))
	_lib.ma_eval(nodeptr,num)

def stats(raster,min=None,max=None,mean=None,std=None):
	if all(n is None for n in [min,max,mean,std]):
		min = Raster( _lib.ma_blockStats(raster,MIN) )
		max = Raster( _lib.ma_blockStats(raster,MAX) )
		mean = Raster() # @
		std = Raster() # @
	return Raster( _lib.ma_stats(raster,min,max,mean,std) );

def barrier(arg):
	return Raster( _lib.ma_barrier(arg) )

def identity(arg):
	return Raster( _lib.ma_identity(arg) )

def value(arg):
	var = _lib.ma_value(arg)
	return var.value()

def read(file):
	return Raster( _lib.ma_read(file) )

def write(raster,file):
	return _lib.ma_write(raster,file)

def _cnst(cnst,ds,dt,mo,bs,gs):
	var = Variant(cnst).convert(dt)
	return Raster( _lib.ma_constant(var,Array(ds),dt,mo,Array(bs),Array(gs)) )

def zeros(ds=[],dt=F32,mo=ROW+BLK,bs=[],gs=[]):
	return _cnst(0,ds,dt,mo,bs,gs)

def ones(ds=[],dt=F32,mo=ROW+BLK,bs=[],gs=[]):
	return _cnst(1,ds,dt,mo,bs,gs)

def trues(ds=[],dt=B8,mo=ROW+BLK,bs=[],gs=[]):
	return _cnst(True,ds,dt,mo,bs,gs)

def falses(ds=[],dt=B8,mo=ROW+BLK,bs=[],gs=[]):
	return _cnst(False,ds,dt,mo,bs,gs)

def full(cnst,ds=[],dt=F32,mo=ROW+BLK,bs=[],gs=[]):
	return _cnst(cnst,ds,dt,mo,bs,gs)

def empty(ds=[],dt=F32,mo=ROW+BLK,bs=[],gs=[]):
	return True # return a Raster of no_value ?

def _like(cnst,raster,dt,mo):
	dt  = raster.datatype() if (dt == NONE_DATATYPE) else dt
	mo  = raster.memorder() if (mo == NONE_MEMORDER) else mo
	var = Variant(cnst).convert(dt)
	ds  = Array( raster.datasize() )
	bs  = Array( raster.blocksize() )
	gs  = Array( raster.groupsize() )
	return Raster( _lib.ma_constant(var,ds,dt,mo,bs,gs) )

def zeros_like(raster,dt=NONE_DATATYPE,mo=NONE_MEMORDER):
	return _like(0,raster,dt,mo)

def ones_like(raster,dt=NONE_DATATYPE,mo=NONE_MEMORDER):
	return _like(1,raster,dt,mo)

def full_like(cnst,raster,dt=NONE_DATATYPE,mo=NONE_MEMORDER):
	return _like(cnst,raster,dt,mo)

def empty_like(raster,dt=NONE_DATATYPE,mo=NONE_MEMORDER):
	return True # no_value_like?

def index(raster,dim):
	assert(raster.numdim() > D0)
	ds = Array( raster.datasize() )
	mo = raster.memorder()
	bs = Array( raster.blocksize() )
	gs = Array( raster.groupsize() )
	return Raster( _lib.ma_index(ds,dim,mo,bs,gs) )

def rand(seed,ds=[],dt=NONE_DATATYPE ,mo=NONE_MEMORDER,bs=[],gs=[]):
	ds = seed.datasize() if isinstance(seed,Raster) else ds
	dt = seed.datatype() if dt==NONE_DATATYPE else dt
	mo = BLK+ROW if mo==NONE_DATATYPE else mo
	bs = seed.blocksize() if isinstance(seed,Raster) else bs
	gs = seed.groupsize() if isinstance(seed,Raster) else gs
	if not isinstance(seed,Raster):
		var = Variant(seed).convert(dt)
		seed = Raster( _lib.ma_constant(var,Array(ds),dt,mo,Array(bs),Array(gs)) )
	return Raster( _lib.ma_rand(seed,dt,mo) )

def astype(raster,dt):
	return Raster( _lib.ma_cast(raster,dt) )

def con(cond,lhs,rhs):
	cond = _const_to_raster(cond)
	lhs = _const_to_raster(lhs)
	rhs = _const_to_raster(rhs)
	return Raster( _lib.ma_conditional(cond,lhs,rhs) )

def convolve(raster,lst):
	assert( all(len(e)==len(lst) for e in lst) )
	ds = Array( [len(lst),len(lst)] )
	unroll = sum(lst,[])
	size = len(unroll)
	VarArr = Variant * size # C array of Variant
	vararr = VarArr(*[Variant(e) for e in unroll])
	varptr = ct.cast(vararr, ct.POINTER(Variant))
	return Raster( _lib.ma_convolution(raster,varptr,size,ds) )

def fsum(raster):
	lst = [[1,1,1],[1,1,1],[1,1,1]]
	return convolve(raster,lst)

def fmin(raster,lst):
	assert( all(len(e)==len(lst) for e in lst) )
	ds = Array( [len(lst),len(lst)] )
	unroll = sum(lst,[])
	size = len(unroll)
	VarArr = Variant * size # C array of Variant
	vararr = VarArr(*[Variant(e) for e in unroll])
	varptr = ct.cast(vararr, ct.POINTER(Variant))
	return Raster( _lib.ma_focalFunc(raster,varptr,size,ds,MIN) )

def zsum(raster,cond=None):
	if cond is not None:
		raster = con(cond,raster,zeros_like(raster))
	return Raster( _lib.ma_zonalReduc(raster,SUM) )

def zprod(raster,cond=None):
	if cond is not None:
		raster = con(cond,raster,ones_like(raster))
	return Raster( _lib.ma_zonalReduc(raster,PROD) )

def zor(raster,cond=None):
	if cond is not None:
		raster = con(cond,raster,zeros_like(raster))
	return Raster( _lib.ma_zonalReduc(raster,rOR) )

def zand(raster,cond=None):
	if cond is not None:
		raster = con(cond,raster,ones_like(raster))
	return Raster( _lib.ma_zonalReduc(raster,rAND) )

def zmax(raster):
	#if cond is not None:
	#	raster = con(cond,raster,full_like(-inf,raster))
	return Raster( _lib.ma_zonalReduc(raster,MAX) )

def zmin(raster):
	#if cond is not None:
	#	raster = con(cond,raster,full_like(-inf,raster))
	return Raster( _lib.ma_zonalReduc(raster,MIN) )

def rsum(raster,coord):
	return Raster( _lib.ma_radialScan(raster,SUM,Array(coord)) )

def rprod(raster,coord):
	return Raster( _lib.ma_radialScan(raster,PROD,Array(coord)) )

def rmax(raster,coord):
	return Raster( _lib.ma_radialScan(raster,MAX,Array(coord)) )

def rmin(raster,coord):
	return Raster( _lib.ma_radialScan(raster,MIN,Array(coord)) )

## Compound functions

def bor(lhs,rhs): # @
	return Raster( _lib.ma_binary(lhs,rhs,OR))

def band(lhs,rhs): # @
	return Raster( _lib.ma_binary(lhs,rhs,AND))

def lmax(lst):
	accu = lst[0]
	for i in range(1,len(lst)):
		accu = max(accu,lst[i])
	return accu

def lmin(lst):
	accu = lst[0]
	for i in range(1,len(lst)):
		accu = min(accu,lst[i])
	return accu

def lmaxpos(lst):
	maxim = lst[0]
	index = zeros_like(maxim,U8)
	for i in range(1,len(lst)):
		index[lst[i] > maxim] = full_like(i,index)
		maxim = max(maxim,lst[i])
	return index

def lminpos(lst):
	minim = lst[0]
	index = zeros_like(minim,U8)
	for i in range(1,len(lst)):
		index[lst[i] < minim] = full_like(i,index)
		minim = min(minim,lst[i])
	return index

def lor(lst):
	accu = lst[0]
	for i in range(1,len(lst)):
		accu = accu + lst[i]
	return accu

def pick(raster,lst):
	assert lst, raster
	acu = _const_to_raster(lst[0])
	for i in range(len(lst)):
		acu = con(raster==i,lst[i],acu)
	return acu;

def degrees(raster):
	return raster / (2*PI) * 360

def radians(raster):
	return raster / 360 * (2*PI)

def distance(raster,coord):
	x = coord[0] - index(data,D1)
	y = coord[1] - index(data,D2)
	return sqrt(x**2 + y**2 + 0.0)

def bifilt(raster,d,r): # Bilinear Filter	
	accu = norm = 0
	i = j = 0
	for k in range(3):
		for l in range(3):
			wd = ((i-k)**2 + (j-l)**2) / (2*d**2)
			wr = (raster**2 + raster([k,l])**2) / (2*r**2)
			w  = exp(-wd-wr)
			accu += raster([k,l]) * w
			norm += w
	return accu / norm

def regions(raster):
	NBH = zip([-1,0,1,-1],[-1,-1,-1,0])
	change = ones_like(raster)
	idx1 = index(c,D1)
	idx2 = index(c,D2)
	label = idx1 + idx2*c.datasize()[0]
	while zor(change):
		for ngb in NBH:
			change = raster == raster(ngb) and label > label(ngb)
			label[cond] = min(label,label(ngb))
	return label

def mean(raster):
	return zsum(raster) / prod(raster.datasize())

def covariance(x,y):
	assert(x.datasize() == y.datasize())
	xydif = (x - mean(x)) * (y - mean(y))
	return zsum(xydif) / prod(x.datasize())

def gradient(raster):
	kx = [ [-1, 0, 1], [-1, 0, 1], [-1, 0, 1] ]
	ky = [ [-1,-1,-1], [ 0, 0, 0], [ 1, 1, 1] ]
	ix = convolve(raster,kx)
	iy = convolve(raster,ky)
	return ix, iy

def blur(raster,size=3):
	if size==3:
		krnl = [ [1, 2, 1], [2, 4, 2], [1, 2, 1] ]
		norm = 16 # sum of weights
	elif size==5:
		krnl = [ [1,  4,  7,  4, 1],
				 [4, 16, 26, 16, 4],
				 [7, 26, 41, 26, 7],
				 [4, 16, 26, 16, 4],
				 [1,  4,  7,  4, 1] ]
		norm = 276 # sum of weights
	else:
		assert(0)
	return convolve(raster,krnl) / norm

def harris(raster):
	ix, iy = gradient(raster)
	xx, yy, xy = blur(ix*ix), blur(iy*iy), blur(ix*iy)
	return (xx*yy - xy) - 0.04*(xx + yy)**2

## Helper functions

def prod(arg):
	if isinstance(arg,(list,tuple)):
		return reduce(operator.mul,arg,1)
	elif isinstance(arg,tuple):
		return arg[0]*arg[1]
	elif isinstance(arg,Array):
		return arg.arr[0]*arg.arr[1]
	else:
		assert(0)

## Error functions

def err_write(result,func,args):
	if result is not 0:
		assert(0)

## CDLL interface: arguments & returns

_lib.ma_setupDevices.argtypes = [ct.c_char_p, ct.c_int, ct.c_char_p]
_lib.ma_setupDevices.restype = None

_lib.ma_setNumRanks.argtypes = [ct.c_int]
_lib.ma_setNumRanks.restype = None

_lib.ma_increaseRef.argtypes = [Raster]
_lib.ma_increaseRef.restype = None

_lib.ma_decreaseRef.argtypes = [Raster]
_lib.ma_decreaseRef.restype = None

_lib.ma_eval.argtypes = [ct.POINTER(Node), ct.c_int]
_lib.ma_eval.restype = None

#_lib.ma_nodename.argtypes = [Raster]
#_lib.ma_nodename.restype = ct.c_char_p

_lib.ma_nodeid.argtypes = [Raster]
_lib.ma_nodeid.restype = ct.c_int

_lib.ma_value.argtypes = [Raster]
_lib.ma_value.restype = Variant

_lib.ma_datatype.argtypes = [Raster]
_lib.ma_datatype.restype = ct.c_int

_lib.ma_numdim.argtypes = [Raster]
_lib.ma_numdim.restype = ct.c_int

_lib.ma_memorder.argtypes = [Raster]
_lib.ma_memorder.restype = ct.c_int

_lib.ma_datasize.argtypes = [Raster]
_lib.ma_datasize.restype = Array

_lib.ma_blocksize.argtypes = [Raster]
_lib.ma_blocksize.restype = Array

_lib.ma_groupsize.argtypes = [Raster]
_lib.ma_groupsize.restype = Array

_lib.ma_read.argtypes = [ct.c_char_p]
_lib.ma_read.restype = Node

_lib.ma_write.argtypes = [Raster, ct.c_char_p]
_lib.ma_write.restype = ct.c_int
_lib.ma_write.errcheck = err_write

_lib.ma_index.argtypes = [Array, ct.c_int, ct.c_int, Array, Array]
_lib.ma_index.restype = Node

_lib.ma_rand.argtypes = [Raster, ct.c_int, ct.c_int]
_lib.ma_rand.restype = Node

_lib.ma_cast.argtypes = [Raster, ct.c_int]
_lib.ma_cast.restype = Node

_lib.ma_conditional.argtypes = [Raster, Raster, Raster]
_lib.ma_conditional.restype = Node

_lib.ma_unary.argtypes = [Raster, ct.c_int]
_lib.ma_unary.restype = Node

_lib.ma_binary.argtypes = [Raster, Raster, ct.c_int]
_lib.ma_binary.restype = Node

_lib.ma_constant.argtypes = [Variant, Array, ct.c_int, ct.c_int, Array, Array]
_lib.ma_constant.restype = Node

_lib.ma_access.argtypes = [Raster, Array]
_lib.ma_access.restype = Node

_lib.ma_lhsAccess.argtypes = [Raster, Raster, Array]
_lib.ma_lhsAccess.restype = Node

_lib.ma_neighbor.argtypes = [Raster, Array]
_lib.ma_neighbor.restype = Node

_lib.ma_boundedNbh.argtypes = [Raster, Raster, Raster]
_lib.ma_boundedNbh.restype = Node

_lib.ma_convolution.argtypes = [Raster, ct.POINTER(Variant), ct.c_int, Array]
_lib.ma_convolution.restype = Node

_lib.ma_focalFunc.argtypes = [Raster, ct.POINTER(Variant), ct.c_int, Array, ct.c_int]
_lib.ma_focalFunc.restype = Node

_lib.ma_zonalReduc.argtypes = [Raster, ct.c_int]
_lib.ma_zonalReduc.restype = Node

_lib.ma_radialScan.argtypes = [Raster, ct.c_int, Array]
_lib.ma_radialScan.restype = Node

_lib.ma_stats.argtypes = [Raster, Raster, Raster, Raster, Raster]
_lib.ma_stats.restype = Node

_lib.ma_blockStats.argtypes = [Raster, ct.c_int]
_lib.ma_blockStats.restype = Node

_lib.ma_barrier.argtypes = [Raster]
_lib.ma_barrier.restype = Node

_lib.ma_identity.argtypes = [Raster]
_lib.ma_identity.restype = Node

_lib.ma_loopStart.argtypes = []
_lib.ma_loopStart.restype = None

_lib.ma_loopCond.argtypes = [Raster]
_lib.ma_loopCond.restype = None

_lib.ma_loopBody.argtypes = []
_lib.ma_loopBody.restype = None

_lib.ma_loopAgain.argtypes = []
_lib.ma_loopAgain.restype = None

_lib.ma_loopAssemble.argtypes = []
_lib.ma_loopAssemble.restype = Node

_lib.ma_loopEnd.argtypes = []
_lib.ma_loopEnd.restype = None

_lib.ma_loopUpdateVars.argtypes = [Raster,ct.POINTER(ct.POINTER(Node)),ct.POINTER(ct.POINTER(Node)),ct.POINTER(ct.c_int)]
_lib.ma_loopUpdateVars.restype = None
