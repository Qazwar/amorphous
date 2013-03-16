#include "PerlinNoiseTextureGeneratorTest.hpp"
#include "gds/Graphics/2DPrimitive/2DRect.hpp"
#include "gds/Graphics/Camera.hpp"
#include "gds/Graphics/Font/BuiltinFonts.hpp"
#include "gds/Graphics/TextureGenerators/PerlinNoiseTextureGenerator.hpp" //<<< This header contains the classes to test
#include "gds/Support/Timer.hpp"
#include "gds/Support/Profile.hpp"
#include "gds/Support/ParamLoader.hpp"
#include "gds/Support/Macro.h"
#include "gds/Support/MTRand.hpp"
#include "gds/Utilities/PerlinAux.hpp"
#include "gds/GUI.hpp"

using std::string;
using namespace boost;


CPerlinNoiseTextureGeneratorTest::CPerlinNoiseTextureGeneratorTest()
:
m_ImageWidth( 64 ),
m_fTextureRepeats( 1.0f )
{
	SetBackgroundColor( SFloatRGBAColor( 0.2f, 0.2f, 0.2f, 1.0f ) );

	g_Camera.SetPosition( Vector3( 0, 1, -120 ) );
//	g_Camera.SetPosition( Vector3( 0, 520, 120 ) );

	m_pTimer.reset( new Timer );

	InitRand( (unsigned long)timeGetTime() );
}


CPerlinNoiseTextureGeneratorTest::~CPerlinNoiseTextureGeneratorTest()
{
}


bool CPerlinNoiseTextureGeneratorTest::InitShader()
{
	// initialize shader
/*	bool shader_loaded = m_Shader.Load( "./shaders/PerlinNoiseTextureGeneratorTest.fx" );
	
	if( !shader_loaded )
		return false;

	ShaderLightManager *pShaderLightMgr = m_Shader.GetShaderManager()->GetShaderLightManager().get();

	HemisphericDirectionalLight light;
	light.Attribute.UpperDiffuseColor.SetRGBA( 1.0f, 1.0f, 1.0f, 1.0f );
	light.Attribute.LowerDiffuseColor.SetRGBA( 0.1f, 0.1f, 0.1f, 1.0f );
	light.vDirection = Vec3GetNormalized( Vector3( -1.0f, -1.8f, -0.9f ) );

//	pShaderLightMgr->SetLight( 0, light );
//	pShaderLightMgr->SetDirectionalLightOffset( 0 );
//	pShaderLightMgr->SetNumDirectionalLights( 1 );
	pShaderLightMgr->SetHemisphericDirectionalLight( light );

	Matrix44 proj = Matrix44PerspectiveFoV_LH( (float)PI / 4, 640.0f / 480.0f, 0.1f, 500.0f );
	m_Shader.GetShaderManager()->SetProjectionTransform( proj );
*/
	return true;
}


void CPerlinNoiseTextureGeneratorTest::CreatePerlinNoiseTexture()
{
	CPerlinNoiseParams pn_params;
	pn_params.seed = (int)RandInt32();
//	int octaves = 1; float freq = 1; float amp = 1; int seed = 0;
	const char *params_file = "PerlinNoiseTextureGeneratorDemo/params.txt";
	LoadParamFromFile( params_file, "image_size",      m_ImageWidth );
	LoadParamFromFile( params_file, "texture_repeats", m_fTextureRepeats );

	int tilable = 0;
	LoadParamFromFile( params_file, "octaves", pn_params.octaves );
	LoadParamFromFile( params_file, "freq",    pn_params.freq );
	LoadParamFromFile( params_file, "amp",     pn_params.amp );
	LoadParamFromFile( params_file, "range",   pn_params.min_value, pn_params.max_value );
	LoadParamFromFile( params_file, "seed",    pn_params.seed );
	LoadParamFromFile( params_file, "tilable", tilable );
	pn_params.tilable = (tilable!=0) ? true : false;

	bool loaded = false;

	TextureResourceDesc tex_desc;
	tex_desc.pLoader.reset( new PerlinNoiseTextureGenerator(pn_params) );
	tex_desc.Width  = m_ImageWidth;
	tex_desc.Height = m_ImageWidth;
	tex_desc.Format = TextureFormat::A8R8G8B8;

	loaded = m_PerlinNoiseTexture.Load( tex_desc );

	TextureResourceDesc nmap_desc;
	nmap_desc.pLoader.reset( new PerlinNoiseNormalMapGenerator(pn_params) );
	nmap_desc.Width  = m_ImageWidth;
	nmap_desc.Height = m_ImageWidth;
	nmap_desc.Format = TextureFormat::A8R8G8B8;

	loaded = m_PerlinNoiseNormalMap.Load( nmap_desc );
}


int CPerlinNoiseTextureGeneratorTest::Init()
{
	shared_ptr<FontBase> pFont( CreateDefaultBuiltinFont() );
	pFont->SetFontSize( 6, 12 );

	CreatePerlinNoiseTexture();

	return 0;
}




void CPerlinNoiseTextureGeneratorTest::Update( float dt )
{
}


void CPerlinNoiseTextureGeneratorTest::Render()
{
	PROFILE_FUNCTION();

/*	shared_ptr<CGraphicsResourceEntry> pTexEntry( m_PerlinNoiseTexture.GetEntry() );
	if( pTexEntry )
	{
		shared_ptr<TextureResource> pTex = GetTextureResource();
		if( pTex )
	}*/

	int w = m_ImageWidth;
	float t = m_fTextureRepeats;
	C2DRect rect( 0, 0, m_ImageWidth - 1, m_ImageWidth - 1 );
	rect.SetTextureUV( TEXCOORD2(0,0), TEXCOORD2(t,t) );
	rect.SetColor( SFloatRGBAColor::White() );
	rect.Draw( m_PerlinNoiseTexture );

	rect.SetPositionLTWH( w, 0, w, w );
	rect.Draw( m_PerlinNoiseNormalMap );

/*	GraphicsResourceManager().GetStatus( GraphicsResourceType::Texture, m_TextBuffer );

	Vector2 vTopLeft(     GetWindowWidth() / 4,  16 );
	Vector2 vBottomRight( GetWindowWidth() - 16, GetWindowHeight() * 3 / 2 );
	C2DRect rect( vTopLeft, vBottomRight, 0x50000000 );
	rect.Draw();

	m_pFont->DrawText( m_TextBuffer, vTopLeft );
*/
}


void CPerlinNoiseTextureGeneratorTest::SaveTexturesAsImageFiles()
{
	TextureHandle textures[] = { m_PerlinNoiseTexture, m_PerlinNoiseNormalMap };
	const char *names[] = { "pn", "pn_nmap" };
	static int s_counter = 0;

	for( int i=0; i<2; i++ )
	{
		shared_ptr<GraphicsResourceEntry> pEntry = textures[i].GetEntry();
		if( !pEntry )
			continue;

		shared_ptr<TextureResource> pTexture = pEntry->GetTextureResource();
		if( !pTexture )
			continue;

		pTexture->SaveTextureToImageFile( fmt_string( "./%s%02d.png", names[i], s_counter++ ) );
	}
}


void CPerlinNoiseTextureGeneratorTest::HandleInput( const InputData& input )
{
	switch( input.iGICode )
	{
	case GIC_F5:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
			CreatePerlinNoiseTexture();
		}
		break;
	case GIC_F12:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
			SaveTexturesAsImageFiles();
		}
		break;
	case GIC_SPACE:
	case GIC_ENTER:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
		}
		break;
	default:
		break;
	}
}
