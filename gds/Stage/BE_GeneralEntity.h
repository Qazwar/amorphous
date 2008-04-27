#ifndef __BE_GENERALENTITY_H__
#define __BE_GENERALENTITY_H__

#include "BaseEntity.h"
#include "BaseEntityHandle.h"
#include "Serialization_BaseEntityHandle.h"
#include "BE_PhysicsBaseEntity.h"


#define GETYPE_LIFETIMER	1


class CBE_GeneralEntity : public CBE_PhysicsBaseEntity
{
private:

	short m_sGEAttribute;
	float m_fLifeTime;

//	CBaseEntityHandle m_SmokeTrace;
	CBaseEntityHandle m_Explosion;
	CBaseEntityHandle m_Child;

	class SmokeTrail : public IArchiveObjectBase
	{
	public:
		CBaseEntityHandle BaseEntity;
		Vector3 vLocalPosition;

		virtual void Serialize( IArchive& ar, const unsigned int version )
		{
			ar & BaseEntity & vLocalPosition;
		}
	};

	std::vector<SmokeTrail> m_vecSmokeTrail;

public:

	CBE_GeneralEntity();

	virtual ~CBE_GeneralEntity();

	void Init();
	void InitCopyEntity( CCopyEntity* pCopyEnt );

	void Act(CCopyEntity* pCopyEnt);	//behavior in in one frame
	void Draw(CCopyEntity* pCopyEnt);
	// void Touch(CCopyEntity* pCopyEnt_Self, CCopyEntity* pCopyEnt_Other) {}
	// void ClipTrace( STrace& rLocalTrace, CCopyEntity* pMyself );
	void MessageProcedure(SGameMessage& rGameMessage, CCopyEntity* pCopyEnt_Self);

//	void ReleaseGraphicsResources();
//	void LoadGraphicsResources( const CGraphicsParameters& rParam );

	bool LoadSpecificPropertiesFromFile( CTextFileScanner& scanner );

	virtual unsigned int GetArchiveObjectID() const { return BE_GENERALENTITY; }

	virtual void Serialize( IArchive& ar, const unsigned int version );

	void SetSmokeTrace( CCopyEntity* pCopyEnt );
};

#endif  /*  __BE_GENERALENTITY_H__  */
