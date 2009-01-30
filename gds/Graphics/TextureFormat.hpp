#ifndef  __TextureFormat_H__
#define  __TextureFormat_H__


#include "Support/Log/DefaultLog.hpp"


class TextureFormat
{
public:
	enum Format
	{
		Invalid = -1,
		R16F = 0,
		A16R16G16B16F,
		A8R8G8B8,
		X8R8G8B8,
		A1R5G5B5,
		X1R5G5B5,
		R5G6B5,
		G16R16F, ///< For variance shadow map with Direct3D
		G32R32F, ///< For variance shadow map with Direct3D
		NumFormats
	};
};


#include <d3dx9.h>

inline TextureFormat::Format FromD3DSurfaceFormat( D3DFORMAT d3d_fmt )
{
	switch(d3d_fmt)
	{
	case D3DFMT_R16F:          return TextureFormat::R16F;
	case D3DFMT_A16B16G16R16F: return TextureFormat::A16R16G16B16F;
	case D3DFMT_A8R8G8B8:      return TextureFormat::A8R8G8B8;
	case D3DFMT_X8R8G8B8:      return TextureFormat::X8R8G8B8;
	case D3DFMT_R5G6B5:        return TextureFormat::R5G6B5;
	case D3DFMT_A1R5G5B5:      return TextureFormat::A1R5G5B5;
	case D3DFMT_X1R5G5B5:      return TextureFormat::X1R5G5B5;
	default:
		LOG_PRINT_ERROR( "An invalid D3DFORMAT" );
		return TextureFormat::Invalid;
	}
}


inline D3DFORMAT ConvertTextureFormatToD3DFORMAT( TextureFormat::Format tex_format )
{
	switch(tex_format)
	{
	case TextureFormat::R16F:          return D3DFMT_R16F;
	case TextureFormat::A16R16G16B16F: return D3DFMT_A16B16G16R16F;
	case TextureFormat::A8R8G8B8:      return D3DFMT_A8R8G8B8;
	case TextureFormat::X8R8G8B8:      return D3DFMT_X8R8G8B8;
	case TextureFormat::R5G6B5:        return D3DFMT_R5G6B5;
	case TextureFormat::A1R5G5B5:      return D3DFMT_A1R5G5B5;
	case TextureFormat::G16R16F:       return D3DFMT_G16R16F;
	case TextureFormat::G32R32F:       return D3DFMT_G32R32F;
	default:
		LOG_PRINT_ERROR( "An invalid texture format" );
		return D3DFMT_A8R8G8B8;
	}
}


#endif		/*  __TextureFormat_H__  */
