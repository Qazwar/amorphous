#include "SkeletalCharacter.hpp"
#include "gds/Input/InputHub.hpp"
#include "gds/Input/InputDevice.hpp"
#include "gds/Graphics/Mesh/SkeletalMesh.hpp"
#include "gds/Graphics/Shader/GenericShaderGenerator.hpp"
#include "gds/MotionSynthesis/MotionDatabase.hpp"
#include "gds/MotionSynthesis/MotionPrimitiveBlender.hpp"
#include "gds/MotionSynthesis/SkeletalMeshTransform.hpp"
#include "gds/Support/DebugOutput.hpp"
#include "gds/Stage/BaseEntity_Draw.hpp"
#include "gds/Physics/Actor.hpp"
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost;
using namespace msynth;


class CDebugItem_MotionFSM : public CDebugItem_ResourceManager
{
public:
	boost::shared_ptr<msynth::CMotionGraphManager> m_pMotionGraphManager;
public:

	CDebugItem_MotionFSM() {}

	void GetTextInfo()
	{
		string buffer;
		m_pMotionGraphManager->GetDebugInfo( buffer );
		strncpy( m_TextBuffer, buffer.c_str(), sizeof(m_TextBuffer) - 1 );
	}
};


CSkeletalCharacter::CSkeletalCharacter()
:
m_fFwdSpeed(0.0f),
m_fTurnSpeed(0.0f)
{
	m_pMotionGraphManager.reset( new CMotionGraphManager );
//	m_pMotionGraphManager = shared_new<CMotionGraphManager>();

	// Not used: motion database filepaths are specified in resources/misc/test_motion_fsm.xml
//	string mdb_filepath = "motions/lws-fwd.mdb";
//	string mdb_filepath = "motions/bvh_to_lws_mapping_test.mdb";
	string mdb_filepath = "motions/basic_moves__az.mdb";

	bool load_embedded_test_data = false;
	if( load_embedded_test_data )
		m_pMotionGraphManager->InitForTest( mdb_filepath );
	else
	{
		// Load FSM from XML file and update the binary file
		string xml_file = "../resources/misc/test_motion_fsm.xml";
		if( boost::filesystem::exists(xml_file) )
		{
			m_pMotionGraphManager->LoadFromXMLFile( xml_file );
			m_pMotionGraphManager->SaveToFile( "motions/test_motion_fsm.bin" );
		}

		// Clear the data loaded from XML
		m_pMotionGraphManager.reset( new CMotionGraphManager );

		// Load from the binary archive
		bool loaded_from_archive = m_pMotionGraphManager->LoadFromFile( "motions/test_motion_fsm.bin" );

		if( loaded_from_archive )
			m_pMotionGraphManager->LoadMotions();
	}


	shared_ptr<CMotionFSM> pLowerLimbsFSM = m_pMotionGraphManager->GetMotionFSM( "lower_limbs" );

	m_pMotionNodes.resize( 4 );
	m_pMotionNodes[0].reset( new CFwdMotionNode );
	m_pMotionNodes[1].reset( new CRunMotionNode );
	m_pMotionNodes[2].reset( new CStandingMotionNode );
	m_pMotionNodes[3].reset( new CJumpMotionNode );
	for( size_t i=0; i<m_pMotionNodes.size(); i++ )
	{
		m_pMotionNodes[i]->SetSkeletalCharacter( this );
	}

	pLowerLimbsFSM->GetNode( "fwd" )->SetAlgorithm( m_pMotionNodes[0] );
	pLowerLimbsFSM->GetNode( "run" )->SetAlgorithm( m_pMotionNodes[1] );
	pLowerLimbsFSM->GetNode( "standing" )->SetAlgorithm( m_pMotionNodes[2] );
//	pLowerLimbsFSM->GetNode( "jump" )->SetAlgorithm( m_pMotionNodes[3] );

	m_pLowerLimbsMotionsFSM = m_pMotionGraphManager->GetMotionFSM( "lower_limbs" );
	if( !m_pLowerLimbsMotionsFSM )
		m_pLowerLimbsMotionsFSM.reset( new CMotionFSM ); // avoid NULL checking

	// mesh

	shared_ptr<CMeshObjectContainer> pContainer( new CMeshObjectContainer );
	pContainer->m_MeshDesc.ResourcePath = "models/male_skinny_young.msh";
	pContainer->m_MeshDesc.MeshType = CMeshType::SKELETAL;
//	pContainer->m_MeshDesc.pMeshGenerator.reset( new CBoxMeshGenerator() );
	m_MeshContainerRootNode.SetMeshContainer( 0, pContainer );

	m_MeshContainerRootNode.LoadMeshesFromDesc();

	m_pRenderMethod.reset( new CMeshContainerRenderMethod );
//	m_pRenderMethod = shared_new<CMeshContainerRenderMethod>();

	m_pRenderMethod->MeshRenderMethod().resize( 1 );
//	m_pRenderMethod->MeshRenderMethod()[0].m_ShaderFilepath = "Shader/VertexBlend.fx";
/*	m_pRenderMethod->MeshRenderMethod()[0].m_ShaderFilepath = "Shader/Default.fx";
	m_pRenderMethod->MeshRenderMethod()[0].m_Technique.SetTechniqueName( "VertBlend_PVL_HSLs" );
//	m_pRenderMethod->MeshRenderMethod()[0].m_Technique.SetTechniqueName( "SingleHSDL_Specular_CTS" );*/
	m_pRenderMethod->LoadRenderMethodResources();

	// TODO: add shader resource desc as a member variable to CSubsetRenderMethod
	CShaderResourceDesc shader_desc;
	CGenericShaderDesc gen_shader_desc;
	gen_shader_desc.Specular = CSpecularSource::DECAL_TEX_ALPHA;
	gen_shader_desc.VertexBlendType = CVertexBlendType::QUATERNION_AND_VECTOR3;
	shader_desc.pShaderGenerator.reset( new CGenericShaderGenerator(gen_shader_desc) );
	m_pRenderMethod->MeshRenderMethod()[0].m_Shader.Load( shader_desc );
	m_pRenderMethod->MeshRenderMethod()[0].m_Technique.SetTechniqueName( "Default" );

	// Init input handler
	m_pInputHandler.reset( new CMotionFSMInputHandler(m_pMotionGraphManager) );
	if( InputHub().GetInputHandler(2) )
		InputHub().GetInputHandler(2)->AddChild( m_pInputHandler.get() );
	else
		InputHub().PushInputHandler( 2, m_pInputHandler.get() );

	// save a motion primitive to get skeleton info in Render()
	CMotionDatabase mdb( mdb_filepath );
	m_pSkeletonSrcMotion = mdb.GetMotionPrimitive("standing");

	m_pMotionGraphManager->GetMotionFSM("lower_limbs")->StartMotion("standing");

	CDebugItem_MotionFSM *pDbgItem = new CDebugItem_MotionFSM;
	pDbgItem->m_pMotionGraphManager = m_pMotionGraphManager;
	DebugOutput.AddDebugItem( "motion_graph_mgr", pDbgItem );
}

