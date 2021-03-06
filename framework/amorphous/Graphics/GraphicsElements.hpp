#ifndef  __GraphicsElements_H__
#define  __GraphicsElements_H__


#include "amorphous/base.hpp"
#include "amorphous/3DMath/Matrix23.hpp"
#include "amorphous/3DMath/Vector3.hpp"
#include "amorphous/3DMath/AABB2.hpp"
#include "amorphous/Graphics/fwd.hpp"
#include "amorphous/Graphics/2DPrimitive/2DRect.hpp"
#include "amorphous/Graphics/2DPrimitive/2DFrameRect.hpp"
#include "amorphous/Graphics/2DPrimitive/2DRoundRect.hpp"
#include "amorphous/Graphics/2DPrimitive/2DTriangle.hpp"
#include "amorphous/Graphics/TextureCoord.hpp"
#include "amorphous/Graphics/FloatRGBAColor.hpp"
#include "amorphous/Graphics/GraphicsComponentCollector.hpp"
#include "amorphous/Graphics/TextureHandle.hpp"
#include "amorphous/Support/Vec3_StringAux.hpp"
#include "amorphous/Support/memory_helpers.hpp"

#include <vector>
#include <list>
#include <algorithm>


namespace amorphous
{


inline AABB2 AABB2Null()
{
	return AABB2( Vector2(FLT_MAX,FLT_MAX), Vector2(-FLT_MAX,-FLT_MAX) );
}


/**
 terms
 - group: a collection of one or more graphics elements.
   - used to set effect on multiple elements at once
 - layer: holds graphics elements
   - controls rendering order
   - elements in layer 0 are rendered on top (rendered last)
   - higher layer index means the element is behind other elements
   - rendering order of elements in the same layer is not defined. They are rendered in random order.
   - group elements do not belong to any layer


 translation and rotation
 - To apply translation to an element
   - Move the element or set a local transform
 - To apply rotation
   - Set a local translation to an element
 - Rotation needs to be applied through local transforms

*/
class GraphicsElement
{
public:

	enum eColor
	{
		COLOR_0,
		COLOR_1,
		COLOR_2,
		COLOR_3,
		NUM_COLORS
	};

protected:

	GraphicsElementManager *m_pManager;

	std::weak_ptr<GraphicsElement> m_pSelf;

	SFloatRGBAColor m_aColor[NUM_COLORS];

	int m_TextureID;

	float m_fScale;

	/// holds non-scaled rect in local coordinates.
	/// - holds global coordinates even if the element is owned by a group element
	/// - m_LocalAABB.vMin == non-scaled global top-left position
	AABB2 m_LocalAABB;

	int m_GroupID; ///< group id to which the element belongs. -1 if the element does not belong to any group element

	int m_LayerIndex; ///< layer index to which the element belongs. -1 if the element is a group element

	int m_ElementIndex; ///< index of the element in GraphicsElementManager::m_vecpElement

	/// top left pos relative to the group element which owns this element
	/// - used by grouped elements
	/// - not used by group elements
	Vector2 m_vLocalTopLeftPos;

//	float m_fRotationAngle; ///< angle of local rotation [deg]

	/// global transform of the element that owns this element
	/// - usu. a group element
	Matrix23 m_ParentTransform;

	Matrix23 m_LocalTransform;

protected:

	/// can only be created by GraphicsElementManager
	/// - make this protected and make GraphicsElementManager a friend class
	/// \param non_scaled_aabb non-scaled axis-aligned bounding box. Needs to be
	///        initialized in derived class if it is not specified here
	GraphicsElement( const AABB2& non_scaled_aabb = AABB2Null() )
		:
	m_pManager(NULL),
	m_LocalAABB(non_scaled_aabb),
	m_TextureID(-1),
	m_fScale(1.0f),
	m_GroupID(-1),
	m_ElementIndex(-1),
	m_LayerIndex(-1),
	m_vLocalTopLeftPos(Vector2(0,0)),
//	m_vLocalRotationCenterPos(Vector2(0,0)),
	m_LocalTransform(Matrix23Identity()),
	m_ParentTransform(Matrix23Identity())
	{
		for( int i=0; i<NUM_COLORS; i++ )
			m_aColor[i] = SFloatRGBAColor(1,1,1,1);
	}

	virtual ~GraphicsElement() {}

	virtual void Release();

	virtual void OnRemovalRequested() {}

	virtual void SetTopLeftPosInternal( Vector2 vPos ) {}

	virtual void UpdatePositionsInternal( const Matrix23& global_transform ) {}

	virtual void UpdatePositionsInternalForNonRotatedElement( const Vector2& vGlobalTranslation ) {}

	virtual void UpdateTransform( const Matrix23& parent_transform );

