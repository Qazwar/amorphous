#ifndef  __GAMETEXTWINDOW_H__
#define  __GAMETEXTWINDOW_H__


#include "amorphous/3DMath/aabb2.hpp"
#include "amorphous/Graphics/fwd.hpp"
#include "amorphous/Graphics/2DPrimitive/2DRect.hpp"
#include "amorphous/Graphics/GraphicsComponentCollector.hpp"
#include "amorphous/Graphics/TextureHandle.hpp"

#include "amorphous/Support/FixedVector.hpp"


namespace amorphous
{

/*
enum eGameTextWindowState
{
	GTW_CLOSED = 0,
	GTW_OPEN,
	GTW_WAITING_USER_RESPONSE,
};
*/

enum eGameTextWindowInput
{
	GTW_INPUT_NOINPUT = 0,
	GTW_INPUT_CANCEL,
	GTW_INPUT_OK,
};


#define NUM_MAX_ICONTEXTURES	64

#define NUM_MAX_TEXTWINDOW_LINES	16

class CGameTextComponent;
class CGameTextSet;

class CGameTextWindow : public GraphicsComponent
{
//	bool m_bWaitingUserResponse;
	int m_iCurrentState;

	int m_iNumMaxLines;

	/// how many letters in one line (in bytes)
	int m_iNumMaxCharsPerLine;

	int m_iCurrentLine;

	float m_fNumCharsForCurrentLine;

	CGameTextComponent *m_apLine[NUM_MAX_TEXTWINDOW_LINES];

	/// text set currently being displayed
	CGameTextSet* m_pCurrentTextSet;

	/// a text component which is currently being handled
	int m_iCurrentComponent;

	/// bounding box of the window
	AABB2 m_BoundingRect;

	/// background rectangle of window
	C2DRect m_WindowRect;

	/// texture for window rectangle
	TextureHandle m_WindowTexture;

	C2DRect m_TexturedIcon;

	TCFixedVector< TextureHandle, NUM_MAX_ICONTEXTURES > m_IconTexture;

//	int m_iNumIconTextures;
//	char m_acTextureFilename[NUM_MAX_ICONTEXTURES][256];
//	LPDIRECT3DTEXTURE9 m_apTexture[NUM_MAX_ICONTEXTURES];

	FontBase* m_pFont;
	std::string m_strFontName;
	float m_fFontWidth;
	float m_fFontHeight;


public:
	CGameTextWindow();
	~CGameTextWindow();

	void InitFont( const char *pcFontName, float fFontWidth, float fFontHeight );

	void SetNumMaxCharsPerLine( int num ) { m_iNumMaxCharsPerLine = num; }

	/// set a text set and initiate display
	void OpenTextWindow( CGameTextSet* pGameTextSet );

	void Render(int iInput);

	inline CGameTextSet* GetCurrentTextSet() { return m_pCurrentTextSet; }

	inline int GetCurrentState() { return m_iCurrentState; }

	enum eGameTextWindowState
	{
		STATE_CLOSED = 0,
		STATE_OPEN,
		STATE_WAITING_USER_RESPONSE,
		STATE_1,
		STATE_2
	};

	void UpdateScreenSize();

	void ReleaseGraphicsResources();
	void LoadGraphicsResources( const GraphicsParameters& rParam );

};

} // namespace amorphous



#endif		/*  __GAMETEXTWINDOW_H__  */