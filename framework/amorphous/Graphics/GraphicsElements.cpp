#include "GraphicsElements.hpp"
#include "GraphicsElementManager.hpp"
#include "Font/TextureFont.hpp"
#include "2DPrimitive/2DPolygon.hpp"
#include "amorphous/Support/Macro.h"
#include "amorphous/Support/Log/DefaultLog.hpp"
#include "amorphous/Support/Profile.hpp"


namespace amorphous
{

using namespace std;


inline int get_num_rows( const string& str )
{
	size_t pos = 0;
	int num_rows = 1;
	while( pos < str.length() && (pos = str.find("\n",pos)) != string::npos )
	{
		num_rows++;
		pos++;
	}

	return num_rows;
}

template<typename T>
inline void erase_dupulicate_elements( std::vector<T>& vec )
{
	// Without 'typename' before 'auto', ndk compiler complains as follows:
	// ---
	// error: need 'typename' before 'std::vector<T>::iterator'
	// because 'std::vector<T>' is a dependent scope
	// ---
	// Same goes with 'auto itr1' inside the loop.
	for( typename std::vector<T>::iterator itr0 = vec.begin(); itr0 != vec.end(); itr0++ )
	{
		typename std::vector<T>::iterator itr1 = itr0 + 1;
		while( itr1 != vec.end() )
		{
			if( (*itr0) == (*itr1) )
				itr1 = vec.erase( itr1 );
			else
				itr1++;

		}
	}
}

void GraphicsElement::Release()
{
	m_pManager = NULL;
	m_LocalAABB.Nullify();
	m_TextureID = -1;
	m_fScale = 1.0f;
	m_GroupID = -1;
	m_ElementIndex = -1;
	m_LayerIndex = -1;
	m_vLocalTopLeftPos = Vector2(0,0);
//	m_vLocalRotationCenterPos = Vector2(0,0);
	m_LocalTransform = Matrix23Identity();
	m_ParentTransform = Matrix23Identity();
	for( int i=0; i<NUM_COLORS; i++ )
		m_aColor[i] = SFloatRGBAColor(1,1,1,1);
}


void GraphicsElement::UpdateTransform( const Matrix23& parent_transform )
{
	m_ParentTransform = parent_transform;

	Matrix23 global_transform = CalculateGlobalTransform();
	if( global_transform.matOrient == Matrix22Identity() )
	{
		// no rotation
		// - Need to update only the translation
		UpdatePositionsInternalForNonRotatedElement( global_transform.vPosition );
	}
	else
	{
		// The global transform includes rotation.
		UpdatePositionsInternal( global_transform );
	}

}


Matrix23 GraphicsElement::CalculateGlobalTransform()
{
	return m_ParentTransform * m_LocalTransform;
}

/*
void GraphicsElement::SetTopLeftPos( Vector2 vGlobalPos )
{
	// local position of this element = (the inverse of the parent transform) * global position
	m_ParentTransform.InvTransform( m_vLocalTopLeftPos, vGlobalPos );

	UpdateTopLeftPos( vGlobalPos );

	// Check just in case
	if( 0 <= m_GroupID )
	{
		// Oh, boy... This is owned by a group element
		// - It's not really desirable to call SetTopLeftPos() of a grouped element.
		//   SetLocalTopLeftPos() should be used instead.
		// - Anyway, need to update the local position as well
		GraphicsElementGroup *pOwner = dynamic_cast<GraphicsElementGroup *>(m_pManager->GetElement(m_GroupID));
		if( pOwner )
			m_vLocalTopLeftPos = vGlobalPos - pOwner->GetLocalOriginInGlobalCoord();
	}

}
*/

void GraphicsElement::SetLocalTopLeftPos( Vector2 vLocalPos )
{
	m_vLocalTopLeftPos = vLocalPos;

	Vector2 vSpan = m_LocalAABB.vMax - m_LocalAABB.vMin;
	m_LocalAABB.vMin = vLocalPos;
	m_LocalAABB.vMax = m_LocalAABB.vMin + vSpan;

	UpdateTransform( m_ParentTransform );
}


void GraphicsElement::SetSizeLTRB( const Vector2& vLocalMin, const Vector2& vLocalMax )
{
	m_LocalAABB.vMin = vLocalMin;
	m_LocalAABB.vMax = vLocalMax;

	UpdateTransform( m_ParentTransform );
}


void GraphicsElement::SetLayer( int layer_index )
{
	// remove the element from the current layer
	// and place it to the new layer
	if( m_pManager )
//		m_pManager->SetElementToLayer( this, layer_index );
		m_pManager->SetElementToLayer( m_pSelf.lock(), layer_index );
}



//==========================================================================================
// PrimitiveElement
//==========================================================================================

void PrimitiveElement::InitPrimitive()
{
	float z = 0;

	m_pPrimitive->SetTextureUV( TEXCOORD2( 0.0f, 0.0f ), TEXCOORD2( 1.0f, 1.0f ) );
	m_pPrimitive->SetZDepth( z );
//	m_pPrimitive->SetColor( fill_color );

	ChangeScale( m_fScale );
}


void PrimitiveElement::DrawPrimitive()
{
	if( !m_pManager )
		return;

	PROFILE_FUNCTION();

	if( 0 <= m_TextureID )
	{
		const TextureHandle& tex = m_pManager->GetTexture(m_TextureID);
		m_pPrimitive->Draw( tex );
	}
	else
		m_pPrimitive->Draw();	// draw rect without a texture
}


/// Use this if the primitive is not rotated ( CalculateGlobalTransform().matOrient == Matrix22Identity() ).
void PrimitiveElement::UpdatePositionsInternalForNonRotatedElement( const Vector2& vGlobalTranslation )
{
	m_pPrimitive->SetPosition(
		( vGlobalTranslation + m_LocalAABB.vMin ) * m_fScale,
		( vGlobalTranslation + m_LocalAABB.vMax ) * m_fScale
		);
}


void PrimitiveElement::SetTopLeftPosInternal( Vector2 vPos )
{
	// m_LocalAABB has been updated in GraphicsElement::SetTopLeftPos().
	// - update the vertex positions
//	m_pPrimitive->SetPosition( m_LocalAABB.vMin * m_fScale, m_LocalAABB.vMax * m_fScale );

	UpdatePositionsInternal( m_ParentTransform * m_LocalTransform );
}


void PrimitiveElement::ChangeScale( float scale )
{
	m_fScale = scale;

	UpdatePositionsInternal( m_ParentTransform * m_LocalTransform );
//	m_pPrimitive->SetPosition( m_LocalAABB.vMin * scale, m_LocalAABB.vMax * scale );
}


void PrimitiveElement::SetFillColor( int color_index, const SFloatRGBAColor& color )
{
//	m_aFillColor
}


void PrimitiveElement::SetFrameColor( int color_index, const SFloatRGBAColor& color )
{
//	m_aFrameColor
}



//==========================================================================================
// RectElement
//==========================================================================================

/*RectElement::RectElement( C2DRect *pFillRect, C2DFrameRect *pFrameRect )
:
m_pFillRect(pFillRect),
m_pFrameRect(pFrameRect)*/
RectElement::RectElement( const SRect& non_scaled_rect, float fScale )
{
//	m_LocalAABB = AABB2( pPrimitive->GetPosition2D(0), pPrimitive->GetPosition2D(2) );
//	m_aColor[0] = color0;

	const int num_corners = 4;
	for( int i=0; i<num_corners; i++ )
		m_aCornerColor[i] = SFloatRGBAColor(1,1,1,1);
/*
	m_pFillRect  = new C2DRect( non_scaled_rect * fScale );
	m_pFrameRect = new C2DFrameRect( non_scaled_rect * fScale );

	m_pFillPrimitive  = m_pFillRect;
	m_pFramePrimitive = m_pFrameRect;

	InitPrimitives();*/
}


RectElement::~RectElement()
{
}


void RectElement::Draw()
{
	PROFILE_FUNCTION();

	if( /* use_corner_colors == */ true )
//	 && m_pPrimitive->GetPrimitiveType() != C2DPrimitive::TYPE_ROUNDRECT )
	{
		// set separate colors to each corner
		// - not available for C2DRoundRect
		SFloatRGBAColor blended_color = GetBlendedColor();
		const int num_corners = 4;
		for( int i=0; i<num_corners; i++ )
		{
			m_pPrimitive->SetCornerColor( i, blended_color * m_aCornerColor[i] );
		}
	}
	else
		SetBlendedColorToPrimitive();

	DrawPrimitive();
}


void RectElement::SetLocalTransform( const Matrix23& local_transform )
{
	GraphicsElement::SetLocalTransform( local_transform );

	UpdatePositionsInternal( m_ParentTransform * local_transform );
}


void RectElement::UpdatePositionsInternal( const Matrix23& global_transform )
{
	const float s = m_fScale;
	Vector2 v;
//	Vector2 vCenter = GetLocalRotationCenterPos();
//	Vector2 vScaledCenter = vCenter * s;
	Vector2 vMin = m_LocalAABB.vMin;// - vCenter;
	Vector2 vMax = m_LocalAABB.vMax;// - vCenter;
	Vector2 avVertPos[] =
	{
		Vector2( vMin.x, vMin.y ),
		Vector2( vMax.x, vMin.y ),
		Vector2( vMax.x, vMax.y ),
		Vector2( vMin.x, vMax.y )
	};

	for( int i=0; i<4; i++ )
	{
		v = global_transform * avVertPos[i];

		m_pPrimitive->SetVertexPosition( i, v * s );
	}
/*
	if( fabs(m_fRotationAngle) < 0.0001 )
	{
		m_pPrimitive->Rotate( deg_to_rad(m_fRotationAngle) );
	}
	else
	{
		m_pPrimitive->Rotate( deg_to_rad(m_fRotationAngle) );
	}*/
}


FillRectElement::FillRectElement( const SRect& non_scaled_rect, float fScale )
:
RectElement(non_scaled_rect,fScale)
{
	m_pFillRect = new C2DRect();
	m_pPrimitive = m_pFillRect;
	InitPrimitive();
}

FillRectElement::~FillRectElement()
{
	SafeDelete( m_pFillRect );
}


FrameRectElement::FrameRectElement( const SRect& non_scaled_rect, float fScale )
:
RectElement(non_scaled_rect,fScale)
{
	m_pFrameRect = new C2DFrameRect();
	m_pPrimitive = m_pFrameRect;
	InitPrimitive();
}

FrameRectElement::~FrameRectElement()
{
	SafeDelete( m_pFrameRect );
}

void FrameRectElement::SetFrameWidth( int width )
{
	m_OrigBorderWidth = width;
	m_pFrameRect->SetBorderWidth( (int)(width * m_fScale) );
}



//==========================================================================================
// RoundRectElement
//==========================================================================================

RoundRectElement::RoundRectElement( const SRect& non_scaled_rect, float fScale )
{
	ChangeScale( fScale );

//	InitPrimitives();
}


RoundRectElement::~RoundRectElement()
{
}


/// TODO: support corner color for RoundFrameRectElement
void RoundRectElement::Draw()
{
	SetBlendedColorToPrimitive();
	DrawPrimitive();
}



void RoundRectElement::SetCornerRadius( float radius )
{
	m_CornerRadius = (int)radius;
}


RoundFillRectElement::RoundFillRectElement( const SRect& non_scaled_rect, float fScale, float corner_radius )
:
RoundRectElement(non_scaled_rect,fScale)
{
	m_pRoundFillRect  = new C2DRoundRect( non_scaled_rect * fScale, 0xFFFFFFFF, (int)(corner_radius * m_fScale) );
	m_pPrimitive = m_pRoundFillRect;
	InitPrimitive();
}

RoundFillRectElement::~RoundFillRectElement()
{
	SafeDelete( m_pRoundFillRect );
}


RoundFrameRectElement::RoundFrameRectElement( const SRect& non_scaled_rect, float fScale, float corner_radius )
:
RoundRectElement(non_scaled_rect,fScale)
{
	m_pRoundFrameRect = new C2DRoundFrameRect( non_scaled_rect * fScale, 0xFFFFFFFF, (int)(corner_radius * m_fScale) );
	m_pPrimitive = m_pRoundFrameRect;
	InitPrimitive();
//	p2DRoundFrameRect = new C2DRoundFrameRect( rect * m_fScale, color.GetARGB32(), (int)(corner_radius * m_fScale), (int)(border_width * m_fScale) );
}

RoundFrameRectElement::~RoundFrameRectElement()
{
	SafeDelete( m_pRoundFrameRect );
}

void RoundFrameRectElement::SetFrameWidth( int width )
{
	m_OrigBorderWidth = width;
	m_pRoundFrameRect->SetBorderWidth( (int)(width * m_fScale) );
}



//==========================================================================================
// TriangleElement
//==========================================================================================

TriangleElement::TriangleElement( const SRect& non_scaled_rect, float fScale )
{
	ChangeScale( m_fScale );

	for( int i=0; i<numof(m_CornerColors); i++ )
		m_CornerColors[i] = SFloatRGBAColor::White();

}


void TriangleElement::Draw()
{
	if( HasDifferentCornerColors() )
	{
		const SFloatRGBAColor blended_color = GetBlendedColor();
		for( int i=0; i<numof(m_CornerColors); i++ )
			m_pPrimitive->SetCornerColor( i, blended_color * m_CornerColors[i] );
	}
	else
	{
		SetBlendedColorToPrimitive();
	}

	DrawPrimitive();
}


void TriangleElement::SetVertexPosition( int index, const Vector2& vPos )
{
	if( m_pPrimitive )
		m_pPrimitive->SetVertexPosition( index, vPos * m_fScale );

	UpdatePositionsInternal( m_ParentTransform * m_LocalTransform );
}


void TriangleElement::UpdatePositionsInternal( const Matrix23& global_transform )
{
//	if( m_pTriangle )
//		for( int i=0; i<3; i++ ) m_pTriangle->SetVertexPosition( i, global_transform * m_avVertexPosition[i] );
//	else if( m_pFTriangle )
//		for( int i=0; i<3; i++ ) m_pTriangle->SetVertexPosition( i, global_transform * m_avVertexPosition[i] );
}


//TriangleElement::TriangleElement( const SFloatRGBAColor& color0, C2DPrimitive *pPrimitive, const SRect& non_scaled_rect )
//TriangleElement::TriangleElement( C2DTriangle *pTriangle, C2DFrameTriangle *pFrameTriangle, const SRect& non_scaled_rect )
FillTriangleElement::FillTriangleElement( C2DTriangle::Direction dir, const SRect& non_scaled_rect, float fScale )
:
TriangleElement(non_scaled_rect,fScale)
{
	m_pFillTriangle = new C2DTriangle( dir, non_scaled_rect * fScale );

	m_LocalAABB = AABB2( Vector2((float)non_scaled_rect.left,(float)non_scaled_rect.top), Vector2((float)non_scaled_rect.right,(float)non_scaled_rect.bottom) );

	m_pPrimitive = m_pFillTriangle;

	InitPrimitive();

//	m_aColor[0] = color0;
	ChangeScale( fScale );
}


FillTriangleElement::FillTriangleElement( const Vector2& v0, const Vector2& v1, const Vector2& v2, const SRect& non_scaled_rect, float fScale )
:
TriangleElement(non_scaled_rect,fScale)
{
	m_pFillTriangle = new C2DTriangle();
	m_pFillTriangle->SetPosition( 0, v0 * fScale );
	m_pFillTriangle->SetPosition( 1, v1 * fScale );
	m_pFillTriangle->SetPosition( 2, v2 * fScale );

	m_LocalAABB = AABB2( Vector2((float)non_scaled_rect.left,(float)non_scaled_rect.top), Vector2((float)non_scaled_rect.right,(float)non_scaled_rect.bottom) );

	m_pPrimitive = m_pFillTriangle;

	InitPrimitive();

//	m_aColor[0] = color0;
	ChangeScale( fScale );
}


FillTriangleElement::~FillTriangleElement()
{
	SafeDelete( m_pFillTriangle );
}


void FillTriangleElement::UpdatePositionsInternal( const Matrix23& global_transform )
{
}


FrameTriangleElement::FrameTriangleElement( C2DTriangle::Direction dir, const SRect& non_scaled_rect, float fScale )
:
TriangleElement(non_scaled_rect,fScale)
{
	m_pFrameTriangle = NULL;//new C2DFrameTriangle( dir, rect * fScale );

	m_LocalAABB = AABB2( Vector2((float)non_scaled_rect.left,(float)non_scaled_rect.top), Vector2((float)non_scaled_rect.right,(float)non_scaled_rect.bottom) );

	m_pPrimitive = m_pFrameTriangle;

	InitPrimitive();

//	m_aColor[0] = color0;
	ChangeScale( fScale );
}


FrameTriangleElement::~FrameTriangleElement()
{
	SafeDelete( m_pFrameTriangle );
}


void FrameTriangleElement::UpdatePositionsInternal( const Matrix23& global_transform )
{
}



//==========================================================================================
// PolygonElement
//==========================================================================================

FillPolygonElement::FillPolygonElement( const SFloatRGBAColor& color0, const SRect& non_scaled_rect )
{
	m_pRegularPolygon = new C2DRegularPolygon();//dynamic_cast<C2DRegularPolygon *>(pPrimitive);
	m_aColor[0] = color0;

//	ChangeScale( m_fScale );
}


FillPolygonElement::FillPolygonElement( int num_polygon_vertices,
										  Vector2 vCenter,
										  int radius,
										  CRegularPolygonStyle::Name style,
										  float fScale )
{
	float m_fRegularPolygonRadius = (float)radius;

	m_pRegularPolygon = new C2DRegularPolygon();
	m_pRegularPolygon->MakeRegularPolygon( num_polygon_vertices, vCenter * fScale, (int)(radius * fScale), style );
	m_pPrimitive = m_pRegularPolygon;

	ChangeScale( fScale );
}


FillPolygonElement::~FillPolygonElement()
{
	SafeDelete( m_pRegularPolygon );
}


void FillPolygonElement::Draw()
{
	SetBlendedColorToPrimitive();

	m_pRegularPolygon->Draw();
}


void FillPolygonElement::SetRadius( int vertex, int radius )
{
	m_pRegularPolygon->SetRadius( vertex, (int)(radius * m_fScale) );
}


Vector2 FillPolygonElement::GetVertexPos( int vertex )
{
	return m_pRegularPolygon->GetVertexPos( vertex ) / m_fScale;
}


void FillPolygonElement::SetVertexColor( int vertex, int color_index, const SFloatRGBAColor& color )
{
	m_pRegularPolygon->SetVertexColor( vertex, color );
}


void CombinedPrimitiveElement::OnRemovalRequested()
{
	if( m_pFillElement )
	{
		m_pManager->RemoveElement( m_pFillElement );
		m_pFillElement.reset();
	}

	if( m_pFrameElement )
	{
		m_pManager->RemoveElement( m_pFrameElement );
		m_pFrameElement.reset();
	}
}


void CombinedPrimitiveElement::Release()
{
	GraphicsElement::Release();

//	OnRemovalRequested();

	m_pFillElement.reset();
	m_pFrameElement.reset();
}


//==========================================================================================
// TextElement
//==========================================================================================

/*
TextElement::TextElement( int font_id, const std::string& text, float x, float y, const SFloatRGBAColor& color0 )
:
m_FontID(font_id),
m_Text(text),
m_vTextPos(Vector2(x,y))
{
	m_LocalAABB.vMin = m_vTextPos;
	m_TextAlignH = TAL_LEFT;
	m_TextAlignV = TAL_TOP;
	m_aColor[0] = color0;
	ChangeScale( m_fScale );

//		m_LocalAABB.vMax = 
}
*/


void TextElement::Release()
{
	GraphicsElement::Release();

	m_FontID = -1;
	m_Text   = "";
	m_vTextPos         = Vector2(0,0);
	m_vScaledPos       = Vector2(0,0);
	m_vLocalTextOffset = Vector2(0,0);
	m_ScaledWidth  = 0;
	m_ScaledHeight = 0;
	m_FontWidth    = 0;
	m_FontHeight   = 0;
}


void TextElement::Draw()
{
	PROFILE_FUNCTION();

	FontBase *pFont = m_pManager->GetFont(m_FontID);
	if( !pFont )
		return;

	if( m_Text.length() == 0 )
		return;

	int orig_font_w=0, orig_font_h=0;
	if( 0 < m_FontWidth && 0 < m_FontHeight )
	{
		// override the default font size with the element-specific font size
		pFont->GetFontSize( orig_font_w, orig_font_h );
//		pFont->SetFontSize( m_FontWidth, m_FontHeight );
		pFont->SetFontSize( m_ScaledWidth, m_ScaledHeight );
	}

	// set texture stage states
	// this is not necessary when the font is not a texture font
	SetRenderStatesForTextureFont( m_DestAlphaBlendMode );
	pFont->SetDestAlphaBlendMode( m_DestAlphaBlendMode );

	pFont->SetFontColor( GetBlendedColor().GetARGB32() );

	pFont->DrawText( m_Text, m_vScaledPos );

	// restore the original font size if the text has its own font size
	if( 0 < m_FontWidth && 0 < m_FontHeight )
		pFont->SetFontSize( orig_font_w, orig_font_h );
}


void TextElement::SetTextAlignment( int horizontal_alignment, int vertical_alignment )
{
	m_TextAlignH = horizontal_alignment;
	m_TextAlignV = vertical_alignment;

	UpdateTextAlignment();
}


void TextElement::UpdateLocalTextOffset()
{
	FontBase *pFont = m_pManager->GetFont( m_FontID );
	if( !pFont )
		return;

	Vector2 vDiagonal = m_LocalAABB.vMax - m_LocalAABB.vMin;
	int box_width  = (int)vDiagonal.x;
	int box_height = (int)vDiagonal.y;
	const float scale = m_fScale;

	// non-scaled font width & height
	int font_width   = 0 < m_FontWidth  ? m_FontWidth  : (int)( pFont->GetFontWidth()  / scale ); 
	int font_height  = 0 < m_FontHeight ? m_FontHeight : (int)( pFont->GetFontHeight() / scale );
	float font_scale = 0 < m_FontWidth  ? (float)m_FontWidth / ((float)pFont->GetFontWidth() / scale) : 1.0f;
//	const int text_length = (int)m_Text.length();
	Vector2 vLocalTextOffset = Vector2(0,0);

	switch(m_TextAlignH)
	{
	case TextElement::TAL_LEFT:
		vLocalTextOffset.x = 0;
		break;
	case TextElement::TAL_CENTER:
		vLocalTextOffset.x = (float)( box_width/2  - (pFont->GetTextWidth(m_Text.c_str())/scale) * font_scale / 2 );
		break;
	case TextElement::TAL_RIGHT:
		// --------------- NOT IMPLEMENTED ---------------
		vLocalTextOffset.x = (float)( box_width  - (pFont->GetTextWidth(m_Text.c_str())/scale) * font_scale );
		break;
	default:
		break;
	}

	switch(m_TextAlignV)
	{
	case TextElement::TAL_TOP:
		vLocalTextOffset.y = 0;	// param.m_TextMargin
		break;
	case TextElement::TAL_CENTER:
//		param.m_vTextOffset.y = (float)(control_height - font_height) * 0.5f;
		vLocalTextOffset.y = (float)( box_height/2 - font_height*get_num_rows(m_Text)/2 );
		break;
	case TextElement::TAL_BOTTOM:
		// --------------- NOT IMPLEMENTED ---------------
		vLocalTextOffset.y = (float)( box_height - font_height*get_num_rows(m_Text) );
		break;
	default:
		break;
	}

	m_vLocalTextOffset = vLocalTextOffset;
}


void TextElement::UpdateTextAlignment()
{
	UpdateLocalTextOffset();

	Matrix23 global_transform = CalculateGlobalTransform();

	// TODO: support rotation
	m_vTextPos = m_vLocalTextOffset + m_LocalAABB.vMin + global_transform.vPosition;

	m_vScaledPos = m_vTextPos * m_fScale;
}


/// TODO: support rotation
void TextElement::UpdatePositionsInternal( const Matrix23& global_transform )
{
	UpdatePositionsInternalForNonRotatedElement( global_transform.vPosition );
}


void TextElement::UpdatePositionsInternalForNonRotatedElement( const Vector2& vGlobalTranslation )
{
	Vector2 vSpan = m_LocalAABB.vMax - m_LocalAABB.vMin;
//	m_FontWidth  = (int)( vSpan.x / (float)m_Text.length() ); 
//	m_FontHeight = (int)vSpan.y;// vSpan.x / (float)GetNumRows(m_Text)

	m_vTextPos = m_vLocalTextOffset + m_LocalAABB.vMin + vGlobalTranslation;

	m_vScaledPos = m_vTextPos * m_fScale;
}



//==================================================================================
// GraphicsElementGroup
//==================================================================================

inline void GraphicsElementGroup::RemoveInvalidElements()
{
	// remove NULL elements
	vector< shared_ptr<GraphicsElement> >::iterator itr = m_vecpElement.begin();
	while( itr != m_vecpElement.end() )
	{
//		if( (*itr) == NULL )
		if( (*itr).get() == NULL )
			itr = m_vecpElement.erase( itr );
		else
			itr++;
	}

	// remove the same elements
	erase_dupulicate_elements( m_vecpElement );
}

/*
GraphicsElementGroup::GraphicsElementGroup( std::vector< shared_ptr<GraphicsElement> >& rvecpElement )
:
m_vecpElement(rvecpElement)
{
	RemoveInvalidElements();

	UpdateAABB();

	// use the top-left corner as the local origin
	SetLocalOrigin( GetTopLeftPos() );

	// set local top-left potitions for grouped elements
	vector< shared_ptr<GraphicsElement> >::iterator itr;
	for( itr = m_vecpElement.begin(); itr != m_vecpElement.end(); itr++ )
	{
		(*itr)->m_vLocalTopLeftPos = (*itr)->GetTopLeftPos() - GetTopLeftPos();
	}
}
*/


GraphicsElementGroup::GraphicsElementGroup( std::vector< shared_ptr<GraphicsElement> >& rvecpElement, Vector2 vLocalOrigin )
:
//m_vLocalOrigin(vLocalOrigin),
m_vecpElement(rvecpElement)
{
	m_vLocalTopLeftPos = vLocalOrigin;

	RemoveInvalidElements();

	UpdateAABB();

	// set local top-left potitions for grouped elements
/*	vector< shared_ptr<GraphicsElement> >::iterator itr;
	for( itr = m_vecpElement.begin(); itr != m_vecpElement.end(); itr++ )
	{
		(*itr)->m_vLocalTopLeftPos = (*itr)->GetTopLeftPos() - m_vLocalOrigin;
	}
	*/

	// update global positions of grouped elements
	SetLocalOrigin( vLocalOrigin );
}


GraphicsElementGroup::~GraphicsElementGroup()
{
	// release the elements from the group
	// - does not release the element itself from the graphics element manager
	// - they just get 'ungrouped'.
	vector< shared_ptr<GraphicsElement> >::iterator itr;
	for( itr = m_vecpElement.begin(); itr != m_vecpElement.end(); itr++ )
	{
		(*itr)->m_GroupID = -1;
	}
}


void GraphicsElementGroup::SetTopLeftPosInternal( Vector2 vGlobalPos )
{
}


Vector2 GraphicsElementGroup::GetLocalOriginInGlobalCoord() const
{
	return m_ParentTransform * m_LocalTransform * GetLocalTopLeftPos();
/*
	if( 0 <= m_GroupID )
	{
		GraphicsElementGroup *pParentGroup = dynamic_cast<GraphicsElementGroup *>(m_pManager->GetElement( m_GroupID ));
		if( pParentGroup )
			return pParentGroup->GetLocalOriginInGlobalCoord() + GetLocalTopLeftPos();
		else
			return GetLocalTopLeftPos();
	}
	else
	{
		// not owned by any group
		// - i.e.) m_vLocalOrigin is in global coord
		return GetLocalTopLeftPos();
	}*/
}


void GraphicsElementGroup::SetLocalOrigin( Vector2 vLocalOrigin )
{
//	m_vLocalOrigin = vLocalOrigin;
	m_vLocalTopLeftPos = vLocalOrigin;

	//UpdateTransform();

	vector< shared_ptr<GraphicsElement> >::iterator itr;
	for( itr = m_vecpElement.begin(); itr != m_vecpElement.end(); itr++ )
	{
//		(*itr)->UpdateGlobalPositions( GetLocalOriginInGlobalCoord() );
		(*itr)->UpdateTransform(
			m_ParentTransform
		  * m_LocalTransform
		  * Matrix23( vLocalOrigin, Matrix22Identity() )
		  );
	}
}


void GraphicsElementGroup::SetLocalTopLeftPos( Vector2 vPos )
{
	SetLocalOrigin( vPos );
}


void GraphicsElementGroup::UpdateTransform( const Matrix23& parent_transform )
{
	m_ParentTransform = parent_transform;

	Matrix23 global_transform
		= m_ParentTransform
		* m_LocalTransform
		* Matrix23( GetLocalTopLeftPos(), Matrix22Identity() );

//	UpdatePositionsInternal( global_transform );

	vector< shared_ptr<GraphicsElement> >::iterator itr;
	for( itr = m_vecpElement.begin(); itr != m_vecpElement.end(); itr++ )
	{
		(*itr)->UpdateTransform( global_transform );
	}
}


void GraphicsElementGroup::ChangeScale( float scale )
{
//	int i, num_elements = m_vecElementID.size();
//	for( i=0; i<num_elements; i++ )

	vector< shared_ptr<GraphicsElement> >::iterator itr;
	for( itr = m_vecpElement.begin(); itr != m_vecpElement.end(); itr++ )
		(*itr)->ChangeScale( scale );
}


void GraphicsElementGroup::ChangeElementScale( float scale )
{
}


void GraphicsElementGroup::SetSizeLTRB( const Vector2& vMin, const Vector2& vMax )
{
	m_LocalAABB.vMin = vMin;
	m_LocalAABB.vMax = vMax;

	vector< shared_ptr<GraphicsElement> >::iterator itr;
	for( itr = m_vecpElement.begin();
		itr != m_vecpElement.end();
		itr++ )
	{
		(*itr)->SetSizeLTRB( vMin, vMax );
	}
}


void GraphicsElementGroup::SetColor( int color_index, const SFloatRGBAColor& color )
{
	GraphicsElement::SetColor( color_index, color );

	vector< shared_ptr<GraphicsElement> >::iterator itr;
	for( itr = m_vecpElement.begin();
		itr != m_vecpElement.end();
		itr++ )
	{
		(*itr)->SetColor( color_index, color );
	}
}


void GraphicsElementGroup::SetAlpha( int color_index, float a )
{
	GraphicsElement::SetAlpha( color_index, a );

	vector< shared_ptr<GraphicsElement> >::iterator itr;
	for( itr = m_vecpElement.begin();
		itr != m_vecpElement.end();
		itr++ )
	{
		(*itr)->SetAlpha( color_index, a );
	}
}


void GraphicsElementGroup::SetDestAlphaBlendMode( AlphaBlend::Mode mode )
{
	vector< shared_ptr<GraphicsElement> >::iterator itr;
	for( itr = m_vecpElement.begin();
		itr != m_vecpElement.end();
		itr++ )
	{
		(*itr)->SetDestAlphaBlendMode( mode );
	}
}


void GraphicsElementGroup::Draw()
{
	// do nothing
	// - drawing calls are managed by GraphicsElementManager
	// - CBE_Group is not intended to batch the draw calls
	//   - rationale: elements in a group might belong to different layers, and in such a case
	//                rendering order need to be managed the same way as non-grouped elements
}


} // namespace amorphous
