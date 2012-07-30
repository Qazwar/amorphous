#include "CustomMeshTest.hpp"
#include "gds/Graphics.hpp"
#include "gds/Graphics/VertexFormat.hpp"
#include "gds/Graphics/Shader/GenericShaderDesc.hpp"
#include "gds/Graphics/Shader/GenericShaderGenerator.hpp"
#include "gds/Support/Profile.hpp"
#include "gds/Support/ParamLoader.hpp"
#include "gds/Support/Macro.h"
#include "gds/Input.hpp"

using std::string;
using namespace boost;


extern CGraphicsTestBase *CreateTestInstance()
{
	return new CCustomMeshTest();
}


extern const std::string GetAppTitle()
{
	return string("CustomMeshTest");
}


CCustomMeshTest::CCustomMeshTest()
{
	m_MeshTechnique.SetTechniqueName( "NoLighting" );

	SetBackgroundColor( SFloatRGBAColor( 0.2f, 0.2f, 0.5f, 1.0f ) );

	g_Camera.SetPosition( Vector3( 0, 1, -120 ) );
//	g_Camera.SetPosition( Vector3( 0, 520, 120 ) );
}


CCustomMeshTest::~CCustomMeshTest()
{
}


void CCustomMeshTest::SetLights( bool use_hemespheric_light )
{
	CShaderManager *pShaderMgr = m_Shader.GetShaderManager();
//	if( !pShaderMgr )
//		return;

	CShaderManager& shader_mgr = pShaderMgr ? (*pShaderMgr) : FixedFunctionPipelineManager();

	CShaderLightManager *pShaderLightMgr = shader_mgr.GetShaderLightManager().get();
	if( !pShaderLightMgr )
		return;

	pShaderLightMgr->ClearLights();

	if( use_hemespheric_light )
	{
//		CHemisphericDirectionalLight light;
//		light.Attribute.UpperDiffuseColor.SetRGBA( 0.0f, 1.0f, 1.0f, 1.0f );
//		light.Attribute.LowerDiffuseColor.SetRGBA( 1.0f, 0.1f, 0.1f, 1.0f );
//		light.vDirection = Vec3GetNormalized( Vector3( -1.0f, -1.8f, -0.9f ) );
		CHemisphericDirectionalLight light;
		light.Attribute.UpperDiffuseColor.SetRGBA( 1.0f, 1.0f, 1.0f, 1.0f );
		light.Attribute.LowerDiffuseColor.SetRGBA( 0.0f, 0.0f, 0.0f, 1.0f );
		light.vDirection = Vec3GetNormalized( Vector3( -1.0f, -1.5f, -0.9f ) );
		pShaderLightMgr->SetHemisphericDirectionalLight( light );
	}
	else
	{
		CDirectionalLight dir_light;
		dir_light.DiffuseColor = SFloatRGBColor(1,1,1);
		dir_light.fIntensity = 1.0f;
		dir_light.vDirection = Vec3GetNormalized( Vector3( 1.2f, -1.8f, 1.0f ) );
		pShaderLightMgr->SetDirectionalLight( dir_light );
	}

	bool set_pnt_light = false;
	if( set_pnt_light )
	{
		CPointLight pnt_light;
		pnt_light.DiffuseColor = SFloatRGBColor(1,1,1);
		pnt_light.fIntensity = 1.0f;
		pnt_light.vPosition = Vector3( 2.0f, 2.8f, -1.9f );
		pnt_light.fAttenuation[0] = 1.0f;
		pnt_light.fAttenuation[1] = 1.0f;
		pnt_light.fAttenuation[2] = 1.0f;
		pShaderLightMgr->SetPointLight( pnt_light );
	}

	pShaderLightMgr->CommitChanges();
}


bool CCustomMeshTest::InitShader()
{
	CShaderResourceDesc shader_desc;
	CGenericShaderDesc gen_shader_desc;
	gen_shader_desc.LightingTechnique = CLightingTechnique::HEMISPHERIC;
//	gen_shader_desc.Specular = CSpecularSource::NONE;
	shader_desc.pShaderGenerator.reset( new CGenericShaderGenerator(gen_shader_desc) );
	bool shader_loaded = m_Shader.Load( shader_desc );

	return shader_loaded;
}


