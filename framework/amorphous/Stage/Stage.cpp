#include "Stage.hpp"
//#include "Stage/BSPMap.hpp"
#include "amorphous/Stage/BSPTree.hpp"
#include "amorphous/Stage/StaticGeometry.hpp"
#include "amorphous/Stage/EntitySet.hpp"
#include "amorphous/Stage/EntityRenderManager.hpp"
#include "amorphous/Stage/trace.hpp"
#include "amorphous/Stage/ViewFrustumTest.hpp"
#include "amorphous/Stage/CopyEntityDesc.hpp"
#include "amorphous/Stage/ScreenEffectManager.hpp"
#include "amorphous/Stage/SurfaceMaterialManager.hpp"
#include "amorphous/Stage/Input/InputHandler_StageDebug.hpp"
#include "amorphous/Script/PyModule_amorphous.hpp"
#include "amorphous/Script/PyModule_3DMath.hpp"
#include "amorphous/Script/PyModule_Graphics.hpp"
#include "amorphous/Script/PyModule_sound.hpp"
#include "amorphous/Script/PyModule_stage.hpp"
#include "amorphous/Script/PyModule_StageUtility.hpp"
#include "amorphous/Script/PyModule_visual_effect.hpp"
#include "amorphous/Script/PyModules.hpp"
#include "amorphous/Script/PythonScriptManager.hpp"
#include "amorphous/Script/BoostPythonScriptManager.hpp"
#include "amorphous/Script/convert_python_to_x.hpp"
#include "amorphous/Script/EmbeddedPythonModule.hpp"
#include "amorphous/Script/EmbeddedPythonModules.hpp"
#include "amorphous/Support/Timer.hpp"
#include "amorphous/Support/macro.h"
#include "amorphous/Support/memory_helpers.hpp"
#include "amorphous/Support/Profile.hpp"
#include "amorphous/Support/Log/DefaultLog.hpp"
#include "amorphous/Support/lfs.hpp"

#include "amorphous/Physics/PhysicsEngine.hpp"
#include "amorphous/Physics/SceneDesc.hpp"
#include "amorphous/Physics/Scene.hpp"
#include "amorphous/Physics/MaterialDesc.hpp"
#include "amorphous/Physics/Material.hpp"
#include "amorphous/Sound/SoundManager.hpp"
#include "amorphous/Input/InputHub.hpp"

// For TestTriggerReport and TestContactReport
#include "amorphous/Physics/Actor.hpp"
#include "amorphous/Physics/Shape.hpp"


