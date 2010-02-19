#ifndef  __NxPhysicsEngine_H__
#define  __NxPhysicsEngine_H__


#include "../PhysicsEngine.hpp"
#include "../fwd.hpp"
#include "fwd.hpp"
#include "NxPhysOutputStream.hpp"

#include "NxPhysics.h"


#pragma comment( lib, "PhysXLoader.lib" )


namespace physics
{


/**
 CNxPhysicsSDK - class to hold novodex physics SDK
 
 NOTE: link "NxPhysics.lib" before compile
 */
class CNxPhysicsEngine : public CPhysicsEngineImpl
{
private:

	NxPhysicsSDK* m_pPhysicsSDK;

	CNxPhysOutputStream m_ErrorStream;

protected:

public:

	CNxPhysicsEngine();

	virtual ~CNxPhysicsEngine();

	virtual bool Init();

	virtual CScene *CreateScene( CSceneDesc& desc );

	virtual void ReleaseScene( CScene*& pScene );

	virtual CTriangleMesh *CreateTriangleMesh( CStream& phys_stream );

	virtual void ReleaseTriangleMesh( CTriangleMesh*& pTriangleMesh );

	virtual CTriangleMesh *CreateClothMesh( physics::CStream& phys_stream );

	virtual void ReleaseClothMesh( CTriangleMesh*& pClothMesh );

//	NxPhysicsSDK* GetPhysicsSDK() { return m_pPhysicsSDK; }
};


} // namespace physics


#endif		/*  __NxPhysicsEngine_H__  */
