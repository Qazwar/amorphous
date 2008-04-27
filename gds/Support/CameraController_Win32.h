#ifndef  __CAMERACONTROLLER_WIN32_H__
#define  __CAMERACONTROLLER_WIN32_H__


#include <d3d9.h>
#include <d3dx9.h>

#include "3DMath/Matrix34.h"

//#include "GameInput/InputHandler.h"



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

/*
class CCameraController_Win32 : public CCameraController
{
public:
};


class CCameraController_Linux: public CCameraController
{
public:
};
*/

class CCameraController_Win32
{
	float m_fYaw;
	float m_fPitch;

	Matrix34 m_Pose;

	/// camera translation speed (meter per second)
	float m_fTranslationSpeed;

	int m_iPrevMousePosX;
	int m_iPrevMousePosY;

	int m_CameraControlCode[CameraControl::NumOperations];

public:

	CCameraController_Win32();
	~CCameraController_Win32();

	void UpdateCameraPosition( float dt );	// must be called once in every frame

	void SetCameraMatrix();

	inline void GetCameraMatrix( D3DXMATRIX& rmatCamera ) const;

	inline D3DXMATRIX GetCameraMatrix() const;

	void AddYaw( float fYaw );

	void AddPitch( float fPitch );

	inline Vector3 GetPosition() const { return m_Pose.vPosition; }

	inline Vector3 GetRightDirection() const   { return m_Pose.matOrient.GetColumn(0); }
	inline Vector3 GetUpDirection() const      { return m_Pose.matOrient.GetColumn(1); }
	inline Vector3 GetForwardDirection() const { return m_Pose.matOrient.GetColumn(2); }

	inline void GetPose( Matrix34& rDestPose ) const { rDestPose = m_Pose; }
	inline const Matrix34& GetPose() const { return m_Pose; }

	inline void SetPosition( const Vector3& vPosition ) { m_Pose.vPosition = vPosition; }

	inline void SetPose( const Matrix34& rSrcPose ) { m_Pose = rSrcPose; }

	inline void Move( float fRight, float fUp, float fDir )
	{
		m_Pose.vPosition += m_Pose.matOrient.GetColumn(0) * fRight + m_Pose.matOrient.GetColumn(1) * fUp + m_Pose.matOrient.GetColumn(2) * fDir;
	}

    void HandleMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	void AssignKeyForCameraControl( int general_input_code, CameraControl::Operation cam_control_op );

	/// set the speed of the camera translation (meter per second)
	inline void SetTranslationSpeed( float meter_per_second ) { m_fTranslationSpeed = meter_per_second; }
};


inline void CCameraController_Win32::GetCameraMatrix( D3DXMATRIX& rmatCamera ) const
{
	Vector3 vRight = m_Pose.matOrient.GetColumn(0);
	Vector3 vUp    = m_Pose.matOrient.GetColumn(1);
	Vector3 vFwd   = m_Pose.matOrient.GetColumn(2);
	rmatCamera._11 = vRight.x;	rmatCamera._12 = vUp.x; rmatCamera._13 = vFwd.x; rmatCamera._14 = 0;
	rmatCamera._21 = vRight.y;	rmatCamera._22 = vUp.y; rmatCamera._23 = vFwd.y; rmatCamera._24 = 0;
	rmatCamera._31 = vRight.z;	rmatCamera._32 = vUp.z; rmatCamera._33 = vFwd.z; rmatCamera._34 = 0;

	rmatCamera._41= -Vec3Dot(&vRight, &m_Pose.vPosition);
	rmatCamera._42= -Vec3Dot(&vUp,    &m_Pose.vPosition);
	rmatCamera._43= -Vec3Dot(&vFwd,   &m_Pose.vPosition);
	rmatCamera._44=1;
}


inline D3DXMATRIX CCameraController_Win32::GetCameraMatrix() const
{
	D3DXMATRIX matCamera;

	GetCameraMatrix( matCamera );

	return matCamera;
}


#endif		/*  __CAMERACONTROLLER_WIN32_H__  */
