#ifndef __BE_EXPLOSIVE_H__
#define __BE_EXPLOSIVE_H__

#include "BE_PhysicsBaseEntity.hpp"

#include "BaseEntityHandle.hpp"
#include "amorphous/Sound/SoundHandle.hpp"


namespace amorphous
{


class CBE_Explosive : public CBE_PhysicsBaseEntity
{
protected:

	enum eTypeFlag
	{
		TYPE_BLAST			= (1 << 0),
		TYPE_FRAGMENTS		= (1 << 1),
		TYPE_TIMER			= (1 << 2),
//#define CBE_EXPLOSIVE_MISSILE		= (1 << 3)
	};

	enum eAnimType
	{
		AT_EXPLOSION = 0,
		AT_SMOKE,
		AT_SPARK,
		AT_SHOCKWAVE,
		NUM_EXPLOSION_ANIMATIONS
	};

	/// flag which determines the type of an explosive
	int m_ExplosiveTypeFlag;

	/// simple blast that gives uniform damage to entites around the explosive
	BaseEntityHandle m_Blast;

	/// shrapnel generated at the explosion (optional)
	BaseEntityHandle m_Fragment;

	/// number of fragments generated at the time of explosion
	int m_iNumFragments;

	/// animations of explosion
	BaseEntityHandle m_aExplosionAnimation[NUM_EXPLOSION_ANIMATIONS];

//	BaseEntityHandle m_Spark;
//	BaseEntityHandle m_ExplosionAnimation;
//	BaseEntityHandle m_SmokeAnimation;

	float m_fExplosionAnimTimeOffset;
	float m_fExplosionAnimRange;
	BaseEntityHandle m_ExplosionLight;
	float m_fTimer;

	float m_fMinimumExplodeImpactSpeed;

	SoundHandle m_ExplosionSound;

public:
	CBE_Explosive();
	virtual ~CBE_Explosive() {}

	void Init();
	void InitCopyEntity( CCopyEntity* pCopyEnt );

	void Act(CCopyEntity* pCopyEnt);
	void Draw(CCopyEntity* pCopyEnt);
	void MessageProcedure(GameMessage& rGameMessage, CCopyEntity* pCopyEnt_Self);
	void Touch(CCopyEntity* pCopyEnt_Self, CCopyEntity* pCopyEnt_Other);

	bool LoadSpecificPropertiesFromFile( CTextFileScanner& scanner );

	virtual unsigned int GetArchiveObjectID() const { return BE_EXPLOSIVE; }

	virtual void Serialize( IArchive& ar, const unsigned int version );

	void GrenadeMove(CCopyEntity* pCopyEnt);
	void Explode(CCopyEntity* pCopyEnt);
	void FlashScreen(CCopyEntity* pCopyEnt);

	friend class CoreBaseEntitiesLoader;
};
} // namespace amorphous



#endif	/*  __BE_EXPLOSIVE_H__  */
