#ifndef  __CameraControllerBase_H__
#define  __CameraControllerBase_H__


#include "amorphous/3DMath/Matrix34.hpp"
#include "amorphous/Input/fwd.hpp"


namespace amorphous
{


class CameraControl
{
public:

	enum Operation
	{
		Forward,
		Backward,
		Right,
		Left,
		Up,
		Down,
		NumOperations
	};
};


/**
 * \brief A class for controlling a simple first-person view using keyboard and mouse
 *
 * Helps to look around objects in a typical 3D scene by providing control for a camera that the app develper can move using keyboard and mouse.
 * Takes keyboard and mouse input events and updates the viewer position.
 */
class CameraControllerBase
{
	float m_fYaw;
	float m_fPitch;

	Matrix34 m_Pose;

	/// camera translation speed (meter per second)
	float m_fTranslationSpeed;

	int m_CameraControlCode[CameraControl::NumOperations];

	enum MouseButton
	{
		MBTN_LEFT,
		MBTN_RIGHT,
		NUM_MOUSE_BUTTONS
	};

	int m_IsMouseButtonPressed[NUM_MOUSE_BUTTONS]; ///< 1: pressed / 0: released

protected:

	/// Move these variables CameraController_Win32 if it's the only one
	/// who needs these.
	int m_iPrevMousePosX;
	int m_iPrevMousePosY;

	bool m_Active;

public:

	CameraControllerBase();

	virtual ~CameraControllerBase();

	/// must be called every frame
	void UpdateCameraPose( float dt );

	virtual void SetActive( bool active ) { m_Active = active; }

	bool IsKeyPressed( int general_input_code );

	void AddYaw( float fYaw );

	void AddPitch( float fPitch );

	inline Vector3 GetPosition() const { return GetPose().vPosition; }

	inline Vector3 GetRightDirection() const   { return GetPose().matOrient.GetColumn(0); }
	inline Vector3 GetUpDirection() const      { return GetPose().matOrient.GetColumn(1); }
	inline Vector3 GetForwardDirection() const { return GetPose().matOrient.GetColumn(2); }

	inline void GetPose( Matrix34& rDestPose ) const { rDestPose = m_Pose; }

	inline const Matrix34 GetPose() const { return m_Pose; }

	inline void SetPosition( const Vector3& vPosition ) { m_Pose.vPosition = vPosition; }

	inline void SetPose( const Matrix34& rSrcPose ) { m_Pose = rSrcPose; }

	inline void Move( float fRight, float fUp, float fDir )
	{
		Matrix34 current_pose = GetPose();
		current_pose.vPosition
			+= current_pose.matOrient.GetColumn(0) * fRight
			+  current_pose.matOrient.GetColumn(1) * fUp
			+  current_pose.matOrient.GetColumn(2) * fDir;

		SetPose( current_pose );
	}

	void AssignKeyForCameraControl( int general_input_code, CameraControl::Operation cam_control_op );

	/// set the speed of the camera translation (meter per second)
	inline void SetTranslationSpeed( float meter_per_second ) { m_fTranslationSpeed = meter_per_second; }

	/// Handles mouse movements
	/// Don't use this with CPlatformDependentCameraController::HandleMessage( UINT, WPARAM, LPARAM ),
	/// otherwise the mouse input will be processed twice
	void HandleInput( const InputData& input );
};


} // namespace amorphous



#endif		/*  __CameraControllerBase_H__  */
