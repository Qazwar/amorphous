#include "GameWindowManager_Win32.h"
#include "3DCommon/Direct3D9.h"
#include "3DCommon/GraphicsComponentCollector.h"
#include <math.h>

#include "Support/WindowMisc_Win32.h"
#include "Support/Log/DefaultLog.h"


LRESULT (WINAPI *g_pMessageProcedureForGameWindow)( HWND, UINT, WPARAM, LPARAM ) = NULL;

// definition of the singleton instance
CGameWindowManager_Win32 CGameWindowManager_Win32::ms_SingletonInstance_;


CGameWindowManager_Win32::CGameWindowManager_Win32()
{
	m_iCurrentScreenMode = SMD_WINDOWED;
}


CGameWindowManager_Win32::~CGameWindowManager_Win32()
{
	DIRECT3D9.Release();
    UnregisterClass( "Stage Test", m_WindowClassEx.hInstance );
}


void GetCurrentResolution(int* piDesktopWidth, int* piDesktopHeight)
{
	// experiment : check the current resolution
	RECT desktop_rect;
	HWND hDesktopWnd = GetDesktopWindow();
	GetClientRect( hDesktopWnd, &desktop_rect );

	*piDesktopWidth  = desktop_rect.right - desktop_rect.left;
	*piDesktopHeight = desktop_rect.bottom   - desktop_rect.top;
}


bool CGameWindowManager_Win32::CreateGameWindow(int iScreenWidth, int iScreenHeight, int screen_mode )
{
	// detemine the specification of the window class
	m_WindowClassEx.cbSize		= sizeof(WNDCLASSEX); 
    m_WindowClassEx.style			= CS_CLASSDC; 
//    m_WindowClassEx.lpfnWndProc	= MsgProc;
//    m_WindowClassEx.lpfnWndProc	= m_pMsgProc; 		// error: wrong function pointer
    m_WindowClassEx.lpfnWndProc	= g_pMessageProcedureForGameWindow; 		// correct: a valid function pointer to a message procedure

    m_WindowClassEx.cbClsExtra	= 0L; 
    m_WindowClassEx.cbWndExtra	= 0L; 
    m_WindowClassEx.hInstance		= GetModuleHandle(NULL); 
    m_WindowClassEx.hIcon			= NULL; 
    m_WindowClassEx.hCursor       = NULL; 
    m_WindowClassEx.hbrBackground = NULL; 
    m_WindowClassEx.lpszMenuName  = NULL; 
    m_WindowClassEx.lpszClassName = "Stage Test"; 
    m_WindowClassEx.hIconSm		= NULL;

    // register the window class
    RegisterClassEx( &m_WindowClassEx );

	int iDesktopWidth, iDesktopHeight;
	GetCurrentResolution( &iDesktopWidth, &iDesktopHeight );

    // create the application's window
    m_hWnd = CreateWindow( "Stage Test", "D3D Stage Test",
 //                          /*WS_OVERLAPPEDWINDOW*/ WS_POPUPWINDOW,
                           WS_OVERLAPPED,
						   (iDesktopWidth  - iScreenWidth ) / 2,
						   (iDesktopHeight - iScreenHeight) / 2,
						   iScreenWidth,
						   iScreenHeight,
                           GetDesktopWindow(),
						   NULL,
						   m_WindowClassEx.hInstance,
						   NULL );

	if( screen_mode == SMD_WINDOWED )
		ChagneClientAreaSize( m_hWnd, iScreenWidth, iScreenHeight );

	g_Log.Print( "window created (size: %d x %d)", iScreenWidth, iScreenHeight );

    // initialize Direct3D & Create the scene geometry
	int d3d_screen_mode = screen_mode == SMD_WINDOWED ? CDirect3D9::WINDOWED : CDirect3D9::FULLSCREEN;
    if( !DIRECT3D9.InitD3D( m_hWnd, iScreenWidth, iScreenHeight, d3d_screen_mode ) )
		return false;

//	g_Log.Print( "CGameWindowManager_Win32::CreateGameWindow() - Direct3D initialized" );

	// show the window
    ShowWindow( m_hWnd, SW_SHOWDEFAULT );
    UpdateWindow( m_hWnd );

	g_Log.Print( "CGameWindowManager_Win32::CreateGameWindow() - window displayed and updated" );

	m_iCurrentScreenWidth  = iScreenWidth;
	m_iCurrentScreenHeight = iScreenHeight;

	// set the window size for all the game components
//	CGameComponent::SetScreenSize( iScreenWidth, iScreenHeight );

	// update parameters for GraphicsComponents
	CGraphicsParameters params;
	params.ScreenWidth  = iScreenWidth;
	params.ScreenHeight = iScreenHeight;
	params.bWindowed    = screen_mode == SMD_WINDOWED ? true : false;

	CGraphicsComponentCollector::Get()->SetGraphicsPargams( params );

	return true;
}


