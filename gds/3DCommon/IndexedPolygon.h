#ifndef __INDEXEDPOLYGON_H__
#define __INDEXEDPOLYGON_H__

#include <vector>
#include "3DMath/Plane.h"
#include "3DMath/aabb3.h"
#include "3DCommon/General3DVertex.h"

#include "Support/Serialization/Serialization.h"
#include "Support/Serialization/Serialization_3DMath.h"
using namespace GameLib1::Serialization;


class CIndexedPolygon : public IArchiveObjectBase
{
	/// Pointer to the vertex buffer (borrowed reference).
	/// Every instance of indexed polygon use this same vertex
	/// in this vertex buffer when one needs to access vertex data.
	/// e.g.) when GetVertex() is called, the object will internally
	/// access this buffer and return the reference to the vertex
	static std::vector<CGeneral3DVertex>* ms_pVertex;

	SPlane m_Plane;

	/// axis-aligned bounding box that contains this polygon
	AABB3 m_AABB;

public:

	enum eOnPlaneCondition { OPC_DONTCARE_NORMAL_DIRECTION, OPC_IF_NORMAL_SAME_DIRECTION };

	/// indices to polygon vertices in the vertex buffer
	std::vector<int> m_index;

	/// surface property of the polygon
	int m_MaterialIndex;

public:

	inline CIndexedPolygon() : m_MaterialIndex(0) {}

	/// creates an indexed triangle
	inline CIndexedPolygon( int i0, int i1, int i2 );

	/// polygon must be convex
	inline void Split( CIndexedPolygon& front, CIndexedPolygon& back, const SPlane& plane );

	inline void Triangulate( std::vector<CIndexedPolygon>& dest_polygon_buffer ) const;

	const CGeneral3DVertex& GetVertex( int vert_index ) const { return (*ms_pVertex)[m_index[vert_index]]; }

	const CGeneral3DVertex& GetVertex( size_t vert_index ) const { return GetVertex( (int)vert_index ); }

	CGeneral3DVertex& Vertex( int vert_index ) { return (*ms_pVertex)[m_index[vert_index]]; }

	const SPlane& GetPlane() const { return m_Plane; }

	const AABB3& GetAABB() const { return m_AABB; }

	inline void UpdateAABB();

	inline void UpdatePlane();

	inline float CalculateArea() const;

	/// Added to use CIndexedPolygon with CAABTree
	/// - Not added for actual use
	/// - ms_pVertex would have to be separately serialized and restored
	inline void Serialize( IArchive& ar, const unsigned int version );

	static void SetVertexBuffer( std::vector<CGeneral3DVertex>* vertex_buffer ) { ms_pVertex = vertex_buffer; }

	static std::vector<CGeneral3DVertex>& VertexBuffer() { return (*ms_pVertex); }
};


//============================== inline implementations ==============================

#define NORMAL_EPSILON	0.001


enum ePolygonStatus { POLYGON_ONPLANE, POLYGON_FRONT, POLYGON_BACK, POLYGON_INTERSECTING };


inline CIndexedPolygon::CIndexedPolygon( int i0, int i1, int i2 )
{
	m_index.resize(3);
	m_index[0] = i0;
	m_index[1] = i1;
	m_index[2] = i2;
}


/**
 * OPC_DONTCARE_NORMAL_DIRECTION
 *	the face is marked as on-plane if it is on the plane without any regard to normal direction
 * OPC_IF_NORMAL_SAME_DIRECTION (default)
 *	the face is regarded as on-plane if its normal is in the same direction as the plane
 */