/*
void CSkeletalCharacter::InitResources(){}
*/

void CSkeletalCharacter::OnEntityCreated( CCopyEntity& entity )
{
//	shared_ptr<CCopyEntity> pEntity = m_Entity.Get();
//	if( !pEntity )
//		return;

//	CCopyEntity& entity = *pEntity;

	entity.m_MeshHandle = m_MeshContainerRootNode.GetMeshContainer(0)->m_MeshObjectHandle;
	entity.m_pMeshRenderMethod = m_pRenderMethod;

	entity.RaiseEntityFlags( BETYPE_SHADOW_CASTER );
	entity.RaiseEntityFlags( BETYPE_SHADOW_RECEIVER );
}


void CSkeletalCharacter::Update( float dt )
{
	shared_ptr<CMotionFSM> pFSM = m_pMotionGraphManager->GetMotionFSM("lower_limbs");
	if( !pFSM )
		return;

	shared_ptr<CCopyEntity> pEntity = m_Entity.Get();
	if( !pEntity )
		return;

	Matrix34 world_pose = pEntity->GetWorldPose();

	static Matrix34 m_PrevWorldPose = world_pose;

	// steering
	world_pose.matOrient = world_pose.matOrient * Matrix33RotationY( GetTurnSpeed() * dt );

	pFSM->Player()->SetCurrentHorizontalPose( world_pose );

	m_pMotionGraphManager->Update( dt );

	Matrix34 updated_world_pose = pFSM->Player()->GetCurrentHorizontalPose();

	// test collision

	// the world pose of the entity -> always stays horizontal
	pEntity->SetWorldPose( updated_world_pose );
//	pEntity->SetWorldPose( Matrix34Identity() );

	if( 0 < pEntity->m_vecpPhysicsActor.size()
	 && pEntity->m_vecpPhysicsActor[0] )
	{
		physics::CActor& actor = *(pEntity->m_vecpPhysicsActor[0]);
		Vector3 vPhysActorOffset = Vector3( 0, 1, 0 );
		Matrix34 phys_actor_world_pose( updated_world_pose );
		phys_actor_world_pose.vPosition += vPhysActorOffset;
		actor.SetWorldPose( phys_actor_world_pose );
	}

//	Matrix34 world_pose = m_pMotionGraphManager->GetCurrentWorldPose();

/*
	const Matrix34 world_pose = m_pMotionGraphManager->GetRootNodeWorldPose();
	SetWorldPose( world_pose );
	shared_ptr<CCopyEntity> pEntity = m_Entity.Get();
*/
}


