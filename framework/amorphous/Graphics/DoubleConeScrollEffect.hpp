#ifndef  __DoubleConeScrollEffect_HPP__
#define  __DoubleConeScrollEffect_HPP__


#include <vector>
#include "fwd.hpp"
#include "TextureHandle.hpp"
#include "MeshObjectHandle.hpp"
#include "amorphous/3DMath/Matrix34.hpp"
#include "amorphous/3DMath/CriticalDamping.hpp"


namespace amorphous
{


//namespace ???
//{


class DoubleConeScrollEffectDesc
{
public:
	float m_fStreakLength;
	float m_fScrollSpeed;
};


class DoubleConeScrollEffect
{
	std::string m_TextureFilepath;

//	TextureHandle m_Texture;

	Matrix34 m_CameraPose;

	Vector3 m_vCameraVelocity;

	MeshHandle m_DoubleConeMesh;

	float m_fTexShiftV;

//	cdv<Quaternion> m_qTilt;
	cdv<float> m_TiltAngle;

public:

	DoubleConeScrollEffect();

	~DoubleConeScrollEffect() {}

	void SetTextureFilepath( const std::string& tex_filepath ) { m_TextureFilepath = tex_filepath; }

	void Init();

	void Update( float dt );

	void Render();

	void SetCameraVelocity( const Vector3& vVel ) { m_vCameraVelocity = vVel; }

	void SetCameraPose( const Matrix34& cam_pose ) { m_CameraPose = cam_pose; }

/*
	const std::string& GetName() const { return m_Name; }
	void SetName( const std::string& name ) { m_Name = name; }
*/
};


//} // namespace ???

} // namespace amorphous



#endif  /*  __DoubleConeScrollEffect_HPP__  */