	Matrix23 CalculateGlobalTransform();

private:

	void SetElementIndex( int element_index ) { m_ElementIndex = element_index; }

	/// sets the layer index
	/// NOTE: does not move the element from the current layer
	void SetLayerIndex( int layer_index ) { m_LayerIndex = layer_index; }

public:

	virtual int GetElementType() const = 0;

	virtual bool BelongsToLayer() const { return true; }

	virtual void Draw() = 0;

	virtual void SetTexture( int texture_id ) { m_TextureID = texture_id; }

	/// TODO: Decide whether this really has to be virtual
	virtual Vector2 GetTopLeftPos() const { return m_LocalAABB.vMin; }

	Vector2 GetLocalTopLeftPos() const { return m_vLocalTopLeftPos; }

	const AABB2& GetAABB() const { return m_LocalAABB; }

	/// calls SetTopLeftPosInternal() to run routines
	/// specific to each element
	/// - The user should use SetLocalTopLeftPos() if the element is grouped.
	///   The result is undefined if this is called when the local origin of the group element is being changed
	/// \param vPos non-scaled global pos
//	void SetTopLeftPos( Vector2 vGlobalPos );

//	void SetTopLeftPos( SPoint pos ) { SetTopLeftPos( Vector2((float)pos.x,(float)pos.y) ); }

	/// Sets the top left corner position of the element
	/// - Moves the element.
	/// - Does not change the local transform.
	/// - Updates the poses of all the descendants.
	virtual void SetLocalTopLeftPos( Vector2 vPos );

	void SetLocalTopLeftPos( SPoint pos ) { SetLocalTopLeftPos(Vector2((float)pos.x,(float)pos.y) ); }

	void SetLocalRect( const SRect& local_rect )
	{
		SetSizeLTRB( Vector2((float)local_rect.left,(float)local_rect.top), Vector2((float)local_rect.right,(float)local_rect.bottom) );
		SetLocalTopLeftPos( local_rect.GetTopLeftCorner() );
	}

	virtual void ChangeScale( float scale ) {}

	virtual void ChangeElementScale( float scale ) {}

	void SetSizeLTRB( const Vector2& vLocalMin, const Vector2& vLocalMax );

//	void SetLocalRotationCenterPos( const Vector2& vLocalRotationCenterPos ) { m_vLocalRotationCenterPos = vLocalRotationCenterPos; }

//	Vector2 GetLocalRotationCenterPos() const { return m_vLocalRotationCenterPos; }

	/// set the local rotation angle
	/// - Rotates the element locally from 0 to 'angle'
	/// \param angle angle of rotation [deg]
	/// \param vCenter center of roation (local or global?)
	void SetRotationAngle( const float angle, Vector2 vCenter )
	{
	}
	
	void SetLocalTransform( const Matrix23& local_transform )
	{
		m_LocalTransform = local_transform;
		UpdateTransform( m_ParentTransform );
	}

	/// Overwrites local orientation
	void SetLocalRotationAngle( const float angle_in_deg )
	{
		m_LocalTransform.matOrient = Matrix22( deg_to_rad( angle_in_deg ) );
/*		GraphicsElementGroup *pParent = GetParent();
		if( pParent )
		{
			m_fRotationAngle = pParent->GetRotationAngle() + angle;
		}
		else
		{*/
			//m_fRotationAngle = angle;
/*		}*/

		UpdateTransform( m_ParentTransform );
	}

//	float GetRotationAngle() const { return m_fRotationAngle; }

	virtual void Rotate( const float angle ) {}

	virtual void Rotate( const float angle, Vector2 vCenter ) {}

	virtual const SFloatRGBAColor& GetColor( int color_index ) const { return m_aColor[color_index]; }

	/// sets RGBA
	virtual void SetColor( int color_index, const SFloatRGBAColor& color ) { m_aColor[color_index] = color; }

	/// sets RGB
	/// - does not change alpha component
	virtual void SetColor( int color_index, const SFloatRGBColor& color ) { m_aColor[color_index].SetRGB( color.red, color.green, color.blue ); }

	float GetAlpha( int color_index ) const { return m_aColor[color_index].alpha; }

	virtual void SetAlpha( int color_index, float a ) { m_aColor[color_index].alpha = a; }

	virtual void SetDestAlphaBlendMode( AlphaBlend::Mode mode ) {}

	inline SFloatRGBAColor GetBlendedColor() const;

	virtual void SetTextureCoord( const TEXCOORD2& vMin, const TEXCOORD2& vMax ) {}

	virtual void SetFillColor( int color_index, const SFloatRGBAColor& color ) {}

	virtual void SetFrameColor( int color_index, const SFloatRGBAColor& color ) {}

