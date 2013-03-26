#include "PlanarReflectionAppBase.hpp"

#include "gds/Graphics/GraphicsElementManager.hpp"
#include "gds/Support.hpp"
#include "gds/Input.hpp"
#include "gds/Stage.hpp"
#include "gds/Stage/Trace.hpp"
#include "gds/Task.hpp"
#include "gds/Script.hpp"
#include "gds/GUI.hpp"
#include "gds/GameCommon/MouseCursor.hpp"
#include "gds/App/GameWindowManager.hpp"


using namespace std;
using namespace boost;


static string sg_TestStageScriptToLoad = "./Script/default.bin";


extern CApplicationBase *amorphous::CreateApplicationInstance() { return new CPlanarReflectionAppBase(); }


class StageSelectListBoxEventHandler : public CGM_ListBoxEventHandler
{
	CPlanarReflectionAppGUITask *m_pTask;

public:

	StageSelectListBoxEventHandler( CPlanarReflectionAppGUITask *pTask )
		:
	m_pTask(pTask)
	{
	}

	void OnItemSelected( CGM_ListBoxItem& item, int item_index )
	{
		switch( item_index )
		{
		case 0:
			m_pTask->LoadStage( "./Script/???.bin" );
			break;
		default:
			break;
		}

	}
};


CPlanarReflectionAppTask::CPlanarReflectionAppTask()
{
	ScriptManager::ms_UseBoostPythonModules = true;

	StageLoader stg_loader;
	m_pStage = stg_loader.LoadStage( sg_TestStageScriptToLoad );

	GetCameraController()->SetPose( Matrix34( Vector3(0,1.8f,0), Matrix33Identity() ) );
}


void CPlanarReflectionAppTask::HandleInput( const InputData& input )
{
	switch( input.iGICode )
    {
	case GIC_MOUSE_BUTTON_L:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
//			OnTriggerPulled();
		}
		break;
	default:
		break;
	}
}



//======================================================================
// CPlanarReflectionAppGUITask
//======================================================================

CPlanarReflectionAppGUITask::CPlanarReflectionAppGUITask()
{
}


void CPlanarReflectionAppGUITask::LoadStage( const std::string& stage_script_name )
{
	m_StageScriptToLoad = stage_script_name;
}


int CPlanarReflectionAppGUITask::FrameMove( float dt )
{
	int ret = GUIGameTask::FrameMove(dt);
	if( ret != ID_INVALID )
		return ret;
/*
	if( 0 < m_StageScriptToLoad.length() )
	{
		sg_TestStageScriptToLoad = m_StageScriptToLoad;
		return GAMETASK_ID_SHADOWS_TEST_STAGE;
	}
*/
	return ID_INVALID;
}



//========================================================================================
// CPlanarReflectionAppBase
//========================================================================================

CPlanarReflectionAppBase::CPlanarReflectionAppBase()
{
}


CPlanarReflectionAppBase::~CPlanarReflectionAppBase()
{
//	Release();
}

/*
void CPlanarReflectionAppBase::Release()
{
}*/


const std::string CPlanarReflectionAppBase::GetStartTaskName() const
{
	return string("");
}


int CPlanarReflectionAppBase::GetStartTaskID() const
{
//	return CGameTask::ID_STAGE_VIEWER_TASK;
	return GAMETASK_ID_BASIC_PHYSICS;
}


bool CPlanarReflectionAppBase::Init()
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
	//
	// Register (task name) : (task ID) maps
	//

	CGameTask::AddTaskNameToTaskIDMap( "TitleFG",           CGameTaskFG::ID_TITLE_FG );
	CGameTask::AddTaskNameToTaskIDMap( "MainMenuFG",        CGameTaskFG::ID_MAINMENU_FG );
	CGameTask::AddTaskNameToTaskIDMap( "ControlCustomizer", CGameTaskFG::ID_CONTROLCUSTOMIZER_FG );
*/
	return true;
}
