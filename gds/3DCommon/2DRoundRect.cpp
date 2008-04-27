#include "2DRoundRect.h"


C2DRoundRect::~C2DRoundRect()
{
	Release();
}


void C2DRoundRect::CalculateLocalVertexPositions()
{
	const int num_segments_per_corner = m_NumSegmentsPerCorner;
	const int num_total_segments = num_segments_per_corner * 4;
	int i,j;
	float radius = (float)m_CornerRadius;
	int vert_index = 0;
	Vector3 avCornerBasePos[4];

	if( m_vecRectVertex.size() == 0 )
		ResizeBuffer();

	// calculate vertex positions at each corner and cache them
	// - used every time the rect gets resized
	// - clockwise order
	// 0 top-left
	// 1 top-right
	// 2 bottom-right
	// 3 bottom-left
	for( i=0; i<4; i++ )
	{
		int seg_offset = i * num_segments_per_corner;
		for( j=0; j<=num_segments_per_corner; j++ )
		{
			float angle = ( 2.0f * (float)PI ) * ( seg_offset + j ) / (float)num_total_segments - (float)PI;
			angle *= -1;
			m_vecLocalVertexPosition[vert_index++] = Vector2( cos( angle ), -sin( angle ) ) * radius;
		}
	}

}


/// draws the rect with render state settings
/// if the rect has a texture, or is handed one as the argument
/// it will be rendered with the texture
/// alpha blend is enabled. The user can set the blend mode
/// by calling SetDestAlphaBlendMode()
/// the default blend mode is D3DBLEND_INVSRCALPHA
void C2DRoundRect::Draw()
{
	if( m_vecRectVertex.size() == 0 )
		return;

	SetBasicRenderStates();

	SetStdRenderStates();

	draw();
}


void C2DRoundRect::Draw( const CTextureHandle& texture )
{
	if( m_vecRectVertex.size() == 0 )
		return;

	SetBasicRenderStates();

	SetStdRenderStatesForTexture( texture );

	draw();
}


/*
/// draws the rect with the current render state settings
/// check the following render states before calling this function
/// - FVF
/// - z-buffer
/// - texture
/// - alpha blend
void C2DRoundRect::Draw()
{

	DIRECT3D9.GetDevice()->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, m_vecRectVertex, sizeof(TLVERTEX) );
}*/

/*
void C2DRoundRect::SetTextureUV( const TEXCOORD2& rvMin, const TEXCOORD2& rvMax)
{
	TLVERTEX* pav2DRoundRect = m_vecRectVertex;

	pav2DRoundRect[0].tu = rvMin.u;		// top-left corner of the rectangle
	pav2DRoundRect[0].tv = rvMin.v;

	pav2DRoundRect[1].tu = rvMax.u;
	pav2DRoundRect[1].tv = rvMin.v;

	pav2DRoundRect[2].tu = rvMax.u;		// bottom-right corner of the rectangle
	pav2DRoundRect[2].tv = rvMax.v;

	pav2DRoundRect[3].tu = rvMin.u;
	pav2DRoundRect[3].tv = rvMax.v;
}
*/


void C2DRoundRect::SetTextureCoords( const TEXCOORD2& rvMin, const TEXCOORD2& rvMax )
{
	const float outer_radius = (float)m_CornerRadius;
	const float outer_diameter = outer_radius * 2.0f;
	const float uv_margin = 0.01f;
	const int num_vertices = GetNumVertices();
//	const int num_points = (int)m_vecLocalVertexPosition.size();

	// Note that the last 2 vertices are wrapping of the border at the top-left corner
	for(int i=0; i<num_vertices; i++)
	{
		Vector2 vLocalPos = m_vecLocalVertexPosition[i];
		TLVERTEX& v = m_vecRectVertex[i];

		v.tu = ( vLocalPos.x + outer_radius ) / outer_diameter;
		v.tv = ( vLocalPos.y + outer_radius ) / outer_diameter;

		v.tu = v.tu + uv_margin / ( 1.0f + uv_margin * 2.0f );
		v.tv = v.tv + uv_margin / ( 1.0f + uv_margin * 2.0f );
	}
}


void C2DRoundRect::SetZDepth(float fZValue)
{
	if( m_vecRectVertex.size() == 0 )
	{
		CalculateLocalVertexPositions();
		UpdateVertexPositions();
	}

	const int num_vertices = GetNumVertices();
	for(int i=0; i<num_vertices; i++)
		m_vecRectVertex[i].vPosition.z   = fZValue;
}


void C2DRoundRect::ScalePosition( float fScale )
{
	if( m_vecRectVertex.size() == 0 )
		return;

	const int num_vertices = GetNumVertices();
	for(int i=0; i<num_vertices; i++)
		m_vecRectVertex[i].vPosition *= fScale;
}



//=============================================================================
// C2DRoundFrameRect
//=============================================================================

void C2DRoundFrameRect::CalculateLocalVertexPositions()
{
	int num_segments_per_corner = m_NumSegmentsPerCorner;
	int num_total_segments = num_segments_per_corner * 4;
	int i,j;
	const float outer_radius = (float)m_CornerRadius;
	const float inner_radius = outer_radius - (float)m_BorderWidth;
	int vert_index = 0;
	Vector3 avCornerBasePos[4];

	if( m_vecRectVertex.size() == 0 )
		ResizeBuffer();

	// calculate vertex positions at each corner and cache them
	// - used every time the rect gets resized
	// - clockwise order
	// 0 top-left
	// 1 top-right
	// 2 bottom-right
	// 3 bottom-left
	for( i=0; i<4; i++ )
	{
		int seg_offset = i * num_segments_per_corner;
		for( j=0; j<=num_segments_per_corner; j++ )
		{
			float angle = ( 2.0f * (float)PI ) * ( seg_offset + j ) / (float)num_total_segments - (float)PI;
			angle *= -1;

			m_vecLocalVertexPosition[vert_index++] = Vector2( cos( angle ), -sin( angle ) ) * inner_radius;

			m_vecLocalVertexPosition[vert_index++] = Vector2( cos( angle ), -sin( angle ) ) * outer_radius;
		}
	}

	// not used in C2DRoundFrameRect::UpdateVertexPositions(),
	// but used in C2DRoundRect::SetTextureCoords()
	m_vecLocalVertexPosition[vert_index++] = m_vecLocalVertexPosition[0];
	m_vecLocalVertexPosition[vert_index++] = m_vecLocalVertexPosition[1];
}