	/// \param stretch_x non-scaled value
	/// \param stretch_y non-scaled value
	virtual void SetTextureCoord( int stretch_x, int stretch_y,
		                           const TEXCOORD2& left_top_offset = TEXCOORD2(0,0),
								   TextureAddress::Mode mode = TextureAddress::Wrap ) {}

	virtual void SetFrameWidth( int width ) {}

	void SetGraphicsElementManager( GraphicsElementManager *pMgr ) { m_pManager = pMgr; }

	/// place the element in a layer
	virtual void SetLayer( int layer_index );

	int GetLayerIndex() const { return m_LayerIndex; }

	int GetGroupID() const { return m_GroupID; }

	void SetGroupID( int group_id ) { m_GroupID = group_id; }

	int GetElementIndex() const { return m_ElementIndex; }

	enum eType
	{
		TYPE_COMBINEDRECT,
		TYPE_FILLRECT,
		TYPE_FRAMERECT,
		TYPE_COMBINEDROUNDRECT,
		TYPE_ROUNDFILLRECT,
		TYPE_ROUNDFRAMERECT,
		TYPE_COMBINEDTRIANGLE,
		TYPE_FILLTRIANGLE,
		TYPE_FRAMETRIANGLE,
		TYPE_COMBINEDPOLYGON,
		TYPE_FILLPOLYGON,
		TYPE_FRAMEPOLYGON,
		TYPE_TEXT,
		TYPE_GROUP,
		NUM_ELEMENT_TYPES
	};

	friend class CombinedPrimitiveElement;
	friend class GraphicsElementGroup;
	friend class GraphicsElementManager;
};


inline SFloatRGBAColor GraphicsElement::GetBlendedColor() const
{
	return m_aColor[0] * m_aColor[1] * m_aColor[2] * m_aColor[3];
}


class PrimitiveElement : public GraphicsElement
{
protected:

	/// scaled primitive
	/// - owned reference
	/// - holds scaled border if it is a frame primitive
	/// - holds scaled corner radius if it is a round-cornered primitive
//	C2DPrimitive *m_pFillPrimitive;  ///< Set by derived class. Borrowed reference.
//	C2DPrimitive *m_pFramePrimitive; ///< Set by derived class. Borrowed reference.
	C2DPrimitive *m_pPrimitive; ///< Set by derived class. Borrowed reference.

//	SFloatRGBAColor m_aFillColor[NUM_COLORS];
	SFloatRGBAColor m_aFrameColor[NUM_COLORS];

	/// Holds non-scaled value of frame border width.
	/// Used by the following primitives
	///   - frame rect
	///   - round frame rect
	int m_OrigBorderWidth; ///< non-scaled border width

	/// Holds non-scaled value of corner radius.
	/// Used by the following primitives.
	///   - round rect
	///   - round frame rect
	int m_CornerRadius;

protected:

	void DrawPrimitive();

	inline void SetBlendedColorToPrimitive();

	void UpdatePositionsInternalForNonRotatedElement( const Vector2& global_transform );

	void InitPrimitive();

public:

	/// \param non_scaled_aabb represents a non-scaled rectangular region of the element
	/// \param pPrimitive owned reference to a 2d primitive that holds scaled position
	PrimitiveElement()
		:
//	GraphicsElement(non_scaled_aabb),
	m_pPrimitive(NULL),
	m_OrigBorderWidth(1),
	m_CornerRadius(0)
	{
	}

	virtual ~PrimitiveElement() {}

	virtual void SetTopLeftPosInternal( Vector2 vPos );

	virtual void ChangeScale( float scale );

//	virtual void SetSizeLTRB( const Vector2& vLocalMin, const Vector2& vLocalMax );

	void SetDestAlphaBlendMode( AlphaBlend::Mode mode ) { m_pPrimitive->SetDestAlphaBlendMode( mode ); }

	virtual void SetTextureCoord( const TEXCOORD2& vMin, const TEXCOORD2& vMax ) { m_pPrimitive->SetTextureUV( vMin, vMax ); }

	virtual void SetTextureCoord( int stretch_x, int stretch_y,
		                           const TEXCOORD2& left_top_offset,
								   TextureAddress::Mode mode )
	{
		m_pPrimitive->SetTextureCoords( (int)(stretch_x * m_fScale), (int)(stretch_y * m_fScale), left_top_offset, mode );
	}

	virtual void SetFillColor( int color_index, const SFloatRGBAColor& color );

	virtual void SetFrameColor( int color_index, const SFloatRGBAColor& color );

