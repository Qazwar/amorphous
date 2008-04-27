#ifndef  __NxPhysShape_H__
#define  __NxPhysShape_H__


#include "fwd.h"
#include "../Shape.h"
#include "NxPhysConv.h"

#include "NxPhysics.h"


namespace physics
{


class CNxPhysBoxShape : public CBoxShape
{
	NxBoxShape *m_pBox;

public:

//	CNxPhysBoxShape() {}
	CNxPhysBoxShape( NxBoxShape *pBox ) : m_pBox(pBox) {}

	virtual ~CNxPhysBoxShape() {}

	virtual int GetMaterialID() const { return (int)m_pBox->getMaterial(); }

	virtual bool Raycast ( const CRay &world_ray, Scalar max_dist, U32 hintFlags, CRaycastHit &hit, bool first_hit ) const;

//	virtual void  SetCollisionGroup ( int group ) { m_pBox->setGroup( group ); }

//	virtual int GetCollisionGroup() const { return (int)m_pBox->getGroup(); }

	/// returns radii of the box
	virtual Vector3 GetDimensions () const { return ToVector3( m_pBox->getDimensions() ); }
};


bool CNxPhysBoxShape::Raycast( const CRay &world_ray,
							   Scalar max_dist, U32 hint_flags,
							   CRaycastHit &hit,
							   bool first_hit ) const
{
	return m_pBox->raycast( ToNxRay(world_ray), max_dist, hint_flags, ToNxRaycastHit(hit), first_hit );
}


class CNxPhysSphereShape : public CSphereShape
{
	NxSphereShape *m_pSphere;

public:

//	CNxPhysSphereShape() {}
	CNxPhysSphereShape( NxSphereShape *pSphere ) : m_pSphere(pSphere) {}

	virtual ~CNxPhysSphereShape() {}

	virtual bool Raycast ( const CRay &world_ray, Scalar max_dist, U32 hintFlags, CRaycastHit &hit, bool first_hit ) const;

	virtual Scalar GetRadius () const { return (Scalar)(m_pSphere->getRadius()); }

	virtual void SetRadius( Scalar radius ) { m_pSphere->setRadius( radius ); }
};


bool CNxPhysSphereShape::Raycast( const CRay &world_ray, Scalar max_dist, U32 hint_flags, CRaycastHit &hit, bool first_hit ) const
{
	return m_pSphere->raycast( ToNxRay(world_ray), max_dist, hint_flags, ToNxRaycastHit(hit), first_hit );
}


class CNxPhysCapsuleShape : public CCapsuleShape
{
	NxCapsuleShape *m_pCapsule;

public:

//	CNxPhysCapsuleShape() {}
	CNxPhysCapsuleShape( NxCapsuleShape *pCapsule ) : m_pCapsule(pCapsule) {}

	virtual ~CNxPhysCapsuleShape() {}

	virtual bool Raycast ( const CRay &world_ray, Scalar max_dist, U32 hintFlags, CRaycastHit &hit, bool first_hit ) const;

	virtual Scalar GetRadius () const { return m_pCapsule->getRadius(); }
	virtual Scalar GetLength () const { return m_pCapsule->getHeight(); }

	virtual void SetRadius( Scalar radius ) { m_pCapsule->setRadius( radius ); }
	virtual void SetLength( Scalar length ) { m_pCapsule->setHeight( length ); }
};


bool CNxPhysCapsuleShape::Raycast ( const CRay &world_ray,
										   Scalar max_dist,
										   U32 hint_flags, 
										   CRaycastHit &hit, bool first_hit ) const
{
	return m_pCapsule->raycast( ToNxRay(world_ray), max_dist, hint_flags, ToNxRaycastHit(hit), first_hit );
}



class CNxPhysTriangleMeshShape : public CTriangleMeshShape
{
	NxTriangleMeshShape *m_pTriangleMesh;

public:

	CNxPhysTriangleMeshShape( NxTriangleMeshShape *pTriangleMesh )
		:
	m_pTriangleMesh(pTriangleMesh) {}

	virtual ~CNxPhysTriangleMeshShape() {}

	virtual bool Raycast ( const CRay &world_ray, Scalar max_dist, U32 hintFlags, CRaycastHit &hit, bool first_hit ) const;
};


bool CNxPhysTriangleMeshShape::Raycast ( const CRay &world_ray,
										   Scalar max_dist,
										   U32 hint_flags, 
										   CRaycastHit &hit, bool first_hit ) const
{
	return m_pTriangleMesh->raycast( ToNxRay(world_ray), max_dist, hint_flags, ToNxRaycastHit(hit), first_hit );
}


} // namespace physics


#endif /* __NxPhysShape_H__ */
