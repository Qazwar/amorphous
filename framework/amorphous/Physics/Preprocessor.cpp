#include "Engines/PhysX/NxPhysPreprocessorImpl.hpp"
#include "PhysicsEngine.hpp"
#include "Stream.hpp"
#include "amorphous/Support/Log/DefaultLog.hpp"
#include "amorphous/Support/SafeDelete.hpp"


namespace amorphous
{

using namespace std;
using namespace physics;


singleton<CPreprocessor> CPreprocessor::m_obj;


CPreprocessor::CPreprocessor()
:
m_pImpl(NULL)
{
	Init();
}


bool CPreprocessor::Init()
{
	LOG_FUNCTION_SCOPE();

	SafeDelete( m_pImpl );

	const std::string& physics_engine = PhysicsEngine().GetName();

	if( physics_engine == "AgeiaPhysX" )
	{
		LOG_SCOPE( "Initializing the preprocessor of AgeiaPhysX" );
		m_pImpl = new CNxPhysPreprocessorImpl();
	}
/*	else if( physics_engine == "JigLib" )
	{
		m_pImpl = new CJigLibPhysicsEngine();
	}*/
	else
	{
		LOG_PRINT_ERROR( "invalid physics engine name: " + physics_engine );
		return false;
	}

	bool initialized = m_pImpl->Init();
	if( !initialized )
		LOG_PRINT_ERROR( "Failed to initialize the preprocessor of the physics engine: " + physics_engine );

	return true;
}


void CPreprocessor::Release()
{
	SafeDelete( m_pImpl );
}


} // namespace amorphous