void CSkeletalCharacter::Render()
{
	if( m_MeshContainerRootNode.GetNumMeshContainers() == 0 )
		return;

	shared_ptr<CMeshObjectContainer> pContainer = m_MeshContainerRootNode.GetMeshContainer( 0 );
	if( !pContainer )
		return;

	shared_ptr<CBasicMesh> pMesh = pContainer->m_MeshObjectHandle.GetMesh();
	CSkeletalMesh *pSkeletalMesh = dynamic_cast<CSkeletalMesh *>(pMesh.get());
	if( !pSkeletalMesh )
	{
		int non_skeletal_mesh = 1;
//		return;
	}

//	pSkeletalMesh->Render();

	if( !m_pSkeletonSrcMotion )
		return;

	shared_ptr<CCopyEntity> pEntity = m_Entity.Get();
	if( !pEntity )
		return;

	CKeyframe m_InterpolatedKeyframe;
	CKeyframe& dest = m_InterpolatedKeyframe;
	m_pMotionGraphManager->GetCurrentKeyframe( dest );
	UpdateMeshBoneTransforms( dest, *(m_pSkeletonSrcMotion->GetSkeleton()), *pSkeletalMesh );

	pSkeletalMesh->SetLocalTransformsFromCache();
///	Matrix34 pose = Matrix34Identity();
	Matrix34 pose = pEntity->GetWorldPose();
	m_pRenderMethod->RenderMeshContainer( *(m_MeshContainerRootNode.MeshContainer( 0 )), pose );
}


void CSkeletalCharacter::SetKeyBind( shared_ptr<CKeyBind> pKeyBind )
{
	m_pKeyBind = pKeyBind;
	for( size_t i=0; i<m_pMotionNodes.size(); i++ )
		m_pMotionNodes[i]->SetKeyBind( m_pKeyBind );

	// Create map from action code to GI codes

//	m_pKeyBind->Update( m_ACtoGIC );

	int action_codes_to_update[] = { ACTION_MOV_FORWARD, ACTION_MOV_BOOST };
	for( int i=0; i<CKeyBind::NUM_ACTION_TYPES; i++ )
	{
		map<int, vector<int> >& ac_to_gics = m_ACtoGICs.m_mapActionCodeToGICodes[i];

		for( int j=0; j<numof(action_codes_to_update); j++ )
		{
			int action_code = action_codes_to_update[j];
			vector<int> gics;
			m_pKeyBind->FindGeneralInputCodesOfAllInputDevices( action_code, i, gics );
			ac_to_gics[action_code] = gics;
		}
	}
}


CInputState::Name CSkeletalCharacter::GetActionInputState( int action_code, CKeyBind::ActionType action_type )
{
	map< int, vector<int> >& ac_to_gics = m_ACtoGICs.m_mapActionCodeToGICodes[action_type];

	map< int, vector<int> >::iterator itr = ac_to_gics.find( action_code );
	if( itr == ac_to_gics.end() )
		return CInputState::RELEASED;

	for( size_t i=0; i<itr->second.size(); i++ )
	{
		CInputState::Name input_stat
			= InputDeviceHub().GetInputDeviceGroup(0)->GetInputState( itr->second[i] ); 

		// Key is considered pressed if any one of the keys for the general input codes are pressed.
		if( input_stat == CInputState::PRESSED )
			return CInputState::PRESSED;
	}

	return CInputState::RELEASED;
}


