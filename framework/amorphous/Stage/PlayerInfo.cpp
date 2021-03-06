#include "PlayerInfo.hpp"

#include "amorphous/Input/InputHub.hpp"
#include "amorphous/Item/GameItem.hpp"
#include "amorphous/Item/GI_Ammunition.hpp"
//#include "amorphous/Item/GI_Aircraft.hpp"
#include "amorphous/Item/ItemDatabaseManager.hpp"
#include "amorphous/Item/GameItemObjectFactory.hpp"
#include "amorphous/Item/WeaponSystem.hpp"
#include "amorphous/Support/Serialization/Serialization_BoostSmartPtr.hpp"
#include "BE_Player.hpp"


namespace amorphous
{

using namespace std;


// definition of the singleton instance
singleton<CSinglePlayerInfo> CSinglePlayerInfo::m_obj;


CSinglePlayerInfo::CSinglePlayerInfo()
{
	m_pCurrentPlayerBaseEntity = NULL;

	m_TaskID = GameTask::ID_INVALID;

	m_pWeaponSystem = new CWeaponSystem;
	m_pWeaponSystem->SetProjectileGroup( ENTITY_GROUP_MIN_ID );// CE_GROUP_PLAYER_PROJECTILE );

	m_vecpActiveItem.reserve( 8 );
	m_pFocusedItem = NULL;

	m_Money = 1000000;
}


CSinglePlayerInfo::~CSinglePlayerInfo()
{
	Release();
}


void CSinglePlayerInfo::Release()
{
	SafeDelete( m_pWeaponSystem );

	m_vecpItem.resize( 0 );

	m_pCurrentAircraft.reset();
}


void CSinglePlayerInfo::SetStage( CStageWeakPtr pStage )
{
	m_pCurrentStage = pStage;

	const size_t num_items = m_vecpItem.size();
	for( size_t i=0; i<num_items; i++ )
	{
		m_vecpItem[i]->SetStageWeakPtr( pStage );
	}
}


CCopyEntity *CSinglePlayerInfo::GetCurrentPlayerEntity()
{
	if( GetCurrentPlayerBaseEntity() )
        return GetCurrentPlayerBaseEntity()->GetPlayerCopyEntity();
	else
		return NULL;
}

/*
void CSinglePlayerInfo::SetInputHandlerForPlayerShip()
{

	if( !GetCurrentPlayerBaseEntity() )
	{
		assert( !" a player base entity has not been set to PlayerInfo" );
		return;
	}

	SafeDelete( m_pInputHandler );

	switch( GetCurrentPlayerBaseEntity()->GetArchiveObjectID() )
	{
	case BaseEntity::BE_PLAYERSHIP:
	case BaseEntity::BE_PLAYERPSEUDOLEGGEDVEHICLE:
		m_pInputHandler = new InputHandler_PlayerShip;
//		m_pHUD = new HUD_PlayerShip;
		break;
	case BaseEntity::BE_PLAYERPSEUDOAIRCRAFT:
		m_pInputHandler = new InputHandler_PlayerPAC;
		break;
	}

}
*/

void CSinglePlayerInfo::AddItemToCategoryList( shared_ptr<GameItem> pItem )
{
	int category;
	switch( pItem->GetTypeFlag() )
	{
	case GameItem::TYPE_WEAPON:
		category = CItemCategory::WEAPON;
		break;
	case GameItem::TYPE_AMMO:
		{
			shared_ptr<CGI_Ammunition> pAmmo = dynamic_pointer_cast<CGI_Ammunition,GameItem>(pItem);
			if( !pAmmo )
				return;

			const string& ammo_type = pAmmo->GetAmmoType();
			if( ammo_type == "7.62x51" )    category = CItemCategory::AMMO_762X51;
			else if( ammo_type == "12GS" )  category = CItemCategory::AMMO_12GAUGE;
			else if( ammo_type == "40MMG" ) category = CItemCategory::AMMO_40MMGRENADE;
			else if( ammo_type == "MSSL" )  category = CItemCategory::AMMO_MISSILE;
			else if( ammo_type == "RBODY" ) category = CItemCategory::AMMO_RIGIDBODY;
//			else if( ammo_type == "50CAL" ) category = CItemCategory::AMMO_50CAL;
			else
				category = -1;
			break;
		}

	case GameItem::TYPE_SUPPLY:
		category = CItemCategory::SUPPLY;
		break;
	case GameItem::TYPE_UTILITY:
		category = CItemCategory::UTILITY;
		break;
	case GameItem::TYPE_KEY:
		category = CItemCategory::KEY;
		break;
	default:
		category = -1;
		break;
	}

	if( 0 <= category )
		m_vecpCategoryItem[category].push_back( pItem );
}


int CSinglePlayerInfo::SupplyItem( GameItem* pItem )
{
	if( !pItem )
		return false;

	int supplied_quantity = SupplyItem( pItem->GetName().c_str(), pItem->GetCurrentQuantity() );

	SafeDelete( pItem );

	return supplied_quantity;
}


int CSinglePlayerInfo::SupplyItem( std::shared_ptr<GameItem> pItem )
{
	if( !pItem )
		return false;

	int supplied_quantity = SupplyItem( pItem->GetName().c_str(), pItem->GetCurrentQuantity() );

	return supplied_quantity;
}


int CSinglePlayerInfo::SupplyItem( const char *pcItemName, const int iSupplyQuantity )
//int CSinglePlayerInfo::SupplyItem( GameItem*& rpItem )/// \param owned reference
{
	size_t i, num_items = m_vecpItem.size();
	for(i=0; i<num_items; i++)
	{
		// search the array of items to find one with the same name as 'pcItemName'
		if( strcmp(pcItemName, m_vecpItem[i]->GetName().c_str() ) == 0 )
//		 && (*pSuppliedItem) == m_vecpItem[i] )	// check the items are the identical instances
		{
			break;
		}
	}

	if( i == num_items )
	{
		// a new item - needs to be copied from the item database
		shared_ptr<GameItem> pItem = GetItemDatabaseManager().GetItem<GameItem>( pcItemName, iSupplyQuantity );
		if( !pItem /*|| !(pItem->GetTypeFlag() & GameItem::TYPE_AMMO)*/ )
			return 0;	// the item was not found in the database

		// initialize quantity
		pItem->SetZeroQuantity();

		pItem->SetStageWeakPtr( m_pCurrentStage );

		// add a new item entry to the list
		m_vecpItem.push_back( pItem );

		// put to a one of the category sorted lists
		AddItemToCategoryList( pItem );

	}
/*
	if( i == num_items )
	{
		// add a new item entry to the list
		m_vecpItem.push_back( pSuppliedItem );

		// put to a one of the category sorted lists
		// the reference is now owned by PlayerInfo
		AddItemToCategoryList( pItem );
	}
	else
	{
		// The supplied item is already in the list
		// just increase the quantity

		// delete the supplied item object
	}
*/
	// supply item(s)

	GameItem& rItem = *(m_vecpItem[i].get());
	if( rItem.GetCurrentQuantity() == rItem.GetMaxQuantity() )
		return 0;	// already been full - can't have any more
	else
	{
		int num = rItem.GetCurrentQuantity();
		int added_quantity;
		if( num + iSupplyQuantity <= rItem.GetMaxQuantity() )
			added_quantity = iSupplyQuantity;
		else
			added_quantity = rItem.GetMaxQuantity() - rItem.GetCurrentQuantity();  // prevent overload

		rItem.AddQuantity( added_quantity );

		return added_quantity;
	}

	return 0;
}

/*
GameItem *CSinglePlayerInfo::GetItemByName( const char *pcItemName )
{
	size_t i=0, num_items = m_vecpItem.size();
	for( i=0; i<num_items; i++ )
	{
		if( m_vecpItem[i]->GetName() == pcItemName )
			return m_vecpItem[i].get();
	}

	LOG_PRINT( "Cannot find the item: " + string(pcItemName) );

	return NULL;
}
*/


void CSinglePlayerInfo::Update( float dt )
{
//	if( m_pHUD )
//		m_pHUD->Update( dt );

	size_t i=0, num = m_vecpActiveItem.size();
	for( i=0; i<num; i++ )
	{
		m_vecpActiveItem[i]->Update( dt );
	}
}

/*
void CSinglePlayerInfo::RenderHUD()
{
	if( m_pHUD )
        m_pHUD->Render();

	// draw the status of the items currently active
	if( m_pHUD->GetFont() )
	{
		size_t i, num = m_vecpActiveItem.size();
		string status_buffer;
		for( i=0; i<num; i++ )
		{
			Vector2 vPos = Vector2( 480.0f, 500.0f + i * 16 );
			m_vecpActiveItem[i]->GetStatus( status_buffer );
			m_pHUD->GetFont()->DrawText( status_buffer.c_str(), vPos, 0xFFFFFFFF );

		}
	}
}
*/

void CSinglePlayerInfo::Serialize( IArchive& ar, const unsigned int version )
{
	if( ar.GetMode() == IArchive::MODE_INPUT )
	{
		m_pCurrentPlayerBaseEntity = NULL;
//		m_pWeaponSystem;

		m_TaskID = -1;
		m_vecpItem.resize( 0 );

		size_t i, num_item_categories = CItemCategory::NUM_CATEGORIES;
		for( i=0; i<num_item_categories; i++ )
            m_vecpCategoryItem[i].resize(0);

		m_vecpActiveItem.resize(0);

		m_pCurrentAircraft.reset();
	}

	GameItemObjectFactory factory;

	ar.Polymorphic( m_vecpItem, factory );

	ar & m_Money;

	ar & m_KeyBind;

//	ar & m_PlayTime;

	if( ar.GetMode() == IArchive::MODE_INPUT )
	{
		// register items to m_vecpCategoryItem
	}

}

} // namespace amorphous