void CGameWindowManager_Win32::ChangeScreenSize( int iNewScreenWidth,
												 int iNewScreenHeight,
												 bool bFullScreen )
{
	// check if the requested window size has a valid aspect ratio
	// (width : height) must be 4:3
	if( 0.01f < fabs( (float)iNewScreenHeight / (float)iNewScreenWidth - 0.75f ) )
		return;


	// release all the graphic resources ( textures, vertex buffers, mesh models, and so on )
	CGraphicsComponentCollector::Get()->ReleaseGraphicsResources();

	if( !DIRECT3D9.ResetD3DDevice( m_hWnd, iNewScreenWidth, iNewScreenHeight, bFullScreen ) )
	{
		// the requested resolution is not available - restore the current settings
		bool bCurrentModeFullScreen = (m_iCurrentScreenMode == SMD_FULLSCREEN) ? true : false;
		DIRECT3D9.ResetD3DDevice( m_hWnd, m_iCurrentScreenWidth, m_iCurrentScreenHeight, bCurrentModeFullScreen );
	}

	// update the current resolution and the screen mode
	m_iCurrentScreenWidth  = iNewScreenWidth;
	m_iCurrentScreenHeight = iNewScreenHeight;
	m_iCurrentScreenMode   = SMD_WINDOWED;


	CGraphicsParameters param;
	param.ScreenWidth = iNewScreenWidth;
	param.ScreenHeight = iNewScreenHeight;
	param.bWindowed = (!bFullScreen);

	// notify all the graphics components to load their resources
	CGraphicsComponentCollector::Get()->LoadGraphicsResources( param );

	// notify changes to all the game components
//	GAMECOMPONENTCOLLECTOR.AdaptToNewScreenSize();

	// adjust the position of the window so that the game screen appear in the middle of the display
	int iDesktopWidth, iDesktopHeight;
	GetCurrentResolution( &iDesktopWidth, &iDesktopHeight );
	int iResult = ::SetWindowPos(m_hWnd, HWND_TOP,
		                         (iDesktopWidth  - m_iCurrentScreenWidth ) / 2,
							     (iDesktopHeight - m_iCurrentScreenHeight) / 2,
							     m_iCurrentScreenWidth  + GetNonClientAreaWidth(m_hWnd),
							     m_iCurrentScreenHeight + GetNonClientAreaHeight(m_hWnd),
							     0 );

}


bool CGameWindowManager_Win32::IsMouseCursorInClientArea()
{
	long frame_w = 0, frame_h = 0;
	GetNonClientAreaSize( m_hWnd, frame_w, frame_h );

	RECT window_rect;
	GetWindowRect( m_hWnd, &window_rect );

	// TODO: accurate client rect position
	RECT offset_window_rect = window_rect;
	offset_window_rect.top   += frame_h;
	offset_window_rect.left  += frame_w;
	offset_window_rect.right -= frame_w;

	POINT pt;
	GetCursorPos( &pt );

	if( offset_window_rect.top <= pt.y
	 && pt.y <= offset_window_rect.bottom
	 && offset_window_rect.left <= pt.x
	 && pt.x <= offset_window_rect.right )
	{
		return true;
	}
	else
		return false;
}

void CGameWindowManager_Win32::SetWindowLeftTopCornerPosition( int left, int top )
{
	// set the position
	// - use SWP_NOSIZE flag to ignore window size parameters
	::SetWindowPos( m_hWnd, HWND_TOP, left, top, 0, 0, SWP_NOSIZE );
}


