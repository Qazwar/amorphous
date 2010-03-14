#include "InputHandler_Cutscene.hpp"
#include "GameCommon/3DActionCode.hpp"
#include "../SystemInputHandler_Debug.hpp"
#include "../BE_CameraController.hpp"


//CInputHandler_Cutscene::CInputHandler_Cutscene()
CInputHandler_Cutscene::CInputHandler_Cutscene( /*CCopyEntity *pEntity,*/ CBE_CameraController *pCameraController )
:
//m_pEntity(pEntity),
m_pCameraController(pCameraController)
{
	// 13:22 2007/07/14 - keybind init moved to SetDefaultKeybindFG()
//	SetDefaultKeyBind();

	// copy keybind stored in PlayerInfo
//	UpdateKeyBind();

//	ResetActionState();


	m_pInputHandler_Debug = new CSystemInputHandler_Debug;
}


CInputHandler_Cutscene::~CInputHandler_Cutscene()
{
	if( m_pInputHandler_Debug )
		delete m_pInputHandler_Debug;
}


void CInputHandler_Cutscene::NormalizeAnalogInput( SInputData& input )
{
	// process the fixed-keybind(non-configurable) controles
	float& fParam = input.fParam1;
	const float gpd_margin = 200.0f;
	if( GIC_GPD_AXIS_X <= input.iGICode && input.iGICode <= GIC_GPD_AXIS_Z ||
		GIC_GPD_ROTATION_X <= input.iGICode && input.iGICode <= GIC_GPD_ROTATION_Z )
	{
		// clamp the analog input from the gamepad to the range [-1,1]
		if( fabsf(fParam) < gpd_margin )
			fParam = 0;
		else
		{
			float param_sign = fParam / fabsf(fParam);
			fParam	= ( fParam - gpd_margin * param_sign ) / (1000.0f - gpd_margin) * 1.0f;
		}

	}
	else if( input.iGICode == GIC_MOUSE_AXIS_X || input.iGICode == GIC_MOUSE_AXIS_Y )
	{
		fParam *= 0.2f;

//		static count = 0;
//		count = (count+1) % 100;
//		if( count == 0 ) PrintLog( "axis(" + to_string(input.iGICode) + ") : " + to_string(fParam) );
	}
}


void CInputHandler_Cutscene::ProcessInput( SInputData& input )
{
	int action_code = 0;

	float fParam = input.fParam1;

	NormalizeAnalogInput( input );

	switch( action_code )
	{
	case ACTION_CUTSCENE_SKIP:
		// skip the current cutscene
//		m_pCameraController->EndCutscene( m_pEntity );
		m_pCameraController->SkipCutscene( m_pEntity );
		break;

	case ACTION_NOT_ASSIGNED:
	default:
		break;	// no action is assigned to this input
	}

	if( m_pInputHandler_Debug )
		m_pInputHandler_Debug->ProcessInput( input );
}
