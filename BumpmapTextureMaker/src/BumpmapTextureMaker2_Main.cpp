//-----------------------------------------------------------------------------
// File: BumpmapTextureMaker_Main.cpp
// Desc: 
//-----------------------------------------------------------------------------
#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <direct.h>

#include "Graphics/Direct3D9.hpp"
#include "Graphics/Font/Font.hpp"
#include "Support/Timer.hpp"
#include "Support/FileOpenDialog_Win32.hpp"
#include "Support/fnop.hpp" // deprecated
#include "Support/lfs.hpp"
#include "Support/Log/DefaultLog.hpp"
#include "Support/MiscAux.hpp"

#include "BumpmapTextureMaker2_LWO2.h"

//#include <vld.h>

// draft
extern void SetCurrentThreadAsRenderThread();


#define	WINDOW_WIDTH	(800)
#define	WINDOW_HEIGHT	(800)

#define WINDOW_SIZE		(512)


//----------------------  Global Variables  -----------------------------------
//CFont g_Font;
CBumpmapTextureMaker2_LWO2 *g_pBumpTexMaker = NULL;
//CCameraController_Win32 g_CameraController;

//----------------------  Forward Declaration  -----------------------------------


//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{

    switch( msg )
    {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
				
		case WM_KEYDOWN:
			switch( wParam )
			{
			case '1':
				g_pBumpTexMaker->SetRenderMode( BTM_RENDERMODE_NORMALMAP );
				break;
			case '2':
				g_pBumpTexMaker->SetRenderMode( BTM_RENDERMODE_FLAT_TEXTURED_SURFACE );
				break;
			case '3':
				g_pBumpTexMaker->SetRenderMode( BTM_RENDERMODE_BUMPY_TEXTURED_SURFACE );
				break;
			case '5':
				g_pBumpTexMaker->SetRenderMode( BTM_RENDERMODE_PREVIEW );
				break;

			case VK_UP:
//				g_fViewVolumeWidth -= 0.1f;
				if( g_pBumpTexMaker )
					g_pBumpTexMaker->SetViewWidth( g_pBumpTexMaker->GetViewWidth() - 0.1f );
				break;

			case VK_DOWN:
//				g_fViewVolumeWidth += 0.1f;
				if( g_pBumpTexMaker )
					g_pBumpTexMaker->SetViewWidth( g_pBumpTexMaker->GetViewWidth() + 0.1f );
				break;

			case VK_F1:
				break;
			case VK_F2:
				break;
			case VK_F4:
				break;
			case VK_F5:
				break;
			case VK_F7:
				break;
			case VK_F12:
				if( g_pBumpTexMaker )
				{
					g_pBumpTexMaker->SaveImages();
				}
				break;
			}
			break;

		case WM_KEYUP:
			break;

		case WM_MOUSEMOVE:
			if( g_pBumpTexMaker && wParam & MK_RBUTTON )
			{
				float scale  = 1.0f;
				float angle_y = (float)LOWORD(lParam) / (float)WINDOW_WIDTH  - 0.5f;
				float angle_x = (float)HIWORD(lParam) / (float)WINDOW_HEIGHT - 0.5f;
				g_pBumpTexMaker->SetDirectionalLightAngleX( angle_x * scale );
				g_pBumpTexMaker->SetDirectionalLightAngleY( angle_y * scale * (-1.0f) );
			}
			break;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}


void Render()
{
	LPDIRECT3DDEVICE9 pd3dDevice = DIRECT3D9.GetDevice();

//	g_CameraController.SetCameraMatrix();

	D3DXMATRIX matWorld;

	D3DXMatrixIdentity( &matWorld );
	pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	g_pBumpTexMaker->Render();
}


bool Init( LPSTR lpCmdLine )
{
	return true;
}


void GetWindowFrameSize( HWND hWnd, long& frame_width, long& frame_height )
{
	RECT rect;
	int ww, wh, cw, ch;

	// calc boundary width outside the client rect
	GetClientRect(hWnd, &rect);		// �N���C�A���g�����̃T�C�Y�̎擾
	cw = rect.right - rect.left;	// �N���C�A���g�̈�O�̉������v�Z
	ch = rect.bottom - rect.top;	// �N���C�A���g�̈�O�̏c�����v�Z

	// calc the entire window size
	GetWindowRect(hWnd, &rect);		// �E�C���h�E�S�̂̃T�C�Y�擾
	ww = rect.right - rect.left;	// �E�C���h�E�S�̂̕��̉������v�Z
	wh = rect.bottom - rect.top;	// �E�C���h�E�S�̂̕��̏c�����v�Z
	frame_width  = ww - cw;			// �N���C�A���g�̈�ȊO�ɕK�v�ȕ�
	frame_height = wh - ch;			// �N���C�A���g�̈�ȊO�ɕK�v�ȍ���
}


