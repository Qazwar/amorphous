#ifndef  __TextureRenderTarget_HPP__
#define  __TextureRenderTarget_HPP__


#include "FloatRGBAColor.hpp"
#include "SurfaceFormat.hpp"
#include "GraphicsComponentCollector.hpp"
#include "GraphicsResourceDescs.hpp"
#include "TextureHandle.hpp"


namespace amorphous
{


class TextureRenderTarget : public GraphicsComponent
{
protected:

	TextureResourceDesc m_TextureDesc;

	U32 m_OptionFlags;

	TextureHandle m_RenderTargetTexture;

	SFloatRGBAColor m_BackgroundColor;

	// Turned on when an instance is initialized by InitScreenSizeRenderTarget().
	// The texture size is automatically resized to screen size (viewport size)
	// in LoadGraphicsResources()
	bool m_bScreenSizeRenderTarget;

public:

	enum OptionFlags
	{
		OPTFLG_NO_DEPTH_BUFFER    = ( 1 << 0 ),
		OPTFLG_ANOTHER_OPTION     = ( 1 << 1 ),
		OPTFLG_YET_ANOTHER_OPTION = ( 1 << 2 )
	};

	TextureRenderTarget();

	TextureRenderTarget( const TextureResourceDesc& texture_desc );

	virtual ~TextureRenderTarget();

//	TextureRenderTarget( TextureFormat::Format texture_format = A8R8G8B8, uint option_flags = 0 );

//	TextureRenderTarget( int texture_width, int texture_height, TextureFormat::Format texture_format = A8R8G8B8, uint option_flags = 0 );

	/// Returns true on success
	bool Init(
		int texture_width,
		int texture_height,
		TextureFormat::Format texture_format = TextureFormat::A8R8G8B8,
		U32 option_flags = 0 );

	bool Init( const TextureResourceDesc& texture_desc, U32 option_flags = 0 );

	/// Creates the render target of the current screen size
	/// - The texture size is automatically resized to screen size (viewport size)
	///   in LoadGraphicsResources().
	bool InitScreenSizeRenderTarget();

//	void SetTextureWidth( const int width, const int height );

	virtual void SetBackgroundColor( const SFloatRGBAColor& bg_color ) { m_BackgroundColor = bg_color; }

	virtual void SetRenderTarget() {}

	virtual void ResetRenderTarget() {}

	virtual void CopyRenderTarget() {}// { GetRenderTargetData( m_RenderTargetSurface, m_RenderTargetCopySurface ); }

	TextureHandle GetRenderTargetTexture() { return m_RenderTargetTexture; }

	TextureHandle& RenderTargetTexture() { return m_RenderTargetTexture; }

	virtual bool LoadTextures() { return false; }

	virtual void ReleaseTextures() {}

	void ReleaseGraphicsResources();

	void LoadGraphicsResources( const GraphicsParameters& rParam );

	static std::shared_ptr<TextureRenderTarget> (*ms_pCreateTextureRenderTarget)(void);

	static void SetInstanceCreationFunction( std::shared_ptr<TextureRenderTarget> (*CreateTextureRenderTarget)(void) ) { ms_pCreateTextureRenderTarget = CreateTextureRenderTarget; }

	static std::shared_ptr<TextureRenderTarget> Create();
};


} // namespace amorphous



#endif		/*  __TextureRenderTarget_HPP__  */