	virtual void SetVertexColor( int vertex, int color_index, const SFloatRGBAColor& color ) {}
};


//=============================== inline implementations ===============================

inline void PrimitiveElement::SetBlendedColorToPrimitive()
{
	// update vertex colors
	// the same color is set to all the 4 vertices of the primitive
	m_pPrimitive->SetColor( GetBlendedColor() );
}



/**
  - graphics element for rectangle
*/
class RectElement : public PrimitiveElement
{
	/// blended with GraphicsElement::m_aColor[]
	SFloatRGBAColor m_aCornerColor[4];

private:

	void UpdatePositionsInternal( const Matrix23& global_transform );

public:

	/// \param pPrimitive - owned reference
	//RectElement( C2DRect *pFillRect, C2DFrameRect *pFrameRect );
	RectElement( const SRect& non_scaled_rect, float fScale );

	~RectElement();

	virtual void Draw();

	virtual void SetLocalTransform( const Matrix23& local_transform );

//	void SetHFrameTextureCoord();
//	void SetVFrameTextureCoord();

	/// not available for C2DRoundRect
	void SetCornerColor( int corner_index, SFloatRGBAColor& color ) { m_aCornerColor[corner_index] = color; }
};



class FillRectElement : public RectElement
{
	C2DRect *m_pFillRect;

public:

	FillRectElement( const SRect& non_scaled_rect, float fScale );

	~FillRectElement();

	int GetElementType() const { return TYPE_FILLRECT; }
};


class FrameRectElement : public RectElement
{
	C2DFrameRect *m_pFrameRect;

public:

	FrameRectElement( const SRect& non_scaled_rect, float fScale );

	~FrameRectElement();

	int GetElementType() const { return TYPE_FRAMERECT; }

	void SetFrameWidth( int width );
};


class RoundRectElement : public PrimitiveElement
{
public:

	RoundRectElement( const SRect& non_scaled_rect, float fScale );

	~RoundRectElement();

	virtual void Draw();

	void SetCornerRadius( float radius );
};


class RoundFillRectElement : public RoundRectElement
{
	C2DRoundRect *m_pRoundFillRect;

public:

	RoundFillRectElement( const SRect& non_scaled_rect, float fScale, float corner_radius );

	~RoundFillRectElement();

	int GetElementType() const { return TYPE_ROUNDFILLRECT; }
};


class RoundFrameRectElement : public RoundRectElement
{
	C2DRoundFrameRect *m_pRoundFrameRect;

public:

	RoundFrameRectElement( const SRect& non_scaled_rect, float fScale, float corner_radius );

	~RoundFrameRectElement();

	int GetElementType() const { return TYPE_ROUNDFRAMERECT; }

	void SetFrameWidth( int width );
};


class TriangleElement : public PrimitiveElement
{
protected:

	/// non-scaled vertex positions in local coord
	Vector2 m_avVertexPosition[3];

	SFloatRGBAColor m_CornerColors[3];

public:

	TriangleElement( const SRect& non_scaled_rect, float fScale );

	virtual ~TriangleElement() {}

	virtual void Draw();

	virtual void SetVertexPosition( int index, const Vector2& vPos );

	void UpdatePositionsInternal( const Matrix23& global_transform );

	void SetCornerColor( unsigned int corner_index, const SFloatRGBAColor& color ) { m_CornerColors[corner_index] = color; }

	bool HasDifferentCornerColors() const
	{
		for( int i=0; i<3; i++ )
		{
			if( m_CornerColors[i] != SFloatRGBAColor::White() )
				return true;
		}

		return false;
	}
};


class FillTriangleElement : public TriangleElement
{
	C2DTriangle *m_pFillTriangle;

private:

	void UpdatePositionsInternal( const Matrix23& global_transform );

public:

//	FillTriangleElement( const SFloatRGBAColor& color0, C2DPrimitive *pPrimitive, const SRect& non_scaled_rect );
	FillTriangleElement( C2DTriangle::Direction dir, const SRect& non_scaled_rect, float fScale );

	FillTriangleElement( const Vector2& v0, const Vector2& v1, const Vector2& v2, const SRect& non_scaled_rect, float fScale );

	~FillTriangleElement();

	virtual int GetElementType() const { return TYPE_FILLTRIANGLE; }
};


class FrameTriangleElement : public TriangleElement
{
	C2DFrameTriangle *m_pFrameTriangle;

private:

	void UpdatePositionsInternal( const Matrix23& global_transform );

public:

//	FrameTriangleElement( const SFloatRGBAColor& color0, C2DPrimitive *pPrimitive, const SRect& non_scaled_rect );
	FrameTriangleElement( C2DTriangle::Direction dir, const SRect& non_scaled_rect, float fScale );

	~FrameTriangleElement();