//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, INT )
{
	// draft - register the render thread id
	SetCurrentThreadAsRenderThread();

    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "D3DTest", NULL };
    RegisterClassEx( &wc );

    // Create the application's window
    HWND hWnd = CreateWindow( "D3DTest", "Bumpmap Texture Maker",
                              WS_OVERLAPPEDWINDOW, 400, 100, WINDOW_WIDTH, WINDOW_HEIGHT,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );


	// resizing the window size
	long frame_width, frame_height;
	GetWindowFrameSize( hWnd, frame_width, frame_height );

	// �E�C���h�E�T�C�Y�̍Đݒ�
	SetWindowPos(hWnd, HWND_TOP, 0, 0, WINDOW_SIZE + frame_width, WINDOW_SIZE + frame_height, SWP_NOMOVE);

    // Initialize Direct3D
	if( !DIRECT3D9.InitD3D(hWnd) )
		return 0;

    // Show the window
    ShowWindow( hWnd, SW_SHOWDEFAULT );
    UpdateWindow( hWnd );

	if( !Init(lpCmdLine) )
		return 1;

	// push current directory and move to the directory where the program is placed
	char exe_filename[1024];
	memset( exe_filename, 0, sizeof(exe_filename) );
	GetModuleFileName( NULL, exe_filename, 1023 );

	fnop::dir_stack dirstack;
	dirstack.setdir( fnop::get_path(exe_filename) );

	CLogOutput_HTML html_log( "BumpmpaTextureMaker2_" + string(GetBuildInfo()) + ".html" );
	g_Log.AddLogOutput( &html_log );

	g_pBumpTexMaker = new CBumpmapTextureMaker2_LWO2;

	string shader_filename = "..\\resources\\Shaders\\bump_tex_maker.fx";

	bool shader_loaded = g_pBumpTexMaker->LoadShader( shader_filename );
	if( !shader_loaded )
		LOG_PRINT_ERROR( " - failed to load shader: " + shader_filename );

	// pop the previous working directory
	dirstack.prevdir();

	char acFilename[1024];
	if( 0 < strlen(lpCmdLine) )
	{	// a filename is specified as an argument
		// trim the double quotation characters at the beginning and the end of the string
		strcpy( acFilename, lpCmdLine+1 );
		acFilename[ strlen(acFilename)-1 ] = '\0';
	}
	else
	{
		if( !GetFilename(acFilename, NULL) )
			return 0;
	}

	if( !g_pBumpTexMaker->LoadModel( acFilename ) )
		return 0;

//	g_pBumpTexMaker->SetViewWidth( 1.0f );

	D3DXMATRIX matCamera;

	D3DXMatrixLookAtLH( &matCamera, &D3DXVECTOR3(0, 0, 5), &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(0, 1, 0) );

	// rotate camera 180 degrees so that it face against positive z direction
//	g_CameraController.AddYaw( 3.14159265359f );


	// initialize font object
//	g_Font.InitFont( "�l�r �S�V�b�N", 12, 24);

//	g_TexFont.InitFont( "Texture\\MainFont.dds", 256, 256, 16, 8 );
//	g_TexFont.SetDefaultLetterSize( 8, 16 );

    // Enter the message loop
    MSG msg;
    ZeroMemory( &msg, sizeof(msg) );
    while( msg.message!=WM_QUIT )
    {
        if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
		{
			// Update FPS
			GlobalTimer().UpdateFrameTime();

//			g_CameraController.UpdateCameraPosition();

			//=======================   Rendering start   ===========================
			LPDIRECT3DDEVICE9 pd3dDevice = DIRECT3D9.GetDevice();

			pd3dDevice->SetTransform( D3DTS_VIEW, &matCamera );

//		    pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );
		    pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(128,128,128), 1.0f, 0 );

			// Begin the scene
			pd3dDevice->BeginScene();

			pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
//			pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_USEW);


			Render();

			// End the scene
		    pd3dDevice->EndScene();
		    // Present the backbuffer contents to the display
		    pd3dDevice->Present( NULL, NULL, NULL, NULL );

			Sleep( 10 );
		}
    }

	SafeDelete( g_pBumpTexMaker );

	// release any singleton class that inherits CGraphicsComponent
	CGraphicsResourceManager::ReleaseSingleton();

    // Clean up everything before finishing
	DIRECT3D9.Release();
    UnregisterClass( "D3DTest", wc.hInstance );

    return 0;
}





