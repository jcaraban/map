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
DEFINE_NODESIGN( Rand , 2 )
DEFINE_NODESIGN( Index , 3 )
DEFINE_NODESIGN( Cast , 4 )
DEFINE_NODESIGN( Unary , 10 )
DEFINE_NODESIGN( Binary , 11 )
DEFINE_NODESIGN( Conditional , 12 )
DEFINE_NODESIGN( Diversity , 13 )
DEFINE_NODESIGN( Neighbor , 20 )
DEFINE_NODESIGN( BoundedNbh , 21 )
DEFINE_NODESIGN( SpreadNeighbor , 22 )
DEFINE_NODESIGN( Convolution , 23 )
DEFINE_NODESIGN( FocalFunc , 24 )
DEFINE_NODESIGN( FocalPercent , 25 )
DEFINE_NODESIGN( FocalFlow , 26 )
DEFINE_NODESIGN( ZonalReduc , 30 ) 
DEFINE_NODESIGN( RadialScan , 35 )
DEFINE_NODESIGN( SpreadScan , 40 )
DEFINE_NODESIGN( Loop , 45 )
DEFINE_NODESIGN( LoopCond , 46 )
DEFINE_NODESIGN( LoopHead , 47 )
DEFINE_NODESIGN( LoopTail , 48 )
DEFINE_NODESIGN( Feedback , 49 )
DEFINE_NODESIGN( Access , 50 )
DEFINE_NODESIGN( LhsAccess , 51 )
//DEFINE_NODESIGN( InputNode , 60 )
//DEFINE_NODESIGN( OutputNode , 61 )
DEFINE_NODESIGN( Read , 65 )
DEFINE_NODESIGN( Write , 66 )
DEFINE_NODESIGN( Scalar , 67 )
DEFINE_NODESIGN( Temporal, 68 )
DEFINE_NODESIGN( Checkpoint, 69 )
DEFINE_NODESIGN( Stats, 80 )
DEFINE_NODESIGN( Barrier, 81 )

#undef DEFINE_NODESIGN

} } // namespace map::detail
