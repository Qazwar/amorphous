#ifndef	__BE_Turret_H__
#define __BE_Turret_H__

#include "BE_Enemy.hpp"


namespace amorphous
{


class CBE_Turret : public CBE_Enemy
{

public:
	CBE_Turret();
	// ~CBE_Turret();

	void Init();
	void InitCopyEntity( CCopyEntity* pCopyEnt );
	// void Act(CCopyEntity* pCopyEnt);
	// void Draw(CCopyEntity* pCopyEnt);
	// void SearchPlayer(CCopyEntity* pCopyEnt);
	// void Fire(CCopyEntity* pCopyEnt);
	// void Touch(CCopyEntity* pCopyEnt_Self, CCopyEntity* pCopyEnt_Other);
	// void MessageProcedure(GameMessage& rGameMessage, CCopyEntity* pCopyEnt_Self);

	bool LoadSpecificPropertiesFromFile( CTextFileScanner& scanner );

	virtual unsigned int GetArchiveObjectID() const { return BE_TURRET; }

	virtual void Serialize( IArchive& ar, const unsigned int version );
};

} // namespace amorphous



#endif /*  __BE_Turret_H__  */
