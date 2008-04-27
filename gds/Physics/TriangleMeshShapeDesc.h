#ifndef  __PhysTriangleMeshShapeDesc_H__
#define  __PhysTriangleMeshShapeDesc_H__


#include "fwd.h"
#include "ShapeDesc.h"

#include "Support/Serialization/Serialization.h"
#include "Support/Serialization/ArchiveObjectFactory.h"
using namespace GameLib1::Serialization;

#include <assert.h>


namespace physics
{


class CTriangleMeshShapeDesc : public CShapeDesc
{
public:

	/// borrowed reference
	CTriangleMesh *pTriangleMesh;

	CTriangleMeshShapeDesc()
		:
	pTriangleMesh(NULL)
	{
	}

	virtual ~CTriangleMeshShapeDesc() {}

	unsigned int GetArchiveObjectID() const { return PhysShape::TriangleMesh; }

	virtual void Serialize( IArchive& ar, const unsigned int version )
	{
		CShapeDesc::Serialize( ar, version );

//#error <How do you serialize pTriangleMesh???>
		assert( !"How do you serialize pTriangleMesh???\n" );

	}
};


} // namespace physics


#endif  /*  __PhysTriangleMeshShapeDesc_H__  */