namespace amorphous
{


static uint gs_DebugInputHandlerIndex = 0;


using namespace std;
using namespace physics;
using std::shared_ptr;


void SetStageForScriptCallback( CStage* pStage )
{
	SetStageForStageScriptCallback( pStage );
	py::entity::SetStageForEntityScriptCallback( pStage );
}


CStage::CStage()
:
//m_pPhysicsManager(NULL),
m_pPhysicsScene(NULL),
//m_pPhysicsVisualizer(NULL),
m_pStaticGeometry(NULL),
m_pCamera(NULL),
m_pStageDebugInputHandler(NULL)
{
	// debug - see the thread id
	boost::thread::id thread_id = boost::this_thread::get_id();

//	m_pTimer = new PrecisionTimer();
	m_pTimer = new Timer();
	PauseTimer();	// don't start until the initialization is complete

	m_pScreenEffectManager.reset( new ScreenEffectManager );
	m_pScreenEffectManager->Init();

	m_pEntitySet = new EntityManager( this );

	m_pScreenEffectManager->SetTargetSceneRenderer( m_pEntitySet->GetRenderManager().get() );

	BSPTreeForAABB::Initialize();

	// create the script manager so that an application can add custom modules
	// before calling CStage::Initialize() and running scripts for initialization
	if( ScriptManager::ms_UseBoostPythonModules )
		m_pScriptManager.reset( new BoostPythonScriptManager );
	else
		m_pScriptManager.reset( new PythonScriptManager );
}


CStage::~CStage()
{
	LOG_FUNCTION_SCOPE();

	for( size_t i=0; i<m_vecpMaterial.size(); i++ )
		m_pPhysicsScene->ReleaseMaterial( m_vecpMaterial[i] );

	SafeDelete( m_pStaticGeometry );
	SafeDelete( m_pEntitySet );
	m_pScreenEffectManager.reset();
	m_pScriptManager.reset();
//	SafeDelete( m_pPhysicsManager );
	PhysicsEngine().ReleaseScene( m_pPhysicsScene );
	m_pMaterialManager.reset();

//	SafeDelete( m_pPhysicsVisualizer );

	// release the input handler registered for debugging
	GetInputHub().RemoveInputHandler( gs_DebugInputHandlerIndex, m_pStageDebugInputHandler );
	SafeDelete( m_pStageDebugInputHandler );

	SafeDelete( m_pTimer );
}


CStaticGeometryBase *CreateStaticGeometry( CStage* pStage, const string& filename )
{
	string ext = lfs::get_ext( filename );

	if( ext == "sga" ) // static geometry archive
		return new CStaticGeometry( pStage );
//	else if( ext == "bspx" ) // bsp level file (deprecated)
//		return new CBSPMap( pStage );
	else
	{
		LOG_PRINT_WARNING( "an invalid static geometry filename: " + filename );
		return NULL;
	}
}


class TestTriggerReport : public physics::CUserTriggerReport
{
public:
	void OnTrigger( const CTriggerEvent& trigger_event )
	{
		CShape *pShape0 = trigger_event.pTriggerShape;
		CShape *pShape1 = trigger_event.pOtherShape;
		CActor& actor0 = pShape0->GetActor();
		CActor& actor1 = pShape1->GetActor();
		U32 status = trigger_event.StatusFlags;
		CCopyEntity *pEntity0 = (CCopyEntity *)actor0.m_pFrameworkData;
		CCopyEntity *pEntity1 = (CCopyEntity *)actor1.m_pFrameworkData;

		if( pEntity0 && pEntity1 )
		{
			pEntity0->OnPhysicsTrigger( *pShape0, *pEntity1, *pShape1, status );

			if( pEntity0->m_pCallback )
				pEntity0->m_pCallback->OnPhysicsTrigger( *pShape0, *pEntity1, *pShape1, status );
		}
	}
};


class TestContactReport : public physics::CUserContactReport
{
public:
	void OnContactNotify( CContactPair& pair, U32 events )
	{
		CActor *pActor0 = pair.pActors[0];
		CActor *pActor1 = pair.pActors[1];
		if( !pActor0 || !pActor1 )
			return;

		CCopyEntity *pEntity0 = (CCopyEntity *)pActor0->m_pFrameworkData;
		CCopyEntity *pEntity1 = (CCopyEntity *)pActor1->m_pFrameworkData;

		if( pEntity0 && pEntity1 )
		{
			pEntity0->OnPhysicsContact( pair, *pEntity1 );
			pEntity1->OnPhysicsContact( pair, *pEntity0 );

			if( pEntity0->m_pCallback )
				pEntity0->m_pCallback->OnPhysicsContact( pair, *pEntity1 );
			if( pEntity1->m_pCallback )
				pEntity1->m_pCallback->OnPhysicsContact( pair, *pEntity0 );
		}
	}
};


bool CStage::InitPhysicsManager()
{
	static TestContactReport s_TCR;
	static TestTriggerReport s_TTR;

	// initialize physics scene
	CSceneDesc phys_scene_desc;
	m_pPhysicsScene = PhysicsEngine().CreateScene( phys_scene_desc );

	if( m_pPhysicsScene == NULL )
		return false;

	m_pPhysicsScene->SetUserContactReport( &s_TCR );
	m_pPhysicsScene->SetActorGroupPairFlags(0,0,ContactPairFlag::NOTIFY_ALL);
//	m_pPhysicsScene->SetActorGroupPairFlags(0,0,NX_NOTIFY_ON_START_TOUCH|NX_NOTIFY_ON_TOUCH|NX_NOTIFY_ON_END_TOUCH);

	m_pPhysicsScene->SetUserTriggerReport( &s_TTR );

	// set pairs that don't collide with each other
/*	m_pPhysicsManager->SetCollisionGroupState( ENTITY_COLL_GROUP_STATICGEOMETRY,ENTITY_COLL_GROUP_STATICGEOMETRY, false );
	m_pPhysicsManager->SetCollisionGroupState( ENTITY_COLL_GROUP_PLAYER,		ENTITY_COLL_GROUP_ITEM, false );
	m_pPhysicsManager->SetCollisionGroupState( ENTITY_COLL_GROUP_DOOR,			ENTITY_COLL_GROUP_STATICGEOMETRY, false );
	m_pPhysicsManager->SetCollisionGroupState( ENTITY_COLL_GROUP_DOOR,			ENTITY_COLL_GROUP_DOOR, false );
	m_pPhysicsManager->SetCollisionGroupState( ENTITY_COLL_GROUP_PLAYER,		ENTITY_COLL_GROUP_PLAYER, false );
	m_pPhysicsManager->SetCollisionGroupState( ENTITY_COLL_GROUP_NOCLIP,		false );
*/

	int default_material_index = 0;
	CMaterial *pDefaultMaterial = m_pPhysicsScene->GetMaterial( default_material_index );
	float default_static_friction  = 1.5f;
	float default_dynamic_friction = 1.2f;
	float default_restitution      = 0.5f;
	float rc = 0;
	if( pDefaultMaterial )
	{
		// Peek at some default parameter values of the default material
		float sf = pDefaultMaterial->GetStaticFriction();
		float df = pDefaultMaterial->GetDynamicFriction();
		float rc = pDefaultMaterial->GetRestitution();
		pDefaultMaterial->SetStaticFriction( default_static_friction );
		pDefaultMaterial->SetDynamicFriction( default_dynamic_friction );
		pDefaultMaterial->SetRestitution( default_restitution );
	}

	// Register another material as a fallback
	CMaterialDesc mat_desc;
	mat_desc.StaticFriction  = default_static_friction;
	mat_desc.DynamicFriction = default_dynamic_friction;
	mat_desc.Restitution     = default_restitution;
	physics::CMaterial *pFallbackMaterial = m_pPhysicsScene->CreateMaterial( mat_desc );
	int mat_id = pFallbackMaterial->GetMaterialID();

	return true;
}


bool CStage::LoadBaseEntity( BaseEntityHandle& base_entity_handle )
{
	return m_pEntitySet->LoadBaseEntity( base_entity_handle );
}


// render stage from the view of an entity which has been specifed as the camera entity
void CStage::Render()
{
	PROFILE_FUNCTION();

	// since no camera has been specified from the client, get the current camera from entity set
	// 9:27 PM 8/26/2007 - changed: camera entity renders the stage using its own rendering routine
	// for example,
	// - camera entity == player
	//   > render the scene and HUD
	// - camera entity == scripted camera for script event (cut scene)
	//   > render the scene

	m_pEntitySet->UpdateCamera();

	CCopyEntity* pCameraEntity = m_pEntitySet->GetCameraEntity();
	if( pCameraEntity )
	{
		pCameraEntity->pBaseEntity->RenderStage( pCameraEntity );
		return;
	}
/*
	Camera *pCurrentCamera = m_pEntitySet->GetCurrentCamera();
	if( pCurrentCamera )
	{
		Render( *pCurrentCamera );
//		m_pEntitySet->GetCameraEntity()->RenderStage( this );
	}
	else
	{
		m_pCamera = NULL;
		return;	// stage cannot be rendered without a camera
	}*/
}

/*
// render stage by directly providing the camera
void CStage::Render( Camera& rCam )
{
	// save the current camera
	// the camera must not be released / destroyed untill the Render() finishes
	m_pCamera = &rCam;

///	g_PerformanceCheck.iNumDrawPrimCalls = 0;
//	g_PerformanceCheck.iNumRenderedCopyEntites = 0;

	// set transforms and effect flags
	m_pScreenEffectManager->BeginRender( rCam );

	// entity set uses shader manager held by screen effect manager
	m_pScreenEffectManager->SetShaderManager();

	// next, render the objects in the world (entities)
	// Here, 'entites' also include
	// - static geometry
	// - skybox
	m_pEntitySet->Render( rCam );

	// the scene has been rendered
	// perform post-process effects on the scene
	m_pScreenEffectManager->RenderPostProcessEffects();

	// render physics-related debug info
	// e.g. bounding boxes, contact positions & normals
//	if( m_pPhysicsVisualizer )
//		m_pPhysicsVisualizer->RenderVisualInfo();

	m_pScreenEffectManager->EndRender();

	m_pCamera = NULL;
}*/


// render stage by directly providing the camera
void CStage::Render( Camera& rCam )
{
	m_pCamera = &rCam;

	m_pScreenEffectManager->Render( rCam );
}


/// Called from external module that wants to let the current camera entity render the stage.
/// Create render tasks necessary for the current camera entity to render the stage,
/// and add them to render task list.
void CStage::CreateRenderTasks()
{
	CCopyEntity* pCameraEntity = m_pEntitySet->GetCameraEntity();
	if( pCameraEntity )
	{
		pCameraEntity->pBaseEntity->CreateRenderTasks( pCameraEntity );
		return;
	}
}


/// called from base entity.
/// Module that wants to render the stage by directly calling CStage::Render()
/// needs to call this function in advance
void CStage::CreateStageRenderTasks( Camera *pCamera )
{
	// creates render tasks for
	// - envmap
	// - shadowmap

	m_pEntitySet->GetRenderManager()->UpdateCamera( pCamera );

	m_pScreenEffectManager->CreateRenderTasks();

/*	if( m_pScreenEffectManager->UsesExtraRenderTarget() )
	{
		// screen effects are applied after rendering the scene
		// - register the scene rendering as a separate render task
		m_pEntitySet->GetRenderManager()->CreateRenderTasks( *pCamera, true );

		m_pScreenEffectManager->CreateRenderTasks();
	}
	else
	{
		// no screen effects that require texture render target
		// - final scene is rendered in CStage::Render() path
		//   and not registered as a separate render task
		m_pEntitySet->GetRenderManager()->CreateRenderTasks( *pCamera, false );
	}*/
}


void CStage::ClipTrace(STrace& tr)
{
	PROFILE_FUNCTION();

	if( tr.vStart == tr.vGoal )
	{
		tr.vEnd = tr.vStart;
///		tr.fFraction = 0;			// commented out at 17:22 2007/04/15

		return;
	}

	tr.SetAABB();
	tr.vEnd = tr.vStart + (tr.vGoal - tr.vStart) * tr.fFraction;

	if( m_pStaticGeometry && !(tr.sTraceType & TRACETYPE_IGNORE_MAP) )
		m_pStaticGeometry->ClipTrace( tr );

	if( !(tr.sTraceType & TRACETYPE_IGNORE_ALL_ENTITIES) )
		m_pEntitySet->ClipTrace( tr );
}

/*
void CStage::ClipTrace( CJL_LineSegment& segment )
{
	m_pPhysicsManager->ClipLineSegment( segment );
}
*/

//check if a bounding volume is in a valid position
//if it is in a solid position, turn 'tr.in_solid' to 'true' and return
void CStage::CheckPosition(STrace& tr)
{
	// 'tr.aabb_swept' represents the bounding volume of the source entity in world coordinate
	if(tr.bvType == BVTYPE_DOT)
		tr.aabb_swept.SetMaxAndMin( tr.vEnd, tr.vEnd );	//swept volume is just a dot
	else
		tr.aabb_swept.TransformCoord( tr.aabb, tr.vEnd );

	// Check position against map
	if( m_pStaticGeometry && !(tr.sTraceType & TRACETYPE_IGNORE_MAP) )
		m_pStaticGeometry->CheckPosition( tr );

	// Check position against entities
	if( !(tr.sTraceType & TRACETYPE_IGNORE_ALL_ENTITIES) )
		m_pEntitySet->CheckPosition( tr );
}


void CStage::CheckCollision(CTrace& tr)
{
	if( tr.GetBVType() == BVTYPE_SPHERE )
	{	// not checked against map. collision check with entities only.
		if( tr.GetStartPosition() == tr.GetGoalPosition() )
		{
			// check position against entities
			m_pEntitySet->CheckPosition( tr );
		}
	}
	else if( tr.GetBVType() == BVTYPE_DOT )
	{
		if(tr.GetStartPosition() == tr.GetGoalPosition())
			return;
		else
		{
			STrace tr2;
			tr2.bvType = BVTYPE_DOT;
			tr2.vStart = tr.GetStartPosition();
			tr2.vGoal  = tr.GetGoalPosition();
			tr2.aabb.SetMaxAndMin( Vector3(0,0,0), Vector3(0,0,0) );
			tr2.SetAABB();
			tr2.sTraceType = tr.GetTraceType();
			
			// Check collision with the map (static geometry)
			if( m_pStaticGeometry )
                m_pStaticGeometry->ClipTrace(tr2);

			// Check collision with other entities
			if( !(tr2.sTraceType & TRACETYPE_IGNORE_ALL_ENTITIES) )
				m_pEntitySet->ClipTrace(tr2);

			tr.SetEndFraction( tr2.fFraction );
			tr.SetEndPosition( tr2.vEnd );
			if( tr2.pTouchedEntity )
				tr.AddTouchEntity( tr2.pTouchedEntity );
		}
	}
		return;
}


void CStage::GetVisibleEntities( ViewFrustumTest& vf_test )
{
	static vector<CCopyEntity *> s_vecpTempVisibleEntityBuffer;
	s_vecpTempVisibleEntityBuffer.resize(0);

	// save the original buffer
	vector<CCopyEntity *> *pvecpOrigVisibleEntityBuffer	= vf_test.m_pvecpVisibleEntity;

	// set temporary buffer
	vf_test.SetBuffer( s_vecpTempVisibleEntityBuffer );

	// get potentially visible entities
	m_pEntitySet->GetVisibleEntities(vf_test);

	// check if the potentially visible entities are occluded by the map or other entities
	STrace tr;
	Vector3 vStart = vf_test.GetCamera()->GetPosition();
	Vector3 vGoal;
	int i, iNumPotentiallyVisibles = vf_test.GetNumVisibleEntities();
	if( 300 < iNumPotentiallyVisibles )
		int iTooManyVisibleEntities = 1;

	tr.vStart = vStart;
	tr.vGoal  = vGoal;
	tr.bvType = BVTYPE_DOT;
	tr.sTraceType = TRACETYPE_IGNORE_NOCLIP_ENTITIES;

	for( i=0; i<iNumPotentiallyVisibles; i++ )
	{
		vGoal = vf_test.GetEntity(i)->GetWorldPosition();
		tr.fFraction = 1.0f;
		tr.pTouchedEntity = NULL;
//		m_pStaticGeometry->ClipTrace( tr );
		ClipTrace( tr );

//		if( tr.fFraction == 1.0f )
		if( tr.pTouchedEntity == vf_test.GetEntity(i) )
			pvecpOrigVisibleEntityBuffer->push_back( vf_test.GetEntity(i) );	// entity is not obstructed
	}

	// restore original buffer
	vf_test.SetBuffer( *pvecpOrigVisibleEntityBuffer );
}

/*
char CStage::IsCurrentlyVisibleCell(short sCellIndex)
{
	return m_pStaticGeometry->IsCurrentlyVisibleCell( sCellIndex );
}
*/

void CStage::Update( float dt )
{
	PROFILE_FUNCTION();

	// update timer for frame time (delta time)
	m_pTimer->UpdateFrameTime();

	ONCE( LOG_PRINT( " - updating all entities" ) );

	// update states of entities
	m_pEntitySet->UpdateAllEntities( dt );

	ONCE( LOG_PRINT( " - updated all entities" ) );

	// event script
	SetStageForScriptCallback( this );
	m_pScriptManager->Update();
	SetStageForScriptCallback( NULL );

//	if( m_pTextMessageManager )
//		m_pTextMessageManager->Update( dt );

	m_pScreenEffectManager->Update( dt );
}


double CStage::GetElapsedTime()
{
//	return m_pTimer->CurrentTime();
	return m_pTimer->GetTime();
}


unsigned long CStage::GetElapsedTimeMS()
{
//	return (unsigned long)(m_pTimer->CurrentTime() * 1000.0f);
	return m_pTimer->GetTimeMS();
}


void CStage::PauseTimer()
{
	if( m_pTimer )
        m_pTimer->Stop();
}


void CStage::ResumeTimer()
{
	if( m_pTimer )
        m_pTimer->Start();
}


void CStage::ReleasePhysicsActor( CActor*& pPhysicsActor )
{
	m_pPhysicsScene->ReleaseActor( pPhysicsActor );
}


Vector3 CStage::GetGravityAccel() const
{
	return Vector3( 0, -9.8f, 0 );
}


EntityHandle<> CStage::LoadStaticGeometryFromFile( const std::string filename )
{
	SafeDelete( m_pStaticGeometry );
	m_pStaticGeometry = CreateStaticGeometry( this, filename );

	if( !m_pStaticGeometry )
		return EntityHandle<>();

	// register the static geometry as an entity
	// - the entity is used to render the static geometry

	BaseEntityHandle baseentity_handle;
	baseentity_handle.SetBaseEntityName( "StaticGeometry" );
	CCopyEntityDesc desc;
	desc.strName = filename;
	desc.pBaseEntityHandle = &baseentity_handle;
	desc.pUserData = m_pStaticGeometry;

	CCopyEntity *pStaticGeometryEntity = CreateEntity( desc );

	if( !pStaticGeometryEntity )
		return EntityHandle<>();

//	shared_ptr<CStaticGeometryEntity> pEntity( new CStaticGeometryEntity );
//	pEntity->SetStaticGeometry( m_pStaticGeometry );
//	EntityHandle<CStaticGeometryEntity> entity
//		= CreateEntity( pEntity, baseentity_handle );

	m_pEntitySet->WriteEntityTreeToFile( "debug/entity_tree - loaded static geometry.txt" );

	// load the static geometry from file
	this->PauseTimer();
	bool loaded = m_pStaticGeometry->LoadFromFile( filename );
	this->ResumeTimer();

	return EntityHandle<>( pStaticGeometryEntity->Self() );
//	return entity;
}


bool CStage::LoadMaterial( /* const string& material_filename */)
{
//	string material_filename = m_strStageFilename;
//	lfs::change_ext( material_filename, "mat" );

	string material_filename = "./Stage/material.bin";

	m_pMaterialManager.reset( new CSurfaceMaterialManager );
	
	bool b = m_pMaterialManager->LoadFromFile( material_filename );

	if( !b )
		return false;

	if( m_pPhysicsScene )
	{
		// register materials to physics simulator
		const int num_materials = m_pMaterialManager->GetNumMaterials();
		m_vecpMaterial.resize( num_materials );
		for( int i=0; i<num_materials; i++ )
		{
			const CSurfaceMaterial& src_material = m_pMaterialManager->GetSurfaceMaterial(i);

			CMaterialDesc desc;
			desc.SetDefault();
			desc.StaticFriction  = src_material.GetPhysicsMaterial().fStaticFriction;
			desc.DynamicFriction = src_material.GetPhysicsMaterial().fDynamicFriction;
			desc.Restitution     = src_material.GetPhysicsMaterial().fElasticity;

			m_vecpMaterial[i] = m_pPhysicsScene->CreateMaterial( desc );
		}
	}
	else
		LOG_PRINT_WARNING( " The physics engine has not been initialized yet." );

	return true;
}


CSurfaceMaterial& CStage::GetMaterial( int index )
{
	return m_pMaterialManager->GetSurfaceMaterial( index );
}


bool CStage::InitEventScriptManager( const string& script_archive_filename )
{
	LOG_FUNCTION_SCOPE();

	if( !m_pScriptManager )
		return false;

	m_ScriptArchiveFilename = script_archive_filename;

	if( ScriptManager::ms_UseBoostPythonModules )
	{
		RegisterPythonModule_amorphous();
		RegisterPythonModule_math3d();
		RegisterPythonModule_gfx();
		RegisterPythonModule_sound();
		RegisterPythonModule_stage();
		RegisterPythonModule_visual_effect();
		stage_util::RegisterPythonModule_stage_util();
	}
	else
	{
		shared_ptr<PythonScriptManager> pPythonScriptManager
			= std::dynamic_pointer_cast<PythonScriptManager,ScriptManager>( m_pScriptManager );

		if( !pPythonScriptManager )
			return false;

		PythonScriptManager& script_mgr = *pPythonScriptManager;

		RegisterPythonModule_PlayerInfo(   script_mgr );
		RegisterPythonModule_Stage(        script_mgr );
		RegisterPythonModule_TextMessage(  script_mgr );
		RegisterPythonModule_Sound(        script_mgr );
		py::entity::RegisterPythonModule_Entity(       script_mgr );
		RegisterPythonModule_HUD(        script_mgr );
		py::cam::RegisterPythonModule_Camera( script_mgr );
		RegisterPythonModule_StageGraph(   script_mgr );
//		RegisterPythonModule_Graphics(     script_mgr );
//		RegisterPythonModule_Animation(    script_mgr );
		AddPythonModule_Graphics();
		AddPythonModule_Animation();
		RegisterPythonModule_Task(         script_mgr );
		py::light::RegisterPythonModule_Light(     script_mgr );
		py::ve::RegisterPythonModule_VisualEffect( script_mgr );

		RegisterEmbeddedPythonModule( GetEmbeddedPythonModule_math3d() );
		RegisterEmbeddedPythonModule( GetEmbeddedPythonModule_gfx() );

		m_pScriptManager = pPythonScriptManager;
	}

	bool res = m_pScriptManager->LoadScriptArchiveFile( script_archive_filename );

	if( !res )
	{
		LOG_PRINT_ERROR( "unable to load the script (archive): " + script_archive_filename );
		return false;
	}

	// run scripts for initialization
	ResumeTimer();
	SetStageForScriptCallback( this );
	m_pScriptManager->InitScripts();
	m_pScriptManager->Update();
	SetStageForScriptCallback( NULL );
	PauseTimer();

	return true;
}


bool CStage::Initialize( const string& script_archive_filename )
{
	LOG_FUNCTION_SCOPE();

	// nullify all the copy-entities in 'm_pEntitySet'
	m_pEntitySet->ReleaseAllEntities();

	// creates a physics manager for the stage
	// rigid-body entities are registered to physics engine when created
	InitPhysicsManager();

	// load scripts initialize
	// also run scripts that initializes stage
	// - static geometry is loaded by init script
	// - entities can be created in this call
	InitEventScriptManager( script_archive_filename );

	if( m_pStaticGeometry )
	{
		// make EntityTree from static geometry
		BSPTree bsptree;
		m_pStaticGeometry->MakeEntityTree( bsptree );
		m_pEntitySet->MakeEntityTree( &bsptree );

		// set dynamic light manager for static geometry
		m_pStaticGeometry->SetDynamicLightManager( m_pEntitySet );

		// m_pEntitySet->RegisterStaticGeometry( m_pStaticGeometry );

		// set shader to the light manager
		// because shader is managed by static geometry
//		m_pEntitySet->GetLightEntityManager()->Init( m_pEntitySet );
	}

	// load the binary material file that has the same body filename as the stage file
	// and different suffix "mat"
	LoadMaterial();

	// register input handler for debugging the stage
	m_pStageDebugInputHandler = new InputHandler_StageDebug( m_pSelf );
	if( GetInputHub().GetInputHandler(gs_DebugInputHandlerIndex) )
		GetInputHub().GetInputHandler(gs_DebugInputHandlerIndex)->AddChild( m_pStageDebugInputHandler );
	else
		GetInputHub().PushInputHandler( gs_DebugInputHandlerIndex, m_pStageDebugInputHandler );

	// stage has been initialized - start the timer
	ResumeTimer();

	double time = m_pTimer->GetTime();

	return true;
}


void CStage::NotifyEntityTerminationToEventManager( CCopyEntity* pEntity )
{
//	m_pScriptManager->OnCopyEntityDestroyed( pEntity );
}


void CStage::UpdateListener()
{
	Matrix34 cam_pose;
	Camera *pCamera;

	// get the pose of the current camera
	pCamera = this->GetCurrentCamera();
	if( pCamera )
	{
		pCamera->GetPose( cam_pose );
	}
	else
	{
		CCopyEntity* pEntity = this->GetEntitySet()->GetCameraEntity();
		if( pEntity )
			cam_pose = pEntity->GetWorldPose();
		else
			cam_pose.Identity();
	}

	// update listener for sound manager
	GetSoundManager().SetListenerPose( cam_pose ); 

	CCopyEntity *pCameraEntity = this->GetEntitySet()->GetCameraEntity();
	if( pCameraEntity )
		GetSoundManager().SetListenerVelocity( pCameraEntity->Velocity() ); 

	GetSoundManager().CommitDeferredSettings();
}



/*
void CStage::PlaySound3D( char* pcSoundName, Vector3& rvPosition )
{
	GetSoundManager().PlayAt( pcSoundName, rvPosition );
}


void CStage::PlaySound3D( int iIndex, Vector3& rvPosition )
{
	GetSoundManager().PlayAt( iIndex, rvPosition );
}


void CStage::PlaySound3D( SoundHandle &rSoundHandle, Vector3& rvPosition )
{
	GetSoundManager().PlayAt( rSoundHandle, rvPosition );
}
*/


/*
void CStage::SaveCurrentState(FILE* fp)
{
	m_pEntitySet->SaveCurrentCopyEntities( fp );
}

void CStage::LoadSavedData(FILE* fp)
{
	m_pEntitySet->LoadCopyEntitiesFromSavedData( fp );
}
*/


} // namespace amorphous
