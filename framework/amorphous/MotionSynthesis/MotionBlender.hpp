#ifndef  __MotionBlender_H__
#define  __MotionBlender_H__


#include "amorphous/3DMath/Matrix34.hpp"
#include "amorphous/3DMath/MathMisc.hpp"
#include "MotionPrimitive.hpp"


namespace amorphous
{


namespace msynth
{


class MotionBlender
{
public:

	enum BlenderType
	{
		STEERING = 0,
		IK,
		SCALING,          ///< should be done in pre-processing or not
		NUM_BLEND_TYPES
	};

public:

	MotionBlender();
	virtual ~MotionBlender();

	virtual void Update( float dt ) {}

	virtual void CalculateKeyframe( Keyframe& dest_keyframe ) = 0;
//	virtual void Update( const MotionPrimitive& motion, Keyframe& current_keyframe, float current_time, float dt ) = 0;
};


/**
 NOTES:
 - 2 types of blend weights
   - weight that is applied to the entire keyframe
   - weight set to each node/joint


*/
/*
class CSourceMotionBlender
{
	enum Params
	{
		NUM_MAX_SIMULTANEOUS_MOTIONS_TO_BLEND = 8;
	};

	/// holds borrowed references to motion primitives
	boost::array<MotionPrimitive *, NUM_MAX_SIMULTANEOUS_MOTIONS_TO_BLEND> m_vecpCurrentMotionPrimitive;

public:

	/// create a source keyframe ?
	virtual void Update( const MotionPrimitive& motion, Keyframe& current_keyframe, float current_time, float dt );
};


void CSourceMotionBlender::Update( const MotionPrimitive& motion, Keyframe& current_keyframe, float current_time, float dt )
{
	num_motions_to_blend
	if( num_motions_to_blend == 0 )
	{
		return;
	}
	else if( num_motions_to_blend == 1 )
	{
		// calc keyframe
		current_keyframe = motion.GetKeyFrame( current_time );
	}
	else
	{
		// blend multiple keyframes
		for_each()
		{
			current_keyframe += m_vecpCurrentMotionPrimitive[i]->GetKeyFrame( current_time ) * blend_weight[0];
		}
	}
}
*/

class SteeringMotionBlender : public MotionBlender
{
	/// TODO: what about steering of non-horizontal motions?
	Vector3 m_vDestDir;

	/// rad per sec
	float m_fSteeringSpeed;

	std::shared_ptr<MotionPrimitiveBlender> m_pMotionPrimitiveBlender;

public:

	inline Vector3 GetRootPoseHorizontalDirection( const Keyframe& keyframe );

public:

//	SteeringMotionBlender() : m_fSteeringSpeed(0.1f), m_vDestDir(Vector3(0,0,1)) {}
	SteeringMotionBlender( std::shared_ptr<MotionPrimitiveBlender> pMotionPrimitiveBlender )
		:
	m_pMotionPrimitiveBlender(pMotionPrimitiveBlender),
	m_fSteeringSpeed(0.0f),
	m_vDestDir(Vector3(0,0,0))
	{}
	virtual ~SteeringMotionBlender() {}

	float GetSteeringSpeed() const { return m_fSteeringSpeed; }

	/// \param turn speed in degrees per second
	void SetSteeringSpeed( float speed ) { m_fSteeringSpeed = deg_to_rad(speed); }

	/// set the destination direction
	/// set Vector3(0,0,0) to specify no target direction
	void SetDestDirection( Vector3 vDestDir );

	virtual void Update( float dt );

	virtual void CalculateKeyframe( Keyframe& dest_keyframe );
//	virtual void Update( const MotionPrimitive& motion, Keyframe& current_keyframe, float current_time, float dt );
};


inline Vector3 SteeringMotionBlender::GetRootPoseHorizontalDirection( const Keyframe& keyframe )
{
	const Vector3 vDir = keyframe.GetRootPose().vPosition;
//	const Vector3 vDir = keyframe.GetMotionDirection().vPosition;

	Vector3 vHDir = vDir;
	vHDir.y = 0;
	Vec3Normalize( vHDir, vHDir );

	return vHDir;
}


class CIKMotionBlender : public MotionBlender
{
	// target joint

	// target pose

public:
};


/**
 Does the scale adjustment during runtime
 - pros.
   - save memory by avoid duplications of similar motion primitives ?

 - cons.
   - additional computation during runtime

*/
/*
class CScalingMotionBlender : public MotionBlender
{
	float m_fFactor;

public:

	virtual void Update( const MotionPrimitive& motion, Keyframe& current_keyframe, float current_time, float dt );
};


void CScalingMotionBlender::Update( const MotionPrimitive& motion, Keyframe& current_keyframe, float current_time, float dt )
{
	current_keyframe.GetRootPose().vPosition * m_fFactor;
}
*/


} // namespace msynth

} // namespace amorphous



#endif		/*  __MotionBlender_H__  */
