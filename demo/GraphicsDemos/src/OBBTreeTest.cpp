#include "OBBTreeTest.hpp"
#include "amorphous/3DMath/OBBTree.hpp"
#include "amorphous/Support/ParamLoader.hpp"
#include "amorphous/Support/CameraController.hpp"
#include "amorphous/Support/ParamLoader.hpp"
#include "amorphous/Graphics/Mesh/BasicMesh.hpp"
#include "amorphous/Graphics/2DPrimitive/2DRect.hpp"
#include "amorphous/Graphics/Shader/ShaderManager.hpp"
#include "amorphous/Graphics/Shader/FixedFunctionPipelineManager.hpp"
#include "amorphous/Graphics/BoundingVolumeTreeRenderers.hpp"
//#include "amorphous/Graphics/SkyboxMisc.hpp"
#include "amorphous/Graphics/MeshModel/3DMeshModelArchive.hpp"

using std::string;
using namespace boost;


OBBTreeTest::OBBTreeTest()
:
m_DrawLevel(0),
m_NumDrawLevels(8)
{
}


OBBTreeTest::~OBBTreeTest()
{
}


int OBBTreeTest::Init()
{
	if( CameraController() )
		CameraController()->SetPosition( Vector3(0,1,-3) );

	const string dir_path = "OBBTreeDemo/";

//	m_SkyboxTechnique.SetTechniqueName( "SkyBox" );
	m_MeshTechnique.SetTechniqueName( "NoLighting" );
	m_DefaultTechnique.SetTechniqueName( "NullShader" );

	// initialize shader
	bool shader_loaded = m_Shader.Load( dir_path + "shaders/OBBTreeTest.fx" );

	// load skybox mesh
//	m_SkyboxMesh = CreateSkyboxMesh( "./textures/skygrad_slim_01.jpg" );

	// load the terrain mesh
//	MeshResourceDesc mesh_desc;
//	mesh_desc.ResourcePath = "./models/terrain06.msh";
//	mesh_desc.MeshType     = CMeshType::BASIC;
//	m_Mesh.Load( mesh_desc );

	string relative_model_pathname = "models/Chevelle.msh";
	LoadParamFromFile( dir_path + "params.txt", "model", relative_model_pathname );

	string model_pathname = dir_path + relative_model_pathname;

	bool model_loaded = m_Mesh.Load( model_pathname );

	C3DMeshModelArchive mesh_archive;
	mesh_archive.LoadFromFile( model_pathname );

	InitOBBTree( mesh_archive );

	if( m_pOBBTree )
		m_pOBBTree->DumpToTextFile( model_pathname + "-obbtree.txt" );

	return 0;
}


void OBBTreeTest::InitOBBTree( C3DMeshModelArchive& mesh_archive )
{
	m_pOBBTree.reset( new OBBTree );

	const string dir_path = "OBBTreeDemo/";

	int level = 8;
	LoadParamFromFile( dir_path + "params.txt", "obb_tree_level", level );

	bool created = m_pOBBTree->Create(
		mesh_archive.GetVertexSet().vecPosition,
		mesh_archive.GetVertexIndex(),
		level
		);

	if( !created )
		LOG_PRINT_ERROR( " Failed to create the OBB tree." );
}


void OBBTreeTest::Update( float dt )
{
}


void OBBTreeTest::Render()
{
	Matrix44 matWorld = Matrix44Identity();
	ShaderManager *pShaderMgr = m_Shader.GetShaderManager();

	ShaderManager& shader_mgr = pShaderMgr ? (*pShaderMgr) : FixedFunctionPipelineManager();

	shader_mgr.SetWorldTransform( matWorld );
/*
	pShaderMgr->SetTechnique( m_MeshTechnique );
*/
//	pShaderMgr->SetTechnique( m_DefaultTechnique );

	GraphicsDevice().Enable( RenderStateType::DEPTH_TEST );
	GraphicsDevice().Disable( RenderStateType::LIGHTING );
	GraphicsDevice().SetCullingMode( CullingMode::COUNTERCLOCKWISE );

	shared_ptr<BasicMesh> pMesh = m_Mesh.GetMesh();
	if( pMesh )
		pMesh->Render( shader_mgr );

	if( m_pOBBTree )
		RenderOBBTree( *m_pOBBTree, m_DrawLevel );

//	C2DRect rect( Vector2( 80, 80 ), Vector2( 100, 100 ), 0xFFFF0000 );
//	rect.Draw();
}


void OBBTreeTest::HandleInput( const InputData& input )
{
	switch( input.iGICode )
	{
	case GIC_UP:
		if( input.iType == ITYPE_KEY_PRESSED )
			m_DrawLevel = (m_DrawLevel+1) % m_NumDrawLevels;
		break;
	case GIC_DOWN:
		if( input.iType == ITYPE_KEY_PRESSED )
			m_DrawLevel = (m_DrawLevel-1+m_NumDrawLevels) % m_NumDrawLevels;
		break;
	default:
		CGraphicsTestBase::HandleInput( input );
		break;
	}
}