
#include "Ageia/NxPhysicsEngine.h"
#include "PhysicsEngine.h"
#include "Support/Log/DefaultLog.h"
#include "Support/SafeDelete.h"

using namespace std;

using namespace physics;


CSingleton<CPhysicsEngine> CPhysicsEngine::m_obj;


bool CPhysicsEngine::Init( const std::string& physics_engine )
{
	if( physics_engine == "AgeiaPhysX" )
	{
		m_pEngine = new CNxPhysicsEngine();
//		PhysShapeDescImplFactoryHolder.Init(  );
	}
	else if( physics_engine == "JigLib" )
	{
		LOG_PRINT_ERROR( " - JigLib physics engine - not implemented." );
//		m_pEngine = new CJigLibPhysicsEngine();
//		PhysShapeDescImplFactoryHolder.Init(  );
		return false;
	}
	else
	{
		LOG_PRINT_ERROR( " - Invalid physics engine name: " + physics_engine );
		return false;
	}

	m_pEngine->Init();

	return true;
}


void CPhysicsEngine::Release()
{
	SafeDelete( m_pEngine );
}


//void CPhysicsEngine::SetDefault(){}