	virtual int GetElementType() const { return TYPE_FRAMETRIANGLE; }
};


class CombinedPrimitiveElement : public GraphicsElement// GraphicsElementGroup
{
	/// Set by  derived classes
	std::shared_ptr<PrimitiveElement> m_pFillElement;
	std::shared_ptr<PrimitiveElement> m_pFrameElement;

public:

	CombinedPrimitiveElement() {}

	CombinedPrimitiveElement( const SRect& non_scaled_rect, float fScale, std::shared_ptr<PrimitiveElement> pFill, std::shared_ptr<PrimitiveElement> pFrame )
		:
	m_pFillElement(pFill),
	m_pFrameElement(pFrame)
	{
		m_fScale = fScale;
		m_LocalAABB.vMin = Vector2( (float)non_scaled_rect.left,  (float)non_scaled_rect.top );
		m_LocalAABB.vMax = Vector2( (float)non_scaled_rect.right, (float)non_scaled_rect.bottom );
	}

	virtual ~CombinedPrimitiveElement() {}

	virtual void Release();

	bool BelongsToLayer() const { return false; }

	void SetTopLeftPosInternal( Vector2 vPos )
	{
		if( m_pFillElement )
			m_pFillElement->SetTopLeftPosInternal( vPos );
		if( m_pFrameElement )
			m_pFrameElement->SetTopLeftPosInternal( vPos );
	}

	void ChangeScale( float scale )
	{
		m_fScale = scale;

		if( m_pFillElement )
			m_pFillElement->ChangeScale( scale );
		if( m_pFrameElement )
			m_pFrameElement->ChangeScale( scale );
	}

	void SetColor( int color_index, const SFloatRGBAColor& color )
	{
		if( m_pFillElement )
			m_pFillElement->SetColor( color_index, color );
		if( m_pFrameElement )
			m_pFrameElement->SetColor( color_index, color );
	}

	void SetAlpha( int color_index, float a )
	{
		if( m_pFillElement )
			m_pFillElement->SetAlpha( color_index, a );
		if( m_pFrameElement )
			m_pFrameElement->SetAlpha( color_index, a );
	}

	/// TODO: Automatically calculate appropriate size of the fill element
	void SetFrameWidth( float width );

	void SetLocalTopLeftPos( Vector2 vPos )
	{
		if( m_pFillElement )
			m_pFillElement->SetLocalTopLeftPos( vPos );
		if( m_pFrameElement )
			m_pFrameElement->SetLocalTopLeftPos( vPos );
	}

	void UpdateTransform( const Matrix23& parent_transform )
	{
		if( m_pFillElement )
			m_pFillElement->UpdateTransform( parent_transform );
		if( m_pFrameElement )
			m_pFrameElement->UpdateTransform( parent_transform );
	}

	/// place the element in a layer
	void SetLayer( int layer_index )
	{
		if( m_pFillElement )
			m_pFillElement->SetLayer( layer_index );
		if( m_pFrameElement )
			m_pFrameElement->SetLayer( layer_index );
	}

	std::shared_ptr<PrimitiveElement>& GetFillElement() { return m_pFillElement; }
	std::shared_ptr<PrimitiveElement>& GetFrameElement() { return m_pFrameElement; }

	void OnRemovalRequested();
};


class CombinedRectElement : public CombinedPrimitiveElement
{
	std::shared_ptr<FillRectElement> m_pFillRectElement;
	std::shared_ptr<FrameRectElement> m_pFrameRectElement;

public:

	CombinedRectElement() {}

	CombinedRectElement( const SRect& non_scaled_rect, float fScale, std::shared_ptr<FillRectElement> pFill, std::shared_ptr<FrameRectElement> pFrame )
		:
	CombinedPrimitiveElement(non_scaled_rect,fScale,pFill,pFrame),
	m_pFillRectElement(pFill),
	m_pFrameRectElement(pFrame)
	{
	}

	int GetElementType() const { return TYPE_COMBINEDRECT; }

	virtual void Release()
	{
		CombinedPrimitiveElement::Release();

		m_pFillRectElement.reset();
		m_pFrameRectElement.reset();
	}

	void Draw() {}

	std::shared_ptr<FillRectElement> GetFillRectElement() { return m_pFillRectElement; }
	std::shared_ptr<FrameRectElement> GetFrameRectElement() { return m_pFrameRectElement; }
};


class CombinedRoundRectElement : public CombinedPrimitiveElement
{
	std::shared_ptr<RoundFillRectElement> m_pRoundFillRectElement;
	std::shared_ptr<RoundFrameRectElement> m_pRoundFrameRectElement;

public:

	CombinedRoundRectElement() {}

	CombinedRoundRectElement( const SRect& non_scaled_rect, float fScale, std::shared_ptr<RoundFillRectElement> pFill, std::shared_ptr<RoundFrameRectElement> pFrame )
		:
	CombinedPrimitiveElement(non_scaled_rect,fScale,pFill,pFrame),
	m_pRoundFillRectElement(pFill),
	m_pRoundFrameRectElement(pFrame)
	{
	}

