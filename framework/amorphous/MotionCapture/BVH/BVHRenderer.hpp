#ifndef  __BVHRenderer_H__
#define  __BVHRenderer_H__


#include <vector>
#include <string>
#include <memory>
#include "amorphous/3DMath/Matrix34.hpp"
#include "amorphous/3DMath/Quaternion.hpp"
#include "amorphous/Graphics/fwd.hpp"
#include "amorphous/Graphics/Direct3D9.hpp"

#include "fwd.hpp"


namespace amorphous
{


class CPVC_JointHub;

class BVHRenderer
{
	SFloatRGBAColor m_SkeletonColor;	// shared by all the bones

	std::shared_ptr<CUnitCube> m_pUnitCube;

	std::shared_ptr<CD3DXMeshObject> m_pTestCube;

public:

	BVHRenderer();

	~BVHRenderer();

	void Draw_r( Vector3* pvPrevPosition = NULL, Matrix34* pParentMatrix = NULL );

	void SetSkeletonColor( const SFloatRGBAColor& color ) { m_SkeletonColor = color; }

	void DrawBoxForBone(D3DXMATRIX &rmatParent, D3DXMATRIX &rmatWorldTransform);
};
} // amorphous



#endif		/*  __BVHBONE_H__  */
