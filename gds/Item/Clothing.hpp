#ifndef  __GameItem_Clothing_HPP__
#define  __GameItem_Clothing_HPP__

#include "GameItem.hpp"

class CClothSystem;


//======================================================================================
// CClothing
//======================================================================================


/**
 game item that represents firearm
*/
class CClothing : public CGameItem
{
protected:

	bool m_ApplyClothSimulation;

	std::string m_NameOfBoneToAttachTo;

	float m_fProjectionMatrixOffset;

	boost::weak_ptr<CClothSystem> m_pClothSystem;

public:

	CClothing();

	virtual ~CClothing() {}

	virtual void Update( float dt );

	virtual bool HandleInput( int input_code, int input_type, float fParam );

	virtual unsigned int GetArchiveObjectID() const { return ID_CLOTHING; }

	virtual void Serialize( IArchive& ar, const unsigned int version );

	virtual void LoadFromXMLNode( CXMLNodeReader& reader );

	friend class CItemDatabaseBuilder;
};



#endif  __GameItem_Clothing_HPP__