inline int	ClassifyPolygon( const SPlane& plane,
						     const CIndexedPolygon& polygon,
							 int iOnPlaneCondition = CIndexedPolygon::OPC_DONTCARE_NORMAL_DIRECTION )
{
	int front = 0;
	int back = 0;
	int iNumPnts = (int)polygon.m_index.size();

	for(int i=0; i<iNumPnts; i++)
	{
		switch( ClassifyPoint( plane, polygon.GetVertex(i).m_vPosition ) )
		{
		case PNT_FRONT:		front++;	break;
		case PNT_BACK:		back++;		break;
		case PNT_ONPLANE:	// ignore vertrices on the plane
		default:;
		}
	}

	if( front == 0 && back == 0 )
	{

//		if( iOnPlaneCondition == OPC_DONTCARE_NORMAL_DIRECTION )
			return POLYGON_ONPLANE;		//the face is on-plane regardless of its normal direction

/*		SPlane &plane2 = face.GetPlane();

		else if( fabs( plane.normal.x - plane2.normal.x ) < NORMAL_EPSILON 
		&&  fabs( plane.normal.y - plane2.normal.y ) < NORMAL_EPSILON 
		&&  fabs( plane.normal.z - plane2.normal.z ) < NORMAL_EPSILON )
		{
			return POLYGON_ONPLANE;	// the 2 normals face the same direction
		}
		else
		{	// Consider the vertex to be behind the palne if the plane and the polygon
			// face the opposite directions
			return POLYGON_BACK;	//the 2 normals face the opposite directions
		}*/
	}
	if( front != 0 && back != 0 )
		return POLYGON_INTERSECTING;
	if( front > 0 )
		return POLYGON_FRONT;
	return POLYGON_BACK;
}


inline void CIndexedPolygon::Split( CIndexedPolygon& front, CIndexedPolygon& back, const SPlane& plane )
{
	size_t i, num_orig_verts = m_index.size();  //the number of points of this face
//	SFloatRGBColor col, col0, col1;

	std::vector<CGeneral3DVertex>& vert_buffer = *ms_pVertex;

	// copy the properties of the source polygon. e.g.) material index
	front = *this;
	back = *this;

	// clear vertex indices
	front.m_index.resize(0);
	back.m_index.resize(0);

	//Initialization for 'front' and 'back': delete the vertices and copy the plane from the original face

	//check if each edge of this face and the 'cutplane' intersect
	for( i=0; i<num_orig_verts; i++)
	{
		const CGeneral3DVertex& mv0 = vert_buffer[m_index[i]];
		const CGeneral3DVertex& mv1 = vert_buffer[m_index[(i+1) % num_orig_verts]];
		const Vector3& p0 = mv0.m_vPosition;      /// pick up 2 vertices on the polygon face
		const Vector3& p1 = mv1.m_vPosition;      /// see if they are in front of, behind or crossing the plane 
		int c0 = ClassifyPoint( plane, p0 );
		int c1 = ClassifyPoint( plane, p1 );

		if( ( c0 == PNT_FRONT && c1 == PNT_BACK )
		||  ( c0 == PNT_BACK && c1 == PNT_FRONT ) )
		{
			// need to split the edge
			float d0 = plane.GetDistanceFromPoint( p0 );
			float d1 = plane.GetDistanceFromPoint( p1 );
			float f = - d0 / ( d1 - d0 );
//			Vector3 pn = p0 + ( p1 - p0 ) * f;  //the cross point between linesegment (p1 - p0) and cutplane

			// calculate properties for the new vertex at the cross point.
			// vertex color, tex coord, etc.
			CGeneral3DVertex new_vert;

			new_vert = mv0 + ( mv1 - mv0 ) * f;

			Vec3Normalize( new_vert.m_vNormal, new_vert.m_vNormal );

//			new_vert.OrthonormalizeLocalSpace();

			// push the new vertex to the buffer
			// note that 2 vertices are added for each split position
			int new_vert_index = (int)vert_buffer.size();
			vert_buffer.push_back( new_vert );
			vert_buffer.push_back( new_vert );

//			vert0_index = 
			if( c0 == PNT_FRONT )
				front.m_index.push_back( m_index[i] );  // add P0 vertex to CFace front
//				front.m_pVertices.push_back( mv0 );  // add P0 vertex to CFace front
			if( c0 == PNT_BACK )
				back.m_index.push_back( m_index[i] );
//				back.m_pVertices.push_back( mv0 );
			front.m_index.push_back( new_vert_index );
			back.m_index.push_back( new_vert_index + 1 );
//			front.m_pVertices.push_back(new_vert);
//			back.m_pVertices.push_back(new_vert);
			continue;
		}
		if( c0 == PNT_FRONT ) front.m_index.push_back( m_index[i] );	// front -> on
		if( c0 == PNT_BACK  ) back.m_index.push_back( m_index[i] );		// back -> on
		if( c0 == PNT_ONPLANE )
		{
			front.m_index.push_back( m_index[i] );
			back.m_index.push_back( m_index[i] );
		}
	}
}