int CCustomMeshTest::Init()
{
	m_pFont = CreateDefaultBuiltinFont();
	m_pFont->SetFontSize( 8, 16 );
/*
	m_vecMesh.push_back( CTestMeshHolder() );
	shared_ptr<CBoxMeshGenerator> pBoxMeshGenerator( new CBoxMeshGenerator() );
	pBoxMeshGenerator->SetEdgeLengths( Vector3(1,1,1) );
	pBoxMeshGenerator->SetDiffuseColor( SFloatRGBAColor::White() );
	m_vecMesh.back().m_MeshDesc.pMeshGenerator = pBoxMeshGenerator;
//	m_vecMesh.back().m_MeshDesc.OptionFlags |= GraphicsResourceOption::DONT_SHARE;
	m_vecMesh.back().Load();
*/
	bool loaded = false;
	string mesh_file_pathname;
	LoadParamFromFile( "params.txt", "model", mesh_file_pathname );
	if( 0 < mesh_file_pathname.length() )
	{
		loaded = m_Mesh.LoadFromFile( mesh_file_pathname );
	}
	else
	{
		CBoxMeshGenerator box_mesh_generator;
//		box_mesh_generator.Generate( Vector3(1,1,1) );
		box_mesh_generator.Generate( Vector3(1,1,1), CMeshGenerator::DEFAULT_VERTEX_FLAGS, SFloatRGBAColor::White() );
		C3DMeshModelArchive ar = box_mesh_generator.GetMeshArchive();
		loaded = m_Mesh.LoadFromArchive( ar );
	}
/*
	m_MeshTechnique.SetTechniqueName( "NoLighting" );
	m_DefaultTechnique.SetTechniqueName( "NoShader" );
*/

	int use_programmable_shader = 1;
	LoadParamFromFile( "params.txt", "use_programmable_shader", use_programmable_shader );
	if( use_programmable_shader )
		InitShader();

	CMeshResourceDesc mesh_desc;
	mesh_desc.pMeshGenerator.reset( new CBoxMeshGenerator );
	m_RegularMesh.Load( mesh_desc );

//	SetLights();

	return 0;
}


void CCustomMeshTest::Update( float dt )
{
}


void CCustomMeshTest::RenderMeshes()
{
	GraphicsDevice().Enable( RenderStateType::LIGHTING );
	GraphicsDevice().Enable( RenderStateType::DEPTH_TEST );

	GraphicsDevice().Disable( RenderStateType::ALPHA_BLEND );

	CShaderManager *pShaderMgr = m_Shader.GetShaderManager();
	CShaderManager& shader_mgr = pShaderMgr ? (*pShaderMgr) : FixedFunctionPipelineManager();

	// render the scene

	shader_mgr.SetViewerPosition( g_Camera.GetPosition() );

	SetLights( pShaderMgr ? true : false );

	FixedFunctionPipelineManager().SetWorldTransform( Matrix44Identity() );

//	ShaderManagerHub.PushViewAndProjectionMatrices( g_Camera );

	m_Mesh.Render( shader_mgr );
/*
	pShaderManager->SetTechnique( m_MeshTechnique );
//	BOOST_FOREACH( CMeshObjectHandle& mesh, m_vecMesh )
	BOOST_FOREACH( CTestMeshHolder& holder, m_vecMesh )
	{
		if( holder.m_Handle.GetEntryState() == GraphicsResourceState::LOADED )
		{
			// set world transform
			const Matrix34 mesh_world_pose = holder.m_Pose;
//			FixedPipelineManager().SetWorldTransform( mesh_world_pose );
			pShaderManager->SetWorldTransform( mesh_world_pose );

			shared_ptr<CBasicMesh> pMesh = holder.m_Handle.GetMesh();

			if( pMesh )
				pMesh->Render( *pShaderManager );
		}
	}
*/
//	ShaderManagerHub.PopViewAndProjectionMatrices_NoRestore();
}


void CCustomMeshTest::Render()
{
	PROFILE_FUNCTION();

	RenderMeshes();

//	if( m_pSampleUI )
//		m_pSampleUI->Render();

//	m_TextBuffer.clear();
//	GraphicsResourceManager().GetStatus( GraphicsResourceType::Texture, m_TextBuffer );

	GraphicsDevice().Disable( RenderStateType::LIGHTING );

	Vector2 vTopLeft(     (float)GetWindowWidth() / 4,  (float)16 );
//	Vector2 vBottomRight( (float)GetWindowWidth() - 16, (float)GetWindowHeight() * 3 / 2 );
//	C2DRect rect( vTopLeft, vBottomRight, 0x50000000 );
//	rect.Draw();

	m_pFont->DrawText( m_TextBuffer, vTopLeft );

	Vector3 vCamPos = g_Camera.GetPosition();
	m_pFont->DrawText( string("camera: ") + to_string( vCamPos ), Vector2( 20, 300 ) );
}


void CCustomMeshTest::HandleInput( const SInputData& input )
{
	switch( input.iGICode )
	{
	case GIC_F12:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
//			SaveTexturesAsImageFiles();
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
