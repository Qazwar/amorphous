#ifndef __GraphicsDevice_HPP__
#define __GraphicsDevice_HPP__

#include <vector>
#include "amorphous/base.hpp"
#include "amorphous/3DMath/fwd.hpp"
#include "amorphous/3DMath/Plane.hpp"
#include "amorphous/Support/singleton.hpp"
#include "FloatRGBAColor.hpp"
#include "AlphaBlend.hpp"
#include "SurfaceFormat.hpp"
#include "TextureHandle.hpp"
#include "Rect.hpp"


namespace amorphous
{


class TextureStage;
class CTextureTransformParams;


class DisplayMode
{
public:
	uint Width;
	uint Height;
	uint RefreshRate;
	TextureFormat::Format Format;

	// true if this is the current display mode.
	// - Invalidated after the graphics device settings are changed
	//   (e.g., resolution, fullscreen/window mode changes).
	// - Call CDirect3D9::GetAdapterModesForDefaultAdapter() to get updated info
	//   after chaging the settings.
	bool Current;

public:

	DisplayMode( uint w=0, uint h=0, uint r=0, TextureFormat::Format fmt = TextureFormat::X8R8G8B8 )
		:
	Width(w),
	Height(h),
	RefreshRate(r),
	Format(fmt),
	Current(false)
	{}
};

class AdapterMode
{
public:

	TextureFormat::Format Format;

	std::vector<DisplayMode> vecDisplayMode;

public:

	AdapterMode( TextureFormat::Format fmt = TextureFormat::X8R8G8B8 )
		:
	Format(fmt)
	{}
};


class Viewport
{
public:
	uint UpperLeftX;
	uint UpperLeftY;
	uint Width;
	uint Height;
	float MinDepth;
	float MaxDepth;

	Viewport()
		:
	UpperLeftX(0),
	UpperLeftY(0),
	Width(0),
	Height(0),
	MinDepth(0.0f),
	MaxDepth(1.0f)
	{}
};


class CullingMode
{
public:
	enum Name
	{
		CLOCKWISE,
		COUNTERCLOCKWISE,
		NUM_CULLING_MODES
	};
};


class CompareFunc
{
public:
	enum Name
	{
		ALWAYS,
		NEVER,
		LESS_THAN,                ///< Passes if the incoming alpha value is less than the reference value
		LESS_THAN_OR_EQUAL_TO,
		EQUAL_TO,
		GREATER_THAN_OR_EQUAL_TO,
		GREATER_THAN,
		NUM_ALPHA_FUNCTIONS
	};
};


class DepthBufferType
{
public:
	enum Name
	{
		ZBUFFER,
		WBUFFER,
		NUM_DEPTH_BUFFER_TYPES
//		DISABLED,
//		NUM_DEPTH_BUFFER_STATES
	};
};


class BufferMask
{
public:
	enum Flag
	{
		COLOR   = (1 << 0),
		DEPTH   = (1 << 1),
		STENCIL = (1 << 2),
	};
};


class ScreenMode
{
public:
	enum Name
	{
		WINDOWED,
		FULLSCREEN,
		NUM_MODES
	};
};


class RenderStateType
{
public:
	enum Name
	{
		DEPTH_TEST,
		ALPHA_BLEND,
		ALPHA_TEST,
		LIGHTING,
		FOG,
		FACE_CULLING,
		WRITING_INTO_DEPTH_BUFFER,
		SCISSOR_TEST,
		NUM_RENDER_STATES
	};
};


class PrimitiveType
{
public:
	enum Name
	{
		TRIANGLE_LIST,
		TRIANGLE_FAN,
		TRIANGLE_STRIP,
		INVALID,
		NUM_PRIMITIVE_TYPES
	};
};


class CGraphicsDevice
{
public:

	enum State
	{
		STATE_NOT_INITIALIZED = 0, ///< Not initialized yet.
		STATE_INITIALIZED,         ///< Has successfully been initialized.
		STATE_INIT_FAILED,         ///< Initialization has failed.
		NUM_STATES
	};

protected:

//	std::vector<AdapterMode> m_vecAdapterMode;

	State m_State;

	/// Each implementation is responsible for setting the culling mode
	/// to this variable in its SetCullingMode().
	CullingMode::Name m_CullMode;

private:

//	void SetDefaultRenderStates();

	/// Retrieve possible adapter modes for the primary display adapter
	/// and store them to m_vecAdapterMode. Called in InitD3D().
//	void EnumAdapterModesForDefaultAdapter();


public:

	CGraphicsDevice() : m_State(STATE_NOT_INITIALIZED) {}

	virtual ~CGraphicsDevice() {}
	
	virtual bool Init( int iWindowWidth, int iWindowHeight, ScreenMode::Name screen_mode ) { return false; }

	//bool ResetD3DDevice( HWND hWnd, int iWindowWidth = 0, int iWindowHeight = 0, bool bFullScreen = false );

//	void Release();


