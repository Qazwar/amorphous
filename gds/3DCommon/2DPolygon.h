#ifndef  __2DPolygon_H__
#define  __2DPolygon_H__


#include "2DRoundRect.h"


/**
  polygon

 */
class C2DRegularPolygon : public C2DRoundRect
{
protected:

	/// polygon vertices (= buffer size + 1)
	int m_NumPolygonVertices;

	Vector2 m_vCenterPosition;

	int m_Radius;

protected:

	virtual int GetNumVertices() const { return m_NumPolygonVertices + 1; }

	virtual void CalculateLocalVertexPositions();

	virtual void UpdateVertexPositions();

//	inline virtual void UpdateColor();

	enum Style
	{
		VERTEX_AT_TOP,
		EDGE_AT_TOP,
		NUM_INIT_LAYOUTS
	};

public:

	inline C2DRegularPolygon();

//	inline C2DRegularPolygon( int num_vertices, const Vector2& vCenter, int radius, InitLayout VERTEX_AT_TOP );

	virtual ~C2DRegularPolygon() {}

	virtual int unsigned GetPrimitiveType() const { return C2DPrimitive::TYPE_POLYGON; }

	/// store a single color to 'm_Color'
	/// - vertex colors are updated if vertex buffers are created
	/// - can be called from constructor
//	inline virtual void SetColor( const SFloatRGBAColor& color );

//	inline void SetColor( U32 color );

	/// calls ResizeBuffer() internally
	/// - must not be called from constructor
//	virtual void SetZDepth(float fZValue);

//	void ScalePosition( float fScale );

	void MakeRegularPolygon( int num_polygon_vertices, int x, int y, int radius ) { MakeRegularPolygon( num_polygon_vertices, Vector2((float)x,(float)y), radius ); }

	void MakeRegularPolygon( int num_polygon_vertices, const Vector2& vCenter, int radius );
};


// =================================== inline implementations =================================== 

inline C2DRegularPolygon::C2DRegularPolygon()
:
m_NumPolygonVertices(6),
m_vCenterPosition(Vector2(0,0))
{
//	CalculateLocalVertexPositions();
//	SetDefault();
	SetColor( 0xFFFFFFFF );
}


#endif		/*  __2DPolygon_H__  */
