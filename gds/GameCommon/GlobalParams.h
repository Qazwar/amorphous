#ifndef __GLOBALPARAMS_H__
#define __GLOBALPARAMS_H__


#include <stdio.h>
#include "Support/ParamLoader.h"


//>>>========================== default config file ==========================
/*
resolution 800	600
fullscreen	yes
*/
//<<<========================== default config file ==========================



class CGlobalParams
{
public:

	bool FullScreen;
	int ScreenWidth;
	int ScreenHeight;

	/// top-left corner of the window
	/// - valid only in windowed mode
	/// - set to -1 if not specified,
	/// and the window will be placed in the center of the desktop
	int WindowLeftPos;
	int WindowTopPos;

	std::string ScreenshotImageFormat;
	int ScreenshotResolutionWidth;
	int ScreenshotResolutionHeight;
//	std::string ScreenshotOutputDirectory;

public:

	CGlobalParams()
		:
	FullScreen(true),
	ScreenWidth(800),
	ScreenHeight(600),
	WindowLeftPos(-1),
	WindowTopPos(-1),
	ScreenshotImageFormat( "bmp" ),
	ScreenshotResolutionWidth( -1 ),
	ScreenshotResolutionHeight( -1 )
	{}

	bool LoadFromFile( const std::string& filename )
	{
		CParamLoader loader( filename );

		if( !loader.IsReady() )
			return false;

		loader.LoadBoolParam( "fullscreen", "yes/no", FullScreen );
		loader.LoadParam( "resolution",        ScreenWidth, ScreenHeight );
		loader.LoadParam( "window_pos",        WindowLeftPos, WindowTopPos );
		loader.LoadParam( "screenshot_format", ScreenshotImageFormat );
		loader.LoadParam( "screenshot_resolution", ScreenshotResolutionWidth, ScreenshotResolutionHeight );

		return true;
	}
};


extern CGlobalParams GlobalParams;


#endif  /*  __GLOBALPARAMS_H__  */