inline void CIndexedPolygon::Triangulate( std::vector<CIndexedPolygon>& dest_polygon_buffer ) const
{
	if( m_index.size() <= 3 )
	{
		dest_polygon_buffer.push_back( *this );
		return;
	}

	size_t i, num_tris = m_index.size() - 2;
	for( i=0; i<num_tris; i++ )
	{
		dest_polygon_buffer.push_back(
			CIndexedPolygon( m_index[0], m_index[i+1], m_index[i+2] ) );
	}

}


inline void CIndexedPolygon::UpdateAABB()
{
	m_AABB.Nullify();

	const size_t num_vertices = m_index.size();
	for( size_t i=0; i<num_vertices; i++ )
	{
		m_AABB.AddPoint( GetVertex(i).m_vPosition );
	}
}


#include <assert.h>

inline void CIndexedPolygon::UpdatePlane()
{
	assert( !"Not implemented yet." );
}


inline float CIndexedPolygon::CalculateArea() const
{
	float fCos, fSin;
	Vector3 vEdge1, vEdge2;
	float fEdgeLen1, fEdgeLen2;
	float fArea = 0.0f;

	const size_t num_vertices = m_index.size();
	for( size_t i=1; i<num_vertices - 1; i++)
	{
		vEdge1 = GetVertex(i).m_vPosition   - GetVertex(0).m_vPosition;
		vEdge2 = GetVertex(i+1).m_vPosition - GetVertex(0).m_vPosition;
		fEdgeLen1 = Vec3Length(vEdge1);
		fEdgeLen2 = Vec3Length(vEdge2);
		vEdge1 = vEdge1 / fEdgeLen1;
		vEdge2 = vEdge2 / fEdgeLen2;
		fCos = Vec3Dot(vEdge1, vEdge2);
		fSin = sqrtf( 1.0f - fCos * fCos );

		fArea += fEdgeLen1 * fEdgeLen2 * fSin / 2.0f;
	}

	return fArea;
}


inline void Triangulate( std::vector<CIndexedPolygon>& dest_polygon_buffer,
						 const std::vector<CIndexedPolygon>& src_polygon_buffer )
{
	const size_t num_pols = src_polygon_buffer.size();
	for( size_t i=0; i<num_pols; i++ )
	{
		const CIndexedPolygon& src_polygon = src_polygon_buffer[i];
		const size_t num_triangles = src_polygon.m_index.size() - 2;

		for( size_t j=0; j<num_triangles; j++ )
		{
			CIndexedPolygon pol( src_polygon );

			pol.m_index.resize(3);
			pol.m_index[0] = src_polygon.m_index[0];
			pol.m_index[1] = src_polygon.m_index[j+1];
			pol.m_index[2] = src_polygon.m_index[j+2];

			dest_polygon_buffer.push_back( pol );
		}
	}
}


inline AABB3 GetAABB( const std::vector<CIndexedPolygon>& polygon_buffer )
{
	AABB3 aabb;
	aabb.Nullify();

	size_t i, num_pols = polygon_buffer.size();
	size_t j, num_verts;
	for( i=0; i<num_pols; i++ )
	{
		// polygon_buffer[i].Update();
		// aabb.MergeAABB( polygon_buffer[i].GetAABB() );

		num_verts = polygon_buffer[i].m_index.size();
		for( j=0; j<num_verts; j++ )
		{
			aabb.AddPoint( polygon_buffer[i].GetVertex((int)j).m_vPosition );
		}
	}

	return aabb;
}


inline void UpdateAABBs( std::vector<CIndexedPolygon>& polygon_buffer )
{
	size_t i, num_pols = polygon_buffer.size();
	for( i=0; i<num_pols; i++ )
	{
		polygon_buffer[i].UpdateAABB();
	}
}


inline void CIndexedPolygon::Serialize( IArchive& ar, const unsigned int version )
{
	ar & m_Plane;

	ar & m_AABB;

	ar & m_index;

	/// surface property of the polygon
	ar & m_MaterialIndex;
}


#endif  /* __INDEXEDPOLYGON_H__*/