	int GetElementType() const { return TYPE_COMBINEDROUNDRECT; }

	virtual void Release()
	{
		CombinedPrimitiveElement::Release();

		m_pRoundFillRectElement.reset();
		m_pRoundFrameRectElement.reset();
	}

	void Draw() {}

	std::shared_ptr<RoundFillRectElement> GetRoundFillRectElement() { return m_pRoundFillRectElement; }
	std::shared_ptr<RoundFrameRectElement> GetRoundFrameRectElement() { return m_pRoundFrameRectElement; }
};


class CombinedTriangleElement : public CombinedPrimitiveElement
{
	std::shared_ptr<FillTriangleElement> m_pFillTriangleElement;
	std::shared_ptr<FrameTriangleElement> m_pFrameTriangleElement;

public:

	CombinedTriangleElement() {}

	CombinedTriangleElement( const SRect& non_scaled_rect, float fScale, std::shared_ptr<FillTriangleElement> pFill, std::shared_ptr<FrameTriangleElement> pFrame )
		:
	CombinedPrimitiveElement(non_scaled_rect,fScale,pFill,pFrame),
	m_pFillTriangleElement(pFill),
	m_pFrameTriangleElement(pFrame)
	{
	}

	virtual int GetElementType() const { return TYPE_COMBINEDTRIANGLE; }

	virtual void Release()
	{
		CombinedPrimitiveElement::Release();

		m_pFillTriangleElement.reset();
		m_pFrameTriangleElement.reset();
	}

	void Draw() {}

	std::shared_ptr<FillTriangleElement> GetFillTriangleElement() { return m_pFillTriangleElement; }
	std::shared_ptr<FrameTriangleElement> GetFrameTriangleElement() { return m_pFrameTriangleElement; }
};


class PolygonElement : public PrimitiveElement
{
public:

	virtual ~PolygonElement() {}

	virtual void SetVertexColor( int vertex, int color_index, const SFloatRGBAColor& color ) {}
};


class FillPolygonElement : public PolygonElement
{
	/// Only the regular polygon is supported
	C2DRegularPolygon *m_pRegularPolygon;

public:

	FillPolygonElement( const SFloatRGBAColor& color0, const SRect& non_scaled_rect );

	FillPolygonElement( int num_polygon_vertices, Vector2 vCenter, int radius, CRegularPolygonStyle::Name style, float fScale );

	~FillPolygonElement();

	virtual int GetElementType() const { return TYPE_FILLPOLYGON; }

	virtual void Draw();

	/// change radius at each vertex
	/// - Use this to draw cobweb charts
	void SetRadius( int vertex, int radius );

	Vector2 GetVertexPos( int vertex );

	void SetVertexColor( int vertex, int color_index, const SFloatRGBAColor& color );
};


class FramePolygonElement : public PolygonElement
{
//	C2DFramePolygon *m_pFramePolygon;

public:

	~FramePolygonElement() { /*SafeDelete( m_pFramePolygon );*/ }

	virtual int GetElementType() const { return TYPE_FRAMEPOLYGON; }
};



/*
class CCircleElement : public GraphicsElement
{
//	C2DCircle *m_pCircle; ???
	// or
//	C2DEllipse *m_pEllipse; ???

public:

	/// \param pPrimitive - owned reference
	CCircleElement( C2DPrimitive *pPrimitive, const SFloatRGBAColor& color0 );

	virtual ~CGE_Circle() { SafeDelete( m_pPrimitive ); }

	virtual void Draw();

	virtual void SetTopLeftPosInternal( Vector2 vPos )
	{
	}

	virtual void ChangeScale( float scale )
	{
	}

	virtual void SetSizeLTRB( const Vector2& vLocalMin, const Vector2& vLocalMax )
	{
	}

	virtual void SetTextureCoord( const TEXCOORD2& vMin, const TEXCOORD2& vMax ) { m_pPrimitive->SetTextureUV( vMin, vMax ); }

	virtual int GetElementType() const { return TYPE_???; }

	/// not available for C2DRoundRect
	void SetCornerColor( int corner_index, SFloatRGBAColor& color ) { m_aCornerColor[corner_index] = color; }

}
*/



class TextElement : public GraphicsElement
{
	int m_FontID;

	std::string m_Text;

	/// top left corner pos of the text
	Vector2 m_vTextPos;

	Vector2 m_vScaledPos;

	Vector2 m_vLocalTextOffset;

	/// scaled font size. when set to 0, size of the font obtained by 'font_id' is used
	int m_ScaledWidth, m_ScaledHeight;

