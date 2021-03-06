#ifndef __D3DGraphicsResourceLoaders_HPP__
#define __D3DGraphicsResourceLoaders_HPP__


#include "../GraphicsResourceLoaders.hpp"


namespace amorphous
{


class CD3DMeshLoader : public MeshLoader
{

	void *m_pVertexBufferContent;

	std::vector<U16> m_vecIndexBufferContent;

	std::vector<D3DXATTRIBUTERANGE> m_vecAttributeRange;

public:

	CD3DMeshLoader();

	~CD3DMeshLoader();

	void LoadMeshSubresources();

	void OnLoadingCompleted( std::shared_ptr<GraphicsResourceLoader> pSelf );

	void OnResourceLoadedOnGraphicsMemory();

	std::vector<U16>& IndexBufferContent() { return m_vecIndexBufferContent; }

	void *VertexBufferContent() { return m_pVertexBufferContent; }

	std::vector<D3DXATTRIBUTERANGE>& AttributeTable() { return m_vecAttributeRange; }
};


class CD3DXMeshLoaderBase : public GraphicsResourceLoader
{
protected:

	/// entry to that stores the loaded resource
//	std::weak_ptr<GraphicsResourceEntry> m_pMeshEntry;

//	MeshResourceDesc m_Desc;

	std::shared_ptr<C3DMeshModelArchive> m_pArchive;

	/// Store the shared_ptr of the mesh loader so that it do not get released
	/// after loading mesh archive.
	std::shared_ptr<CD3DMeshLoader> m_pMeshLoader;

public:

	CD3DXMeshLoaderBase( std::weak_ptr<GraphicsResourceEntry> pEntry )
		:
	GraphicsResourceLoader(pEntry)
	{}

	virtual ~CD3DXMeshLoaderBase() {}

	bool AcquireResource() { return true; }

	inline BasicMesh *GetMesh();

	Result::Name Load();

	virtual bool LoadFromArchive() = 0;

	inline GraphicsResourceState::Name GetSubResourceState( MeshSubResource::Name subresource ) const;

	inline void SetSubResourceState( MeshSubResource::Name subresource, GraphicsResourceState::Name state );

	CD3DXMeshObjectBase *GetD3DMeshImpl();

	friend class CD3DMeshLoader;
};

class CD3DXMeshVerticesLoader : public CD3DXMeshLoaderBase
{
	// hold data to be copied to VB of the mesh
//	std::vector<unsigned char> m_vecVertexBufferContent;
	void *m_pVertexBufferContent;
	
	int m_VertexBufferSize;

	void *m_pLockedVertexBuffer;

protected:

//	const std::string& GetSourceFilepath() { return m_Desc.Filename; }

public:

	CD3DXMeshVerticesLoader( std::weak_ptr<GraphicsResourceEntry> pEntry )
		:
	CD3DXMeshLoaderBase(pEntry),
	m_pLockedVertexBuffer(NULL),
	m_pVertexBufferContent(NULL)
	{}

	~CD3DXMeshVerticesLoader() { SafeDelete(m_pVertexBufferContent); }

	bool LoadFromArchive();

	bool CopyLoadedContentToGraphicsResource();

	// Lock the index buffer and save the pointer to the locked buffer
	bool Lock();

	bool Unlock();

	void OnResourceLoadedOnGraphicsMemory();

	bool IsReadyToLock() const;
};


class CD3DXMeshIndicesLoader : public CD3DXMeshLoaderBase
{

//	std::vector<unsigned char> m_vecIndexBufferContent;

	void *m_pIndexBufferContent;

	int m_IndexBufferSize;

	void *m_pLockedIndexBuffer;

protected:

//	const std::string& GetSourceFilepath() { return m_Desc.Filename; }

public:

	CD3DXMeshIndicesLoader( std::weak_ptr<GraphicsResourceEntry> pEntry )
		:
	CD3DXMeshLoaderBase(pEntry),
	m_pLockedIndexBuffer(NULL),
	m_pIndexBufferContent(NULL)
	{}

	~CD3DXMeshIndicesLoader() { SafeDelete(m_pIndexBufferContent); }

	bool LoadFromArchive();

	bool CopyLoadedContentToGraphicsResource();

	// Lock the index buffer and save the pointer to the locked buffer
	bool Lock();

	bool Unlock();

	void OnResourceLoadedOnGraphicsMemory();

	bool IsReadyToLock() const;
};


class CD3DXMeshAttributeTableLoader : public CD3DXMeshLoaderBase
{
	DWORD *m_pLockedAttributeBuffer;

protected:

//	const std::string& GetSourceFilepath() { return m_Desc.Filename; }

public:

	CD3DXMeshAttributeTableLoader( std::weak_ptr<GraphicsResourceEntry> pEntry )
		:
	CD3DXMeshLoaderBase(pEntry),
	m_pLockedAttributeBuffer(NULL)
	{}

	~CD3DXMeshAttributeTableLoader() {}

	// copy the resource directly from mesh archive
	// -> No need to convert data format
	virtual bool LoadFromArchive() { return true; }

	bool CopyLoadedContentToGraphicsResource();

	// Lock the index buffer and save the pointer to the locked buffer
	bool Lock();

	bool Unlock();

	void OnResourceLoadedOnGraphicsMemory();

	bool IsReadyToLock() const;

};


//--------------------------- inline implementations ---------------------------

//==============================================================================
// CD3DXMeshObjectBase
//==============================================================================

inline BasicMesh *CD3DXMeshLoaderBase::GetMesh()
{
	std::shared_ptr<GraphicsResourceEntry> pEntry = GetResourceEntry();
	if( pEntry )
	{
		std::shared_ptr<MeshResource> pMesh = pEntry->GetMeshResource();

		if( pMesh )
			return pMesh->GetMeshInLoading().get();
		else
			return NULL;
	}
	else
		return NULL;
}


inline GraphicsResourceState::Name CD3DXMeshLoaderBase::GetSubResourceState( MeshSubResource::Name subresource ) const
{
	if( GetResourceEntry()
	 && GetResourceEntry()->GetMeshResource() )
	{
		return GetResourceEntry()->GetMeshResource()->GetSubResourceState( subresource );
	}
	else
		return GraphicsResourceState::RELEASED;
}


inline void CD3DXMeshLoaderBase::SetSubResourceState( MeshSubResource::Name subresource,
													  GraphicsResourceState::Name state )
{
	if( GetResourceEntry()
	 && GetResourceEntry()->GetMeshResource() )
	{
		GetResourceEntry()->GetMeshResource()->SetSubResourceState( subresource, state );
	}
}


} // namespace amorphous



#endif /* __D3DGraphicsResourceLoaders_HPP__ */
