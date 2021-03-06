#ifndef  __CONVEXPOLYGON_H__
#define  __CONVEXPOLYGON_H__


#include "Vector3.hpp"
#include "Plane.hpp"
#include "AABB3.hpp"

#include <stdio.h>


namespace amorphous
{


#define EDGE_COLLISION_FLAG_OFFSET	16

class ConvexPolygon
{
	int m_iNumVertices;
	Vector3 *m_pavVertex;
	SPlane m_Plane;

	AABB3 m_AABB;

	// indicates which vertices and edges should be checked for collision (experimental)
	// higher 16 bits: for edges / lower 16 bits: for vertices
	unsigned int m_uiCollisionFlag;

public:

  int m_iPolygonIndex;

  ConvexPolygon();
  ConvexPolygon( Vector3 *pavVertex, int iNumVertices );

  ~ConvexPolygon();

  void SetVertices( Vector3 *pavVertex, int iNumVertices );

  void ComputeAABB();

  void SetNormal( Vector3& rvNormal );
  void SetDistance( Scalar fDist );
  void SetPlane( SPlane &plane ) { m_Plane = plane; }

  inline Vector3 GetNormal() { return m_Plane.normal; }
  inline Scalar GetDistance() { return m_Plane.dist; }
  inline SPlane& GetPlane() { return m_Plane; }
  inline AABB3& GetAABB() { return m_AABB; }

  inline int GetNumVertices() { return m_iNumVertices; }
  inline Vector3 GetVertex(int i) { return m_pavVertex[i]; }

  inline bool GetVertexCollisionFlag(int iIndex);
  inline bool GetEdgeCollisionFlag(int iIndex);

  Vector3 GetSupportPoint(const Vector3&) const;

	void LoadFromFile( FILE *fp );
	void WriteToFile( FILE *fp );


  // set flags for collision detection (experimental)
  void SetVertexCollisionFlag(int iIndex, bool bCheckCollision);
  void SetEdgeCollisionFlag(int iIndex, bool bCheckCollision);

};

} // namespace amorphous



#endif		/*  __CONVEXPOLYGON_H__  */