#include "MeshObjectHandle.hpp"
#include "GraphicsResourceManager.hpp"

#include "Support/Log/DefaultLog.hpp"

using namespace std;


//==================================================================================================
// CMeshObjectHandle
//==================================================================================================

boost::shared_ptr<CMeshResource> CMeshObjectHandle::GetMeshResource()
{
	if( GetEntry() )
		return GetEntry()->GetMeshResource();
	else
		return boost::shared_ptr<CMeshResource>();
}


CMeshType::Name CMeshObjectHandle::GetMeshType()
{
	if( GetEntry()
	 && GetEntry()->GetMeshResource() )
		return GetEntry()->GetMeshResource()->GetMeshType();
	else
		return CMeshType::INVALID; // TODO: return a value that means invalid request
}


bool CMeshObjectHandle::Load( const std::string& resource_path )
{
	CMeshResourceDesc desc;
	desc.ResourcePath = resource_path;
	return Load( desc );
}


bool CMeshObjectHandle::Load( const CMeshResourceDesc& desc )
{
	// if currently holding a mesh, release it
	Release();

	if( desc.LoadingMode == CResourceLoadingMode::SYNCHRONOUS )
	{
		m_pResourceEntry = GraphicsResourceManager().LoadMesh( desc );
	}
	else
	{
		m_pResourceEntry = GraphicsResourceManager().LoadAsync( desc );
	}

	if( m_pResourceEntry )
		return true;
	else
		return false;
}



/*
bool CMeshObjectHandle::LoadAsync( int priority )
{
	CMeshResourceDesc desc;
	desc.Filename = filename;


	return true;
}
*/
/*
void CMeshObjectHandle::Serialize( IArchive& ar, const unsigned int version )
{
	CGraphicsResourceHandle::Serialize( ar, version );
}
*/