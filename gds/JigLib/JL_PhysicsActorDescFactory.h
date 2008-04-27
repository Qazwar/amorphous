
#ifndef __CJL_PhysicsActorDescFACTORY_H__
#define __CJL_PhysicsActorDescFACTORY_H__


#include "Support/Serialization/Serialization.h"
#include "Support/Serialization/ArchiveObjectFactory.h"
using namespace GameLib1::Serialization;


//===========================================================================
// CJL_PhysicsActorDescFactory
//===========================================================================

class CJL_PhysicsActorDescFactory : public IArchiveObjectFactory
{
public:
	IArchiveObjectBase *CreateObject( const unsigned int id );
};


#endif  /* __CJL_PhysicsActorDescFACTORY_H__   */