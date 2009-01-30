#include "GraphicsResourceCacheManager.hpp"

using namespace std;
using namespace boost;


//===============================================================
// CD3DGraphicsResourceFactoryImpl
//===============================================================

shared_ptr<CTextureResource> CD3DGraphicsResourceFactoryImpl::CreateTextureResource( const CTextureResourceDesc& desc )
{
	return shared_ptr<CTextureResource>( new CTextureResource(&desc) );
}

shared_ptr<CMeshResource> CD3DGraphicsResourceFactoryImpl::CreateMeshResource( const CMeshResourceDesc& desc )
{
	return shared_ptr<CMeshResource>( new CMeshResource(&desc) );
}

shared_ptr<CShaderResource> CD3DGraphicsResourceFactoryImpl::CreateShaderResource( const CShaderResourceDesc& desc )
{
	return shared_ptr<CShaderResource>( new CShaderResource(&desc) );
}




//===============================================================
// CGraphicsResourceFactory
//===============================================================

/// define the singleton instance
CSingleton<CGraphicsResourceFactory> CGraphicsResourceFactory::m_obj;


CGraphicsResourceFactory::CGraphicsResourceFactory()
:
m_pImpl(NULL)
{
	m_pImpl = new CD3DGraphicsResourceFactoryImpl;
}


CGraphicsResourceFactory::~CGraphicsResourceFactory()
{
	SafeDelete( m_pImpl );
}

shared_ptr<CGraphicsResource> CGraphicsResourceFactory::CreateGraphicsResource( const CGraphicsResourceDesc &desc )
{
	switch(desc.GetResourceType())
	{
	case GraphicsResourceType::Texture: return CreateTextureResource( *dynamic_cast<const CTextureResourceDesc*>(&desc) );
	case GraphicsResourceType::Mesh:    return CreateMeshResource( *dynamic_cast<const CMeshResourceDesc*>(&desc) );
	case GraphicsResourceType::Shader:  return CreateShaderResource( *dynamic_cast<const CShaderResourceDesc*>(&desc) );
	default:
		return shared_ptr<CGraphicsResource>();
	}
}


//===============================================================
// CGraphicsResourceCacheManager
//===============================================================

/// define the singleton instance
CSingleton<CGraphicsResourceCacheManager> CGraphicsResourceCacheManager::m_obj;


bool CGraphicsResourceCacheManager::Preload( const std::string& xml_filename )
{
	return true;
}


void CGraphicsResourceCacheManager::AddCache( CGraphicsResourceDesc& desc )
{
	// create as a cached resource
	desc.m_IsCachedResource = true;

	shared_ptr<CGraphicsResource> ptr = GraphicsResourceFactory().CreateGraphicsResource( desc );

//	ptr->IsCachedResource = true;

	// create an empty texture / mesh
	ptr->Create();

	m_vecpResurceCache.push_back( ptr );
}


shared_ptr<CGraphicsResource> CGraphicsResourceCacheManager::GetCachedResource( const CGraphicsResourceDesc& desc )
{
	/// find preloaded resource that matches the description

	const size_t num_resources = m_vecpResurceCache.size();
	for( size_t i=0; i<num_resources; i++ )
	{
		shared_ptr<CGraphicsResource> pResource = m_vecpResurceCache[i];
		if( pResource->CanBeUsedAsCache( desc ) )
		{
			// copy some desc attributes pResource
			// - Need this to set attributes that are unique to each resource
			//   - e.g., resource path
			pResource->UpdateDescForCachedResource( desc );

			return pResource;
		}
	}

	return shared_ptr<CGraphicsResource>();
}


void CGraphicsResourceCacheManager::LoadGraphicsResources( const CGraphicsParameters& rParam )
{
	const size_t num_resources = m_vecpResurceCache.size();
	for( size_t i=0; i<num_resources; i++ )
	{
		m_vecpResurceCache[i]->Load();
	}
}


// Release all the cached resources
void CGraphicsResourceCacheManager::ReleaseGraphicsResources()
{
	const size_t num_resources = m_vecpResurceCache.size();
	for( size_t i=0; i<num_resources; i++ )
	{
		m_vecpResurceCache[i]->ReleaseCachedResource();
	}
}
