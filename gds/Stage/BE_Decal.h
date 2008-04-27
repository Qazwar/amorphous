
#ifndef	__BASEENTITYDECAL_H__
#define __BASEENTITYDECAL_H__

#include "BaseEntity.h"

#include "../3DCommon/FVF_TextureVertex.h"

#include <d3d9.h>
#include <d3dx9.h>


class CBE_Decal : public CBaseEntity
{
	enum param { NUM_MAX_DECALS = 128 };

//	static int ms_iNumTotalUsedDecals;
	int m_NumTotalUsedDecals;

	CTextureHandle m_DecalTexture;

	TEXTUREVERTEX m_avDecalRect[4];

	float m_fDecalRadius;

	/// number of segments along each axis on a texture
	/// used for generating random patterns
	int m_iNumSegments;

public:

	CBE_Decal();
	~CBE_Decal();

	void Init();
	void InitCopyEntity( CCopyEntity* pCopyEnt );
	void Act(CCopyEntity* pCopyEnt);
	void Draw(CCopyEntity* pCopyEnt);
	//void Touch(CCopyEntity* pCopyEnt_Self, CCopyEntity* pCopyEnt_Other);
	//void MessageProcedure(SGameMessage& rGameMessage, CCopyEntity* pCopyEnt_Self);

	void SweepRender();

	bool LoadSpecificPropertiesFromFile( CTextFileScanner& scanner );

	virtual unsigned int GetArchiveObjectID() const { return BE_DECAL; }

	virtual void Serialize( IArchive& ar, const unsigned int version );

	void ReleaseGraphicsResources();
	void LoadGraphicsResources( const CGraphicsParameters& rParam );
};

#endif /*  __BASEENTITYDECAL_H__  */