/*
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// �t���X�N���[���ɂ��邩�ǂ����̔���
	// �R�}���h���C����/f��/F���ݒ肳��Ă�����t���X�N���[���ɂ���
	BOOL isFullScreen = FALSE;
    for(int i = 0; i < nCmdShow; i++) {
		if(_stricmp((char*)&lpCmdLine[i], "/f") == 0) {	// �R�}���h���C����/f�𔭌�
			isFullScreen = TRUE;	 // �t���O��TRUE�ɐݒ�
			break;
		}
    }

	char clsName[]		= "D3DFWSampleClass";	// �E�B���h�E�N���X��

	HWND		hWnd;
	MSG			msg;

	// �E�B���h�E�N���X�̏�����
	WNDCLASSEX	wcex = {
		sizeof(WNDCLASSEX),				// ���̍\���̂̃T�C�Y
		NULL,							// �E�C���h�E�̃X�^�C��(default)
		WindowProc,						// ���b�Z�[�W�����֐��̓o�^
		0,								// �ʏ�͎g��Ȃ��̂ŏ��0
		0,								// �ʏ�͎g��Ȃ��̂ŏ��0
		hInstance,						// �C���X�^���X�ւ̃n���h��
		NULL,							// �A�C�R���i�Ȃ��j
		LoadCursor(NULL, IDC_ARROW),	// �J�[�\���̌`
		NULL, NULL,						// �w�i�Ȃ��A���j���[�Ȃ�
		clsName,						// �N���X���̎w��
		NULL							// ���A�C�R���i�Ȃ��j
	};

	// �E�B���h�E�N���X�̓o�^
	if(RegisterClassEx(&wcex) == 0){
		return 0;	// �o�^���s
	}
	
	// �E�B���h�E�̍쐬
	if(isFullScreen) { // �t���X�N���[��
		int sw;
		int sh;
		// ��ʑS�̂̕��ƍ������擾
		sw = GetSystemMetrics(SM_CXSCREEN);
		sh = GetSystemMetrics(SM_CYSCREEN);

		hWnd = CreateWindow( 
					clsName, 				// �o�^����Ă���N���X��
					WINDOW_NAME, 			// �E�C���h�E��
					WS_POPUP,				// �E�C���h�E�X�^�C���i�|�b�v�A�b�v�E�C���h�E���쐬�j
					0, 						// �E�C���h�E�̉������̈ʒu
					0, 						// �E�C���h�E�̏c�����̈ʒu
					CLIENT_WIDTH, 			// �E�C���h�E�̕�
					CLIENT_HEIGHT,			// �E�C���h�E�̍���
					NULL,					// �e�E�C���h�E�̃n���h���i�ȗ��j
					NULL,					// ���j���[��q�E�C���h�E�̃n���h��
					hInstance, 				// �A�v���P�[�V�����C���X�^���X�̃n���h��
					NULL					// �E�C���h�E�̍쐬�f�[�^
				);
	}
	else {
		hWnd = CreateWindow(clsName, 
							WINDOW_NAME, 
							WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT, CW_USEDEFAULT, 
							CW_USEDEFAULT, CW_USEDEFAULT,
							NULL, NULL, hInstance, NULL);

		// �E�B���h�E�T�C�Y���Đݒ肷��
		RECT rect;
		int ww, wh;
		int cw, ch;
		// �N���C�A���g�̈�̊O�̕����v�Z
		GetClientRect(hWnd, &rect);		// �N���C�A���g�����̃T�C�Y�̎擾
		cw = rect.right - rect.left;	// �N���C�A���g�̈�O�̉������v�Z
		ch = rect.bottom - rect.top;	// �N���C�A���g�̈�O�̏c�����v�Z

		// �E�C���h�E�S�̂̉����̕����v�Z
		GetWindowRect(hWnd, &rect);		// �E�C���h�E�S�̂̃T�C�Y�擾
		ww = rect.right - rect.left;	// �E�C���h�E�S�̂̕��̉������v�Z
		wh = rect.bottom - rect.top;	// �E�C���h�E�S�̂̕��̏c�����v�Z
		ww = ww - cw;					// �N���C�A���g�̈�ȊO�ɕK�v�ȕ�
		wh = wh - ch;					// �N���C�A���g�̈�ȊO�ɕK�v�ȍ���

		// �E�B���h�E�T�C�Y�̍Čv�Z
		ww = CLIENT_WIDTH + ww;			// �K�v�ȃE�C���h�E�̕�
		wh = CLIENT_HEIGHT + wh;		// �K�v�ȃE�C���h�E�̍���

		// �E�C���h�E�T�C�Y�̍Đݒ�
		SetWindowPos(hWnd, HWND_TOP, 0, 0, ww, wh, SWP_NOMOVE);

	}
*/