	/// non-scaled font size
	int m_FontWidth, m_FontHeight;

	int m_TextAlignH;
	int m_TextAlignV;

	AlphaBlend::Mode m_DestAlphaBlendMode;

protected:

	void UpdatePositionsInternal( const Matrix23& global_transform );

	void UpdatePositionsInternalForNonRotatedElement( const Vector2& vGlobalTranslation );

	void UpdateLocalTextOffset();

public:

	enum eTextAlignment { TAL_LEFT, TAL_TOP, TAL_CENTER, TAL_RIGHT, TAL_BOTTOM, NUM_TEXT_ALIGNMENTS };

	TextElement() {}

	TextElement( int font_id, const std::string& text, const AABB2& textbox, int align_h, int align_v, const SFloatRGBAColor& color0 )
		: m_FontID(font_id), m_Text(text), m_vTextPos(textbox.vMin),
		m_vLocalTextOffset(Vector2(0,0)),
		m_DestAlphaBlendMode(AlphaBlend::InvSrcAlpha)
	{
		m_LocalAABB = textbox;
		m_TextAlignH = align_h;
		m_TextAlignV = align_v;
		m_aColor[0] = color0;
		ChangeScale( m_fScale );
	}

//	TextElement( int font_id, const std::string& text, float x, float y, const SFloatRGBAColor& color0 );

	virtual int GetElementType() const { return TYPE_TEXT; }

	void Release();

	/**
	 draws text
	 - text color set for this text and not restored
	 - text font size - set for this text if the font size has been specified explicitly
	   and the original font size is restored after draw
	 */
	virtual void Draw();

	virtual void SetTopLeftPosInternal( Vector2 vPos )
	{
		UpdateTextAlignment();
	}

	virtual void ChangeScale( float scale )
	{
		m_fScale = scale;
		m_vScaledPos = m_vTextPos * scale;
		m_ScaledWidth = (int)(m_FontWidth * scale);
		m_ScaledHeight = (int)(m_FontHeight * scale);
	}

//	virtual void SetSizeLTRB( const Vector2& vLocalMin, const Vector2& vLocalMax );

	void SetDestAlphaBlendMode( AlphaBlend::Mode mode ) { m_DestAlphaBlendMode = mode; }

	void SetFontID( int font_id ) { m_FontID = font_id; }
	
	const std::string& GetText() const { return m_Text; }

	/// Returns a copy of the text
	/// - Exposed to Python as a function 'GetText'
	std::string GetTextCopy() { return m_Text; }

	void SetText( const std::string& text )
	{
		m_Text = text;
		UpdateTextAlignment();
	}

	void SetFontSize( int w, int h ) { m_FontWidth = w; m_FontHeight = h; }

	/// \param horizontal_alignment TextElement::TAL_LEFT, TAL_CENTER or TAL_RIGHT
	/// \param vertical_alignment TextElement::TAL_TOP, TAL_CENTER or TAL_BOTTOM
	void SetTextAlignment( int horizontal_alignment, int vertical_alignment );

	void UpdateTextAlignment();
};


class GraphicsElementGroup : public GraphicsElement
{
	std::vector< std::shared_ptr<GraphicsElement> > m_vecpElement;

	/// origin of local top-left positions which are grouped by the group element.
	/// - Represented in global screen coordinates
//	Vector2 m_vLocalOrigin;

private:

	inline void RemoveInvalidElements();

	inline void UpdateAABB();

	void UpdateTransform( const Matrix23& parent_transform );

public:

//	GraphicsElementGroup( std::vector< std::shared_ptr<GraphicsElement> >& rvecpElement );

	/// Calculates the local top-left positions of the specified graphics elements from vLocalOrigin
	/// \param vLocalOrigin local origin in global screen coordinates
	GraphicsElementGroup( std::vector< std::shared_ptr<GraphicsElement> >& rvecpElement, Vector2 vLocalOrigin );

	virtual ~GraphicsElementGroup();

	virtual int GetElementType() const { return TYPE_GROUP; }

	bool BelongsToLayer() const { return false; }

	virtual void Draw();

	Vector2 GetTopLeftPos() const { return GetLocalOriginInGlobalCoord(); }

	/// 12:31 2009/07/09 Made virtual since CombinedPrimitiveElement needs to override this.
	/// 13:19 2009/07/09 Reverted. CombinedPrimitiveElement does not inherit GraphicsElementGroup
	void SetLocalTopLeftPos( Vector2 vPos );

	void SetLocalTopLeftPos( SPoint pos ) { SetLocalTopLeftPos( Vector2((float)pos.x,(float)pos.y) ); }

	Vector2 GetLocalOriginInGlobalCoord() const;

