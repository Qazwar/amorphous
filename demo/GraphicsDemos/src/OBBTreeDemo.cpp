#include "OBBTreeDemo.hpp"
#include "amorphous/3DMath/OBBTree.hpp"
#include "amorphous/Support/ParamLoader.hpp"
#include "amorphous/Support/CameraController.hpp"
#include "amorphous/Graphics/Mesh/BasicMesh.hpp"
#include "amorphous/Graphics/2DPrimitive/2DRect.hpp"
#include "amorphous/Graphics/Shader/ShaderManager.hpp"
#include "amorphous/Graphics/Shader/CommonShaders.hpp"
#include "amorphous/Graphics/BoundingVolumeTreeRenderers.hpp"
//#include "amorphous/Graphics/SkyboxMisc.hpp"
#include "amorphous/Graphics/MeshModel/3DMeshModelArchive.hpp"

using namespace std;


OBBTreeDemo::OBBTreeDemo()
:
m_DrawLevel(0),
m_NumDrawLevels(8)
{
}


OBBTreeDemo::~OBBTreeDemo()
{
}


int OBBTreeDemo::Init()
{
	if( CameraController() )
		CameraController()->SetPosition( Vector3(0,1,-3) );

	const string dir_path = "OBBTreeDemo/";

//	m_SkyboxTechnique.SetTechniqueName( "SkyBox" );
	m_MeshTechnique.SetTechniqueName( "Default" );
	m_DefaultTechnique.SetTechniqueName( "Default" );

	// initialize shader
	m_Shader = GetNoLightingShader();

	// load skybox mesh
//	m_SkyboxMesh = CreateSkyboxMesh( "./textures/skygrad_slim_01.jpg" );

	string mesh_pathnames[] =
	{
		"../Common/models/bunny.msh",
		"../Common/models/Chevelle.msh",
		"../Common/models/gerzi_chapel.msh",
		"../Common/models/wc1_1.00.msh"
	};

	const int num_meshes = numof(mesh_pathnames);
	m_Meshes.resize( num_meshes );
	for( int i=0; i<num_meshes; i++ )
	{
		string model_pathname = dir_path + mesh_pathnames[i];

		m_Meshes[i].reset( new CustomMesh );

		bool model_loaded = m_Meshes[i]->LoadFromFile( model_pathname );
	}

	UpdateOBB();

//	LoadParamFromFile( dir_path + "params.txt", "model", relative_model_pathname );
	
//	bool model_loaded = m_Mesh.Load( model_pathname );

	string model_pathname = mesh_pathnames[0];

//	C3DMeshModelArchive mesh_archive;
//	mesh_archive.LoadFromFile( model_pathname );

//	InitOBBTree( mesh_archive );

	if( m_pOBBTree )
		m_pOBBTree->DumpToTextFile( model_pathname + "-obbtree.txt" );

	return 0;
}


void OBBTreeDemo::UpdateOBB()
{
	std::shared_ptr<CustomMesh> pMesh;
	m_Meshes.get_current( pMesh );

	if( !pMesh )
		return;

	std::vector<Vector3> points;
	std::vector<unsigned int> indices;
	pMesh->GetVertexPositions(points);
	pMesh->GetVertexIndices(indices);

	InitOBBTree( points, indices );
}


//void OBBTreeDemo::InitOBBTree( C3DMeshModelArchive& mesh_archive )
void OBBTreeDemo::InitOBBTree( const std::vector<Vector3>& vertex_positions, const std::vector<unsigned int>& vertex_indices )
{
	m_pOBBTree.reset( new OBBTree );

	const string dir_path = "OBBTreeDemo/";

	int level = 8;
	LoadParamFromFile( dir_path + "params.txt", "obb_tree_level", level );

	bool created = m_pOBBTree->Create(
//		mesh_archive.GetVertexSet().vecPosition,
//		mesh_archive.GetVertexIndex(),
		vertex_positions,
		vertex_indices,
		level
		);

	if( !created )
		LOG_PRINT_ERROR( " Failed to create the OBB tree." );
}


void OBBTreeDemo::Update( float dt )
{
}


void OBBTreeDemo::Render()
{
	Matrix44 matWorld = Matrix44Identity();
	ShaderManager *pShaderMgr = m_Shader.GetShaderManager();

	if(!pShaderMgr)
		return;

	ShaderManager& shader_mgr = *pShaderMgr;

	shader_mgr.SetWorldTransform( matWorld );

/*
	pShaderMgr->SetTechnique( m_MeshTechnique );
*/
//	pShaderMgr->SetTechnique( m_DefaultTechnique );

	GraphicsDevice().Enable( RenderStateType::DEPTH_TEST );
	GraphicsDevice().Disable( RenderStateType::LIGHTING );
	GraphicsDevice().SetCullingMode( CullingMode::COUNTERCLOCKWISE );

	shared_ptr<CustomMesh> pMesh;
	m_Meshes.get_current(pMesh);
//	shared_ptr<BasicMesh> pMesh = m_Mesh.GetMesh();
	if( pMesh )
		pMesh->Render( shader_mgr );

	if( m_pOBBTree )
		RenderOBBTree( *m_pOBBTree, m_DrawLevel );

//	C2DRect rect( Vector2( 80, 80 ), Vector2( 100, 100 ), 0xFFFF0000 );
//	rect.Draw();
}


void OBBTreeDemo::HandleInput( const InputData& input )
{
	switch( input.iGICode )
	{
	case GIC_RIGHT:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
			m_Meshes.next();
			UpdateOBB();
		}
		break;
	case GIC_LEFT:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
			m_Meshes.prev();
			UpdateOBB();
		}
		break;
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
