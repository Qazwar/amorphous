#ifndef  __COPYENTITYDESCFILEARCHIVE_H__
#define  __COPYENTITYDESCFILEARCHIVE_H__


#include "amorphous/Support/Serialization/Serialization.hpp"
#include "amorphous/Support/Serialization/Serialization_3DMath.hpp"
#include "CopyEntityDesc.hpp"


namespace amorphous
{

using namespace serialization;


//===============================================================================================
// CCopyEntityDescFileData
//===============================================================================================

class CCopyEntityDescFileData : public IArchiveObjectBase
{
public:

	std::string strBaseEntityName;
	CCopyEntityDesc CopyEntityDesc;

	std::vector<CCopyEntityDescFileData> vecChild;

	void Serialize( IArchive& ar, const unsigned int version );

};



//===============================================================================================
// CCopyEntityDescFileArchive
//===============================================================================================

class CCopyEntityDescFileArchive : public IArchiveObjectBase
{
	std::vector<CCopyEntityDescFileData> m_vecCopyEntityDesc;


public:

//	CCopyEntityDescFileArchive();
//	~CCopyEntityDescFileArchive();

	size_t GetNumEntityDescs() const { return m_vecCopyEntityDesc.size(); }
	CCopyEntityDescFileData& GetCopyEntityDesc( int index ) { return m_vecCopyEntityDesc[index]; }

	void AddCopyEntityDescData( CCopyEntityDescFileData& rDescData ) { m_vecCopyEntityDesc.push_back( rDescData ); }

	void Serialize( IArchive& ar, const unsigned int version );
};


} // namespace amorphous



#endif		/*  __COPYENTITYDESCFILEARCHIVE_H__  */