	/// Updates the global coordinates of grouped elements in this call or later?
	/// - in this call: not good for performance
	/// - later: you need to set a flag such as 'm_bNeedToUpdateGlobalPositionsOfGroupedElements' to true.
	///   Later, before the rendering begins, update global positions of the grouped elements if the flag is true
	/// NOTE: Called from ctor
	void SetLocalOrigin( Vector2 vLocalOrigin );

	void SetLocalOrigin( SPoint local_origin ) { SetLocalOrigin( Vector2((float)local_origin.x,(float)local_origin.y) ); }

	virtual void SetTopLeftPosInternal( Vector2 vPos );

	virtual void ChangeScale( float scale );

	virtual void ChangeElementScale( float scale );

	virtual void SetSizeLTRB( const Vector2& vMin, const Vector2& vMax );

	virtual void SetColor( int color_index, const SFloatRGBAColor& color );

	virtual void SetAlpha( int color_index, float a );

	void SetDestAlphaBlendMode( AlphaBlend::Mode mode );

	inline void RemoveElementFromGroup( std::shared_ptr<GraphicsElement> pElement );

	std::vector< std::shared_ptr<GraphicsElement> >& GetElementBuffer() { return m_vecpElement; }
};


inline void GraphicsElementGroup::UpdateAABB()
{
	m_LocalAABB.Nullify();
	std::vector< std::shared_ptr<GraphicsElement> >::iterator itr;
	for( itr = m_vecpElement.begin(); itr != m_vecpElement.end(); itr++ )
	{
		m_LocalAABB.MergeAABB( (*itr)->GetAABB() );
	}
}


inline void GraphicsElementGroup::RemoveElementFromGroup( std::shared_ptr<GraphicsElement> pElement )
{
	// remove the arg element from this group
	// - does not release the element
	std::vector< std::shared_ptr<GraphicsElement> >::iterator itr = std::find( m_vecpElement.begin(), m_vecpElement.end(), pElement );
	if( itr != m_vecpElement.end() )
		m_vecpElement.erase( itr );
	else
		LOG_PRINT_ERROR( "Failed to find a graphics element in the list of its owner group element." );
}

// provides callback mechanism invoked when an element is created / destroyed
// - used by animated graphics manager to remove effects when the targt element is removed
class GraphicsElementManagerCallback
{
public:

	GraphicsElementManagerCallback() {}
	virtual ~GraphicsElementManagerCallback() {}

	virtual void OnCreated( std::shared_ptr<GraphicsElement> pElement ) {}

	/// called right before the element is released in GraphicsElementManager::RemoveElement()
	virtual void OnDestroyed( std::shared_ptr<GraphicsElement> pElement ) {}
};


/*
class FrameRectElement: public GraphicsElement
{
	C2DFrameRect m_FrameRect; ///< scaled rect with scaled border width

public:

	FrameRectElement( const C2DFrameRect& framerect, const SFloatRGBAColor& color0 )
		: m_FrameRect(framerect)
	{
		m_LocalAABB = AABB2(framerect.GetCornerPos2D(0),framerect.GetCornerPos2D(2));
		m_OrigBorderWidth = framerect.GetBorderWidth();
		m_aColor[0] = color0;
		ChangeScale( m_fScale );
	}

//	virtual void Draw();
	virtual void Draw()
	{
		SFloatRGBAColor blended_color = GetBlendedColor();
		m_FrameRect.SetColor( blended_color.GetARGB32() );

		m_FrameRect.Draw();
	}

	virtual void SetTopLeftPosInternal( Vector2 vPos )
	{
		Vector2 vSpan = m_LocalAABB.vMax - m_LocalAABB.vMin;
		m_LocalAABB.vMin = vPos;
		m_LocalAABB.vMax = vPos + vSpan;
		m_FrameRect.SetPosition( m_LocalAABB.vMin * m_fScale, m_LocalAABB.vMax * m_fScale );
	}

	virtual void ChangeScale( float scale )
	{
		m_fScale = scale;
		m_FrameRect.SetPosition( m_LocalAABB.vMin * scale, m_LocalAABB.vMax * scale );
		m_FrameRect.SetBorderWidth( (int)(m_OrigBorderWidth * scale) );
	}

	/// does not change border width
	virtual void SetSizeLTRB( const Vector2& vMin, const Vector2& vMax )
	{
		m_LocalAABB.vMin = vMin;
		m_LocalAABB.vMax = vMax;
		m_FrameRect.SetPosition( m_LocalAABB.vMin * m_fScale, m_LocalAABB.vMax * m_fScale );
	}

	virtual int GetElementType() const { return TYPE_FRAMERECT; }
};
*/

} // namespace amorphous



#endif  /* __GraphicsElements_H__ */
