#include "GameTaskManager.hpp"
#include "GameTask.hpp"
//#include "amorphous/Graphics/RenderTask.hpp"
//#include "amorphous/Graphics/RenderTaskProcessor.hpp"
#include "amorphous/Graphics/GraphicsDevice.hpp"
#include "amorphous/GameTaskFactoryBase.hpp"
#include "amorphous/Support/SafeDelete.hpp"
#include "amorphous/Support/Profile.hpp"


namespace amorphous
{


GameTask *GameTaskManager::CreateTask( int iTaskID )
{
	return m_pFactory->CreateTask( iTaskID );
}


GameTaskManager::GameTaskManager( GameTaskFactoryBase* pFactory,
								    int iInitialTaskID )
:
m_pFactory(pFactory),
m_pCurrentTask(NULL),
m_CurrentTaskID(GameTask::ID_INVALID),
m_NextTaskID(iInitialTaskID)
{
}


GameTaskManager::GameTaskManager( GameTaskFactoryBase* pFactory, const std::string& initial_task_name )
:
m_pFactory(pFactory),
m_pCurrentTask(NULL),
m_CurrentTaskID(GameTask::ID_INVALID),
m_NextTaskID(GameTask::GetTaskIDFromTaskName(initial_task_name))
{
}


GameTaskManager::~GameTaskManager()
{
	SafeDelete( m_pCurrentTask );

	SafeDelete( m_pFactory );
}


void GameTaskManager::Update( float dt )
{
	PROFILE_FUNCTION();

	// get the prev task id if it has been requested
	int next_task_id;
	if( m_NextTaskID == GameTask::ID_PREVTASK )
	{
		if( 0 < m_vecTaskIDStack.size() )
		{
			next_task_id = m_vecTaskIDStack.back();
			m_vecTaskIDStack.pop_back();
		}
		else
		{
			// prev task has been requested, but there is no task in the stack
			next_task_id = GameTask::ID_INVALID;
		}
	}
	else
		next_task_id = m_NextTaskID;

	// switch tasks if new one has been requested
	if( next_task_id != GameTask::ID_INVALID )
	{
		if( m_pCurrentTask )
		{
			if( m_NextTaskID != GameTask::ID_PREVTASK )
                m_vecTaskIDStack.push_back( m_CurrentTaskID );
//			if( TASKID_STACK_SIZE < m_vecTaskIDStack.size() )
//				m_vecTaskIDStack.erase( m_vecTaskIDStack.begin() );

			m_pCurrentTask->OnLeaveTask();

			delete m_pCurrentTask;

			m_pCurrentTask = NULL;
		}

		// create a new task
		m_pCurrentTask = CreateTask( next_task_id );

		// init
		if( m_pCurrentTask )
		{
			int prev_task_id = 0 < m_vecTaskIDStack.size() ? m_vecTaskIDStack.back() : GameTask::ID_INVALID;
			m_pCurrentTask->SetPrevTaskID( prev_task_id );

			m_pCurrentTask->OnEnterTask();
		}

		// save the current task id
		m_CurrentTaskID = next_task_id;
	}

	if( m_pCurrentTask )
	{
		m_NextTaskID = m_pCurrentTask->FrameMove( dt );

		// When app exit request is made, should there be a callback which is called after FrameMove()?
//		if( m_pCurrentTask->IsAppExitRequested() )
//		{
//			m_pCurrentTask->OnAppExitRequested();
//		}
	}

}


void GameTaskManager::Render()
{
	PROFILE_FUNCTION();

	if( m_pCurrentTask )
	{
		const bool use_render_task = false;
		if( use_render_task )
		{
/*			// create list of non-hierarchical render operations
			m_pCurrentTask->CreateRenderTasks();

			RenderTaskProcessor.Render();
*/
		}
		else
		{
			// BeginScene() here if render task system is NOT used
			Result::Name res = GraphicsDevice().BeginScene();

			m_pCurrentTask->RenderBase();

			// EndScene() and Present() here if render task system is NOT used
			res = GraphicsDevice().EndScene();

			res = GraphicsDevice().Present();
		}
	}
}


} // namespace amorphous
