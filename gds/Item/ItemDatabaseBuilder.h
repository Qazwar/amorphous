#ifndef  __ITEMDATABASEBUILDER_H__
#define  __ITEMDATABASEBUILDER_H__


#include <vector>
#include <string>

class CGameItem;
class CTextFileScanner;
class CGI_Weapon;
class CGI_GravityGun;
class CGI_Ammunition;
class CGI_Binocular;
class CGI_NightVision;
class CGI_MissileLauncher;
class CGI_Aircraft;

class CMeshBoneController_AircraftBase;


class CItemDatabaseBuilder
{
	std::vector<CGameItem *> m_vecpItem;

	/// stores the path for the item list file and item files
//	std::string m_strOutputPath;

	/// path added to the head of the each item file
//	std::string m_strItemFilePath;

	void LoadGameItemSharedProperty( CTextFileScanner& scanner, CGameItem* pItem );
	void LoadFirearms( CTextFileScanner& scanner,	CGI_Weapon* pWeapon );
	void LoadGravityGun( CTextFileScanner& scanner, CGI_GravityGun* pGravityGun );
	void LoadAmmunition( CTextFileScanner& scanner, CGI_Ammunition* pAmmo );
	void LoadBinocular( CTextFileScanner& scanner,	CGI_Binocular* pItem );
	void LoadNightVision( CTextFileScanner& scanner,CGI_NightVision* pItem );
	void LoadMissileLauncher( CTextFileScanner& scanner, CGI_MissileLauncher* pItem );
	void LoadAircraft( CTextFileScanner& scanner,	CGI_Aircraft* pItem );
//	void LoadKey( CTextFileScanner& scanner, CGI_Key* pItem );

	bool OutputDatabaseFile( const std::string output_filename );

	/// old db scheme that puts item as separate files
//	bool OutputSeparateFiles( const std::string& strDBFilename );

	bool OutputSingleDBFile( const std::string& strDBFilename );

    void AddMeshBoneControllerForAircraft( CGI_Aircraft& aircraft, 
		                                   CTextFileScanner& scanner,
		                                   std::vector<CMeshBoneController_AircraftBase *>& vecpMeshController );

public:
	CItemDatabaseBuilder();
	~CItemDatabaseBuilder();

//	void SetOutputPath( const char *pcPath ) { m_strOutputPath = pcPath; }
//	void SetItemFilePath( const char *pcPath ) { m_strItemFilePath = pcPath; }

	bool LoadItemsFromTextFile( const std::string filename );

	bool CreateItemDatabaseFile( const std::string& src_filename );

//	unsigned int GetItemID( string& strClassName );
	int GetItemID( const std::string& class_name );

};



#endif		/*  __ITEMDATABASEBUILDER_H__  */
