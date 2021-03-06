#ifndef  __NxPhysicsScene_H__
#define  __NxPhysicsScene_H__


#include "NxPhysics.h"

#include <vector>
#include <memory>
#include "amorphous/Physics/Scene.hpp"
#include "fwd.hpp"
#include "NxMathConv.hpp"
#include "NxPhysContactStreamIterator.hpp"


namespace amorphous
{


namespace physics
{


class CNxPhysicsUserTriggerReport : public NxUserTriggerReport
{
public:
//	CNxPhysicsScene *m_pScene;
	CUserTriggerReport *m_pUserTriggerReport;

	CNxPhysicsUserTriggerReport( CUserTriggerReport *pUserTriggerReport = NULL )
		:
	m_pUserTriggerReport(pUserTriggerReport)
	{}

	void onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status)
	{
		CTriggerEvent trigger_event;
		trigger_event.pTriggerShape = (CShape *)triggerShape.userData;
		trigger_event.pOtherShape   = (CShape *)otherShape.userData;
		trigger_event.StatusFlags   = status;

		m_pUserTriggerReport->OnTrigger( trigger_event );
	}
};


class CNxPhysicsUserContactReport : public NxUserContactReport
{
public:
//	CNxPhysicsScene *m_pScene;
	CUserContactReport *m_pUserContactReport;

	CNxPhysicsUserContactReport( CUserContactReport *pUserContactReport = NULL )
		:
	m_pUserContactReport(pUserContactReport)
	{}

	void onContactNotify(NxContactPair& pair, NxU32 events);
};


class CNxPhysScene : public CScene
{
	NxScene *m_pScene;

	/// owner of the scene (borrowed reference)
	NxPhysicsSDK* m_pPhysicsSDK;

	CNxPhysicsUserTriggerReport m_NxUserTriggerReport;

	CNxPhysicsUserContactReport m_NxUserContactReport;

	std::vector< std::shared_ptr<CNxPhysMaterial> > m_vecpNxPhysMaterial;

public:

	CNxPhysScene( NxScene *pScene, NxPhysicsSDK* pPhysicsSDK );

	virtual ~CNxPhysScene();

	/// Initialize the scene
	/// called from physics engine?
	/// Returns true on success
	virtual bool Init( const CSceneDesc& desc );

	/// Get the scene flags.
	// NOT IMPLEMENTED YET. Just calls NxScene::getFlags()
	// TODO: define flags for physics module
	virtual U32 GetFlags () const;

	/// Get the simulation type. 
//	virtual NxSimulationType GetSimType () const;

	/// Gets a private interface to an internal debug object. 
//	virtual void * GetInternal (void);

	/// Sets a constant gravity for the entire scene. 
	virtual void SetGravity ( const Vector3 &vec ) { m_pScene->setGravity( ToNxVec3(vec) ); }

	/// Retrieves the current gravity setting. 
	virtual void GetGravity ( Vector3 &vec );

	/// Flush the scene's command queue for processing. 
	virtual void FlushStream () { m_pScene->flushStream(); }

	/// Sets simulation timing parameters used in simulate(elapsedTime). 
	virtual void SetTiming ( Scalar maxTimestep, U32 maxIter, Timestep::Method timestep_method );

	/// Retrieves simulation timing parameters. 
	virtual void GetTiming ( Scalar &maxTimestep, U32 &maxIter, Timestep::Method& timestep_method, U32 *numSubSteps=NULL) const;

	/// Retrieves the debug renderable. 
//	virtual const NxDebugRenderable * GetDebugRenderable ();

	/// Call this method to retrieve the Physics SDK. 
//	virtual NxPhysicsSDK & GetPhysicsSDK ();

	/// Call this method to retrieve statistics about the current scene. 
	virtual void GetStats ( PhysSceneStats &stats ) const;

	/// Call this method to retrieve extended statistics about the current scene. 
//	virtual const CSceneStats2 * GetStats2 () const;

	/// Call to retrieve the expected object count limits set in the scene descriptor. 
	virtual void GetLimits ( CSceneLimits &limits ) const;

	/// Not yet implemented! 
//	virtual void SetMaxCPUForLoadBalancing (Scalar cpuFraction);

	/// Call to get the maximum CPU for use when load-balancing. 
//	virtual Scalar GetMaxCPUForLoadBalancing ();

	/// This is a query to see if the scene is in a state that allows the application to update scene state. 
//	virtual bool IsWritable ();

	/// Advances the simulation by an elapsedTime time. 
	virtual void Simulate ( Scalar elapsedTime ) { m_pScene->simulate( elapsedTime ); }

	/// This checks to see if the part of the simulation run whose results you are interested in has completed. 
	virtual bool CheckResults( SimulationStatus::Status status, bool block );

	virtual bool FetchResults( SimulationStatus::Status status, bool block, U32 *errorState );