// Moved to CCharacterMotionNodeAlgorithm::HandleInput()
/*
void CSkeletalCharacter::ProcessInput( const SInputData& input, int action_code )
{
	switch( action_code )
	{
	case ACTION_MOV_FORWARD:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
			m_fFwdSpeed =  input.fParam1;
			m_pLowerLimbsMotionsFSM->RequestTransition( "fwd" );
		}
		break;
	case ACTION_MOV_BACKWARD:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
			m_fFwdSpeed = -input.fParam1;
//			m_pLowerLimbsMotionsFSM->RequestTransition( "bwd" );
		}
		break;
	case ACTION_MOV_TURN_R:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
			m_fTurnSpeed =  input.fParam1;
		}
		break;
	case ACTION_MOV_TURN_L:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
			m_fTurnSpeed = -input.fParam1;
		}
		break;
	default:
		break;
	}
}
*/

void CSkeletalCharacter::OnPhysicsTrigger( physics::CShape& my_shape, CCopyEntity &other_entity, physics::CShape& other_shape )
{
	physics::CRay ray;

	// Check contact with ground
/*	ray.Origin = my_shape.GetActor().GetWorldPosition() + Vector3(0,-0.5,0);
	ray.Direction = Vector3( 0, -1, 0 );
	float ray_max_dist = 10;
	physics::CRaycastHit hit;
	bool is_hit = other_shape.Raycast( ray, ray_max_dist, 0, hit, false );
//	if( hit.pShape )

	// Check contact with ceiling

	// Check contact with the environment in the direction of the current motion
	Vector3 vCovered = world_pose.vPosition - m_PrevPose.vPosition;
	ray.Origin = m_PrevPose.vPosition;
	ray.Direction = Vec3GetNormalized(vCovered);
	is_hit = pOtherShape->Raycast( ray, ray_max_dist, 0, hit, false );
*/

}

/*
CInputState::Name CCharacterMotionNodeAlgorithm::GetGeneralInputState( int gi_code )
{
//	m_pCharacter->GetActionInputState( ACTION_MOV_BOOST );
	return InputDeviceHub().GetInputDeviceGroup(0)->GetInputState( gi_code );
}*/

CInputState::Name CCharacterMotionNodeAlgorithm::GetActionInputState( int action_code )
{
	return m_pCharacter->GetActionInputState( action_code );
}


bool CCharacterMotionNodeAlgorithm::HandleInput( const SInputData& input, int action_code )
{
	switch( action_code )
	{
	case ACTION_MOV_FORWARD:
		if( input.iType == ITYPE_KEY_PRESSED
		 || input.iType == ITYPE_VALUE_CHANGED )
		{
			float fwd_speed = 0;
			if( input.IsKeyboardInput() )
			{
				fwd_speed = (GetActionInputState(ACTION_MOV_BOOST) == CInputState::PRESSED) ? 1.0f : 0.5f;
			}
			else // analog input
				fwd_speed = input.fParam1;

			m_pCharacter->SetFwdSpeed( fwd_speed );
		}
/*		if( input.iType == ITYPE_KEY_PRESSED )
			m_pCharacter->SetFwdSpeed(  input.fParam1 );
		else if( input.iType == ITYPE_VALUE_CHANGED )
			m_pCharacter->SetFwdSpeed(  input.fParam1 );*/
		else
			m_pCharacter->SetFwdSpeed( 0 );
		break;
	case ACTION_MOV_BACKWARD:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
			m_pCharacter->SetFwdSpeed( -input.fParam1 );
//			m_fFwdSpeed = -input.fParam1;
//			m_pLowerLimbsMotionsFSM->RequestTransition( "bwd" );
		}
		break;
	case ACTION_MOV_BOOST: // Keyboard only. Analog gamepad has analog stick to control walk/run
		if( input.iType == ITYPE_KEY_PRESSED )
		{
			float fwd_speed = 0;
			CInputState::Name fwd_input = GetActionInputState( ACTION_MOV_FORWARD );
			if( fwd_input == CInputState::PRESSED )
				fwd_speed = 1.0f;
//			else
//				fwd_speed = 0.5f;

			m_pCharacter->SetFwdSpeed( fwd_speed );
		}
		else if( input.iType == ITYPE_KEY_RELEASED )
		{
			float fwd_speed = 0;
			CInputState::Name fwd_input = GetActionInputState( ACTION_MOV_FORWARD );
			if( fwd_input == CInputState::PRESSED )
				fwd_speed = 0.5f;
			else
				fwd_speed = 0.0f;

			m_pCharacter->SetFwdSpeed( fwd_speed );
		}
		break;
	case ACTION_MOV_TURN_R:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
			m_pCharacter->SetTurnSpeed(  input.fParam1 );
//			m_fTurnSpeed =  input.fParam1;
		}
		else
			m_pCharacter->SetTurnSpeed( 0 );
		break;
	case ACTION_MOV_TURN_L:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
			m_pCharacter->SetTurnSpeed( -input.fParam1 );
