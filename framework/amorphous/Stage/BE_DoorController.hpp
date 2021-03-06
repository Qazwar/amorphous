#ifndef	__BE_DOORCONTROLLER_H__
#define __BE_DOORCONTROLLER_H__


#include "BaseEntity.hpp"
#include "BaseEntityHandle.hpp"
#include "Serialization_BaseEntityHandle.hpp"

#include "amorphous/Support/FixedVector.hpp"
#include "amorphous/3DMath/Matrix34.hpp"

#include "amorphous/Sound/SoundHandle.hpp"


namespace amorphous
{


#define NUM_MAX_DOOR_COMPONENTS		8

class CBE_DoorController : public BaseEntity
{
	struct SDoorComponentSet : public IArchiveObjectBase
	{
		BaseEntityHandle entity;
		Matrix34 matLocalPose;	// offset position in the local space of door controller

		SDoorComponentSet();
        
		virtual void Serialize( IArchive& ar, const unsigned int version ) { ar & entity & matLocalPose; }
	};

	TCFixedVector<SDoorComponentSet, NUM_MAX_DOOR_COMPONENTS> m_vecDoorComponent;

	SoundHandle m_OpenSound;
	SoundHandle m_CloseSound;

	/// passward string to unlock the door
	std::string m_strKeyCode;

public:

	CBE_DoorController();
	~CBE_DoorController();
	void Init();
	void InitCopyEntity( CCopyEntity* pCopyEnt );
	void Act(CCopyEntity* pCopyEnt);
//	void Touch(CCopyEntity* pCopyEnt_Self, CCopyEntity* pCopyEnt_Other);
	void MessageProcedure(GameMessage& rGameMessage, CCopyEntity* pCopyEnt_Self);
//	void Draw(CCopyEntity* pCopyEnt);

	bool LoadSpecificPropertiesFromFile( CTextFileScanner& scanner );

	virtual unsigned int GetArchiveObjectID() const { return BE_DOORCONTROLLER; }

	virtual void Serialize( IArchive& ar, const unsigned int version );
};

} // namespace amorphous



#endif /*  __BE_DOORCONTROLLER_H__  */
