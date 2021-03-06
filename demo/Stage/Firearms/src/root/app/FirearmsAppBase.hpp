#ifndef  __FirearmsAppBase_HPP__
#define  __FirearmsAppBase_HPP__


#include "amorphous/App/GameApplicationBase.hpp"
#include "amorphous/Task/StageViewerGameTask.hpp"
#include "amorphous/Task/GameTaskFactoryBase.hpp"
#include "amorphous/Stage.hpp"
#include "amorphous/GameCommon/fwd.hpp"
#include "amorphous/GameCommon/KeyBind.hpp"
#include "amorphous/Item/ItemEntity.hpp"
#include "amorphous/Graphics/DoubleConeScrollEffect.hpp"
using namespace amorphous;


class CArmedCharacterOperations;


enum ShadowAppTaskID
{
//	GAMETASK_ID_SHADOWS_TEST_STAGE,
	GAMETASK_ID_BASIC_PHYSICS,
	NUM_SHADOW_APP_GAMETASK_IDS
};


class FirearmsAppTask : public StageViewerGameTask
{
	std::shared_ptr<KeyBind> m_pKeyBind;


	std::shared_ptr< CInputDataDelegate<FirearmsAppTask> > m_pInputHandler;

	EntityHandle<ItemEntity> m_CharacterItemEntity;

	std::vector< std::shared_ptr<SkeletalCharacter> > m_pCharacterItems;

	std::shared_ptr<ThirdPersonCameraController> m_pThirdPersonCameraController;

	std::shared_ptr<ThirdPersonMotionController> m_pThirdPersonMotionController;

//	std::shared_ptr<CArmedCharacterOperations> m_pArmedCharacterOperations;

	DoubleConeScrollEffect m_ScrollEffect;

	Vector3 m_vPrevCamPos;

private:

	void UpdateThirdPersonCamera( float dt );

	void ArmCharacter( SkeletalCharacter& character );

	void InitKeybind( KeyBind& keybind );

public:

	FirearmsAppTask();

	~FirearmsAppTask() {}

	int FrameMove( float dt );

	void Render();

	void HandleInput( InputData& input );
};

class CCharacterMotionControlAppGUITask : public StageViewerGameTask
{
	enum GUI_ID
	{
		GUI_ID_DLG_ROOT_STAGE_SELECT = 1000,
		GUI_ID_LBX_STAGE_SELECT,
	};

//	std::string m_StageScriptToLoad;

public:

	CCharacterMotionControlAppGUITask();

	~CCharacterMotionControlAppGUITask() {}

	int FrameMove( float dt );

//	void LoadStage( const std::string& stage_script_name );
};


class FirearmsAppTaskFactory : public GameTaskFactoryBase
{
public:

	GameTask *CreateTask( int iTaskID )
	{
		switch( iTaskID )
		{
		case GAMETASK_ID_BASIC_PHYSICS:
			return new FirearmsAppTask();
//		case GAMETASK_ID_SHADOWS_STAGE_SELECT:
//			return new CCharacterMotionControlAppGUITask();
		default:
			return GameTaskFactoryBase::CreateTask( iTaskID );
		}
	}
};


class FirearmsAppBase : public GameApplicationBase
{
public:

	FirearmsAppBase();

	virtual ~FirearmsAppBase();

	virtual bool Init();

	// virtual function implementations

	const std::string GetApplicationTitle() { return "Firearms"; }

	const std::string GetStartTaskName() const;

	int GetStartTaskID() const;

	GameTaskFactoryBase *CreateGameTaskFactory() const { return new FirearmsAppTaskFactory(); }

	void Release();
};


#endif		/*  __FirearmsAppBase_HPP__  */