//			m_fTurnSpeed = -input.fParam1;
		}
		else
			m_pCharacter->SetTurnSpeed( 0 );
		break;
	default:
		break;
	}

	return false;
}


void CFwdMotionNode::Update( float dt )
{
	float fFwdSpeed = m_pCharacter->GetFwdSpeed();
	if( fFwdSpeed < 0.2f )
	{
		RequestTransition( "standing" );
	}
	else if( 0.55 <= fFwdSpeed )
	{
		RequestTransition( "run" );
	}
}


bool CFwdMotionNode::HandleInput( const SInputData& input, int action_code )
{
	CCharacterMotionNodeAlgorithm::HandleInput( input, action_code );

	switch( action_code )
	{
	case ACTION_MOV_FORWARD:
		if( input.iType == ITYPE_KEY_PRESSED
		 || input.iType == ITYPE_VALUE_CHANGED )
		{
			m_pNode->SetExtraSpeedFactor( m_pCharacter->GetFwdSpeed() * 0.5f );
		}
		break;
	case ACTION_MOV_BOOST:
		if( input.iType == ITYPE_KEY_PRESSED )
			RequestTransition( "run" );
		break;
	case ACTION_MOV_JUMP:
		RequestTransition( "jump" );
		break;
/*	case ACTION_MOV_TURN_L:
		break;
	case ACTION_MOV_TURN_R:
		break;*/
	default:
		break;
	}

	return false;
}


void CFwdMotionNode::EnterState()
{
	// update the forward speed
	m_pNode->SetExtraSpeedFactor( m_pCharacter->GetFwdSpeed() );
}


void CRunMotionNode::Update( float dt )
{
	float fFwdSpeed = m_pCharacter->GetFwdSpeed();
	if( fFwdSpeed < 0.2f )
	{
		RequestTransition( "standing" );
	}
	else if( fFwdSpeed < 0.6f )
	{
		RequestTransition( "fwd" ); // walk
	}
}


bool CRunMotionNode::HandleInput( const SInputData& input, int action_code )
{
	CCharacterMotionNodeAlgorithm::HandleInput( input, action_code );

	switch( action_code )
	{
	case ACTION_MOV_FORWARD:
		if( input.iType == ITYPE_KEY_PRESSED
		 || input.iType == ITYPE_VALUE_CHANGED )
		{
			m_pNode->SetExtraSpeedFactor( input.fParam1 * 0.5f );
		}
		break;
//	case ACTION_MOV_BOOST:
//		if( input.iType == ITYPE_KEY_RELEASED )
//			RequestTransition( "fwd" );
//		break;
	case ACTION_MOV_JUMP:
		RequestTransition( "jump" );
		break;
/*	case ACTION_MOV_TURN_L:
		break;
	case ACTION_MOV_TURN_R:
		break;*/
	default:
		break;
	}

	return false;
}


void CRunMotionNode::EnterState()
{
	// update the forward speed
	m_pNode->SetExtraSpeedFactor( m_pCharacter->GetFwdSpeed() );
}


void CJumpMotionNode::Update( float dt )
{
}


bool CJumpMotionNode::HandleInput( const SInputData& input, int action_code )
{
	return false;
}



void CStandingMotionNode::Update( float dt )
{
}


bool CStandingMotionNode::HandleInput( const SInputData& input, int action_code )
{
	CCharacterMotionNodeAlgorithm::HandleInput( input, action_code );

	switch( action_code )
	{
	case ACTION_MOV_FORWARD:
		if( input.iType == ITYPE_KEY_PRESSED
		 && 0.2f < input.fParam1 )
		{
			RequestTransition( "fwd" );
		}
		else if( input.iType == ITYPE_VALUE_CHANGED
		 && 0.2f < input.fParam1 )
		{
			RequestTransition( "fwd" );
		}
		break;
	case ACTION_MOV_JUMP:
		RequestTransition( "jump" );
		break;
/*	case ACTION_MOV_TURN_L:
		break;
	case ACTION_MOV_TURN_R:
		break;*/
	default:
		break;
	}

	return false;
}
