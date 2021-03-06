#ifndef __BASEENTITYBLAST_H__
#define __BASEENTITYBLAST_H__

#include "BaseEntity.hpp"
#include "CopyEntity.hpp"
#include "amorphous/Graphics/fwd.hpp"


namespace amorphous
{


class CBE_Blast : public BaseEntity
{
	float m_fBaseDamage;
	float m_fMaxBlastRadius;	///< how large the blast expands. blast is a spherical volume
	float m_fBlastDuration;		///< how long the blast lasts. longer duration means that the blast expands more slowly

	/// The speed at which the blast radius expand up to m_fMaxBlastRadius
	float m_fLinearExpansionSpeed;
/*
	enum ExpansionType
	{
		ET_LINEAR,
		ET_EXPONENTIAL,     ///< expansion speed slows down exponentially?
		NUM_EXPANSION_TYPES
	};
*/
	/// impulse given to the entity hit by the blast
	/// Impulse is calculated every frame as m_vImpulse * frametime
	/// and applied to the entity if it overlaps with the blast volume
	float m_fImpulse;

//	std::shared_ptr<CUnitCube> m_pUnitCube;	// draw boundary for debug

public:
	CBE_Blast();
	~CBE_Blast();
	void Init();
	void InitCopyEntity( CCopyEntity* pCopyEnt );
	void Act(CCopyEntity* pCopyEnt);
	void Draw(CCopyEntity* pCopyEnt);
	//void Touch(CCopyEntity* pCopyEnt_Self, CCopyEntity* pCopyEnt_Other);
	//void MessageProcedure(GameMessage& rGameMessage, CCopyEntity* pCopyEnt_Self);

	bool LoadSpecificPropertiesFromFile( CTextFileScanner& scanner );

	virtual unsigned int GetArchiveObjectID() const { return BE_BLAST; }

	virtual void Serialize( IArchive& ar, const unsigned int version );

	static float& CurrentBlastTime( CCopyEntity *pEntity )   { return pEntity->f2; }
	static float& CurrentBlastRadius( CCopyEntity *pEntity ) { return pEntity->f3; }

	friend class CoreBaseEntitiesLoader;
};

} // namespace amorphous



#endif	/*  __BASEENTITYBLAST_H__  */
