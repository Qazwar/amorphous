#include "GameTask_Stage.hpp"

#include "amorphous/Support/macro.h"
#include "amorphous/Support/memory_helpers.hpp"
#include "amorphous/Support/Profile.hpp"
#include "amorphous/Support/Log/DefaultLog.hpp"
#include "amorphous/Support/DebugOutput.hpp"
#include "amorphous/Graphics/2DPrimitive/2DRect.hpp"
#include "amorphous/Graphics/GraphicsEffectManager.hpp"
#include "amorphous/Stage/Stage.hpp"
#include "amorphous/Stage/StageLoader.hpp"
#include "amorphous/Stage/EntitySet.hpp"
#include "amorphous/Stage/ScreenEffectManager.hpp"
#include "amorphous/Stage/PlayerInfo.hpp"
#include "amorphous/Input/InputHub.hpp"
#include "amorphous/Sound/SoundManager.hpp"


namespace amorphous
{

using namespace std;


CStageSharedPtr g_pStage;


static string gs_NextGlobalStageScript;


void SetGlobalStageScriptFilename( const string& filename )
{
	gs_NextGlobalStageScript = filename;
}


const string& GetGlobalStageScriptFilename()
{
	return gs_NextGlobalStageScript;
}


static int gs_iState = GameTask_Stage::STAGE_NOT_LOADED;


void LoadStage( const string& strStageScriptFilename )
{
	// update the database of base entity before intitializing the stage
	// 070223 moved to ctor in GameTask_TitleFG
//	UpdateBaseEntityDatabase();

	g_pStage.reset();

	StageLoader stage_loader;
	g_pStage = stage_loader.LoadStage( strStageScriptFilename );

	bool bResult = false;
	if( g_pStage )
		bResult = true;

	if( bResult )
	{
		gs_iState = GameTask_Stage::STAGE_LOADED;
		LOG_PRINT( "loaded the stage: " + strStageScriptFilename );
	}
	else
	{
		gs_iState = GameTask_Stage::STAGE_NOT_LOADED;
		LOG_PRINT_ERROR( "cannot load the stage: " + strStageScriptFilename );
	}

//	if( gs_iState != GameTask_StageSelect::STAGE_LOADED )
//		LOG_PRINT_ERROR( "unable to load stage" + strStageScriptFilename

	// do not start the stage timer until the stage task is initiated
	g_pStage->PauseTimer();

//	LOG_PRINT( "stage loaded" );
}


unsigned int GameTask_Stage::ms_FadeInTime = 500;

GameTask_Stage::GameTask_Stage()
{
	if( !g_pStage )
	{
		// Initialized at app startup in CApplicationBase::InitBase()
//		InitAnimatedGraphicsManager();

		// set animated graphics manager here
		// since init routines of the scripts use graphics manager
		SetAnimatedGraphicsManagerForScript();

		if( 0 < GetGlobalStageScriptFilename().length() )
		{
            LoadStage( GetGlobalStageScriptFilename() );
		}
		else
		{
			LOG_PRINT_WARNING( "No global stage has been specified." );
			return;
		}
	}

	g_pStage->ResumeTimer();

	SetDefaultFadeInTimeMS( ms_FadeInTime );
}


GameTask_Stage::~GameTask_Stage()
{
	ms_FadeInTime = 500;	// set the default fade-in time
}


int GameTask_Stage::FrameMove( float dt )
{
	ONCE( LOG_PRINT( " - entered." ) );

	int ret = GameTask::FrameMove(dt);
	if( ret != ID_INVALID )
		return ret;

	if( !g_pStage )
		return GameTask::ID_INVALID;	// stage is not loaded

	ProfileBegin( "Main Loop" );

//	ONCE( LOG_PRINTF(( "stage state: %d", g_pStage->GetState() )) );

	int state = STATE_PLAYER_IN_STAGE;//this->GetState();

	switch( state )
	{
	case STATE_PLAYER_IN_STAGE:
		{
			g_pStage->UpdateListener();

//			GetAnimatedGraphicsManager()->UpdateEffects( dt );
			SetAnimatedGraphicsManagerForScript();

			// update the world and progress by one frame
			g_pStage->Update( dt );

			RemoveAnimatedGraphicsManagerForScript();
		}
		break;

	case STATE_PLAYER_LEFT_STAGE:
		return GameTask::ID_STAGE_SELECT;
/*
	case CStage::MISSION_FAILED:
//		return GameTaskFG::ID_ON_MISSIONFAILED;

	case CStage::MISSION_ACCOMPLISHED:
//		return GameTaskFG::ID_MAINMENU;
*/
	default:
		LOG_PRINT_WARNING( " An unsupported stage state: " + to_string(state) );
		return GameTask::ID_INVALID;

	}

	// update player info (includes update routines for items)
	SinglePlayerInfo().Update( dt );

	ONCE( LOG_PRINT( " - updated player info" ) );

	// update text message
	// 0:40 2007/05/06 - moved to CStage::Update()
//	GetTextMessageManager().Update( dt );

	if( SinglePlayerInfo().GetTaskRequest() != GameTask::ID_INVALID )
	{
		g_pStage->PauseTimer();

//		GameTask::eGameTask next_task = SinglePlayerInfo().GetTaskRequest();
		int next_task = SinglePlayerInfo().GetTaskRequest();
		SinglePlayerInfo().RequestTaskChange( GameTask::ID_INVALID );
		return next_task;
	}
	else
	{
		return GameTask::ID_INVALID;
	}
}


void GameTask_Stage::Render()
{
	ONCE( LOG_PRINT( " - rendering the stage" ) );

	// set the camera and draw the scene
	if( g_pStage )
		g_pStage->Render();

	// render text messages
//	GetTextMessageManager().Render();
//	if( g_pStage && g_pStage->GetTextMessageManager() )
//		g_pStage->GetTextMessageManager()->Render();

	// need to finish profile before calling DrawDebugInfo()
	ProfileEnd( "Main Loop" );
//	ProfileDumpOutputToBuffer();
}


} // namespace amorphous
