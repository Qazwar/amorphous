#include "ItemDatabaseBuilder.hpp"

#include "amorphous/Item/GameItemObjectFactory.hpp"
#include "amorphous/Item/GameItem.hpp"
#include "amorphous/Item/MiscGameItems.hpp"
#include "amorphous/Item/GI_Weapon.hpp"
#include "amorphous/Item/GI_Ammunition.hpp"
#include "amorphous/Item/GI_GravityGun.hpp"
//#include "amorphous/Item/GI_NightVision.h"
#include "amorphous/Item/GI_MissileLauncher.hpp"
#include "amorphous/Item/GI_Aircraft.hpp"
#include "amorphous/Item/GameItemDatabase.hpp"

#include "amorphousXML/XMLDocumentBase.hpp"
#include "amorphousXML/XMLNode.hpp"

#include "amorphousGameCommon/MeshBoneController_Aircraft.hpp"
#include "amorphousSupport/TextFileScanner.hpp"
#include "amorphousSupport/Log/DefaultLog.hpp"
#include "amorphousSupport/SafeDeleteVector.hpp"
#include "amorphousSupport/lfs.hpp"
#include "amorphousSupport/Serialization/BinaryDatabase.hpp"
#include <boost/filesystem.hpp>


namespace amorphous
{

using namespace std;


ItemDatabaseBuilder::ItemDatabaseBuilder()
{
}


ItemDatabaseBuilder::~ItemDatabaseBuilder()
{
	SafeDeleteVector( m_vecpItem );
}


void ItemDatabaseBuilder::LoadGameItemSharedProperty( CTextFileScanner& scanner, GameItem* pItem )
{
	LOG_PRINT_ERROR( " Removed." );
/*
	scanner.TryScanLine( "name",			pItem->m_strName );
	scanner.TryScanLine( "max_quantity",	pItem->m_iMaxQuantity );
	scanner.TryScanLine( "price",			pItem->m_Price );

//	scanner.TryScanLine( "desc_jp",			pItem->m_Desc.text[Lang::Japanese] );
//	scanner.TryScanLine( "desc_en",			pItem->m_Desc.text[Lang::English] );
	scanner.TryScanSentence( "desc_jp",			pItem->m_Desc.text[Lang::Japanese] );
	scanner.TryScanSentence( "desc_en",			pItem->m_Desc.text[Lang::English] );

//	scanner.TryScanLine( "model_file",		pItem->m_MeshObjectContainer.m_MeshDesc.ResourcePath );

//	string type_str;
//	if( scanner.TryScanLine( "mesh_type",	type_str ) )
//	{
//		MeshType::Name mesh_type = MeshType::BASIC;
//		if( type_str == "skeletal" )         mesh_type = MeshType::SKELETAL;
//		else if( type_str == "progressive" ) mesh_type = MeshType::PROGRESSIVE;
//		else if( type_str == "normal" )      mesh_type = MeshType::BASIC;
//		pItem->m_MeshObjectContainer.m_MeshDesc.MeshType = mesh_type;
//	}
*/
}


void ItemDatabaseBuilder::LoadFirearms( CTextFileScanner& scanner, CGI_Weapon* pWeapon )
{
	LoadGameItemSharedProperty( scanner, pWeapon );

	string str;

//	scanner.TryScanLine( "grouping",			pWeapon->m_fGrouping );
	scanner.TryScanLine( "fire_rate",			pWeapon->m_fFireInterval );
	scanner.TryScanLine( "muzzle_speed_factor",	pWeapon->m_fMuzzleSpeedFactor );
	scanner.TryScanLine( "num_bursts",			pWeapon->m_iNumBursts );
//	scanner.TryScanLine( "burst_interval",		pWeapon->m_fBurstInterval );
	scanner.TryScanLine( "local_recoil_force",	pWeapon->m_vLocalRecoilForce );
	scanner.TryScanLine( "muzzle_end_pos",		pWeapon->m_MuzzleEndLocalPose.vPosition );
	scanner.TryScanLine("ammo_type",			pWeapon->m_strAmmoType );
	scanner.TryScanLine("caliber",				pWeapon->m_strAmmoType );

	if( scanner.TryScanLine( "fire_sound", str ) )
	{
		pWeapon->m_FireSound.SetResourceName( str.c_str() );
	}
}


void ItemDatabaseBuilder::LoadGravityGun( CTextFileScanner& scanner, GravityGun* pGravityGun )
{
	LoadGameItemSharedProperty( scanner, pGravityGun );

	scanner.TryScanLine( "grasp_range",	pGravityGun->m_fGraspRange );
//	scanner.TryScanLine( "power",		pGravityGun->m_fPower );
}


void ItemDatabaseBuilder::LoadAmmunition( CTextFileScanner& scanner, CGI_Ammunition* pAmmo )
{
	LoadGameItemSharedProperty( scanner, pAmmo );

	string str;

	scanner.TryScanLine( "muzzle_speed",	pAmmo->m_fMuzzleSpeed );
	scanner.TryScanLine( "power",			pAmmo->m_fPower );
	scanner.TryScanLine( "ammo_type",		pAmmo->m_strAmmoType );
	scanner.TryScanLine( "caliber",			pAmmo->m_strAmmoType );
	if( scanner.TryScanLine( "ammo_entity", str ) )
	{
		pAmmo->m_AmmoBaseEntity.SetBaseEntityName( str.c_str() );
	}
	if( scanner.TryScanLine( "muzzle_flash_entity", str ) )
	{
		pAmmo->m_MuzzleFlashBaseEntity.SetBaseEntityName( str.c_str() );
	}

	// range - used by missile item
	scanner.TryScanLine( "range",	pAmmo->m_fRange );
}


void ItemDatabaseBuilder::LoadBinocular( CTextFileScanner& scanner, Binocular* pBinocular )
{
	LoadGameItemSharedProperty( scanner, pBinocular );

	scanner.TryScanLine( "max_zoom",		pBinocular->m_fMaxZoom );
}


void ItemDatabaseBuilder::LoadNightVision( CTextFileScanner& scanner, CGI_NightVision* pNV )
{
	LoadGameItemSharedProperty( scanner, pNV );

	string tag;
	scanner.GetTagString( tag );

	if( scanner.TryScanLine( "battery_life", pNV->m_fMaxBatteryLife ) )
	{
		pNV->m_fBatteryLeft = pNV->m_fMaxBatteryLife;	// battery is charged full by default
	}
	scanner.TryScanLine( "charge_speed",		pNV->m_fChargeSpeed );
}


void ItemDatabaseBuilder::LoadMissileLauncher( CTextFileScanner& scanner, MissileLauncher* pItem )
{
	LoadFirearms( scanner, pItem );
//	LoadGameItemSharedProperty( scanner, pItem );

	string tag;
	scanner.GetTagString( tag );
	int num_release_positions;

	Camera m_SensorCamera;

	scanner.TryScanLine( "sensor_angle",	pItem->m_fValidSensorAngle );

	scanner.TryScanLine( "sensor_range",	pItem->m_fMaxSensorRange );

	/// the number of targets this launcher can simultaneously lock on
	scanner.TryScanLine( "max_simul_targets",		pItem->m_NumMaxSimulTargets );

	if( scanner.TryScanLine( "num_release_positions",	num_release_positions ) )
	{
		pItem->SetNumReleasePositions( num_release_positions );
	}
}

void ItemDatabaseBuilder::AddMeshBoneControllerForAircraft( CGI_Aircraft& aircraft,
															 CTextFileScanner& scanner,
						                                     vector< shared_ptr<MeshBoneController_AircraftBase> >& vecpMeshController )
{
	string tag, type;
	char _tag[32], _type[32];
	float angle_per_accel, angle_per_pitchaccel, angle_per_rollaccel;
	int dir;
	char dir_name[32];
//	string flapR_name, flapL_name, vflap0_name, vflap1_name, rotor_name;
	char flapR_name[64], flapL_name[64], vflap0_name[64], vflap1_name[64], rotor_name[64];
//	const char* pCurrentLine = scanner.GetCurrentLine().c_str();
	string current_line = scanner.GetCurrentLine();

	scanner.ScanLine( tag, type );
	if( type == "flap" )
	{
//		scanner.ScanLine( tag, type, angle_per_pitchaccel, angle_per_rollaccel, flapR_name, flapL_name );
		sscanf( current_line.c_str(), "%s %s %f %f %s %s",
			_tag, _type, &angle_per_pitchaccel, &angle_per_rollaccel, flapR_name, flapL_name );
		MeshBoneController_Flap* pFlapControl = new MeshBoneController_Flap();
		pFlapControl->m_fAnglePerPitchAccel = angle_per_pitchaccel;
		pFlapControl->m_fAnglePerRollAccel = angle_per_rollaccel;
		pFlapControl->m_vecBoneControlParam.resize( 2 );
		pFlapControl->m_vecBoneControlParam[0].Name = flapR_name;
		pFlapControl->m_vecBoneControlParam[1].Name = flapL_name;
		aircraft.m_vecpMeshController.push_back( shared_ptr<MeshBoneController_AircraftBase>(pFlapControl) );
	}
	else if( type == "tvflap" )
	{
//		scanner.ScanLine( tag, type, vflap_type, vflap0_name, vflap1_name );
		sscanf( current_line.c_str(), "%s %s %f %s %s",
			_tag, _type, &angle_per_accel, vflap0_name, vflap1_name );
		MeshBoneController_VFlap* pFlapControl = new MeshBoneController_VFlap();
		pFlapControl->m_fAnglePerYawAccel = angle_per_accel;
		pFlapControl->m_vecBoneControlParam.resize( 2 );
		pFlapControl->m_vecBoneControlParam[0].Name = vflap0_name;
		pFlapControl->m_vecBoneControlParam[1].Name = vflap1_name;
		aircraft.m_vecpMeshController.push_back( shared_ptr<MeshBoneController_AircraftBase>(pFlapControl) );
	}
	else if( type == "svflap" )
	{
//		scanner.ScanLine( tag, type, angle_per_accel, vflap0_name );
		sscanf( current_line.c_str(), "%s %s %f %s", _tag, _type, &angle_per_accel, vflap0_name );
		MeshBoneController_VFlap* pFlapControl = new MeshBoneController_VFlap();
		pFlapControl->m_fAnglePerYawAccel = angle_per_accel;
		pFlapControl->m_vecBoneControlParam.resize( 1 );
		pFlapControl->m_vecBoneControlParam[0].Name = vflap0_name;
		aircraft.m_vecpMeshController.push_back( shared_ptr<MeshBoneController_AircraftBase>(pFlapControl) );
	}
	else if( type == "rotor" )
	{
//		scanner.ScanLine( tag, type, rotation_direction, rotor_name );
		sscanf( current_line.c_str(), "%s %s %s %s", _tag, _type, dir_name, rotor_name );
		MeshBoneController_Rotor* pRotorControl = new MeshBoneController_Rotor();
		if( dir_name == "ccw" )		dir = MeshBoneController_Rotor::DIR_CCW;
		else if( dir_name == "cw" )	dir = MeshBoneController_Rotor::DIR_CW;
		else dir = MeshBoneController_Rotor::DIR_CW;
		pRotorControl->m_RotationDirection = dir;
		pRotorControl->m_vecBoneControlParam.resize( 1 );
		pRotorControl->m_vecBoneControlParam[0].Name = rotor_name;
		aircraft.m_vecpMeshController.push_back( shared_ptr<MeshBoneController_AircraftBase>(pRotorControl) );
	}
}


void ItemDatabaseBuilder::LoadAircraft( CTextFileScanner& scanner, CGI_Aircraft* pItem )
{
	LoadGameItemSharedProperty( scanner, pItem );

	string tag;
	scanner.GetTagString( tag );

	int index;
	Vector3 pos;

	string ammo_name;
	int weapon_slot=0, max_quantity=0;

	scanner.TryScanLine( "default_accel",	pItem->m_fAccel );
	scanner.TryScanLine( "boost_accel",		pItem->m_fBoostAccel );
	scanner.TryScanLine( "brake_accel",		pItem->m_fBrakeAccel );

	scanner.TryScanLine( "max_pitch_accel",	pItem->m_fMaxPitchAccel );
	scanner.TryScanLine( "max_roll_accel",	pItem->m_fMaxRollAccel );
	scanner.TryScanLine( "max_yaw_accel",	pItem->m_fMaxYawAccel );

	scanner.TryScanLine( "gun_muzzle_end_pos",pItem->m_vGunMuzzleEndLocalPos );

	scanner.TryScanLine( "cockpit_pos",	pItem->m_CockpitLocalPose.vPosition );
	scanner.TryScanLine( "3rd_person_view",	pItem->m_vThirdPersonViewOffset );
	scanner.TryScanLine( "ceiling",		pItem->m_fCeiling );
	scanner.TryScanLine( "armor",		pItem->m_fArmor );
	scanner.TryScanLine( "rcs",			pItem->m_fRCS );
/*	scanner.TryScanLine( "gear_height",	pItem->m_fGearUnitHeight );

	if( scanner.TryScanLine( "nozzle_pos", index, pos ) )
	{
		for( int i=pItem->m_vecNozzleExhaustPose.size(); i<=index; i++ )
			pItem->m_vecNozzleExhaustPose.push_back( Matrix34Identity() );

		pItem->m_vecNozzleExhaustPose[index].vPosition = pos;
	}
*/
	if( scanner.TryScanLine( "ammo_release_pos", index, pos ) )
	{
		if( index < (int)pItem->m_AmmoReleaseLocalPose.size() )
			pItem->m_AmmoReleaseLocalPose[index].vPosition = pos;
	}

	if( scanner.TryScanLine( "payload", ammo_name, weapon_slot, max_quantity ) )
	{
//		MsgBoxFmt( "setting payload info to %s: %s, %d, %d",
//			pItem->GetName().c_str(), ammo_name.c_str(), weapon_slot, max_quantity );
		pItem->m_vecSupportedAmmo.push_back( CGI_Aircraft::AmmoPayload( ammo_name, weapon_slot, max_quantity ) );
	}

	static NozzleFlameParams s_NozzleFlameParams = NozzleFlameParams();

	if( tag == "nozzle_flame_sld" )
	{
		// start position, length and diameter (round nozzle flame)
		// this is interpreted as a new nozzle flame param set
		// - stored to the static variable and used as shared properties for the following nozzle_flame_pos params
		s_NozzleFlameParams.LoadFromFile( scanner );
		s_NozzleFlameParams.LocalPose.matOrient = Matrix33RotationY( 3.141592f );	// direction is set to backwards by default
		return;
	}

	if( tag == "nozzle_flame_slwh" )
	{
		// start position, length, width and height (rectangular nozzle flame)
		// this is interpreted as a new nozzle flame param set
		// temporarily store it to the static variable
		// - stored to the static variable and used as shared properties for the following nozzle_flame_pos params
		s_NozzleFlameParams.LoadFromFile( scanner );
		s_NozzleFlameParams.LocalPose.matOrient = Matrix33RotationY( 3.141592f );	// direction is set to backwards by default
		return;
	}

	if( tag == "nozzle_flame_pos" )
	{
		// nozzle flame position - must be defined after (start,length,...) params and before any other params
		pItem->m_vecNozzleFlameParams.push_back( s_NozzleFlameParams );
		if( pItem->m_vecNozzleFlameParams.back().LoadFromFile( scanner ) )
			return;
	}

	if( tag == "mesh_ctrl" )
	{
		AddMeshBoneControllerForAircraft( *pItem, scanner, pItem->m_vecpMeshController );
	}

//	CAircraftRotor rotor = CAircraftRotor();
//	if( scanner.TryScanLine( "rotor", rotor.fRotationSpeed, rotor.fAngleOffset ) )
//		pItem->m_vecRotor.push_back( rotor );
}


bool ItemDatabaseBuilder::LoadItemsFromTextFile( const std::string& filepath )
{
	CTextFileScanner scanner;

	if( !scanner.OpenFile( filepath ) )
		return false;

	GameItem *pObject = NULL;

	GameItemObjectFactory factory;

	string tag, strClassName, strLine;
//	unsigned int id;
	int id = IArchiveObjectBase::INVALID_ID;

	for( ; !scanner.End(); scanner.NextLine() )
	{
		scanner.GetCurrentLine( strLine );

		if( !scanner.GetTagString( tag ) )
			break;

		if( strLine.find( "\\begin:" ) == 0 )
		{
			// a new item desc has started
			strClassName = strLine.substr( 7, 1024 );
			size_t len = strClassName.length();
			if( len == 0 )
				continue;

//			if( strClassName[len-1] == '\n' )
			while( strClassName[len-1] == '\n' || strClassName[len-1] == '\t' || strClassName[len-1] == ' ' )
			{
//				strClassName[len-1] = '\0';
				strClassName = strClassName.substr( 0, strClassName.length() - 1 );
				len--;
			}

			id = GetItemID( strClassName );

//			if( id < 0 )
			if( id == IArchiveObjectBase::INVALID_ID )
				continue;

			pObject = (GameItem *)factory.CreateObject( id );

			if( !pObject )
				continue;

//			result = LoadCharacter( scanner );
//			if( !result )
//				return false;
		}

		else if( strncmp(strLine.c_str(), "\\end:", 5) == 0 )
		{
			if( pObject )
			{
				m_vecpItem.push_back( pObject );
				pObject = NULL;
			}
			id = -1;
		}

		// load properties according to the type of item 
		switch( id )
		{
		case GameItem::ID_AMMUNITION:
			LoadAmmunition( scanner, (CGI_Ammunition *)pObject );
			break;
		case GameItem::ID_WEAPON:
			LoadFirearms( scanner, (CGI_Weapon *)pObject );
			break;
		case GameItem::ID_GRAVITY_GUN:
			LoadGravityGun( scanner, (GravityGun *)pObject );
			break;
		case GameItem::ID_BINOCULAR:
			LoadBinocular( scanner, (Binocular *)pObject );
			break;
		case GameItem::ID_NIGHT_VISION:
			LoadNightVision( scanner, (CGI_NightVision *)pObject );
			break;
		case GameItem::ID_MISSILELAUNCHER:
			LoadMissileLauncher( scanner, (MissileLauncher *)pObject );
			break;
		case GameItem::ID_AIRCRAFT:
			LoadAircraft( scanner, (CGI_Aircraft *)pObject );
			break;
		}
	}

//	OutputDatabaseFile( pcDestFilename );

	return true;
}


void ItemDatabaseBuilder::LoadItem( XMLNode& items_node )
{
	GameItemObjectFactory factory;
	string classname;

	items_node.GetChildElementTextContent( "ClassName", classname );

	int item_id = GetItemID( classname );

	GameItem *pObject = factory.CreateGameItem( item_id );

	if( !pObject )
		return;

	m_vecpItem.push_back( pObject );
	m_vecpItem.back()->LoadFromXMLNode( items_node );
}


void ItemDatabaseBuilder::LoadItems( XMLNode& items_node_reader )
{
	vector<XMLNode> item_nodes = items_node_reader.GetImmediateChildren( "Item" );

	for( size_t i=0; i<item_nodes.size(); i++ )
	{
		LoadItem( item_nodes[i] );
	}

	// recursively load the items from the child <Items> nodes
	vector<XMLNode> items_nodes = items_node_reader.GetImmediateChildren( "Items" );
	for( size_t i=0; i<items_nodes.size(); i++ )
		LoadItems( items_nodes[i] );
}


bool ItemDatabaseBuilder::LoadItemsFromXMLFile( const string& xml_file_pathname )
{
	using namespace boost::filesystem;

	path parent_path = path(m_RootXMLFilePathname).parent_path();

	shared_ptr<XMLDocumentBase> pDoc = CreateXMLDocument( xml_file_pathname );

	if( !pDoc )
		return false;

	// The root node is supposed to be named either "Root" or "Items".
	XMLNode root_node = pDoc->GetRootNode();

	vector<XMLNode> children = root_node.GetImmediateChildren();
	const size_t num_children = children.size();
	for( size_t i=0; i<num_children; i++ )
	{
		string node_name = children[i].GetName();

		if( node_name == "Include" )
		{
			string relative_path = children[i].GetAttributeText("relative_path");
			LoadItemsFromXMLFile( path(parent_path/relative_path).string() );
		}
		else if( node_name == "Item" )
		{
			LoadItem( children[i] );
		}
		else if( node_name == "Items" )
		{
			LoadItems( children[i] );
		}
	}

	return true;
}


bool ItemDatabaseBuilder::CreateItemDatabaseFileFromXMLFile( const std::string& filepath, const std::string& output_filepath )
{
	m_RootXMLFilePathname = filepath;

	bool res = LoadItemsFromXMLFile( filepath );
	if( !res )
	{
		LOG_PRINT_ERROR( "Failed to load game items data from the xml file: " + filepath );
		return false;
	}

	return OutputDatabaseFile( output_filepath );
}


int ItemDatabaseBuilder::GetItemID( const string& class_name )
{
	if( class_name == "Cartridge" )              return GameItem::ID_CARTRIDGE;
	else if( class_name == "Magazine" )          return GameItem::ID_MAGAZINE;
	else if( class_name == "Firearm" )           return GameItem::ID_FIREARM;
	else if( class_name == "Ammunition" )        return GameItem::ID_AMMUNITION;
	else if( class_name == "GravityGun" )        return GameItem::ID_GRAVITY_GUN;
	else if( class_name == "Binocular" )         return GameItem::ID_BINOCULAR;
	else if( class_name == "NightVision" )       return GameItem::ID_NIGHT_VISION;
	else if( class_name == "Key" )               return GameItem::ID_KEY;
	else if( class_name == "CamouflageDevice" )  return GameItem::ID_CAMFLOUGE_DEVICE;
	else if( class_name == "Suppressor" )        return GameItem::ID_SUPPRESSOR;
	else if( class_name == "Aircraft" )          return GameItem::ID_AIRCRAFT;
	else if( class_name == "MissileLauncher" )   return GameItem::ID_MISSILELAUNCHER;
	else if( class_name == "Radar" )             return GameItem::ID_RADAR;
	else if( class_name == "RotatableTurret" )   return GameItem::ID_ROTATABLE_TURRET;
	else if( class_name == "LandVehicle" )       return GameItem::ID_LAND_VEHICLE;
	else if( class_name == "ArmedVehicle" )      return GameItem::ID_ARMED_VEHICLE;
//	else if( class_name == "SkeletalCharacter" ) return GameItem::ID_SKELETAL_CHARACTER;
//	else if( class_name == "Clothing" )          return GameItem::ID_CLOTHING;

	else
	{
		LOG_PRINT_WARNING( "An unknown item name: " + class_name );
		return IArchiveObjectBase::INVALID_ID;
	}
}	


bool ItemDatabaseBuilder::OutputDatabaseFile( const std::string output_filename )
{
///	string strBodyFilename;
///	CFileNameOperation::GetBodyFilename( strBodyFilename, output_filename );

///	string strDatabaseFilename = m_strOutputPath + strBodyFilename;

//	return OutputSeparateFiles( strDatabaseFilename );

	return OutputSingleDBFile( output_filename );
}


bool ItemDatabaseBuilder::OutputSingleDBFile( const string& strDBFilename )
{
	CBinaryDatabase<string> m_ItemDB;

	GameItemObjectFactory factory;

	bool db_open = m_ItemDB.Open( strDBFilename.c_str(), CBinaryDatabase<string>::DB_MODE_NEW );
	if( !db_open )
	{
		LOG_PRINT_ERROR( "Cannot open file: " + strDBFilename );
		return false;
	}

	size_t i, num_items = m_vecpItem.size();
	for( i=0; i<num_items; i++ )
	{
		GameItem* pItem = m_vecpItem[i];

		m_ItemDB.AddPolymorphicData( pItem->GetName(), pItem, factory );
	}

	return true;
}


bool ItemDatabaseBuilder::CreateItemDatabaseFile( const string& src_filename )
{
	vector<string> input_filename_list;
	string input_filename;
	string output_filename;

	CTextFileScanner scanner( src_filename );
	if( scanner.IsReady() )
	{
		for( ; !scanner.End(); scanner.NextLine() )
		{
			if( scanner.TryScanLine( "input", input_filename ) )
			{
				input_filename_list.push_back( input_filename );
			}

			scanner.TryScanLine( "output", output_filename );
		}
	}
	else
		return false;

	if( input_filename_list.size() == 0 )
		return false;	// no input file - cannot create an item database

	// input&output filenames are given with relative paths
	// - set the path of src_filename as working directory
	lfs::dir_stack dirstk( lfs::get_parent_path(src_filename) );

	// load item parameters from text
	size_t i, num_input_files = input_filename_list.size();
	for( i=0; i<num_input_files; i++ )
	{
		if( input_filename_list[i].length() ==  0 )
			continue;

/*		if( !has_abspath(input_filename_list[i]) )
		{
			dir_stack dirstk( get_path(src_filename) );
			LoadItemsFromTextFile( input_filename_list[i] );
			dirstk.prevdir();	// back to the previous work directory
		}
		else*/
            LoadItemsFromTextFile( input_filename_list[i] );
	}

	// output a database file
	bool db_created = OutputDatabaseFile( output_filename );
	if( !db_created )
	{
	    dirstk.pop_and_chdir();	// back to the previous work directory
		return false;
	}

    dirstk.pop_and_chdir();	// back to the previous work directory

	return true;
}

/*
bool ItemDatabaseBuilder::OutputSeparateFiles( const string& strDBFilename )
{	
//	CBinaryArchive_Output archive( strDatabaseFilename.c_str() );

	GameItemDatabase database;

	string strFilename;
	char acItemBodyFilename[256];

	size_t i, num_items = m_vecpItem.size();
	for( i=0; i<num_items; i++ )
	{
		// make a filename for an item object
		sprintf( acItemBodyFilename, "A%03d.dat", i );
		strFilename = m_strItemFilePath + acItemBodyFilename;

		// store the pair of the filename & the corresponding item name to the item database
		database.AddItemInfo( strFilename, m_vecpItem[i]->GetName() );

		// make a filename for an item file
		strFilename = m_strOutputPath + acItemBodyFilename;

		// save the item object into a separate binary file
		database.SaveItem( strFilename.c_str(), m_vecpItem[i] );
//		GameItemSerializer temp_serializer;
//		CBinaryArchive_Output archive( strFilename.c_str() );
//		temp_serializer.pItem = m_vecpItem[i];
//		archive << temp_serializer;
	}

	// save only the filenames and item names to the item database
	return database.SaveToFile( strDBFilename.c_str() );
//	return archive << database;
}
*/


} // namespace amorphous
