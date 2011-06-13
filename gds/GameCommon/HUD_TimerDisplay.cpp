#include "HUD_TimerDisplay.hpp"
#include "Graphics/Font/FontBase.hpp"


void HUD_TimerDisplay::Render( CFontBase *pFont, int sx, int sy )
{
	if( !m_bDisplay 
	 || !pFont )
		return;

	pFont->SetFontColor( m_Color );

	GetTimeMMSS( m_TextBuffer );

	pFont->DrawText( m_TextBuffer, sx, sy );
}