	virtual void FlushCaches () { m_pScene->flushCaches(); }

//	virtual const NxProfileData * ReadProfileData (bool clearData);

	/// Poll for work to execute on the current thread. 
//	virtual NxThreadPollResult PollForWork (NxThreadWait waitType);

	/// Reset parallel simulation. 
//	virtual void  resetPollForWork ();

	/// Polls for background work. 
//	virtual NxThreadPollResult  pollForBackgroundWork (NxThreadWait waitType);

	/// Release  threads which are blocking to allow the SDK to be destroyed safely. 
//	virtual void  shutdownWorkerThreads ();

	/// Blocks all parallel raycast/overlap queries. 
//	virtual void  lockQueries ();

	/// Unlock parallel raycast/overlap queries. 
//	virtual void  unlockQueries ();

	/// Create a batched query object. 
//	virtual NxSceneQuery * CreateSceneQuery (const NxSceneQueryDesc &desc);

	/// Release  a scene query object. 
//	virtual bool  releaseSceneQuery (NxSceneQuery &query);

	/// Creates an actor in this scene. 
	virtual CActor * CreateActor(const CActorDesc &desc);

	/// Deletes the specified actor. 
//	virtual void ReleaseActor (CActor &actor);
	virtual void ReleaseActor( CActor*& pActor );

	/// Creates a joint. 
	virtual CJoint * CreateJoint(const CJointDesc &jointDesc);

	/// Deletes the specified joint. 
	virtual void ReleaseJoint( CJoint*& pJoint );

	/// Creates an effector. 
//	virtual NxEffector * CreateEffector (const NxEffectorDesc &desc);

	/// Deletes the effector passed. 
//	virtual void ReleaseEffector (NxEffector &effector);

	/// Creates a force field. 
//	virtual NxForceField * CreateForceField (const NxForceFieldDesc &forceFieldDesc);

	/// Deletes the force field passed. 
//	virtual void ReleaseForceField (NxForceField &forceField);

	/// Gets the number of force fields in the scene. 
//	virtual U32 GetNbForceFields () const;

	/// Gets the force fields in the scene. 
//	virtual NxForceField ** GetForceFields ();

	/// Creates a new Material. 
	virtual CMaterial * CreateMaterial (const CMaterialDesc &matDesc);

	/// Deletes the specified material. 
	virtual void ReleaseMaterial( CMaterial*& pMaterial );

	virtual CMaterial *GetMaterial( int material_id );
/*
	/// Creates a scene compartment. 
	virtual NxCompartment * CreateCompartment (const NxCompartmentDesc &compDesc);

	/// Returns the number of compartments created in the scene. 
	virtual U32 GetNbCompartments () const;

	/// Writes the scene's array of NxCompartment pointers to a user buffer.  
	virtual U32 GetCompartmentArray (NxCompartment **userBuffer, U32 bufferSize, U32 &usersIterator) const;
*/
	void SetGroupCollisionFlag( U16 group1, U16 group2, bool enable );

	bool GetGroupCollisionFlag( U16 group1, U16 group2 ) const;

	void SetActorGroupPairFlags( U16 actor_group1, U16 actor_group2, U32 flags );

	U32 GetActorGroupPairFlags( U16 actor_group1, U16 actor_group2 ) const;

	void SetUserTriggerReport( CUserTriggerReport *pCallback );

	void SetUserContactReport( CUserContactReport *pCallback );

	virtual CShape *RaycastClosestShape( const CRay& world_ray, CRaycastHit& hit, int coll_gorup, Scalar max_dist );

	// Creates a sweep cache, for use with NxActor::linearSweep(). See the Guide, "Sweep API" section for more information. 
//	virtual CSweepCache *CreateSweepCache () = 0;

	// Deletes a sweep cache. See the Guide, "Sweep API" section, for more information on sweep caches. Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls). 
//	virtual void  ReleaseSweepCache (CSweepCache *cache) = 0;

	// Performs a linear sweep through space with an oriented box. 
	U32 LinearOBBSweep( const OBB3 &world_box,             const Vector3 &motion, U32 flags, void *pUserData, U32 num_max_shapes, CSweepQueryHit &shapes, CUserEntityReport< CSweepQueryHit > *pCallback, U32 active_groups, const CGroupsMask *pGroupsMask );

	// Performs a linear sweep through space with an oriented capsule. 
	U32 LinearCapsuleSweep( const Capsule &world_capsule, const Vector3 &motion, U32 flags, void *pUserData, U32 num_max_shapes, CSweepQueryHit &shapes, CUserEntityReport< CSweepQueryHit > *pCallback, U32 active_groups, const CGroupsMask *pGroupsMask );


	CCloth *CreateCloth( CClothDesc& desc );

	void ReleaseCloth( CCloth*& pCloth );
};


} // namespace physics

} // namespace amorphous



#endif		/*  __NxPhysicsScene_H__  */
