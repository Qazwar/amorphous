#include "PrimitiveRendererTest.hpp"
#include "amorphous/Graphics/PrimitiveRenderer.hpp"
#include "amorphous/Graphics/Camera.hpp"
#include "amorphous/Graphics/2DPrimitive/2DRect.hpp"
#include "amorphous/Graphics/Shader/ShaderManager.hpp"
#include "amorphous/Graphics/Shader/FixedFunctionPipelineManager.hpp"
//#include "amorphous/Graphics/Shader/ShaderLightManager.hpp"
#include "amorphous/Support/Profile.hpp"
#include "amorphous/Support/ParamLoader.hpp"
#include "amorphous/Support/Macro.h"
#include "amorphous/Support/linear_interpolation.hpp"

using std::string;
using std::vector;
using namespace boost;


CPrimitiveRendererTest::CPrimitiveRendererTest()
{
	m_MeshTechnique.SetTechniqueName( "NoLighting" );

	SetBackgroundColor( SFloatRGBAColor( 0.1f, 0.1f, 0.1f, 1.0f ) );

//	LoadParamFromFile( "params.txt", "num_primitives_to_draw", m_NumPrimitivesToDraw );

	if( GetCameraController() )
		GetCameraController()->SetPosition( Vector3( 0, 2, -10 ) );
}


CPrimitiveRendererTest::~CPrimitiveRendererTest()
{
}


/*
bool CPrimitiveRendererTest::InitShader()
{
	// initialize shader
	bool shader_loaded = m_Shader.Load( "./shaders/PrimitiveRendererTest.fx" );
	
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

	return true;
}*/


int CPrimitiveRendererTest::Init()
{
//	InitShader();

	return 0;
}


void CPrimitiveRendererTest::Update( float dt )
{
}


void RenderColoredLines()
{
	SFloatRGBAColor colors[] =
	{
		SFloatRGBAColor::Red(),
		SFloatRGBAColor::Green(),
		SFloatRGBAColor::Blue(),
		SFloatRGBAColor::Magenta(),
		SFloatRGBAColor::Aqua(),
		SFloatRGBAColor::Yellow()
	};

	vector<Vector3> points;
	points.resize( 5 );
	points[0] = Vector3( 1, 0, 1) + Vector3(-1,0,0);
	points[1] = Vector3( 1, 0,-1) + Vector3(-1,0,0);
	points[2] = Vector3(-1, 0,-1) + Vector3(-1,0,0);
	points[3] = Vector3(-1, 0, 1) + Vector3(-1,0,0);
	points[4] = points[0];
	FixedFunctionPipelineManager().SetWorldTransform( Matrix44Identity() );
	int num_loops = 50;
	for( int i=0; i<=num_loops; i++ )
	{
		for( size_t j=0; j<points.size(); j++ )
			points[j].y = i * 0.2f;

		SFloatRGBAColor color = get_linearly_interpolated_value( colors, numof(colors), (float)i / (float)num_loops );

		GetPrimitiveRenderer().DrawConnectedLines( points, color );
	}

}


void CPrimitiveRendererTest::Render()
{
	PROFILE_FUNCTION();

	FixedFunctionPipelineManager().SetTexture( 0, TextureHandle() );

	GraphicsDevice().Disable( RenderStateType::LIGHTING );

	Vector3 shift = Vector3(2,0,0);
	vector<Vector3> points;
	points.resize( 4 * 20 );
	vector<SFloatRGBAColor> colors;
	colors.resize( points.size() );
	for( int i=0; i<(int)points.size() / 4; i++ )
	{
		shift.y = (float)i * 0.3f;
		points[i*4  ] = Vector3( 1, 0, 1) + shift;
		points[i*4+1] = Vector3( 1, 0,-1) + shift;
		points[i*4+2] = Vector3(-1, 0,-1) + shift;
		points[i*4+3] = Vector3(-1, 0, 1) + shift;

		float b = get_clamped( (float)(20 - i) / 20.0f, 0.0f, 1.0f );

		const SFloatRGBAColor color = SFloatRGBAColor(b,b,b,1);
		colors[i*4  ] = color;
		colors[i*4+1] = color;
		colors[i*4+2] = color;
		colors[i*4+3] = color;
	}

	GetPrimitiveRenderer().DrawConnectedLines( points, colors );

	for( int i=0; i<(int)points.size(); i++ )
		points[i] += Vector3(3,0,0);

	GetPrimitiveRenderer().DrawPoints( points, SFloatRGBAColor::White() );

	points.resize( 20 * 2 );
	for( int i=0; i<(int)points.size() / 2; i++ )
	{
		float y = (float)i * 0.2f;
		points[i*2  ] = Vector3(-2,y,5);
		points[i*2+1] = Vector3(-1,y,5);
	}

	RenderColoredLines();

//	GetPrimitiveRenderer().DrawConnectedLines( points, SFloatRGBAColor::White() );
}


void CPrimitiveRendererTest::HandleInput( const InputData& input )
{

	switch( input.iGICode )
	{
	case GIC_SPACE:
	case GIC_ENTER:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
		}
		break;
	default:
		CGraphicsTestBase::HandleInput( input );
		break;
	}
}