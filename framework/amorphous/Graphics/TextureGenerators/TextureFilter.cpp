#include "TextureFilter.hpp"


namespace amorphous
{


void ContrastBrightnessFilter::ApplyFilter( LockedTexture& texture )
{
	SFloatRGBAColor min_color( 1000.0f, 1000.0f, 1000.0f, 1.0f );
	SFloatRGBAColor max_color(-1000.0f,-1000.0f,-1000.0f, 1.0f );
	const int w = texture.GetWidth();
	const int h = texture.GetHeight();
	for( int y=0; y<h; y++ )
	{
		for( int x=0; x<w; x++ )
		{
			SFloatRGBAColor src;
			texture.GetPixel( x, y, src );
			min_color.red   = take_min( min_color.red,   src.red );
			min_color.green = take_min( min_color.green, src.green );
			min_color.blue  = take_min( min_color.blue,  src.blue );
			max_color.red   = take_max( max_color.red,   src.red );
			max_color.green = take_max( max_color.green, src.green );
			max_color.blue  = take_max( max_color.blue,  src.blue );
		}
	}

	float min_val = take_min( min_color.red, min_color.green, min_color.blue );
	float max_val = take_max( max_color.red, max_color.green, max_color.blue );

	LOG_PRINTF(( "min_val: %f, max_val: %f", min_val, max_val ));

	float contrast_factor = m_ContrastFactor;
	float brightness_shift = m_BrightnessShift;
	for( int y=0; y<h; y++ )
	{
		for( int x=0; x<w; x++ )
		{
			SFloatRGBAColor src;
			texture.GetPixel( x, y, src );
			src.red   = get_clamped( src.red   * contrast_factor + brightness_shift, 0.0f, 1.0f );
			src.green = get_clamped( src.green * contrast_factor + brightness_shift, 0.0f, 1.0f );
			src.blue  = get_clamped( src.blue  * contrast_factor + brightness_shift, 0.0f, 1.0f );
			texture.SetPixelARGB32( x, y, src.GetARGB32() );
		}
	}
}


} // amorphous
