#include "LightingAppBase.hpp"

#include "gds/Graphics/GraphicsElementManager.hpp"
#include "gds/Graphics/GraphicsEffectManager.hpp"
#include "gds/Graphics/3DtoScreenSpaceConversions.hpp"
#include "gds/Support.hpp"
#include "gds/Input.hpp"
#include "gds/Stage.hpp"
#include "gds/Task.hpp"
#include "gds/Script.hpp"
#include "gds/GUI.hpp"
#include "gds/GameCommon/MouseCursor.hpp"
#include "gds/App/GameWindowManager.hpp"

using std::string;
using std::vector;
using namespace boost;


static string sg_TestStageScriptToLoad = "./Script/hs_lights.bin";


extern CApplicationBase *amorphous::CreateApplicationInstance() { return new CLightingAppBase(); }


CLightingAppTask::CLightingAppTask()
{
	CScriptManager::ms_UseBoostPythonModules = true;

	StageLoader stg_loader;
//	m_pStage = stg_loader.LoadStage( "shadow_for_directional_light.bin" );
	m_pStage = stg_loader.LoadStage( sg_TestStageScriptToLoad );

	GetCameraController()->SetPose( Matrix34( Vector3(0,20,-15), Matrix33Identity() ) );
}


void CLightingAppTask::DisplayEntityPositions( GraphicsElementAnimationManager& animated_graphics_manager )
{
	AABB3 aabb;
	aabb.vMin = Vector3(1,1,1) * (-100.0f);
	aabb.vMax = Vector3(1,1,1) * (100.0f);
	vector<CCopyEntity *> pEntities;
	COverlapTestAABB aabb_test( aabb, &pEntities );
	m_pStage->GetEntitySet()->GetOverlappingEntities( aabb_test );

	shared_ptr<GraphicsElementManager> pElementMgr = animated_graphics_manager.GetGraphicsElementManager();

	static vector< shared_ptr<FrameRectElement> > pFrameRects;
	const size_t num_max_frame_rects = 128;
	const int frame_width = 4;
	int rect_edge_length = 50;
	int layer = 0;
	if( pFrameRects.empty() )
	{
		pFrameRects.resize( num_max_frame_rects );
		for( size_t i=0; i<pFrameRects.size(); i++ )
			pFrameRects[i] = pElementMgr->CreateFrameRect( SRect(0,0,rect_edge_length,rect_edge_length), SFloatRGBAColor(0,0,0,0), frame_width, layer );
	}

	for( size_t i=0; i<pFrameRects.size(); i++ )
	{
		pFrameRects[i]->SetLocalTopLeftPos( Vector2(500,20) );
		pFrameRects[i]->SetColor( 0, SFloatRGBAColor(0,0,0,0) );
	}
	
	static int s_layer = 10;
	for( size_t i=0; i<pFrameRects.size(); i++ )
		pFrameRects[i]->SetLayer( s_layer );

	size_t num_displayed_entities = 0;
	const size_t num_entities = pEntities.size();
	for( size_t i=0; i<num_entities; i++ )
	{

		if( num_displayed_entities == pFrameRects.size() )
			break;

		if( !pEntities[i] )
			continue;

		bool entity_is_in_camera = GetCamera().ViewFrustumIntersectsWith( Sphere( pEntities[i]->GetWorldPosition(), 0.1f ) );
		if( !entity_is_in_camera )
			continue;

		bool is_light_entity = false;
		const uint archive_id = pEntities[i]->GetBaseEntity()->GetArchiveObjectID();
		if( archive_id == BaseEntity::BE_POINTLIGHT
		 || archive_id == BaseEntity::BE_DIRECTIONALLIGHT )
		{
			is_light_entity = true;
		}
		else if( pEntities[i]->bNoClip )
			continue;
		else
			is_light_entity = false;

		Vector2 pos = CalculateScreenCoordsFromWorldPosition( GetCamera(), pEntities[i]->GetWorldPosition() );
//		Vector2 pos = Vector2(0,0);
//		clamp( pos.x, 0.0f, 800.0f );
//		clamp( pos.y, 0.0f, 600.0f );

		if( 0 <= pos.x && pos.x <= GraphicsComponent::GetReferenceScreenWidth()
		 && 0 <= pos.y && pos.y <= GraphicsComponent::GetReferenceScreenHeight() )
		{
			// display the entity position
			Vector2 top_left_pos = pos - Vector2((float)rect_edge_length,(float)rect_edge_length) * 0.5f;
			pFrameRects[num_displayed_entities]->SetLocalTopLeftPos( top_left_pos );

			SFloatRGBAColor color = is_light_entity ? SFloatRGBAColor(1.0f,0.6f,0.6f,0.7f) : SFloatRGBAColor(0.6f,1.0f,0.6f,0.7f);
			pFrameRects[num_displayed_entities]->SetColor( 0, color );
			num_displayed_entities++;
		}
	}
}


int CLightingAppTask::FrameMove( float dt )
{
	int ret = StageViewerGameTask::FrameMove(dt);
	if( ret != ID_INVALID )
		return ret;

	if( GetAnimatedGraphicsManager() )
	{
		DisplayEntityPositions( *GetAnimatedGraphicsManager() );
	}

	return ID_INVALID;
}



//========================================================================================
// CLightingAppBase
//========================================================================================

CLightingAppBase::CLightingAppBase()
{
}


CLightingAppBase::~CLightingAppBase()
{
//	Release();
}

/*
void CLightingAppBase::Release()
{
}*/


const std::string CLightingAppBase::GetStartTaskName() const
{
	return string("");
}


int CLightingAppBase::GetStartTaskID() const
{
//	return GameTask::ID_STAGE_VIEWER_TASK;
	return GAMETASK_ID_LIGHTING;
}


bool CLightingAppBase::Init()
{
	//
	// Register base entity factory
	//
/*
	GetBaseEntityManager().RegisterBaseEntityFactory( BaseEntityFactorySharedPtr( new CFG_BaseEntityFactory() ) );

	GetBaseEntityManager().AddBaseEntityClassNameToIDMap( "CFG_AIAircraftBaseEntity", CFG_BaseEntityID::BE_AIAIRCRAFTBASEENTITY );
*/
	bool base_init = CGameApplicationBase::Init();
	if( !base_init )
		return false;


	//
	// Do initial settings using scripts
	//

	SRect cursor_rect = SRect( 0, 0, 27, 27 ) * ((float)GameWindowManager().GetScreenWidth()) / 800.0f;
	MouseCursor().Load( 0, cursor_rect, "./textures/crosshair_cursor.dds", 0xFFFFFFFF );

/*
	CScriptManager script_mgr;
	script_mgr.AddModule( "PlayerInfo",	g_PyModulePlayerMethod );
	script_mgr.LoadScriptArchiveFile( "Script/init.bin" );
	script_mgr.Update();

	//
	// Register (task name) : (task ID) maps
	//

	GameTask::AddTaskNameToTaskIDMap( "TitleFG",           CGameTaskFG::ID_TITLE_FG );
	GameTask::AddTaskNameToTaskIDMap( "MainMenuFG",        CGameTaskFG::ID_MAINMENU_FG );
	GameTask::AddTaskNameToTaskIDMap( "ControlCustomizer", CGameTaskFG::ID_CONTROLCUSTOMIZER_FG );
*/
	return true;
}
