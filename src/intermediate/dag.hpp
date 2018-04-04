/** 
 * @file    dag.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Includes all nodes
 */

#ifndef MAP_RUNTIME_DAG_HPP_
#define MAP_RUNTIME_DAG_HPP_

#include "Node.hpp"
//#include "Cluster.hpp"
#include "dag/Constant.hpp"
#include "dag/Empty.hpp"
#include "dag/Index.hpp"
#include "dag/Identity.hpp"
#include "dag/Rand.hpp"
#include "dag/Cast.hpp"
#include "dag/Unary.hpp"
#include "dag/Binary.hpp"
#include "dag/Conditional.hpp"
#include "dag/Diversity.hpp" 
#include "dag/Neighbor.hpp"
#include "dag/BoundedNeighbor.hpp"
#include "dag/SpreadNeighbor.hpp"
#include "dag/Convolution.hpp"
#include "dag/FocalFunc.hpp"
#include "dag/FocalPercent.hpp"
#include "dag/ZonalReduc.hpp"
#include "dag/RadialScan.hpp"
#include "dag/SpreadScan.hpp"
#include "dag/LoopCond.hpp"
#include "dag/LoopHead.hpp"
#include "dag/LoopTail.hpp"
#include "dag/Merge.hpp"
#include "dag/Switch.hpp"
#include "dag/Access.hpp"
#include "dag/LhsAccess.hpp"
#include "dag/IO.hpp"
#include "dag/Read.hpp"
#include "dag/Write.hpp"
#include "dag/Scalar.hpp"
#include "dag/Temporal.hpp"
#include "dag/Checkpoint.hpp"
#include "dag/Barrier.hpp"
#include "dag/Summary.hpp"
#include "dag/DataSummary.hpp"
#include "dag/BlockSummary.hpp"
#include "dag/GroupSummary.hpp"

#endif