	/// experimental: Could this be a platform-independent way to retrieve resolutions?
	virtual void GetAdapterModesForDefaultAdapter( std::vector<AdapterMode>& dest_buffer ) = 0;

	virtual bool IsCurrentDisplayMode( const DisplayMode& display_mode ) const = 0;
	
//	virtual void SetWorldTransform();
//	virtual void SetViewTransform();
//	virtual void SetProjectionTransform();

	virtual Result::Name SetTexture( int stage, const TextureHandle& texture ) = 0;

	virtual Result::Name SetTextureStageParams( uint stage, const TextureStage& params ) = 0;

	virtual Result::Name SetTextureTrasnformParams( uint stage, const CTextureTransformParams& params ) = 0;

	virtual Result::Name SetTextureCoordTrasnform( uint stage, const Matrix44& transform ) = 0;

	inline Result::Name Enable( RenderStateType::Name type ) { return SetRenderState( type, true ); }

	inline Result::Name Disable( RenderStateType::Name type ) { return SetRenderState( type, false ); }

	virtual bool GetRenderState( RenderStateType::Name type ) = 0;

	virtual Result::Name SetRenderState( RenderStateType::Name type, bool enable ) = 0;

	virtual void SetSourceBlendMode( AlphaBlend::Mode src_blend_mode ) = 0;

	virtual void SetDestBlendMode( AlphaBlend::Mode dest_blend_mode ) = 0;

	virtual void SetAlphaFunction( CompareFunc::Name alpha_func ) = 0;

	virtual void SetReferenceAlphaValue( float ref_alpha ) = 0;

	virtual Result::Name SetFogParams( const FogParams& fog_params ) = 0;

	virtual CullingMode::Name GetCullingMode() { return m_CullMode; }

	virtual Result::Name SetCullingMode( CullingMode::Name cull_mode ) = 0;

	virtual Result::Name GetViewport( Viewport& viewport ) = 0;

	virtual Result::Name SetViewport( const Viewport& viewport ) = 0;

	inline Result::Name GetViewportSize( uint& width, uint& height );

	virtual Result::Name SetClearColor( const SFloatRGBAColor& color ) = 0;

	virtual Result::Name SetClearDepth( float depth ) = 0;

	virtual Result::Name Clear( U32 buffer_mask ) = 0;

	virtual Result::Name BeginScene() { return Result::SUCCESS; }

	virtual Result::Name EndScene() { return Result::SUCCESS; }

	virtual Result::Name Present() { return Result::SUCCESS; }

	virtual Result::Name SetClipPlane( uint index, const Plane& clip_plane ) = 0;

	virtual Result::Name EnableClipPlane( uint index ) = 0;

	virtual Result::Name DisableClipPlane( uint index ) = 0;

	virtual Result::Name UpdateViewProjectionTransformsForClipPlane( uint index, const Matrix44& view_transform, const Matrix44& proj_transform ) = 0;

	virtual Result::Name SetScissorRect( const SRect& rect ) = 0;

	inline Result::Name SetTextureAddressModes2D( uint sampler_index, TextureAddressMode::Name mode_axis_0, TextureAddressMode::Name mode_axis_1 );

	virtual Result::Name SetSamplingParameter( uint sampler_index, SamplingParameter::Name param, uint value ) = 0;

	State GetState() const { return m_State; }
};

//============================== inline implementations ==============================

inline Result::Name CGraphicsDevice::GetViewportSize( uint& width, uint& height )
{
	Viewport vp;
	Result::Name res = GetViewport( vp );
	width  = vp.Width;
	height = vp.Height;
	return res;
}


inline Result::Name CGraphicsDevice::SetTextureAddressModes2D( uint sampler_index, TextureAddressMode::Name mode_axis_0, TextureAddressMode::Name mode_axis_1 )
{
	Result::Name res0 = SetSamplingParameter( sampler_index, SamplingParameter::TEXTURE_WRAP_AXIS_0, mode_axis_0 );
	Result::Name res1 = SetSamplingParameter( sampler_index, SamplingParameter::TEXTURE_WRAP_AXIS_1, mode_axis_1 );

	return (res0 == Result::SUCCESS && res1 == Result::SUCCESS) ? Result::SUCCESS : Result::UNKNOWN_ERROR;
}



class CGraphicsDeviceHolder
{
	CGraphicsDevice *m_pGraphicsDevice;

protected:

	/// singleton
	static singleton<CGraphicsDeviceHolder> m_obj;

public:

	CGraphicsDeviceHolder()
		:
	m_pGraphicsDevice(NULL)
	{}

	static CGraphicsDeviceHolder* Get() { return m_obj.get(); }

	CGraphicsDevice& GetDevice() { return *m_pGraphicsDevice; }

	void SetDevice( CGraphicsDevice *pGraphicsDevice ) { m_pGraphicsDevice = pGraphicsDevice; }
};


inline CGraphicsDevice& GraphicsDevice()
{
	return CGraphicsDeviceHolder::Get()->GetDevice();
}


} // namespace amorphous



#endif // __GraphicsDevice_HPP__
