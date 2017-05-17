/** 
 * @file    dag.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: the signature of Nodes, apart from unique, must be constant across compilations;
 *       this is why an id-number is a more appropiate solution than using the class address
 */

#include "dag.hpp"


namespace map { namespace detail {

#define DEFINE_NODESIGN(node,value) char node::classSignature() const { return value; }

DEFINE_NODESIGN( Constant , 1 )
DEFINE_NODESIGN( Index , 2 )
DEFINE_NODESIGN( Identity , 5 )
DEFINE_NODESIGN( Rand , 6 )
DEFINE_NODESIGN( Cast , 7 )
DEFINE_NODESIGN( Unary , 10 )
DEFINE_NODESIGN( Binary , 11 )
DEFINE_NODESIGN( Conditional , 12 )
DEFINE_NODESIGN( Diversity , 13 )
DEFINE_NODESIGN( Neighbor , 20 )
DEFINE_NODESIGN( BoundedNeighbor , 21 )
DEFINE_NODESIGN( SpreadNeighbor , 22 )
DEFINE_NODESIGN( Convolution , 23 )
DEFINE_NODESIGN( FocalFunc , 24 )
DEFINE_NODESIGN( FocalPercent , 25 )
DEFINE_NODESIGN( ZonalReduc , 30 ) 
DEFINE_NODESIGN( RadialScan , 35 )
DEFINE_NODESIGN( SpreadScan , 40 )
DEFINE_NODESIGN( LoopCond , 45 )
DEFINE_NODESIGN( LoopHead , 46 )
DEFINE_NODESIGN( LoopTail , 47 )
DEFINE_NODESIGN( Merge , 48 )
DEFINE_NODESIGN( Switch , 49 )
DEFINE_NODESIGN( Access , 50 )
DEFINE_NODESIGN( LhsAccess , 51 )
//DEFINE_NODESIGN( InputNode , 60 )
//DEFINE_NODESIGN( OutputNode , 61 )
DEFINE_NODESIGN( Read , 65 )
DEFINE_NODESIGN( Write , 66 )
DEFINE_NODESIGN( Scalar , 67 )
DEFINE_NODESIGN( Temporal, 68 )
DEFINE_NODESIGN( Checkpoint, 69 )
DEFINE_NODESIGN( Barrier, 80 )
DEFINE_NODESIGN( Summary, 81 )
DEFINE_NODESIGN( DataSummary, 82 )
DEFINE_NODESIGN( BlockSummary, 83 )
DEFINE_NODESIGN( GroupSummary, 84 )

#undef DEFINE_NODESIGN

} } // namespace map::detail
