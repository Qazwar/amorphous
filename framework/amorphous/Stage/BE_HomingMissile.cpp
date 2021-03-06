#include "BE_HomingMissile.hpp"
#include "GameMessage.hpp"
#include "CopyEntityDesc.hpp"
#include "trace.hpp"
#include "ViewFrustumTest.hpp"
#include "Stage.hpp"
#include "Serialization_BaseEntityHandle.hpp"
#include "amorphous/Physics/Actor.hpp"
#include "amorphous/Support/Profile.hpp"
#include "amorphous/Support/Macro.h"
#include "amorphous/Support/Log/DefaultLog.hpp"
#include "amorphous/Support/Vec3_StringAux.hpp"


namespace amorphous
{

using namespace std;
using namespace physics;


static vector<CCopyEntity *> s_vecpVisibleEntity;

inline float& TraveledDist(CCopyEntity* pCopyEnt) { return pCopyEnt->f2; }
inline float& SensorTimer(CCopyEntity* pCopyEnt) { return pCopyEnt->f3; }
inline float& DesiredDeltaAngle(CCopyEntity* pCopyEnt) { return pCopyEnt->f4; }
inline Vector3& RefAxis(CCopyEntity* pCopyEnt) { return pCopyEnt->v2; }


CBE_HomingMissile::CBE_HomingMissile()
{
	m_bSensor			= true; 
	m_fValidSensorAngle	= 1.0f;
	m_fMaxSensorRange	= 200.0f;
	m_fMaximumRange		= 5000.0f;
	m_fAcceleration		= 1.0f;
	m_fMaxSpeed			= 10.0f;
	m_fSensoringInterval= 0.3f;
	m_fTurnSpeed		= 1.0f;

	m_fProximityThresholdDistSq = 0.0f;

	m_MissileFlag = DONT_LOSE_TARGET_EVEN_IF_OBSTRUCTED;

	m_SmokeTrail.SetBaseEntityName( "smkt" );

	// register to physics simulator for UpdatePhysics() to be called
	RaiseEntityFlag( BETYPE_RIGIDBODY );

	// ignore physics simulator results:
	// pose & vel is calculated in MissileMove()
	ClearEntityFlag( BETYPE_USE_PHYSSIM_RESULTS );

	// register as no-clip to physics simulator
	// and do the collision detection by itself
	m_ActorDesc.CollisionGroup = ENTITY_COLL_GROUP_NOCLIP;

	m_ActorDesc.BodyDesc.Flags = BodyFlag::Kinematic;
}


void CBE_HomingMissile::Init()
{
	CBE_Explosive::Init();

	m_InitSensorCamera.SetFOV( m_fValidSensorAngle );
	m_InitSensorCamera.SetFarClip( m_fMaxSensorRange );
	m_InitSensorCamera.CreateVFTree();	// update VFTree for 'm_fValidSensorAngle'

	s_vecpVisibleEntity.reserve(256);

	LoadBaseEntity( m_SmokeTrail );
	LoadBaseEntity( m_FlameTrail );
	LoadBaseEntity( m_Light );

	CCapsuleShapeDesc desc;
	desc.fLength = take_max( 0.10f, m_aabb.GetExtents().z );
	desc.fRadius = take_max( 0.05f, m_aabb.GetExtents().x * 0.5f );
	m_ActorDesc.vecpShapeDesc.push_back( new CCapsuleShapeDesc(desc) );
}


// notify object lock-on by this missile
void OnTargetLocked( CCopyEntity* pCopyEnt, CCopyEntity* pTarget )
{
	GameMessage msg( GM_MISSILE_TARGET );
	msg.sender = pCopyEnt->Self();

	SendGameMessageTo( msg, pTarget );
}


void CBE_HomingMissile::FindTarget( CCopyEntity* pCopyEnt )
{
	float fLength = 5.0f;	// length of the missile
	Vector3 vCameraPos = pCopyEnt->GetWorldPosition() + pCopyEnt->GetDirection() * (fLength * 0.5f + 0.05f);

	// initial sensor camera is used to lock the target
	m_InitSensorCamera.UpdatePosition( vCameraPos,
		                               pCopyEnt->GetDirection(),
									   pCopyEnt->GetRightDirection(),
									   pCopyEnt->GetUpDirection() );


	// check visible entities
	ViewFrustumTest vf_test;
//	vf_test.m_Flag = VFT_IGNORE_NOCLIP_ENTITIES;
	vf_test.SetCamera( &m_InitSensorCamera );
	vf_test.SetBuffer( s_vecpVisibleEntity );
	vf_test.ClearEntities();	// clear any previous data

	// collect entities that are in the view frustum volume of the sensor camera
	m_pStage->GetVisibleEntities( vf_test );

	if( 0 < vf_test.GetNumVisibleEntities() )
	{
		int i, iNumVisibleEntities = vf_test.GetNumVisibleEntities();
		for( i=0; i<iNumVisibleEntities; i++ )
		{
			CCopyEntity *pTarget = vf_test.GetEntity(i);
			if( pTarget->bNoClip )
				continue;	// don't target a no-clip entity

//			if( pTarget->GroupIndex != pCopyEnt->GroupIndex )
			if( m_pStage->GetEntitySet()->IsCollisionEnabled( pCopyEnt->GroupIndex, pTarget->GroupIndex ) )
			{
				pCopyEnt->m_Target = EntityHandle<>( pTarget->Self() );
				OnTargetLocked( pCopyEnt, pTarget );
				break;
			}
		}
	}
	else
	{	// couldn't find a target to lock on
		pCopyEnt->m_Target.Reset();
	}
}


inline void CBE_HomingMissile::CorrectCourse( CCopyEntity* pCopyEnt, float frametime )
{
	float& rfDesiredDeltaAngle = DesiredDeltaAngle(pCopyEnt);
	Vector3& rvAxis = RefAxis(pCopyEnt);

	float fDeltaAngle = m_fTurnSpeed * frametime;
	if( rfDesiredDeltaAngle < fDeltaAngle )
		fDeltaAngle = rfDesiredDeltaAngle;

	rfDesiredDeltaAngle -= fDeltaAngle;

//	PERIODICAL( 100, g_Log.Print( "CBE_HomingMissile - correcting course (delta_angle = %.3f)", rfDesiredDeltaAngle ) )

	Matrix33 matRot = Matrix33RotationAxis( fDeltaAngle, rvAxis );

	Matrix33 matCurrentOrient = pCopyEnt->GetWorldPose().matOrient;

	matCurrentOrient = matRot * matCurrentOrient;

	// should do orthonormalization?
//	matCurrentOrient.Orthonormalize();

	pCopyEnt->SetWorldOrientation( matCurrentOrient );
}


void CBE_HomingMissile::Ignite( CCopyEntity* pCopyEnt )
{
	if( MissileState(pCopyEnt) == MS_IGNITED )
		return;

	MissileState(pCopyEnt) = MS_IGNITED;

	// move a bit in advance at initial velocity to avoid hitting the shooter entity if it is moving foward
	Vector3 vPrevPos = pCopyEnt->GetWorldPosition();
	MissileMove( pCopyEnt, m_pStage->GetFrameTime() );

	// generate a smoke & flame trail entity
	CCopyEntityDesc trail;

	BaseEntityHandle *pTrailEntityHandle[2] = { &m_SmokeTrail, &m_FlameTrail };

	trail.SetWorldPosition( pCopyEnt->GetWorldPosition() );

	Vector3& rvEmitterPrevPos = trail.v1;
	rvEmitterPrevPos = vPrevPos;

	float& rfLeftTraceLength = trail.f3;
	rfLeftTraceLength = 0.0f;

	float& rfCurrentTime = trail.f2;
	rfCurrentTime = 0.0f;

	trail.pParent = pCopyEnt;	// a smoke trace is linked as a child of a homing missile

	for( int i=0; i<2; i++ )
	{
		trail.pBaseEntityHandle = pTrailEntityHandle[i];

		m_pStage->CreateEntity( trail );
	}

	// create a child entity to render dynamic light that moves with the missile
	if( 0 < strlen(m_Light.GetBaseEntityName()) )
	{
		CCopyEntityDesc& light = trail;
		light.pBaseEntityHandle = &m_Light;
		light.v1 = Vector3(0,0,0);	// set v1 to zero in order ot use the default color of the light
		m_pStage->CreateEntity( light );
	}
}


void CBE_HomingMissile::InitCopyEntity( CCopyEntity* pCopyEnt )
{
	// set the durability of this missile
	pCopyEnt->fLife = m_fLife;

	if( m_bSensor )
		FindTarget(pCopyEnt);

	// reset sensor timer
	SensorTimer(pCopyEnt) = 0.0f;

	DesiredDeltaAngle(pCopyEnt) = 0.0f;

	RefAxis(pCopyEnt) = pCopyEnt->GetUpDirection();

	// set speed from initial velocity
	pCopyEnt->fSpeed = Vec3Length( pCopyEnt->vVelocity );

	if( MissileState(pCopyEnt) == MS_IGNITED )
	{
		// ignite immediately
		MissileState(pCopyEnt) = MS_LOADED;	// clear the ignited state once since Ignite() reject repeated calls
		Ignite( pCopyEnt );
	}
}


void CBE_HomingMissile::Act( CCopyEntity* pCopyEnt )
{
	switch( MissileState(pCopyEnt) )
	{
	case MS_LOADED:
		return;

	case MS_WAITING_IGNITION:
		MissileIgnitionTimer(pCopyEnt) -= m_pStage->GetFrameTime();
		if( MissileIgnitionTimer(pCopyEnt) <= 0 )
		{
			Ignite(pCopyEnt);
			return;
		}
		break;

	case MS_DETONATING:
		CBE_Explosive::Explode( pCopyEnt );
		return;

	default:
		break;
	}

	// UpdateIgnitedMissile?

	ProfileBegin( "missile & its child entities" );

	UpdateMissile( pCopyEnt, m_pStage->GetFrameTime() );

	int i, num_children = pCopyEnt->GetNumChildren();

	for( i=0; i<num_children; i++ )
	{
		CCopyEntity *pChildEntity = pCopyEnt->GetChild(i);

		// Alas, child entity need to be checked before being updated
		if( pChildEntity )
			pChildEntity->Act();
	}

	ProfileEnd( "missile & its child entities" );

	// the child entities above are supposed to be smoke trail, flame trail and dynamic light
	// i.e.
/*
	CCopyEntity *pSmokeTrail	= pCopyEnt->GetChild(0);	pSmokeTrail->Act();	// update smoke trail
	CCopyEntity *pFlameTrail	= pCopyEnt->GetChild(1);	pFlameTrail->Act();	// update flame trail
	CCopyEntity *pLight			= pCopyEnt->GetChild(2);	pLight->Act();		// update dynamic light
*/
}


void CBE_HomingMissile::UpdateMissile( CCopyEntity* pCopyEnt, float frametime )
{
	PROFILE_FUNCTION();

	float& rfSensoringTimer    = pCopyEnt->f3;
	float& rfDesiredDeltaAngle = pCopyEnt->f4;
	Vector3& rvAxis = RefAxis(pCopyEnt);

	// save the kinetic energy - 'pCopyEnt->f1' is supposed to represent energy
	pCopyEnt->f1 = Vec3LengthSq( pCopyEnt->vVelocity );

	if( pCopyEnt->bInSolid
	|| m_fMaximumRange < TraveledDist(pCopyEnt) )
	{
		LOG_PRINT( " Calling Explode() (case 1)" );
		Explode(pCopyEnt);
		return;
	}

	CCopyEntity *pTarget = pCopyEnt->m_Target.GetRawPtr();

	// if the proximity sensor is enabled (i.e. 0 < m_fProximityThresholdDistSq ),
	// check the distance to the target and see whether the missile is close enough to explode
	if( 0 < m_fProximityThresholdDistSq && pTarget )
	{
		float fDistSq = Vec3LengthSq( pCopyEnt->GetWorldPosition() - pTarget->GetWorldPosition() );
		if( fDistSq <= m_fProximityThresholdDistSq
		 && /* m_CheckCollisionGroup == false  || */ m_pStage->GetEntitySet()->IsCollisionEnabled( pCopyEnt->GroupIndex, pTarget->GroupIndex ) )
		 
		{
			LOG_PRINT( " Calling Explode() (case 2)" );
			Explode(pCopyEnt);
			return;
		}
	}

	// check target

	rfSensoringTimer += frametime;

	if( m_fSensoringInterval <= rfSensoringTimer
	 && pTarget )
	{
		// update 'rfDesiredDeltaAngle' and 'rvAxis'
		// check the target is still in sight
		rfSensoringTimer = 0.0f;

		Vector3 vStart = pCopyEnt->GetWorldPosition() + pCopyEnt->GetDirection() * (m_aabb.vMax.z * 2.0f);
		STrace tr;
		tr.vStart = vStart;	// current missile position
		Vector3 vGoal = pTarget->GetWorldPosition();	// target position
		tr.vGoal = vGoal;
		tr.bvType = BVTYPE_DOT;
		tr.sTraceType = TRACETYPE_IGNORE_NOCLIP_ENTITIES;
		tr.GroupIndex = pCopyEnt->GroupIndex;
		tr.aabb.vMin = tr.aabb.vMax = Vector3(0,0,0);
		tr.SetAABB();

		m_pStage->ClipTrace( tr );

//		CCopyEntity *pTarget = pCopyEnt->m_Target.GetRawPtr();

		if( tr.pTouchedEntity != pTarget
		 && !(m_MissileFlag & DONT_LOSE_TARGET_EVEN_IF_OBSTRUCTED) )
		{
			// lost sight of the target
//			LOG_PRINT( "Lost target" );
			pCopyEnt->m_Target.Reset();
			return;
		}
		else
		{
			// target is still locked on
			float dp;
			Vector3 vMissileToTarget;

			vMissileToTarget = pTarget->GetWorldPosition() - pCopyEnt->GetWorldPosition();
			Vec3Normalize( vMissileToTarget, vMissileToTarget );
			dp = Vec3Dot( pCopyEnt->GetDirection(), vMissileToTarget );
			if( dp < 0.05f ||			// angle too is steep - consider this case as lost target
				1.0f - dp < 0.001f)		// the missile is on a right course
			{	// maintain the current course
//				pCopyEnt->pTarget = NULL;
				return;
			}
			Vec3Cross( rvAxis, pCopyEnt->GetDirection(), vMissileToTarget );
			Vec3Normalize( rvAxis, rvAxis );
			rfDesiredDeltaAngle = (float)acos( dp );

			// prevents the problem caused by the error of float value
			// TODO: orthonormalize
			Vector3 v;
			Vec3Normalize( v, pCopyEnt->GetDirection() );
			pCopyEnt->SetDirection( v );
			Vec3Normalize( v, pCopyEnt->GetRightDirection() );
			pCopyEnt->SetDirection_Right( v );

		}
	}

	// correct the course of the missile
	CorrectCourse( pCopyEnt, frametime );
}


void CBE_HomingMissile::UpdatePhysics( CCopyEntity *pCopyEnt, float dt )
{
	bool bUpdatePhysicsOnlyIfIgnited = true;
	if( bUpdatePhysicsOnlyIfIgnited
	 && MissileState(pCopyEnt) == MS_IGNITED )
        MissileMove( pCopyEnt, dt );

	physics::CActor *pPhysicsActor = pCopyEnt->GetPrimaryPhysicsActor();
	if( pPhysicsActor )
	{
		pPhysicsActor->SetWorldPose( pCopyEnt->GetWorldPose() );

		// Commented out - the actor must be (non-kinematic) dynamic to call its SetLinearVelocity()
//		pPhysicsActor->SetLinearVelocity( Vector3(0,0,0) );
//		pPhysicsActor->SetVelocity( pCopyEnt->Velocity() );
	}
}


void CBE_HomingMissile::MessageProcedure(GameMessage& rGameMessage, CCopyEntity* pCopyEnt_Self)
{
	switch( rGameMessage.effect )
	{
	case GM_SET_TARGET:
		{
//			pCopyEnt_Self->m_Target = EntityHandle<>( ((CCopyEntity *)rGameMessage.pUserData)->Self() );
			CCopyEntity *pTarget = (CCopyEntity *)rGameMessage.pUserData;
			pCopyEnt_Self->m_Target = EntityHandle<>( pTarget->Self() );
			break;
		}
	}
}


void CBE_HomingMissile::MissileMove( CCopyEntity* pCopyEnt, float frametime )
{
	PROFILE_FUNCTION();

	STrace tr;

	if( MissileState(pCopyEnt) == MS_DETONATING )
		return; // already hit something - no more moves

	// update velocity
	if( MissileState(pCopyEnt) == MS_IGNITED )
	{
		pCopyEnt->fSpeed += m_fAcceleration * frametime;
		if( m_fMaxSpeed < pCopyEnt->fSpeed )
			pCopyEnt->fSpeed = m_fMaxSpeed;	// clamp to max missile speed
	}

	pCopyEnt->Velocity() = pCopyEnt->GetDirection() * pCopyEnt->fSpeed;

	// ApplyQFriction(  );

	// apply gravity
//	Vector3 vGravityAccel = this->m_pStage->GetGravityAccel();
//	pCopyEnt->vVelocity += vGravityAccel * fFrametime * 0.5f;
	Vector3 vStart = pCopyEnt->GetWorldPosition();
	tr.vStart = vStart;
	Vector3 vGoal = pCopyEnt->GetWorldPosition() + pCopyEnt->Velocity() * frametime;
	tr.vGoal = vGoal;
	tr.aabb = this->m_aabb;
//	tr.bvType = this->m_BoundingVolumeType;
	tr.bvType = BVTYPE_SMALLSPHERE;
	tr.fRadius = this->m_fRadius;
	tr.pSourceEntity = pCopyEnt;
	tr.sTraceType = TRACETYPE_IGNORE_NOCLIP_ENTITIES;
	tr.GroupIndex = pCopyEnt->GroupIndex;
//	tr.vecTargetGroup.push_back( CE_GROUP_ENEMY );

//	PERIODICAL( 1500, MsgBoxFmt( "start: %s, goal: %s", to_string(pCopyEnt->GetWorldPosition()).c_str(), to_string(vGoal).c_str() ) )


	this->m_pStage->ClipTrace( tr );

	TraveledDist(pCopyEnt) += Vec3Length( tr.vEnd - pCopyEnt->GetWorldPosition() );

	pCopyEnt->SetWorldPosition( tr.vEnd );
	pCopyEnt->touch_plane = tr.plane;
	pCopyEnt->bInSolid = tr.in_solid;

//	if( tr.in_solid )
//		NudgePosition( pCopyEnt );

	if( tr.fFraction < 1.0f )	// hit something
	{
		pCopyEnt->AddTouchedEntity( tr.pTouchedEntity );
		pCopyEnt->fSpeed = 0;
//		pCopyEnt->Direction( Vector3(0,0,0) );
		pCopyEnt->vVelocity = Vector3(0, 0, 0);
///		pCopyEnt->sState |= CESTATE_ATREST;

		// CBE_Explosive::Explode() is called in the next CBE_HomingMissile::Act()
		MissileState(pCopyEnt) = MS_DETONATING;
	}
}


bool CBE_HomingMissile::LoadSpecificPropertiesFromFile( CTextFileScanner& scanner )
{
	CBE_Explosive::LoadSpecificPropertiesFromFile(scanner);

	float dist;
	string light_for_flame, smoke_trail, flame_trail;

	if( scanner.TryScanBool( "SENSOR", "YES/NO",	m_bSensor ) ) return true;
	if( scanner.TryScanLine( "SNS_ANGLE",	m_fValidSensorAngle ) ) return true;
	if( scanner.TryScanLine( "MAXRANGE",	m_fMaximumRange ) ) return true;
	if( scanner.TryScanLine( "SNSR_RANGE",	m_fMaxSensorRange ) ) return true;

	if( scanner.TryScanLine( "ACCEL",		m_fAcceleration ) ) return true;
	if( scanner.TryScanLine( "MAXSPEED",	m_fMaxSpeed ) ) return true;
	if( scanner.TryScanLine( "TURN_SPD",	m_fTurnSpeed ) ) return true;

	if( scanner.TryScanLine( "SNSR_INTVL",	m_fSensoringInterval ) ) return true;

	if( scanner.TryScanLine( "PROXIMITY_THRESHOLD", dist ) )
	{
		m_fProximityThresholdDistSq = dist * dist;
		return true;
	}

	if( scanner.TryScanLine( "LIGHT_FOR_FLAME", light_for_flame ) )
	{
		m_Light.SetBaseEntityName( light_for_flame.c_str() );
		return true;
	}

	if( scanner.TryScanLine( "TRACESMOKE", smoke_trail ) )
	{
		m_SmokeTrail.SetBaseEntityName( smoke_trail.c_str() );
		return true;
	}

	if( scanner.TryScanLine( "FLAMETRACE", flame_trail ) )
	{
		m_FlameTrail.SetBaseEntityName( flame_trail.c_str() );
		return true;
	}

	return false;
}


void CBE_HomingMissile::Serialize( IArchive& ar, const unsigned int version )
{
	CBE_Explosive::Serialize( ar, version );

	ar & m_bSensor;
	ar & m_fValidSensorAngle;
	ar & m_fMaxSensorRange;
	ar & m_fMaximumRange;
	ar & m_fAcceleration;
	ar & m_fMaxSpeed;
	ar & m_fTurnSpeed;
	ar & m_fSensoringInterval;
	ar & m_fProximityThresholdDistSq;

	ar & m_Light;
	ar & m_SmokeTrail;
	ar & m_FlameTrail;
}


} // namespace amorphous
