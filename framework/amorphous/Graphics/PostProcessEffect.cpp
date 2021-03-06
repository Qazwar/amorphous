#include "PostProcessEffect.hpp"
#include "PostProcessEffectManager.hpp"
#include "amorphous/3DMath/Gaussian.hpp"
#include "amorphous/Graphics/SurfaceFormat.hpp"
//#include "amorphous/Graphics/Direct3D/Direct3D9.hpp"
//#include "amorphous/Graphics/Direct3D/D3DSurfaceFormat.hpp"
#include "amorphous/Graphics/GraphicsResourceDescs.hpp"
#include "amorphous/Graphics/2DPrimitive/2DRect.hpp"
#include "amorphous/Graphics/TextureRenderTarget.hpp"
#include "amorphous/Graphics/Shader/ShaderManager.hpp"
#include "amorphous/Graphics/TextureGenerators/SingleColorTextureGenerator.hpp"
#include "amorphous/Support/Log/DefaultLog.hpp"
#include "amorphous/Graphics/OpenGL/GLGraphicsDevice.hpp"


namespace amorphous
{


using namespace std;


//#define V(x) { hr = x; if( FAILED(hr) ) { LOG_PRINT_ERROR( string(#x) + " failed." ); } }
//#define V_RETURN(x) { hr = x; if(FAILED(hr)) return hr; }


// Texture coordinate rectangle
struct CoordRect
{
    float fLeftU, fTopV;
    float fRightU, fBottomV;
};


//const D3DSURFACE_DESC *GetD3D9BackBufferSurfaceDesc()
//{
//	IDirect3DDevice9* pd3dDevice = DIRECT3D9.GetDevice();
//
//	// retrieve the back buffer size
//	static D3DSURFACE_DESC s_back_buffer_desc;
//	IDirect3DSurface9 *pBackBuffer;
//	pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
//	pBackBuffer->GetDesc( &s_back_buffer_desc );
//	return &s_back_buffer_desc;
//}


SRectangular GetBackBufferWidthAndHeight()
{
//	if( DIRECT3D9.GetDevice() )
//	{
//		// We are running Direct3D
//		const D3DSURFACE_DESC *pBB = GetD3D9BackBufferSurfaceDesc();
//		return SRectangular( (int)(pBB->Width), (int)(pBB->Height) );
//	}
//	else
	{
		// We are running OpenGL

//		LOG_GL_ERROR( "Calling glGetRenderbufferParameterivEXT() ..." );
		GLint width = 0;
		GLint height = 0;
//		glGetRenderbufferParameterivEXT( GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_WIDTH_EXT,  &width );
//		glGetRenderbufferParameterivEXT( GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_HEIGHT_EXT, &height );

		// The calls above return 0s (width and height), and cause GL_INVALID_OPERATION
		// if zero is bound to target, so for now let's assume that the viewport has
		// the same dimension.

		LOG_GL_ERROR( "Retrieving the viewport size..." );
		uint w=0, h=0;
		GraphicsDevice().GetViewportSize(w,h);
		width  = w;
		height = h;

		return SRectangular(width,height);
	}
}


SRectangular GetCropWidthAndHeight()
{
	SRectangular bb = GetBackBufferWidthAndHeight();
	return SRectangular( bb.width - bb.width % 8, bb.height - bb.height % 8 );
}


inline SRectangular GetCropWidthAndHeight( const SRectangular& src )
{
	return SRectangular( src.width - src.width % 8, src.height - src.height % 8 );
}


void GetTextureRect( shared_ptr<RenderTargetTextureHolder>& pSrc, SRect *pDest )
{
//	SRectangular tex_size = pSrc->m_Texture.GetSize2D(0);
	pDest->left   = 0;
	pDest->top    = 0;
	pDest->right  = pSrc->m_Desc.Width; // tex_size.width;
	pDest->bottom = pSrc->m_Desc.Height; // tex_size.height;
}


/**
 \brief Get the texture coordinate offsets to be used inside the DownScale4x4 pixel shader.
*/
Result::Name GetSampleOffsets_DownScale4x4( unsigned int width, unsigned int height, Vector2 avSampleOffsets[] )
{
    if( NULL == avSampleOffsets )
		return Result::INVALID_ARGS;

    float tU = 1.0f / (float)width;
    float tV = 1.0f / (float)height;

    // Sample from the 16 surrounding points. Since the center point will be in
    // the exact center of 16 texels, a 0.5f offset is needed to specify a texel
    // center.
    int index = 0;
    for( int y = 0; y < 4; y++ ) // y = { -1.5, -0.5, 0.5, 1.5 }
    {
        for( int x = 0; x < 4; x++ ) // x = { -1.5, -0.5, 0.5, 1.5 }
        {
            avSampleOffsets[ index ].x = ( x - 1.5f ) * tU;
            avSampleOffsets[ index ].y = ( y - 1.5f ) * tV;

            index++;
        }
    }

	return Result::SUCCESS;
}



/**
 \brief Get the texture coordinate offsets to be used inside the DownScale2x2 pixel shader.
*/
Result::Name GetSampleOffsets_DownScale2x2( unsigned int width, unsigned int height, Vector2 avSampleOffsets[] )
{
    if( NULL == avSampleOffsets )
		return Result::INVALID_ARGS;

    float tU = 1.0f / (float)width;
    float tV = 1.0f / (float)height;

    // Sample from the 4 surrounding points. Since the center point will be in
    // the exact center of 4 texels, a 0.5f offset is needed to specify a texel
    // center.
    int index = 0;
    for( int y = 0; y < 2; y++ )
    {
        for( int x = 0; x < 2; x++ )
        {
            avSampleOffsets[ index ].x = ( x - 0.5f ) * tU;
            avSampleOffsets[ index ].y = ( y - 0.5f ) * tV;

            index++;
        }
    }

	return Result::SUCCESS;
}


/**
 Draw a properly aligned quad covering the entire render target.
 Calculates the size of the rect to draw from the surface
 of the current render target.
*/
/*void DrawFullScreenQuad( float fLeftU, float fTopV, float fRightU, float fBottomV )
{
	IDirect3DDevice9* pd3dDevice = DIRECT3D9.GetDevice();

//	D3DSURFACE_DESC dtdsdRT;
//	PDIRECT3DSURFACE9 pSurfRT;
	HRESULT hr = S_OK;

	// Acquire render target width and height
	// Commented out: changed to retrieve the dimensions from viewports
	// beacuse OpenGL does not have a GetRenderTarget() equivalent,
	// but both Direct3D and OpenGL have APIs to retrieve viewport dimensions.
//	hr = pd3dDevice->GetRenderTarget( 0, &pSurfRT );
//	hr = pSurfRT->GetDesc( &dtdsdRT );
//	ULONG ref_count = pSurfRT->Release();

	Viewport vp;
	GraphicsDevice().GetViewport( vp );

	// Ensure that we're directly mapping texels to pixels by offset by 0.5
	// For more info see the doc page titled "Directly Mapping Texels to Pixels"
//	float fWidth5 = ( float )dtdsdRT.Width - 0.5f;
//	float fHeight5 = ( float )dtdsdRT.Height - 0.5f;
	float fWidth5  = (float)vp.Width  - 0.5f;
	float fHeight5 = (float)vp.Height - 0.5f;

	// Draw the quad
	ScreenVertex svQuad[4];

	svQuad[0].p = Vector4( -0.5f, -0.5f, 0.5f, 1.0f );
	svQuad[0].t = TEXCOORD2( fLeftU, fTopV );

	svQuad[1].p = Vector4( fWidth5, -0.5f, 0.5f, 1.0f );
	svQuad[1].t = TEXCOORD2( fRightU, fTopV );

	svQuad[2].p = Vector4( -0.5f, fHeight5, 0.5f, 1.0f );
	svQuad[2].t = TEXCOORD2( fLeftU, fBottomV );

	svQuad[3].p = Vector4( fWidth5, fHeight5, 0.5f, 1.0f );
	svQuad[3].t = TEXCOORD2( fRightU, fBottomV );

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );

	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	hr = pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	hr = pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	hr = pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	hr = pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	hr = pd3dDevice->SetFVF( ScreenVertex::FVF );
	hr = pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, svQuad, sizeof( ScreenVertex ) );
	hr = pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
}
*/

void DrawFullScreenQuad( ShaderManager& shader_mgr, float fLeftU, float fTopV, float fRightU, float fBottomV )
{
	Viewport vp;
	GraphicsDevice().GetViewport( vp );

	// Ensure that we're directly mapping texels to pixels by offset by 0.5
	// For more info see the doc page titled "Directly Mapping Texels to Pixels"
	float fWidth5  = (float)vp.Width  - 0.5f;
	float fHeight5 = (float)vp.Height - 0.5f;

	shader_mgr.SetParam( "g_ViewportWidth",  (float)vp.Width );
	shader_mgr.SetParam( "g_ViewportHeight", (float)vp.Height );

	// TODO: support vertices with different elements, such as ScreenVertex
	// defined in this file.
	// C2DRect includes elements unnecessary for drawing fullscreen rects
	// in order to render post process effects.
	C2DRect quad;
	quad.SetPositionLTRB( -0.5f, -0.5f, fWidth5, fHeight5 );
	quad.SetTextureUV( TEXCOORD2( fLeftU, fTopV ), TEXCOORD2( fRightU, fBottomV ) );
	quad.SetZDepth( 0.5f );

	// shader_mgr should have texture blend settings suitable for drawing the specified rects

	GraphicsDevice().Disable( RenderStateType::ALPHA_BLEND );
	GraphicsDevice().Disable( RenderStateType::ALPHA_TEST );
//	hr = pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	GraphicsDevice().Enable( RenderStateType::FACE_CULLING );
	GraphicsDevice().SetCullingMode( CullingMode::COUNTERCLOCKWISE );
	GraphicsDevice().Disable( RenderStateType::DEPTH_TEST );

	Get2DPrimitiveRenderer().RenderRect( shader_mgr, quad );

	GraphicsDevice().Enable( RenderStateType::DEPTH_TEST );
}


void RenderFullScreenQuad( ShaderManager& shader_mgr, const CoordRect& c )
{
	DrawFullScreenQuad( shader_mgr, c.fLeftU, c.fTopV, c.fRightU, c.fBottomV );
}


void RenderFullScreenQuad( ShaderManager& shader_mgr, float fLeftU, float fTopV, float fRightU, float fBottomV )
{
	DrawFullScreenQuad( shader_mgr, fLeftU, fTopV, fRightU, fBottomV );
}


/**
 Get the texture coordinate offsets to be used inside the GaussBlur5x5 pixel shader.
*/
Result::Name GetSampleOffsets_GaussBlur5x5( unsigned int tex_width,
                                       unsigned int tex_height,
                                       Vector2* avTexCoordOffset,
                                       Vector4* avSampleWeight,
									   float standard_deviation = 1.0f,
                                       float fMultiplier = 1.0f )
{
    float tu = 1.0f / ( float )tex_width;
    float tv = 1.0f / ( float )tex_height;

    Vector4 vWhite( 1.0f, 1.0f, 1.0f, 1.0f );

    float totalWeight = 0.0f;
    int index = 0;
    for( int x = -2; x <= 2; x++ )
    {
        for( int y = -2; y <= 2; y++ )
        {
            // Exclude pixels with a block distance greater than 2. This will
            // create a kernel which approximates a 5x5 kernel using only 13
            // sample points instead of 25; this is necessary since 2.0 shaders
            // only support 16 texture grabs.
            if( abs( x ) + abs( y ) > 2 )
                continue;

            // Get the unscaled Gaussian intensity for this offset
            avTexCoordOffset[index] = Vector2( x * tu, y * tv );
            avSampleWeight[index] = vWhite * GaussianDistribution( ( float )x, ( float )y, standard_deviation );
            totalWeight += avSampleWeight[index].x;

            index++;
        }
    }

    // Divide the current weight by the total weight of all the samples; Gaussian
    // blur kernels add to 1.0f to ensure that the intensity of the image isn't
    // changed when the blur occurs. An optional multiplier variable is used to
    // add or remove image intensity during the blur.
    for( int i = 0; i < index; i++ )
    {
        avSampleWeight[i] /= totalWeight;
        avSampleWeight[i] *= fMultiplier;
    }

    return Result::SUCCESS;
}


/**
 Get the texture coordinate offsets to be used inside the Bloom pixel shader.
 By default, weights stored in avColorWeight are
  [0]  0.26596150
  [1]  0.25158879
  [2]  0.21296531
  [3]  0.16131380
  [4]  0.10934003
  [5]  0.066318087
  [6]  0.035993975
  [7]  0.017481256
  [8]  0.25158879
  [9]  0.21296531
  [10] 0.16131380
  [11] 0.10934003
  [12] 0.066318087
  [13] 0.035993975
  [14] 0.017481256
  [15] 0.00000000
  sum: approx. 1.976
*/
Result::Name GetSampleOffsets_Bloom( unsigned int dwD3DTexSize,
							   float afTexCoordOffset[15],
							   Vector4* avColorWeight,
							   float fDeviation,
							   float fMultiplier )
{
	int i = 0;
	float tu = 1.0f / ( float )dwD3DTexSize;

	// Fill the center texel
	float weight = fMultiplier * GaussianDistribution( 0, 0, fDeviation );
	avColorWeight[0] = Vector4( weight, weight, weight, 1.0f );

	afTexCoordOffset[0] = 0.0f;

	// Fill the first half
	for( i = 1; i < 8; i++ )
	{
		// Get the Gaussian intensity for this offset
		weight = fMultiplier * GaussianDistribution( ( float )i, 0, fDeviation );
		afTexCoordOffset[i] = i * tu;

		avColorWeight[i] = Vector4( weight, weight, weight, 1.0f );
	}

	// Mirror to the second half
	for( i = 8; i < 15; i++ )
	{
		avColorWeight[i] = avColorWeight[i - 7];
		afTexCoordOffset[i] = -afTexCoordOffset[i - 7];
	}

	return Result::SUCCESS;
}


/**
 Get the texture coordinate offsets to be used inside the Bloom pixel shader.
*/
Result::Name GetSampleOffsets_Star( unsigned int dwD3DTexSize,
							  float afTexCoordOffset[15],
							  Vector4* avColorWeight,
							  float fDeviation )
{
	int i = 0;
	float tu = 1.0f / ( float )dwD3DTexSize;

	// Fill the center texel
	float weight = 1.0f * GaussianDistribution( 0, 0, fDeviation );
	avColorWeight[0] = Vector4( weight, weight, weight, 1.0f );

	afTexCoordOffset[0] = 0.0f;

	// Fill the first half
	for( i = 1; i < 8; i++ )
	{
		// Get the Gaussian intensity for this offset
		weight = 1.0f * GaussianDistribution( ( float )i, 0, fDeviation );
		afTexCoordOffset[i] = i * tu;

		avColorWeight[i] = Vector4( weight, weight, weight, 1.0f );
	}

	// Mirror to the second half
	for( i = 8; i < 15; i++ )
	{
		avColorWeight[i] = avColorWeight[i - 7];
		afTexCoordOffset[i] = -afTexCoordOffset[i - 7];
	}

	return Result::SUCCESS;
}


/**
 Get the texture coordinates to use when rendering into the destination
 texture, given the source and destination rectangles
*/
Result::Name GetTextureCoords(
	TextureResourceDesc& tex_src,  const SRect* pRectSrc,
	TextureResourceDesc& tex_dest, const SRect* pRectDest, CoordRect* pCoords
	)
{
	float tU, tV;

	// Validate arguments
	if( pCoords == NULL )
		return Result::INVALID_ARGS;

	// Start with a default mapping of the complete source surface to complete 
	// destination surface
	pCoords->fLeftU = 0.0f;
	pCoords->fTopV = 0.0f;
	pCoords->fRightU = 1.0f;
	pCoords->fBottomV = 1.0f;

	// If not using the complete source surface, adjust the coordinates
	if( pRectSrc != NULL )
	{
		// Get destination texture description
//		const SRectangular src_tex_size = tex_src.GetSize2D( 0 );

		if( tex_src.Width == 0 || tex_src.Height )
			return Result::UNKNOWN_ERROR;

		// These delta values are the distance between source texel centers in 
		// texture address space
		tU = 1.0f / tex_src.Width;
		tV = 1.0f / tex_src.Height;

		pCoords->fLeftU += pRectSrc->left * tU;
		pCoords->fTopV  += pRectSrc->top * tV;
		pCoords->fRightU  -= ( tex_src.Width  - pRectSrc->right ) * tU;
		pCoords->fBottomV -= ( tex_src.Height - pRectSrc->bottom ) * tV;
	}

	// If not drawing to the complete destination surface, adjust the coordinates
	if( pRectDest != NULL )
	{
		// Get source texture description
//		const SRectangular dest_tex_size = tex_dest.GetSize2D();

		if( tex_dest.Width == 0 || tex_dest.Height )
			return Result::UNKNOWN_ERROR;

		// These delta values are the distance between source texel centers in 
		// texture address space
		tU = 1.0f / tex_dest.Width;
		tV = 1.0f / tex_dest.Height;

		pCoords->fLeftU -= pRectDest->left * tU;
		pCoords->fTopV  -= pRectDest->top * tV;
		pCoords->fRightU  += ( tex_dest.Width  - pRectDest->right ) * tU;
		pCoords->fBottomV += ( tex_dest.Height - pRectDest->bottom ) * tV;
	}

	return Result::SUCCESS;
}


ShaderManager *GetShaderManager(PostProcessEffectFilter& filter)
{
	std::shared_ptr<PostProcessFilterShader> pFilterShader = filter.GetFilterShader();
	if( !pFilterShader )
		return NULL;

	return pFilterShader->GetShader().GetShaderManager();
}


//============================================================================
// DownScale4x4Filter
//============================================================================

DownScale4x4Filter::DownScale4x4Filter()
{
	m_Technique.SetTechniqueName( "DownScale4x4" );

//	m_ScalingFactor = 0.25f;

	m_MaxInputTextureIndex = 0;

	m_SetSamplerParameters[0] = 1;
	m_MagFilters[0] = TextureFilter::LINEAR;
	m_MinFilters[0] = TextureFilter::LINEAR;

	m_SetTextureWrapParameters[0] = 1;
	m_TextureWrapAxis0[0] = TextureAddressMode::CLAMP_TO_EDGE;
	m_TextureWrapAxis1[0] = TextureAddressMode::CLAMP_TO_EDGE;
}


Result::Name DownScale4x4Filter::Init( RenderTargetTextureCache& cache, FilterShaderContainer& filter_shader_container )
{
	m_pCache = cache.GetSelfPtr().lock();
	
	SetFilterShader( filter_shader_container.AddPostProcessEffectShader("down_scale_4x4") );

	return Result::SUCCESS;
}


/*
bool DownScale4x4Filter::GetRenderTarget( PostProcessEffectFilter& prev_filter, std::shared_ptr<RenderTargetTextureHolder>& pDest )
{
	int src_w = prev_filter.pDest->m_Desc.Width;
	int src_h = prev_filter.pDest->m_Desc.Height;
	int crop_width  = src_w - src_w % 8;
	int crop_height = src_h - src_h % 8;

	m_Desc.Width  = crop_width  / 4;
	m_Desc.Height = crop_height / 4;

	m_pDest = m_pCache->GetTexture( m_Desc );
	return m_pDest ? true : false;
}
*/


void DownScale4x4Filter::Render()
{
	ShaderManager *pShaderMgr = GetShaderManager(*this);
	if( !pShaderMgr )
		return;

	ShaderManager& shader_mgr = *pShaderMgr;

	Vector2 avSampleOffsets[MAX_SAMPLES];
	memset( avSampleOffsets, 0, sizeof(avSampleOffsets) );

//	const D3DSURFACE_DESC* pBackBufferDesc = GetD3D9BackBufferSurfaceDesc();
	int prev_scene_width  = m_pPrevScene->m_Desc.Width;// pBackBufferDesc->Width;
	int prev_scene_height = m_pPrevScene->m_Desc.Height;// pBackBufferDesc->Height;

//	m_Desc.Width  = prev_scene_width;
//	m_Desc.Height = prev_scene_height;
//	m_pDest = GetRenderTarget( m_Desc );
///	if( !m_pDest )
///		return;

//	DWORD dwCropWidth = ???;
//	DWORD dwCropHeight = ???;

	// Create a 1/4 x 1/4 scale copy of the HDR texture. Since bloom textures
	// are 1/8 x 1/8 scale, border texels of the HDR texture will be discarded 
	// to keep the dimensions evenly divisible by 8; this allows for precise 
	// control over sampling inside pixel shaders.
//	pEffect->SetTechnique( "DownScale4x4" ); // set in RenderBase()

	// Place the rectangle in the center of the back buffer surface
	SRect rectSrc;
//	rectSrc.left = ( pBackBufferDesc->Width  - dwCropWidth )  / 2;
//	rectSrc.top  = ( pBackBufferDesc->Height - dwCropHeight ) / 2;
//	rectSrc.right = rectSrc.left + dwCropWidth;
//	rectSrc.bottom = rectSrc.top + dwCropHeight;
//	SRect rectSrc = m_SourceRect;
	GetTextureRect( m_pPrevScene, &rectSrc );

	// rectSrc(l,t,r,b) = ( 0, 0, pBackBufferDesc->Width, pBackBufferDesc->Height ), when m_dwCropWidth and m_dwCropHeight are
	// same as those of the back buffer, which happens when back buffer width and height is divisible by 8.

	// Get the texture coordinates for the render target
	CoordRect coords;
	if( m_pDest )
	{
		GetTextureCoords( m_pPrevScene->m_Desc, &rectSrc, m_pDest->m_Desc, NULL, &coords );
	}
	else
	{
		coords.fLeftU   = 0;
		coords.fTopV    = 0;
		coords.fRightU  = 1;
		coords.fBottomV = 1;
	}

	// Get the sample offsets used within the pixel shader
	GetSampleOffsets_DownScale4x4( (unsigned int)prev_scene_width, (unsigned int)prev_scene_height, avSampleOffsets );
//	shader_mgr.SetParam( "g_avSampleOffsets", (float *)avSampleOffsets, numof(avSampleOffsets) * 2 );
	shader_mgr.SetParam( "g_avSampleOffsets", avSampleOffsets, numof(avSampleOffsets) );

//	LPDIRECT3DDEVICE9 pd3dDevice = DIRECT3D9.GetDevice();
//	HRESULT hr = S_OK;
//	hr = pd3dDevice->SetTexture( 0, m_pPrevScene->m_Texture.GetTexture() ); // done in RenderBase()
/*	hr = pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );*/
//	hr = pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
//	hr = pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	Result::Name res = GraphicsDevice().Disable( RenderStateType::ALPHA_BLEND );

	RenderFullScreenQuad( shader_mgr, coords );
}


//============================================================================
// DownScale2x2Filter
//============================================================================

DownScale2x2Filter::DownScale2x2Filter()
{
	m_Technique.SetTechniqueName( "DownScale2x2" );

//	m_ScalingFactor = 0.50f;

	m_MaxInputTextureIndex = 0;

	m_SetSamplerParameters[0] = 1;
	m_MagFilters[0] = TextureFilter::NEAREST;
	m_MinFilters[0] = TextureFilter::NEAREST;

	m_SetTextureWrapParameters[0] = 1;
	m_TextureWrapAxis0[0] = TextureAddressMode::CLAMP_TO_EDGE;
	m_TextureWrapAxis1[0] = TextureAddressMode::CLAMP_TO_EDGE;
}


Result::Name DownScale2x2Filter::Init( RenderTargetTextureCache& cache, FilterShaderContainer& filter_shader_container )
{
	m_pCache = cache.GetSelfPtr().lock();

	SetFilterShader( filter_shader_container.AddPostProcessEffectShader("down_scale_2x2") );

	return Result::SUCCESS;
}


//-----------------------------------------------------------------------------
// Name: StarSource_To_BloomSource
// Desc: Scale down m_pTexStarSource by 1/2 x 1/2 and place the result in 
//       m_pTexBloomSource
//-----------------------------------------------------------------------------
void DownScale2x2Filter::Render()
{
	ShaderManager *pShaderMgr = GetShaderManager(*this);
	if( !pShaderMgr )
		return;

	ShaderManager& shader_mgr = *pShaderMgr;

	Vector2 avSampleOffsets[MAX_SAMPLES];
	memset( avSampleOffsets, 0, sizeof(avSampleOffsets) );

	// Get the rectangle describing the sampled portion of the source texture.
	// Decrease the rectangle to adjust for the single pixel black border.
	SRect rectSrc;
	GetTextureRect( m_pPrevScene, &rectSrc );
	rectSrc.Inflate( -1, -1 );

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	SRect rectDest;
	GetTextureRect( m_pDest, &rectDest );
	rectDest.Inflate( -1, -1 );

	// Get the correct texture coordinates to apply to the rendered quad in order
	// to sample from the source rectangle and render into the destination rectangle
	CoordRect coords;
	GetTextureCoords( m_pPrevScene->m_Desc, &rectSrc, m_pDest->m_Desc, &rectDest, &coords );

	// Get the sample offsets used within the pixel shader

	// Get the width and height of the first texture in the mipmap chain.
	const uint tex_width  = (uint)m_pPrevScene->m_Desc.Width;
	const uint tex_height = (uint)m_pPrevScene->m_Desc.Height;

	GetSampleOffsets_DownScale2x2( tex_width, tex_height, avSampleOffsets );
//	shader_mgr.SetParam( "g_avSampleOffsets", (float *)avSampleOffsets, numof(avSampleOffsets) * 2 );
	shader_mgr.SetParam( "g_avSampleOffsets", avSampleOffsets, numof(avSampleOffsets) );

	// Create an exact 1/2 x 1/2 copy of the source texture
	//pEffect->SetTechnique( "DownScale2x2" );

//	pd3dDevice->SetTexture( 0, m_pTexStarSource );
	GraphicsDevice().SetScissorRect( rectDest );
	GraphicsDevice().Enable( RenderStateType::SCISSOR_TEST ); // original D3D sample
	GraphicsDevice().Disable( RenderStateType::SCISSOR_TEST );

//	LPDIRECT3DDEVICE9 pd3dDevice = DIRECT3D9.GetDevice();
//	pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
//	pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
//	pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	RenderFullScreenQuad( shader_mgr, coords );

	GraphicsDevice().Disable( RenderStateType::SCISSOR_TEST );
}



//============================================================================
// HDRBrightPassFilter
//============================================================================

HDRBrightPassFilter::HDRBrightPassFilter()
{
	m_Technique.SetTechniqueName( "BrightPassFilter" );

	m_SetSamplerParameters[0] = 1;
	m_MagFilters[0] = TextureFilter::NEAREST;
	m_MinFilters[0] = TextureFilter::NEAREST;

	m_SetSamplerParameters[1] = 1;
	m_MagFilters[1] = TextureFilter::NEAREST;
	m_MinFilters[1] = TextureFilter::NEAREST;

}


Result::Name HDRBrightPassFilter::Init( RenderTargetTextureCache& cache, FilterShaderContainer& filter_shader_container )
{
	m_pCache = cache.GetSelfPtr().lock();

	SetFilterShader( filter_shader_container.AddPostProcessEffectShader("hdr") );

	return Result::SUCCESS;
}


/// Run the bright-pass filter on source render target texture
/// and place the result in dest render target texture.
void HDRBrightPassFilter::Render()
{
	ShaderManager *pShaderMgr = GetShaderManager(*this);
	if( !pShaderMgr )
		return;

	ShaderManager& shader_mgr = *pShaderMgr;

//	HRESULT hr = S_OK;

//	D3DSURFACE_DESC desc;
//	m_pPrevScene->m_Texture.GetTexture()->GetLevelDesc( 0, &desc );

	// Get the rectangle describing the sampled portion of the source texture.
	// Decrease the rectangle to adjust for the single pixel black border.
	SRect rectSrc;
//	GetTextureRect( m_pPrevScene->m_Texture.GetTexture(), &rectSrc );
	GetTextureRect( m_pPrevScene, &rectSrc );
	rectSrc.Inflate( -1, -1 );

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	SRect rectDest;
	GetTextureRect( m_pDest, &rectDest );
	rectDest.Inflate( -1, -1 );

	// Get the correct texture coordinates to apply to the rendered quad in order 
	// to sample from the source rectangle and render into the destination rectangle
	CoordRect coords;
	GetTextureCoords( m_pPrevScene->m_Desc, &rectSrc, m_pDest->m_Desc, &rectDest, &coords );

	// The bright-pass filter removes everything from the scene except lights and
	// bright reflections
//	if( pEffect )
//		hr = pEffect->SetTechnique( "BrightPassFilter" );
	shader_mgr.SetTechnique( m_Technique );

	float fKeyValue = 3.5f;
	shader_mgr.SetParam( "g_fMiddleGray", fKeyValue );

//	IDirect3DDevice9* pd3dDevice = DIRECT3D9.GetDevice();

//	hr = pd3dDevice->SetRenderTarget( 0, m_pDest->m_pTexSurf );
//	hr = pd3dDevice->SetTexture( 0, m_pPrevScene->m_Texture.GetTexture() ); // done in RenderBase()

//	hr = pd3dDevice->SetTexture( 1, m_pAdaptedLuminanceTexture->m_Texture.GetTexture() );
///	hr = pd3dDevice->SetTexture( 1, m_pAdaptedLuminanceTexture->m_pTextureRenderTarget->GetRenderTargetTexture().GetTexture() );
	GraphicsDevice().SetTexture( 1, m_pAdaptedLuminanceTexture->m_pTextureRenderTarget->GetRenderTargetTexture() );

//	GraphicsDevice().Enable( RenderStateType::SCISSOR_TEST ); // original D3D sample
	GraphicsDevice().Disable( RenderStateType::SCISSOR_TEST );
	GraphicsDevice().SetScissorRect( rectDest );

/*	pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
*/
	RenderFullScreenQuad( shader_mgr, coords );

	GraphicsDevice().Disable( RenderStateType::SCISSOR_TEST );

	return;
}



//=======================================================================
// GaussianBlurFilter
//=======================================================================

GaussianBlurFilter::GaussianBlurFilter()
:
m_fStandardDeviation(1.0f)
{
	m_Technique.SetTechniqueName( "GaussBlur5x5" );

	m_ExtraTexelBorderWidth = 1;
	m_UseTextureSizeDivisibleBy8 = true;

	m_MaxInputTextureIndex = 0;

	m_SetSamplerParameters[0] = 1;
	m_MagFilters[0] = TextureFilter::NEAREST;
	m_MinFilters[0] = TextureFilter::NEAREST;

	m_SetTextureWrapParameters[0] = 1;
	m_TextureWrapAxis0[0] = TextureAddressMode::CLAMP_TO_EDGE;
	m_TextureWrapAxis1[0] = TextureAddressMode::CLAMP_TO_EDGE;
}


Result::Name GaussianBlurFilter::Init( RenderTargetTextureCache& cache, FilterShaderContainer& filter_shader_container )
{
	m_pCache = cache.GetSelfPtr().lock();

	SetFilterShader( filter_shader_container.AddPostProcessEffectShader("gauss_blur_5x5") );

	return Result::SUCCESS;
}


//-----------------------------------------------------------------------------
// Name: BrightPass_To_StarSource
// Desc: Perform a 5x5 gaussian blur on m_pTexBrightPass and place the result
//       in m_pTexStarSource. The bright-pass filtered image is blurred before
//       being used for star operations to avoid aliasing artifacts.
//-----------------------------------------------------------------------------
void GaussianBlurFilter::Render()
{
	ShaderManager *pShaderMgr = GetShaderManager(*this);
	if( !pShaderMgr )
		return;

	ShaderManager& shader_mgr = *pShaderMgr;

	Vector2 avSampleOffsets[MAX_SAMPLES];
	Vector4 avSampleWeights[MAX_SAMPLES];

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	SRect rectDest;
//	GetTextureRect( m_pDest->m_Texture, &rectDest );
	if( m_pDest )
		GetTextureRect( m_pDest, &rectDest );
	else
		GetTextureRect( m_pCache->m_pOrigSceneHolder, &rectDest );

	rectDest.Inflate( -1, -1 );

	// Get the correct texture coordinates to apply to the rendered quad in order 
	// to sample from the source rectangle and render into the destination rectangle
	CoordRect coords;
	if( m_pDest )
		GetTextureCoords( m_pPrevScene->m_Desc, NULL, m_pDest->m_Desc, &rectDest, &coords );
	else
	{
		coords.fLeftU = 0;
		coords.fTopV  = 0;
		coords.fRightU  = 1;
		coords.fBottomV = 1;
	}

	// Get the sample offsets used within the pixel shader

	const uint tex_width  = (uint)m_pPrevScene->m_Desc.Width;
	const uint tex_height = (uint)m_pPrevScene->m_Desc.Height;

	GetSampleOffsets_GaussBlur5x5( tex_width, tex_height, avSampleOffsets, avSampleWeights, m_fStandardDeviation );

//	shader_mgr.SetParam( "g_avSampleOffsets", (float *)avSampleOffsets, numof( avSampleOffsets ) * 2 );
//	shader_mgr.SetParam( "g_avSampleWeights", (float *)avSampleWeights, numof( avSampleWeights ) * 4 );
	shader_mgr.SetParam( "g_avSampleOffsets", avSampleOffsets, numof( avSampleOffsets ) );
	shader_mgr.SetParam( "g_avSampleWeights", avSampleWeights, numof( avSampleWeights ) );

	// The gaussian blur smooths out rough edges to avoid aliasing effects
	// when the star effect is run

//	GraphicsDevice().SetScissorRect( rectDest );
//	GraphicsDevice().Enable( RenderStateType::SCISSOR_TEST ); // original D3D sample
	GraphicsDevice().Disable( RenderStateType::SCISSOR_TEST );

//	pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
//	pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
//	pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	RenderFullScreenQuad( shader_mgr, coords );

	GraphicsDevice().Disable( RenderStateType::SCISSOR_TEST );

	return;
}



//=======================================================================
// BloomFilter
//=======================================================================

BloomFilter::BloomFilter()
{
	m_Technique.SetTechniqueName( "Bloom" );

	memset( m_avSampleOffsets, 0, sizeof( m_avSampleOffsets ) );
	memset( m_avSampleWeights, 0, sizeof( m_avSampleWeights ) );

	m_DoScissorTesting = false;

	// default value for deviation
	// - used when the filter is used for bloom in HDR lighting
	// - full screen blur filter may set different values
	m_fDeviation = 3.0f;

	m_fBloomFactor = 1.0f;

	m_UseForBlurFilter = false;

	// Shouldn't these be LINEAR instead of NEAREST
	m_SetSamplerParameters[0] = 1;
	m_MaxInputTextureIndex = 0;
	m_MagFilters[0] = TextureFilter::LINEAR;
	m_MinFilters[0] = TextureFilter::LINEAR;
}


Result::Name BloomFilter::Init( RenderTargetTextureCache& cache, FilterShaderContainer& filter_shader_container )
{
	m_pCache = cache.GetSelfPtr().lock();

	SetFilterShader( filter_shader_container.AddPostProcessEffectShader("bloom") );

	return Result::SUCCESS;
}


void BloomFilter::Render()
{
	ShaderManager *pShaderMgr = GetShaderManager(*this);
	if( !pShaderMgr )
		return;

	ShaderManager& shader_mgr = *pShaderMgr;

	GetSampleOffsets();

	if( m_UseForBlurFilter )
	{
		// need to normalize weight
		float total_weight = 0.0f;
		for( int i = 0; i < MAX_SAMPLES; i++ )
			total_weight += m_avSampleWeights[i].x;

		for( int i = 0; i < MAX_SAMPLES; i++ )
			m_avSampleWeights[i] /= total_weight;
	}

//	pEffect->SetTechnique( "Bloom" );
//	shader_mgr.SetParam( "g_avSampleOffsets", (float *)m_avSampleOffsets, numof( m_avSampleOffsets ) * 2 );
//	shader_mgr.SetParam( "g_avSampleWeights", (float *)m_avSampleWeights, numof( m_avSampleWeights ) * 4 );
	shader_mgr.SetParam( "g_avSampleOffsets", m_avSampleOffsets, numof( m_avSampleOffsets ) );
	shader_mgr.SetParam( "g_avSampleWeights", m_avSampleWeights, numof( m_avSampleWeights ) );

	// blur filter  -> set to 1/16
	// bloom filter -> set to 1
//	hr = pEffect->SetFloat( "g_fBloomFactor", m_fBloomFactor );
	shader_mgr.SetParam( "g_fBloomFactor", m_fBloomFactor );

//	pd3dDevice->SetRenderTarget( 0, pSurfTempBloom );
//	pd3dDevice->SetTexture( 0, m_apTexBloom[2] );

	// horizontal & vertical sets the tex coords differently
	// because the latter writes the scene texture that has not extra border texels

	SRect rectSrc;
	GetTextureRect( m_pPrevScene, &rectSrc );
	rectSrc.Inflate( -1, -1 );

	CoordRect coords;
	SRect rectDest;
	bool writing_to_texture_with_border_texels = m_DoScissorTesting;

	if( !m_pDest )
	{
		coords.fLeftU   = 0.0f;
		coords.fTopV    = 0.0f;
		coords.fRightU  = 1.0f;
		coords.fBottomV = 1.0f;
		memset( &rectDest, 0, sizeof(SRect) );
	}
	else if( writing_to_texture_with_border_texels )
	{
		// horizontal blur
		GetTextureRect( m_pDest, &rectDest );
		rectDest.Inflate( -1, -1 );

		GetTextureCoords( m_pPrevScene->m_Desc, &rectSrc, m_pDest->m_Desc, &rectDest, &coords );
	}
	else
	{
		// vertical blur settings
		GetTextureCoords( m_pPrevScene->m_Desc, &rectSrc, m_pDest->m_Desc, NULL, &coords );
	}


	if( m_DoScissorTesting )
	{
//		GraphicsDevice().SetScissorRect( rectDest );
//		GraphicsDevice().Enable( RenderStateType::SCISSOR_TEST ); // original D3D sample
		GraphicsDevice().Disable( RenderStateType::SCISSOR_TEST );
	}

//	pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
//	pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );

	RenderFullScreenQuad( shader_mgr, coords );

	if( m_DoScissorTesting )
		GraphicsDevice().Disable( RenderStateType::SCISSOR_TEST );
}


HorizontalBloomFilter::HorizontalBloomFilter()
{
	m_DoScissorTesting = true;
}


/// The greater deviation value makes the image more blurrier
void HorizontalBloomFilter::GetSampleOffsets()
{
//	float fDeviation = 3.0f; // original D3D sample
	static float fDeviation = 0.5f; // experiment
	memset( m_afSampleOffsets, 0, sizeof(m_afSampleOffsets) );
	memset( m_avSampleWeights, 0, sizeof(m_avSampleWeights) );
	GetSampleOffsets_Bloom( m_Desc.Width, m_afSampleOffsets, m_avSampleWeights, m_fDeviation, 2.0f );
	for( int i = 0; i < MAX_SAMPLES; i++ )
	{
		m_avSampleOffsets[i] = Vector2( m_afSampleOffsets[i], 0.0f );
	}
}


VerticalBloomFilter::VerticalBloomFilter()
{
	m_DoScissorTesting = false;
}


void VerticalBloomFilter::GetSampleOffsets()
{
	memset( m_afSampleOffsets, 0, sizeof(m_afSampleOffsets) );
	memset( m_avSampleWeights, 0, sizeof(m_avSampleWeights) );
	GetSampleOffsets_Bloom( m_Desc.Height, m_afSampleOffsets, m_avSampleWeights, m_fDeviation, 2.0f );
	for( int i = 0; i < MAX_SAMPLES; i++ )
	{
		m_avSampleOffsets[i] = Vector2( 0.0f, m_afSampleOffsets[i] );
	}
}



CombinedBloomFilter::CombinedBloomFilter()
{
	SRectangular cbb = GetCropWidthAndHeight(); // cropped back buffer

	// based on the original Direct3D HDR Lighting sample
	m_BasePlane.width  = cbb.width  / 8;
	m_BasePlane.height = cbb.height / 8;
}


Result::Name CombinedBloomFilter::Init( RenderTargetTextureCache& cache, FilterShaderContainer& filter_shader_container )
{
	m_pCache = cache.GetSelfPtr().lock();

	SRectangular bb = GetBackBufferWidthAndHeight();
	SRectangular cbb = GetCropWidthAndHeight(); // cropped back buffer

	uint vp_width = 0, vp_height = 0;
	GraphicsDevice().GetViewportSize( vp_width, vp_height );
	SRectangular vp( (int)vp_width, (int)vp_height );
	SRectangular cvp = GetCropWidthAndHeight( vp );

	const SRectangular base_plane = m_BasePlane;

	// shared settings for bloom textures
	TextureResourceDesc desc;
	desc.UsageFlags = UsageFlag::RENDER_TARGET;
	desc.Format     = TextureFormat::A8R8G8B8;
	desc.MipLevels  = 1;

	desc.Width      = base_plane.width  + 2;
	desc.Height     = base_plane.height + 2;

	int n = cache.GetNumTextures( desc );
	for( int i=n; i<2; i++ )
	{
		Result::Name res = cache.AddTexture( desc );
		if( res != Result::SUCCESS )
			LOG_PRINT_ERROR( " Failed to create a render target texture." );
	}

	desc.Width  = base_plane.width;
	desc.Height = base_plane.height;

	n = cache.GetNumTextures( desc );
	if( n == 0 )
	{
		// render target to store the result of bloom filter
		Result::Name res = cache.AddTexture( desc );
		if( res != Result::SUCCESS )
			LOG_PRINT_ERROR( " Failed to create a render target texture." );
	}

	m_pGaussianBlurFilter.reset( new GaussianBlurFilter );
	m_pGaussianBlurFilter->SetRenderTargetSize( base_plane.width + 2, base_plane.height + 2 );
	m_pGaussianBlurFilter->SetRenderTargetSurfaceFormat( TextureFormat::A8R8G8B8 );
	m_pGaussianBlurFilter->SetFilterShader( filter_shader_container.AddPostProcessEffectShader( "gauss_blur_5x5" ) );
	m_pGaussianBlurFilter->SetTextureCache( cache.GetSelfPtr().lock() );
	m_pGaussianBlurFilter->SetDebugImageFilenameExtraString( "-for-bloom" );

	m_pHBloomFilter.reset( new HorizontalBloomFilter );
	m_pHBloomFilter->SetRenderTargetSize( base_plane.width + 2, base_plane.height + 2 );
	m_pHBloomFilter->SetRenderTargetSurfaceFormat( TextureFormat::A8R8G8B8 );
	m_pHBloomFilter->SetFilterShader( filter_shader_container.AddPostProcessEffectShader( "bloom" ) );
	m_pHBloomFilter->SetTextureCache( cache.GetSelfPtr().lock() );
	m_pHBloomFilter->SetDebugImageFilenameExtraString( "-for-horizontal-bloom" );

	m_pVBloomFilter.reset( new VerticalBloomFilter );
	m_pVBloomFilter->SetRenderTargetSize( base_plane.width,     base_plane.height );
	m_pVBloomFilter->SetRenderTargetSurfaceFormat( TextureFormat::A8R8G8B8 );
	m_pVBloomFilter->SetFilterShader( filter_shader_container.AddPostProcessEffectShader( "bloom" ) );
	m_pVBloomFilter->SetTextureCache( cache.GetSelfPtr().lock() );
	m_pVBloomFilter->SetDebugImageFilenameExtraString( "-for-vertical-bloom" );

	m_pGaussianBlurFilter->AddNextFilter( m_pHBloomFilter );
	m_pHBloomFilter->AddNextFilter( m_pVBloomFilter );

	// last filter of this interface filter
	m_pLastFilter = m_pVBloomFilter;

	return Result::SUCCESS;
}



void CombinedBloomFilter::RenderBase( PostProcessEffectFilter& prev_filter )
{
	return m_pGaussianBlurFilter->RenderBase( prev_filter );
}


void CombinedBloomFilter::UseAsGaussianBlurFilter( bool use_as_gauss_blur )
{
//	float fGaussianWeightsSum = 1.976f;
//	m_pHBloomFilter->SetBloomFactor( 1.0f / fGaussianWeightsSum );
//	m_pVBloomFilter->SetBloomFactor( 1.0f / fGaussianWeightsSum );
	m_pHBloomFilter->SetUseForBlurFilter( true );
	m_pVBloomFilter->SetUseForBlurFilter( true );
	m_pHBloomFilter->SetDebugImageFilenameExtraString( "-for-horizontal-blur" );
	m_pVBloomFilter->SetDebugImageFilenameExtraString( "-for-vertical-blur" );
}


void CombinedBloomFilter::SetBlurStrength( float strength )
{
	m_pGaussianBlurFilter->SetStandardDeviation( strength );
	m_pHBloomFilter->SetDeviation( strength );
	m_pVBloomFilter->SetDeviation( strength );
}


/*
void CombinedBloomFilter::AddNextFilter( std::shared_ptr<PostProcessEffectFilter> pFilter )
{
	m_vecpNextFilter.push_back( pFilter );
}
*/

void GetLuminanceTextureDesc( TextureResourceDesc& dest )
{
#ifdef BUILD_WITH_DIRECT3D
	LPDIRECT3D9 pD3D = DIRECT3D9.GetD3D();
	LPDIRECT3DDEVICE9 pd3dDevice = DIRECT3D9.GetDevice();

    D3DCAPS9 caps;
	D3DFORMAT luminance_format = D3DFMT_R16F;
    D3DDISPLAYMODE display_mode;
    pd3dDevice->GetDeviceCaps( &caps );
    pd3dDevice->GetDisplayMode( 0, &display_mode );
    // IsDeviceAcceptable already ensured that one of D3DFMT_R16F or D3DFMT_R32F is available.
    if( FAILED( pD3D->CheckDeviceFormat( caps.AdapterOrdinal, caps.DeviceType,
                                         display_mode.Format, D3DUSAGE_RENDERTARGET,
                                         D3DRTYPE_TEXTURE, D3DFMT_R16F ) ) )
        luminance_format = D3DFMT_R32F;
    else
        luminance_format = D3DFMT_R16F;

	dest.Format = FromD3DSurfaceFormat( luminance_format );
//	dest.MipLevels = 1;
	dest.UsageFlags = UsageFlag::RENDER_TARGET;

	vector<AdapterMode> adapter_modes;
	GraphicsDevice().GetAdapterModesForDefaultAdapter( adapter_modes );
#endif /* BUILD_WITH_DIRECT3D */
}



//=======================================================================
// LuminanceCalcFilter
//=======================================================================

LuminanceCalcFilter::LuminanceCalcFilter( const std::string& technique_name, int num_samples, int render_target_size )
{
	m_Technique.SetTechniqueName( technique_name.c_str() );
	m_NumSamples = num_samples;
	m_RenderTargetSize = render_target_size;

//	TextureResourceDesc tex_desc;
	GetLuminanceTextureDesc( m_Desc );
	m_Desc.Width  = m_RenderTargetSize;
	m_Desc.Height = m_RenderTargetSize;

	m_SetSamplerParameters[0] = 1;
	m_SetSamplerParameters[1] = 1;
	m_MaxInputTextureIndex = 1;
	m_MagFilters[0] = TextureFilter::LINEAR;
	m_MinFilters[0] = TextureFilter::LINEAR;
	m_MagFilters[1] = TextureFilter::LINEAR;
	m_MinFilters[1] = TextureFilter::LINEAR;
}


Result::Name LuminanceCalcFilter::Init( RenderTargetTextureCache& cache, FilterShaderContainer& filter_shader_container )
{
	m_pCache = cache.GetSelfPtr().lock();

	SetFilterShader( filter_shader_container.AddPostProcessEffectShader("hdr") );

	Result::Name res = Result::SUCCESS;
	if( m_pCache->GetNumTextures( m_Desc ) == 0 )
		res = m_pCache->AddTexture( m_Desc );

	return res;
}


void LuminanceCalcFilter::GetSampleOffsets_DownScale3x3( int width, int height, Vector2 avSampleOffsets[] )
{
    if( NULL == avSampleOffsets )
        return;

	// Initialize the sample offsets for the initial luminance pass.
	float tU, tV;
	tU = 1.0f / ( 3.0f * width );
	tV = 1.0f / ( 3.0f * height );

	int index = 0;
	int x = 0, y = 0;
	for( x = -1; x <= 1; x++ )
	{
		for( y = -1; y <= 1; y++ )
		{
			avSampleOffsets[index].x = x * tU;
			avSampleOffsets[index].y = y * tV;

			index++;
		}
	}
}


//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_DownScale4x4
// Desc: Get the texture coordinate offsets to be used inside the DownScale4x4
//       pixel shader.
//-----------------------------------------------------------------------------
void LuminanceCalcFilter::GetSampleOffsets_DownScale4x4( int width, int height, Vector2 avSampleOffsets[] )
{
    if( NULL == avSampleOffsets )
        return;

    float tU = 1.0f / (float)width;
    float tV = 1.0f / (float)height;

    // Sample from the 16 surrounding points. Since the center point will be in
    // the exact center of 16 texels, a 0.5f offset is needed to specify a texel
    // center.
    int index = 0;
    for( int y = 0; y < 4; y++ )
    {
        for( int x = 0; x < 4; x++ )
        {
            avSampleOffsets[ index ].x = ( x - 1.5f ) * tU;
            avSampleOffsets[ index ].y = ( y - 1.5f ) * tV;

            index++;
        }
    }
}


void LuminanceCalcFilter::Render()
{
	ShaderManager *pShaderMgr = GetShaderManager(*this);
	if( !pShaderMgr )
		return;

	ShaderManager& shader_mgr = *pShaderMgr;

	Vector2 avSampleOffsets[MAX_SAMPLES];
	memset( avSampleOffsets, 0, sizeof(avSampleOffsets) );

	int w = m_pDest->m_Desc.Width;
	int h = m_pDest->m_Desc.Height;

	switch( m_NumSamples )
	{
	case 9:  GetSampleOffsets_DownScale3x3( w, h, avSampleOffsets ); break;
	case 16: GetSampleOffsets_DownScale4x4( w, h, avSampleOffsets ); break;
	default:
		break;
	}

//	shader_mgr.SetParam( "g_avSampleOffsets", (float *)avSampleOffsets, numof(avSampleOffsets) * 2 );
	shader_mgr.SetParam( "g_avSampleOffsets", avSampleOffsets, numof(avSampleOffsets) );
/*
//	pd3dDevice->SetRenderTarget( 0, apSurfToneMap[dwCurTexture] );
//	pd3dDevice->SetTexture( 0, g_pTexSceneScaled );
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
*/
	// Check the current states: should all be D3DTEXF_LINEAR. See above.
//	LPDIRECT3DDEVICE9 pd3dDevice = DIRECT3D9.GetDevice();
//	DWORD mag0=0, min0=0, mag1=0, min1=0;
//	pd3dDevice->GetSamplerState( 0, D3DSAMP_MAGFILTER, &mag0 );
//	pd3dDevice->GetSamplerState( 0, D3DSAMP_MINFILTER, &min0 );
//	pd3dDevice->GetSamplerState( 1, D3DSAMP_MAGFILTER, &mag1 );
//	pd3dDevice->GetSamplerState( 1, D3DSAMP_MINFILTER, &min1 );

	RenderFullScreenQuad( shader_mgr, 0.0f, 0.0f, 1.0f, 1.0f );
}



//=======================================================================
// AdaptationCalcFilter
//=======================================================================

AdaptationCalcFilter::AdaptationCalcFilter()
:
m_fElapsedTime(0.01666666667f),
m_fLuminanceAdaptationRate(0.02f)
{
	m_Technique.SetTechniqueName( "CalculateAdaptedLum" );

	m_SetSamplerParameters[0] = 1;
	m_SetSamplerParameters[1] = 1;
	m_MaxInputTextureIndex = 1;
	m_MagFilters[0] = TextureFilter::NEAREST;
	m_MinFilters[0] = TextureFilter::NEAREST;
	m_MagFilters[1] = TextureFilter::NEAREST;
	m_MinFilters[1] = TextureFilter::NEAREST;
}


AdaptationCalcFilter::~AdaptationCalcFilter()
{
	if( m_pTexAdaptedLuminanceLast )
		m_pTexAdaptedLuminanceLast->DecrementLockCount();

	if( m_pTexAdaptedLuminanceCur )
		m_pTexAdaptedLuminanceCur->DecrementLockCount();
}


Result::Name AdaptationCalcFilter::SetRenderTarget( PostProcessEffectFilter& prev_filter )
{
	// The render target is set in AdaptationCalcFilter::Render()
	return Result::SUCCESS;
}


Result::Name AdaptationCalcFilter::Init( RenderTargetTextureCache& cache, FilterShaderContainer& filter_shader_container )
{
	m_pCache = cache.GetSelfPtr().lock();

	SetFilterShader( filter_shader_container.AddPostProcessEffectShader("hdr") );

	m_Desc.Width  = 1;
	m_Desc.Height = 1;
	GetLuminanceTextureDesc( m_Desc );

	int num_cached_texes = cache.GetNumTextures( m_Desc );

//	shared_ptr<RenderTargetTextureHolder> apHolder[2];
//	for( int i=num_cached_texes; i<2; i++ )
	for( int i=0; i<2; i++ )
	{
		cache.AddTexture( m_Desc );
	}

	// Get the textures and lock them at this init phase and always keep them.

	m_pTexAdaptedLuminanceLast = cache.GetTexture( m_Desc );

	if( m_pTexAdaptedLuminanceLast )
		m_pTexAdaptedLuminanceLast->IncrementLockCount();
	else
		LOG_PRINT_ERROR( "Failed to get texture for storing adapted luminance (last)." );

	m_pTexAdaptedLuminanceCur  = cache.GetTexture( m_Desc );

	if( m_pTexAdaptedLuminanceCur )
		m_pTexAdaptedLuminanceCur->IncrementLockCount();
	else
		LOG_PRINT_ERROR( "Failed to get texture for storing adapted luminance (current)." );

	return Result::SUCCESS;
}


/**
 Increment the user's adapted luminance
*/
void AdaptationCalcFilter::Render()
{
//	LPDIRECT3DDEVICE9 pd3dDevice = DIRECT3D9.GetDevice();
	ShaderManager *pShaderMgr = GetShaderManager(*this);
	if( !pShaderMgr )
		return;

	ShaderManager& shader_mgr = *pShaderMgr;

//	HRESULT hr = S_OK;
//	UINT uiPass, uiPassCount;

	// Swap current & last luminance
	shared_ptr<RenderTargetTextureHolder> pTexSwap = m_pTexAdaptedLuminanceLast;
	m_pTexAdaptedLuminanceLast = m_pTexAdaptedLuminanceCur;
	m_pTexAdaptedLuminanceCur = pTexSwap;

	m_pDest = m_pTexAdaptedLuminanceCur;
//	hr = m_pDest->m_Texture.GetTexture()->GetSurfaceLevel( 0, &(m_pDest->m_pTexSurf) );

	/// increment the lock count to avoid decrement it to be zero-ed by the next filter
	m_pDest->IncrementLockCount();

//	PDIRECT3DSURFACE9 pSurfAdaptedLum = NULL;
//	V( m_pTexAdaptedLuminanceCur->GetSurfaceLevel( 0, &pSurfAdaptedLum ) );

	// This simulates the light adaptation that occurs when moving from a 
	// dark area to a bright area, or vice versa. The g_pTexAdaptedLuminance
	// texture stores a single texel cooresponding to the user's adapted 
	// level.
//	pEffect->SetTechnique( "CalculateAdaptedLum" );
	shader_mgr.SetParam( "g_fElapsedTime",    m_fElapsedTime );
	shader_mgr.SetParam( "g_fAdaptationRate", m_fLuminanceAdaptationRate );

//	hr = pd3dDevice->SetRenderTarget( 0, pSurfAdaptedLum );
//	hr = pd3dDevice->SetRenderTarget( 0, m_pTexAdaptedLuminanceCur->m_pTexSurf );

//	hr = pd3dDevice->SetRenderTarget( 0, m_pDest->m_pTexSurf );
	m_pDest->m_pTextureRenderTarget->SetRenderTarget();

/*	hr = pd3dDevice->SetTexture( 0, m_pTexAdaptedLuminanceLast->m_pTextureRenderTarget->GetRenderTargetTexture().GetTexture() );
//	hr = pd3dDevice->SetTexture( 1, g_apTexToneMap[0] );
	hr = pd3dDevice->SetTexture( 1, m_pPrevScene->m_pTextureRenderTarget->GetRenderTargetTexture().GetTexture() );
*/
	GraphicsDevice().SetTexture( 0, m_pTexAdaptedLuminanceLast->m_pTextureRenderTarget->GetRenderTargetTexture() );
	GraphicsDevice().SetTexture( 1, m_pPrevScene->m_pTextureRenderTarget->GetRenderTargetTexture() );
/*
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );
*/
	// Check the current states: should all be D3DTEXF_POINT. See above.
//	DWORD mag0=0, min0=0, mag1=0, min1=0;
//	pd3dDevice->GetSamplerState( 0, D3DSAMP_MAGFILTER, &mag0 );
//	pd3dDevice->GetSamplerState( 0, D3DSAMP_MINFILTER, &min0 );
//	pd3dDevice->GetSamplerState( 1, D3DSAMP_MAGFILTER, &mag1 );
//	pd3dDevice->GetSamplerState( 1, D3DSAMP_MINFILTER, &min1 );

	RenderFullScreenQuad( shader_mgr, 0.0f, 0.0f, 1.0f, 1.0f );

//	SAFE_RELEASE( pSurfAdaptedLum );
//	return S_OK;
}



//====================================================================================
// HDRLightingFinalPassFilter
//====================================================================================

HDRLightingFinalPassFilter::HDRLightingFinalPassFilter()
:
m_fKeyValue(0.5f),
m_ToneMappingEnabled(true),
m_StarEffectEnabled(false)
{
	m_Technique.SetTechniqueName( "FinalScenePass" );

	// Create a small black texture which is used when the star effect is off
	TextureResourceDesc desc;
	desc.Width  = 16;
	desc.Height = 16;
	desc.Format = TextureFormat::A8R8G8B8;
	desc.pLoader.reset( new SingleColorTextureGenerator( SFloatRGBAColor::Black() ) );
	m_BlancTextureForDisabledStarEffect.Load( desc );

	m_MaxInputTextureIndex = 3;

	m_SetSamplerParameters[0] = 1;
	m_SetSamplerParameters[1] = 1;
	m_SetSamplerParameters[2] = 1;
	m_SetSamplerParameters[3] = 1;
	m_MagFilters[0] = TextureFilter::NEAREST;
	m_MinFilters[0] = TextureFilter::NEAREST;
	m_MagFilters[1] = TextureFilter::LINEAR;
	m_MinFilters[1] = TextureFilter::LINEAR;
	m_MagFilters[2] = TextureFilter::LINEAR;
	m_MinFilters[2] = TextureFilter::LINEAR;
	m_MagFilters[3] = TextureFilter::NEAREST;
	m_MinFilters[3] = TextureFilter::NEAREST;

	for( int i=0; i<4; i++ )
	{
		m_SetTextureWrapParameters[i] = 1;
		m_TextureWrapAxis0[i] = TextureAddressMode::CLAMP_TO_EDGE;
		m_TextureWrapAxis1[i] = TextureAddressMode::CLAMP_TO_EDGE;
	}
}


Result::Name HDRLightingFinalPassFilter::Init( RenderTargetTextureCache& cache, FilterShaderContainer& filter_shader_container )
{
	m_pCache = cache.GetSelfPtr().lock();

	SetFilterShader( filter_shader_container.AddPostProcessEffectShader("hdr") );

	return Result::SUCCESS;
}


bool HDRLightingFinalPassFilter::IsReadyToRender()
{
	if( m_pBloom
	 && ( m_pStar || !m_StarEffectEnabled )
	 && m_pAdaptedLuminance
	 && m_pPrevResult )
		return true;
	else
		return false;
}


void HDRLightingFinalPassFilter::StorePrevFilterResults( PostProcessEffectFilter& prev_filter )
{
	switch( prev_filter.GetFilterType() )
	{
//	case PostProcessEffectFilter::TYPE_COMBINED_BLOOM:
	case PostProcessEffectFilter::TYPE_VERTICAL_BLOOM: // last filter of CombinedBloomFilter
		m_pBloom = prev_filter.GetDestRenderTarget();
		break;
	case PostProcessEffectFilter::TYPE_STAR:
		m_pStar = prev_filter.GetDestRenderTarget();
		break;
	case PostProcessEffectFilter::TYPE_ADAPTATION_CALC:
		m_pAdaptedLuminance = prev_filter.GetDestRenderTarget();
		break;
	default:
		// none of the above - assume this is the prev scene
		m_pPrevResult = prev_filter.GetDestRenderTarget();
		break;
	}
}


void HDRLightingFinalPassFilter::Render()
{
	ShaderManager *pShaderMgr = GetShaderManager(*this);
	if( !pShaderMgr )
		return;

	ShaderManager& shader_mgr = *pShaderMgr;

//	m_pLumCalcFilter->RenderBase();

//	m_pBrightPassFilter->SetAvarageLuminanceTexture( m_pLumCalcFilter->GetAverageLuminanceTexture() );

//	m_pBrightPassFilter->RenderBase();

	// Draw the high dynamic range scene texture to the low dynamic range
	// back buffer. As part of this final pass, the scene will be tone-mapped
	// using the user's current adapted luminance, blue shift will occur
	// if the scene is determined to be very dark, and the post-process lighting
	// effect textures will be added to the scene.

//	V( pEffect->SetTechnique( "FinalScenePass" ) );
	shader_mgr.SetParam( "g_fMiddleGray", m_fKeyValue );
//	V( pEffect->SetBool( "g_bEnableToneMap", m_ToneMappingEnabled ) );
	shader_mgr.SetBool( "g_bEnableToneMap", m_ToneMappingEnabled );

	m_pPrevResult->GetTexture().SetSamplingParameter( SamplingParameter::MAG_FILTER, TextureFilter::NEAREST );
	m_pPrevResult->GetTexture().SetSamplingParameter( SamplingParameter::MIN_FILTER, TextureFilter::NEAREST );
	m_pBloom->GetTexture().SetSamplingParameter( SamplingParameter::MAG_FILTER, TextureFilter::LINEAR );
	m_pBloom->GetTexture().SetSamplingParameter( SamplingParameter::MIN_FILTER, TextureFilter::LINEAR );
//	???.SetSamplingParameter( SamplingParameter::MAG_FILTER, TextureFilter::LINEAR );
//	???.SetSamplingParameter( SamplingParameter::MIN_FILTER, TextureFilter::LINEAR );
	m_pAdaptedLuminance->GetTexture().SetSamplingParameter( SamplingParameter::MAG_FILTER, TextureFilter::NEAREST );
	m_pAdaptedLuminance->GetTexture().SetSamplingParameter( SamplingParameter::MIN_FILTER, TextureFilter::NEAREST );

	GraphicsDevice().SetTexture( 0, m_pPrevResult->GetTexture() );
	GraphicsDevice().SetTexture( 1, m_pBloom->GetTexture() );
	GraphicsDevice().SetTexture( 3, m_pAdaptedLuminance->GetTexture() );
/*
//	V( pd3dDevice->SetRenderTarget( 0, pSurfLDR ) );
	V( pd3dDevice->SetTexture( 0, m_pPrevResult->m_Texture.GetTexture() ) );
	V( pd3dDevice->SetTexture( 1, m_pBloom->m_Texture.GetTexture() ) );
	V( pd3dDevice->SetTexture( 3, m_pAdaptedLuminance->m_Texture.GetTexture() ) );
*/
	if( m_pStar )
	{
//		V( pd3dDevice->SetTexture( 2, m_pStar->m_Texture.GetTexture() ) );
		GraphicsDevice().SetTexture( 2, m_pStar->GetTexture() );
	}
	else
	{
//		V( pd3dDevice->SetTexture( 2, m_BlancTextureForDisabledStarEffect.GetTexture() ) );
		GraphicsDevice().SetTexture( 2, m_BlancTextureForDisabledStarEffect );
	}
/*
	V( pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT ) );
	V( pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT ) );
	V( pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR ) );
	V( pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR ) );
	V( pd3dDevice->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR ) );
	V( pd3dDevice->SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR ) );
	V( pd3dDevice->SetSamplerState( 3, D3DSAMP_MAGFILTER, D3DTEXF_POINT ) );
	V( pd3dDevice->SetSamplerState( 3, D3DSAMP_MINFILTER, D3DTEXF_POINT ) );
*/
	// Check the current states; should return the values set above.
//	LPDIRECT3DDEVICE9 pd3dDevice = DIRECT3D9.GetDevice();
//	DWORD mag0=0, min0=0, mag1=0, min1=0;
//	DWORD mag2=0, min2=0, mag3=0, min3=0;
//	pd3dDevice->GetSamplerState( 0, D3DSAMP_MAGFILTER, &mag0 );
//	pd3dDevice->GetSamplerState( 0, D3DSAMP_MINFILTER, &min0 );
//	pd3dDevice->GetSamplerState( 1, D3DSAMP_MAGFILTER, &mag1 );
//	pd3dDevice->GetSamplerState( 1, D3DSAMP_MINFILTER, &min1 );
//	pd3dDevice->GetSamplerState( 2, D3DSAMP_MAGFILTER, &mag2 );
//	pd3dDevice->GetSamplerState( 2, D3DSAMP_MINFILTER, &min2 );
//	pd3dDevice->GetSamplerState( 3, D3DSAMP_MAGFILTER, &mag3 );
//	pd3dDevice->GetSamplerState( 3, D3DSAMP_MINFILTER, &min3 );

//	pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
//	pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
//	pd3dDevice->SetSamplerState( 1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
//	pd3dDevice->SetSamplerState( 1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
//	pd3dDevice->SetSamplerState( 2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
//	pd3dDevice->SetSamplerState( 2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
//	pd3dDevice->SetSamplerState( 3, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
//	pd3dDevice->SetSamplerState( 3, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	RenderFullScreenQuad( shader_mgr, 0.0f, 0.0f, 1.0f, 1.0f );

//	V( pd3dDevice->SetTexture( 1, NULL ) );
//	V( pd3dDevice->SetTexture( 2, NULL ) );
//	V( pd3dDevice->SetTexture( 3, NULL ) );

	GraphicsDevice().SetTexture( 0, TextureHandle() );
	GraphicsDevice().SetTexture( 1, TextureHandle() );
	GraphicsDevice().SetTexture( 3, TextureHandle() );

	m_pBloom->DecrementLockCount();
	m_pAdaptedLuminance->DecrementLockCount();

	if( m_pStar )
		m_pStar->DecrementLockCount();

	m_pBloom = shared_ptr<RenderTargetTextureHolder>();
	m_pStar = shared_ptr<RenderTargetTextureHolder>();
	m_pAdaptedLuminance = shared_ptr<RenderTargetTextureHolder>();
}



//=======================================================================
// HDRLightingFilter
//=======================================================================

HDRLightingFilter::HDRLightingFilter()
:
//m_fKeyValue(0)
m_EnableStarFilter(false)
{
}


Result::Name HDRLightingFilter::Init( RenderTargetTextureCache& cache, FilterShaderContainer& filter_shader_container )
{
	m_pCache = cache.GetSelfPtr().lock();

	// first, calculate the luminance
//	m_vecpChildFilter.push_back( m_pLumCalcFilter );

	// bloom and star effect
//	m_vecpChildFilter.push_back( m_pBrightPassFilter );

	Result::Name res = Result::SUCCESS;

	SRectangular cbb = GetCropWidthAndHeight();

	m_pDownScale4x4Filter.reset( new DownScale4x4Filter );
	m_pDownScale4x4Filter->SetRenderTargetSize( cbb.width / 4, cbb.height / 4 );
	m_pDownScale4x4Filter->SetRenderTargetSurfaceFormat( TextureFormat::A16R16G16B16F );
//	m_pDownScale4x4Filter->SetFilterShader( filter_shader_container.GetFilterShader( "HDRPostProcessor" ) );
	res = m_pDownScale4x4Filter->Init( cache, filter_shader_container );

	m_pBrightPassFilter.reset( new HDRBrightPassFilter );
	m_pBrightPassFilter->SetRenderTargetSize( cbb.width / 4 + 2, cbb.height / 4 + 2 );
	m_pBrightPassFilter->SetRenderTargetSurfaceFormat( TextureFormat::A8R8G8B8 );
//	m_pBrightPassFilter->SetFilterShader( filter_shader_container.GetFilterShader( "HDRPostProcessor" ) );
	res = m_pBrightPassFilter->Init( cache, filter_shader_container );
//	m_pBrightPassFilter->SetExtraTexelBorderWidth( 1 );

	m_pGaussianBlurFilter.reset( new GaussianBlurFilter );
	m_pGaussianBlurFilter->SetRenderTargetSize( cbb.width / 4 + 2, cbb.height / 4 + 2 );
	m_pGaussianBlurFilter->SetRenderTargetSurfaceFormat( TextureFormat::A8R8G8B8 );
//	m_pGaussianBlurFilter->SetFilterShader( filter_shader_container.GetFilterShader( "HDRPostProcessor" ) );
	res = m_pGaussianBlurFilter->Init( cache, filter_shader_container );
	m_pGaussianBlurFilter->SetDebugImageFilenameExtraString( "-for-hdrl" );
//	m_pGaussianBlurFilter->Init();
//	m_pDownScale2x2Filter->SetExtraTexelBorderWidth( 1 );

	m_pDownScale2x2Filter.reset( new DownScale2x2Filter );
	m_pDownScale2x2Filter->SetRenderTargetSize( cbb.width / 8 + 2, cbb.height / 8 + 2 );
	m_pDownScale2x2Filter->SetRenderTargetSurfaceFormat( TextureFormat::A8R8G8B8 );
//	m_pDownScale2x2Filter->SetFilterShader( filter_shader_container.GetFilterShader( "HDRPostProcessor" ) );
	res = m_pDownScale2x2Filter->Init( cache, filter_shader_container );
//	m_pDownScale2x2Filter->SetExtraTexelBorderWidth( 1 );

	if( m_EnableStarFilter )
	{
		m_pStarFilter.reset( new StarFilter );
	}

	m_apLumCalcFilter[0].reset( new LuminanceCalcFilter( "SampleAvgLum",       9, 64 ) );
	m_apLumCalcFilter[1].reset( new LuminanceCalcFilter( "ResampleAvgLum",    16, 16 ) );
	m_apLumCalcFilter[2].reset( new LuminanceCalcFilter( "ResampleAvgLum",    16,  4 ) );
	m_apLumCalcFilter[3].reset( new LuminanceCalcFilter( "ResampleAvgLumExp", 16,  1 ) );

	for( int i=0; i<NUM_TONEMAP_TEXTURES; i++ )
	{
		m_apLumCalcFilter[i]->Init( cache, filter_shader_container );
//		m_apLumCalcFilter[i]->SetFilterShader( filter_shader_container.GetFilterShader( "HDRPostProcessor" ) );
	}

	m_pAdaptationCalcFilter.reset( new AdaptationCalcFilter() );
//	m_pAdaptationCalcFilter->SetFilterShader( filter_shader_container.GetFilterShader( "HDRPostProcessor" ) );
	res = m_pAdaptationCalcFilter->Init( cache, filter_shader_container );

	m_pBloomFilter.reset( new CombinedBloomFilter );
	res = m_pBloomFilter->Init( cache, filter_shader_container );

//	m_pHorizontalBloomFilter = shared_ptr<DownScale2x2Filter>( new DownScale2x2Filter );
//	m_pVerticalBloomFilter   = shared_ptr<DownScale2x2Filter>( new DownScale2x2Filter );

	m_pFinalPassFilter.reset( new HDRLightingFinalPassFilter );
//	m_pFinalPassFilter->SetRenderTargetSize( cbb.width / 4 + 2, cbb.height / 4 + 2 );
	m_pFinalPassFilter->SetRenderTargetSize( cbb.width, cbb.height );
	m_pFinalPassFilter->SetRenderTargetSurfaceFormat( TextureFormat::A8R8G8B8 );
//	m_pFinalPassFilter->SetFilterShader( filter_shader_container.GetFilterShader( "HDRPostProcessor" ) );
	res = m_pFinalPassFilter->Init( cache, filter_shader_container );

	//
	// create textures to use as render targets of filters
	//

	cbb = GetCropWidthAndHeight();
	TextureResourceDesc tex_desc;
	tex_desc.UsageFlags = UsageFlag::RENDER_TARGET;

	// for 4x4 scaled down scene
	tex_desc.Width  = cbb.width  / 4;
	tex_desc.Height = cbb.height / 4;
	tex_desc.Format = TextureFormat::A16R16G16B16F;
	tex_desc.MipLevels = 1;
	if( m_pCache->GetNumTextures( tex_desc ) == 0 )
		m_pCache->AddTexture( tex_desc );

	// for bright pass and gaussian blur
	tex_desc.Width  = cbb.width  / 4 + 2;
	tex_desc.Height = cbb.height / 4 + 2;
	tex_desc.Format = TextureFormat::A8R8G8B8;
	tex_desc.MipLevels = 1;
	int num = m_pCache->GetNumTextures( tex_desc );
	for( int i=num; i<2; i++ )
		m_pCache->AddTexture( tex_desc );

	// for combined bloom filter
	tex_desc.Width  = cbb.width  / 8;
	tex_desc.Height = cbb.height / 8;
	tex_desc.Format = TextureFormat::A8R8G8B8;
	tex_desc.MipLevels = 1;
	if( m_pCache->GetNumTextures( tex_desc ) == 0 )
		m_pCache->AddTexture( tex_desc );

	tex_desc.Width  = cbb.width  / 8 + 2;
	tex_desc.Height = cbb.height / 8 + 2;
	tex_desc.Format = TextureFormat::A8R8G8B8;
	tex_desc.MipLevels = 1;
	num = m_pCache->GetNumTextures( tex_desc );
	for( int i=num; i<3; i++ )
		m_pCache->AddTexture( tex_desc );

	// for final pass filter
	// - need this when there are subsequent filters after this HDR lighting filter
	tex_desc.Width  = cbb.width;
	tex_desc.Height = cbb.height;
	tex_desc.Format = TextureFormat::A8R8G8B8;
	tex_desc.MipLevels = 1;
	if( m_pCache->GetNumTextures( tex_desc ) == 0 )
		m_pCache->AddTexture( tex_desc );


	//
	// set up filter lists
	//

	m_apLumCalcFilter[0]->AddNextFilter( m_apLumCalcFilter[1] );
	m_apLumCalcFilter[1]->AddNextFilter( m_apLumCalcFilter[2] );
	m_apLumCalcFilter[2]->AddNextFilter( m_apLumCalcFilter[3] );
	m_apLumCalcFilter[3]->AddNextFilter( m_pAdaptationCalcFilter );
	m_pAdaptationCalcFilter->AddNextFilter( m_pFinalPassFilter );

	// preprocessing for bloom and star
	m_pDownScale4x4Filter->AddNextFilter( m_pBrightPassFilter );
	m_pBrightPassFilter->AddNextFilter( m_pGaussianBlurFilter );

	// bloom filters
	m_pGaussianBlurFilter->AddNextFilter( m_pDownScale2x2Filter );
	m_pDownScale2x2Filter->AddNextFilter( m_pBloomFilter );
	m_pBloomFilter->AddNextFilter( m_pFinalPassFilter );

	// star filters
	if( m_pStarFilter )
	{
		m_pGaussianBlurFilter->AddNextFilter( m_pStarFilter );
		m_pStarFilter->AddNextFilter( m_pFinalPassFilter );
	}

	m_pLastFilter = m_pFinalPassFilter;

	// set shader params
//	shared_ptr<PostProcessFilterShader> pShader = filter_shader_container.GetFilterShader( "HDRPostProcessor" );
//	if( pShader && pShader->GetShader().GetShaderManager() )
//	{
//		ShaderManager *pShaderMgr = pShader->GetShader().GetShaderManager();
//		ShaderParameter< vector<float> > bloom_scale( "g_fBloomScale" ), star_scale( "g_fStarScale" );
//		bloom_scale.Parameter().resize(1);
//		star_scale.Parameter().resize(1);
//		bloom_scale.Parameter()[0] = 1.0f;
//		star_scale.Parameter()[0] = 0.5f;
//		pShaderMgr->SetParam( bloom_scale );
//		pShaderMgr->SetParam( star_scale );
//	}

	return Result::SUCCESS;
}


void HDRLightingFilter::RenderBase( PostProcessEffectFilter& prev_filter )
{
	m_apLumCalcFilter[0]->RenderBase( prev_filter );

	m_pBrightPassFilter->SetAdaptedLuminanceTexture( m_pAdaptationCalcFilter->GetAdaptedLuminanceTexture() );

	if( prev_filter.GetDestRenderTarget() )
		prev_filter.GetDestRenderTarget()->IncrementLockCount();

	// scale down by 4x4 -> bright pass -> gaussian blur
	//   -> scale down by 2x2 -> bloom -> final HDR lighting pass
	//   -> star -> final HDR lighting pass
	m_pDownScale4x4Filter->RenderBase( prev_filter );

	if( prev_filter.GetDestRenderTarget() )
		prev_filter.GetDestRenderTarget()->IncrementLockCount();

	m_pFinalPassFilter->RenderBase( prev_filter );
}



//============================================================================
// FullScreenBlurFilter
//============================================================================

FullScreenBlurFilter::FullScreenBlurFilter()
:
m_fBlurStrength(1.0f)
{
}


Result::Name FullScreenBlurFilter::Init( RenderTargetTextureCache& cache, FilterShaderContainer& filter_shader_container )
{
	m_pCache = cache.GetSelfPtr().lock();

	Result::Name res;
	const SRectangular cbb = GetCropWidthAndHeight();

	m_pDownScale4x4Filter.reset( new DownScale4x4Filter );
	m_pDownScale4x4Filter->SetRenderTargetSize( cbb.width / 4, cbb.height / 4 );
	m_pDownScale4x4Filter->SetRenderTargetSurfaceFormat( TextureFormat::A8R8G8B8 );
	m_pDownScale4x4Filter->SetFilterShader( filter_shader_container.GetFilterShader( "HDRPostProcessor" ) );
	res = m_pDownScale4x4Filter->Init( cache, filter_shader_container );
	m_pDownScale4x4Filter->SetDebugImageFilenameExtraString( "-for-gaussblur" );
/*
	for( int i=0; i<2; i++ )
	{
		m_apHorizontalBloomFilter[i] = shared_ptr<HorizontalBloomFilter>( new HorizontalBloomFilter );
		m_apHorizontalBloomFilter[i]->SetRenderTargetSize( cbb.width / 4, cbb.height / 4 );
		m_apHorizontalBloomFilter[i]->SetRenderTargetSurfaceFormat( TextureFormat::A8R8G8B8 );
		m_apHorizontalBloomFilter[i]->SetFilterShader( filter_shader_container.GetFilterShader( "HDRPostProcessor" ) );
		m_apHorizontalBloomFilter[i]->SetTextureCache( cache.GetSelfPtr().lock() );
	}
	res = m_pDownScale4x4Filter->Init( cache, filter_shader_container );
*/

	m_pBloomFilter.reset( new CombinedBloomFilter );
	m_pBloomFilter->SetBasePlane( SRectangular( cbb.width / 4, cbb.height / 4 ) );
	res = m_pBloomFilter->Init( cache, filter_shader_container );
	m_pBloomFilter->UseAsGaussianBlurFilter( true );

	TextureResourceDesc tex_desc;
	tex_desc.Width  = cbb.width  / 4;
	tex_desc.Height = cbb.height / 4;
	tex_desc.Format = TextureFormat::A8R8G8B8;
	tex_desc.UsageFlags = UsageFlag::RENDER_TARGET;
	tex_desc.MipLevels = 1;
	int num = m_pCache->GetNumTextures( tex_desc );
	for( int i=num; i<2; i++ )
		m_pCache->AddTexture( tex_desc );

	tex_desc.Width  = cbb.width  / 4 + 2;
	tex_desc.Height = cbb.height / 4 + 2;
	tex_desc.Format = TextureFormat::A8R8G8B8;
	tex_desc.UsageFlags = UsageFlag::RENDER_TARGET;
	tex_desc.MipLevels = 1;
	num = m_pCache->GetNumTextures( tex_desc );
	for( int i=num; i<2; i++ )
		m_pCache->AddTexture( tex_desc );

	m_pDownScale4x4Filter->AddNextFilter( m_pBloomFilter );

	m_pLastFilter = m_pBloomFilter;

	// test the 4x4 down scale filter
//	m_pLastFilter = m_pDownScale4x4Filter;

	return Result::SUCCESS;
}


void FullScreenBlurFilter::RenderBase( PostProcessEffectFilter& prev_filter )
{
	m_pBloomFilter->SetBlurStrength( m_fBlurStrength );

	m_pDownScale4x4Filter->RenderBase( prev_filter );

//	if( prev_filter.GetDestRenderTarget() )
//		prev_filter.GetDestRenderTarget()->IncrementLockCount();

	// Don't do this; m_pBloomFilter->RenderBase() is called in m_pDownScale4x4Filter->RenderBase() above.
//	m_pBloomFilter->RenderBase( prev_filter );
}



//============================================================================
// MonochromeColorFilter
//============================================================================

MonochromeColorFilter::MonochromeColorFilter()
{
	m_Technique.SetTechniqueName( "MonochromeColor" );

	// set the render target size to that of back buffer by default
	const SRectangular cbb = GetCropWidthAndHeight();
	SetRenderTargetSize( cbb.width, cbb.height );

	SetRenderTargetSurfaceFormat( TextureFormat::A8R8G8B8 );

	m_SetSamplerParameters[0] = 1;
	m_SetSamplerParameters[1] = 1;
	m_MaxInputTextureIndex = 1;
	m_MagFilters[0] = TextureFilter::NEAREST;
	m_MinFilters[0] = TextureFilter::NEAREST;
	m_MagFilters[1] = TextureFilter::NEAREST;
	m_MinFilters[1] = TextureFilter::NEAREST;
}


Result::Name MonochromeColorFilter::Init( RenderTargetTextureCache& cache, FilterShaderContainer& filter_shader_container )
{
	m_pCache = cache.GetSelfPtr().lock();
//	SetFilterShader( filter_shader_container.GetShader( "HDRPostProcessor" ) );

	SetFilterShader( filter_shader_container.AddPostProcessEffectShader("monochrome") );

	const SRectangular cbb = GetCropWidthAndHeight();
	SetRenderTargetSize( cbb.width, cbb.height );

	TextureResourceDesc tex_desc;
	tex_desc.Width  = cbb.width;
	tex_desc.Height = cbb.height;
	tex_desc.Format = TextureFormat::A8R8G8B8;
	tex_desc.MipLevels = 1;
	tex_desc.UsageFlags = UsageFlag::RENDER_TARGET;
	int num = cache.GetNumTextures( tex_desc );
	for( int i=num; i<2; i++ )
		cache.AddTexture( tex_desc );

	return Result::SUCCESS;
}


void MonochromeColorFilter::Render()
{
	ShaderManager *pShaderMgr = GetShaderManager(*this);
	if( !pShaderMgr )
		return;

	ShaderManager& shader_mgr = *pShaderMgr;

/*
	D3DSURFACE_DESC desc;
	m_pPrevScene->m_Texture.GetTexture()->GetLevelDesc( 0, &desc );

	// Get the rectangle describing the sampled portion of the source texture.
	// Decrease the rectangle to adjust for the single pixel black border.
	SRect rectSrc;
//	GetTextureRect( m_pPrevScene->m_Texture.GetTexture(), &rectSrc );
	GetTextureRect( m_pPrevScene, &rectSrc );
	InflateRect( &rectSrc, -1, -1 );

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	SRect rectDest;
//	GetTextureRect( m_pDest->m_Texture.GetTexture(), &rectDest );
	GetTextureRect( m_pDest, &rectDest );
	InflateRect( &rectDest, -1, -1 );

	// Get the correct texture coordinates to apply to the rendered quad in order 
	// to sample from the source rectangle and render into the destination rectangle
	CoordRect coords;
	GetTextureCoords( m_pPrevScene->m_Texture, &rectSrc, m_pDest->m_Texture, &rectDest, &coords );
*/
	CoordRect coords;
	coords.fLeftU   = 0.0f;
	coords.fTopV    = 0.0f;
	coords.fRightU  = 1.0f;
	coords.fBottomV = 1.0f;

	GraphicsDevice().Disable( RenderStateType::SCISSOR_TEST );
//	hr = pd3dDevice->SetScissorRect( &rectDest );

	RenderFullScreenQuad( shader_mgr, coords );

	GraphicsDevice().Disable( RenderStateType::SCISSOR_TEST );

	return;
}


} // namespace amorphous
