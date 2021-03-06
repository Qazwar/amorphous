#ifndef __ThirdPersonCameraController_HPP__
#define __ThirdPersonCameraController_HPP__


#include "../Stage/EntityHandle.hpp"
#include "../3DMath/Quaternion.hpp"
#include "../3DMath/CriticalDamping.hpp"


namespace amorphous
{


class ThirdPersonCameraController
{
	EntityHandle<> m_TargetEntity;

	cdv<Vector3> m_CameraPosition;

	cdv<Quaternion> m_CameraOrientation;

	cdv<float> m_VerticalAngle;

	Matrix33 m_CurrentCameraOrientation;

	bool m_CloseUpCamera;

public:

	ThirdPersonCameraController();
	~ThirdPersonCameraController(){}

	void Update( float dt );

	Matrix34 GetCameraPose() const;

	void SetCameraPose( const Matrix34& pose );

	void EnableCloseUpCamera( bool enable ) { m_CloseUpCamera = enable; }

	void SetTargetEntity( EntityHandle<> target ) { m_TargetEntity = target; }

	void SetTargetVerticalAngle( float target_vertical_angle ) { m_VerticalAngle.target = target_vertical_angle; }

	float GetTargetVerticalAngle() { return m_VerticalAngle.target; }
};

} // namespace amorphous



#endif /* __ThirdPersonCameraController_HPP__ */
