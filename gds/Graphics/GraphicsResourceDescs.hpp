#ifndef  __GraphicsResourceDescs_H__
#define  __GraphicsResourceDescs_H__


#include <string>
#include <boost/weak_ptr.hpp>

#include "fwd.hpp"
#include "GraphicsResource.hpp"
#include "XML/fwd.hpp"
#include "Support/Serialization/Serialization.hpp"
using namespace GameLib1::Serialization;


class CResourceLoadingMode
{
public:
	enum Name
	{
		SYNCHRONOUS,
		ASYNCHRONOUS,
		NUM_LOADING_MODES
	};
};


class UsageFlag
{
public:
	enum Name
	{
		RENDER_TARGET           = (1 << 0),
//		ANOTHER_USAGE_FLAG     = (1 << 1),
//		YET_ANOTHER_USAGE_FLAG = (1 << 2),
	};
};


class CGraphicsResourceDesc : public IArchiveObjectBase
{
	bool m_IsCachedResource;

public:

	/// filled out by the system
	/// - User chooses a mode by calling CGraphicsResourceHandle::Load() or CGraphicsResourceHandle::LoadAsync();
	CResourceLoadingMode::Name LoadingMode;

	int LoadingPriority;

	/// Used when the resource is loaded from disk
	std::string ResourcePath;

public:

	inline CGraphicsResourceDesc();

	virtual GraphicsResourceType::Name GetResourceType() const = 0;

	virtual bool IsDiskResource() const { return true; }

	bool IsCachedResource() const { return m_IsCachedResource; }

	virtual boost::shared_ptr<CGraphicsResourceDesc> GetCopy() const = 0;

	virtual bool CanBeSharedAsSameTextureResource( const CTextureResourceDesc& desc ) const { return false; }
	virtual bool CanBeSharedAsSameMeshResource( const CMeshResourceDesc& desc ) const { return false; }
	virtual bool CanBeSharedAsSameShaderResource( const CShaderResourceDesc& desc ) const { return false; }

	/// Returns the score that shows how much the cache is preferable to be used as the requested resource.
	/// 0 means the cache cannot be used for a requested resource
	virtual int CanBeUsedAsTextureCache( const CTextureResourceDesc& desc ) const { return 0; }
	virtual int CanBeUsedAsMeshCache( const CMeshResourceDesc& desc ) const { return 0; }
	virtual int CanBeUsedAsShaderCache( const CShaderResourceDesc& desc ) const { return 0; }

	/// Copy attributes to the desc of the cached resource
	/// e.g., resource path
	/// - Cached texture resources maintain preloaded empty textures.
	///   A resource path (usu. a filepath) needs to be copied to it every time a new texture is loaded
	///   so that the same texture can be shared by multiple texture handles.
	virtual void UpdateCachedTextureResourceDesc( CTextureResourceDesc& desc ) const {}
	virtual void UpdateCachedMeshResourceDesc( CMeshResourceDesc& desc ) const {}
	virtual void UpdateCachedShaderResourceDesc( CTextureResourceDesc& desc ) const {}

	virtual void Serialize( IArchive& ar, const unsigned int version )
	{
		ar & (int&)LoadingMode;
		ar & LoadingPriority;
		ar & ResourcePath;
	}

	virtual void LoadFromXMLNode( CXMLNodeReader& reader );

	friend class CGraphicsResourceCacheManager;
};


class CTextureResourceDesc : public CGraphicsResourceDesc
{
public:

	int Width;
	int Height;
	int MipLevels; ///< 0 is set to create complete mipmap chain. (default: 0)

	TextureFormat::Format Format;

	uint UsageFlags;

	boost::weak_ptr<CTextureLoader> pLoader;

public:

	CTextureResourceDesc()
		:
	Width(0),
	Height(0),
	MipLevels(0),
	Format(TextureFormat::Invalid),
	UsageFlags(0)
	{}

	virtual GraphicsResourceType::Name GetResourceType() const { return GraphicsResourceType::Texture; }

	virtual boost::shared_ptr<CGraphicsResourceDesc> GetCopy() const { return boost::shared_ptr<CTextureResourceDesc>( new CTextureResourceDesc(*this) ); }

	bool CanBeSharedAsSameTextureResource( const CTextureResourceDesc& desc ) const
	{
		if( ResourcePath == desc.ResourcePath )
			return true;
		else
			return false;
	}

	int CanBeUsedAsTextureCache( const CTextureResourceDesc& desc ) const
	{
		if( Width     == desc.Width
		 && Height    == desc.Height
		 && MipLevels == desc.MipLevels
		 && Format    == desc.Format )
			return 1;
		else
			return 0;
	}

	void UpdateCachedTextureResourceDesc( CTextureResourceDesc& desc ) const
	{
		desc.ResourcePath = ResourcePath;
		desc.pLoader      = pLoader;
	}

	void Serialize( IArchive& ar, const unsigned int version )
	{
		CGraphicsResourceDesc::Serialize( ar, version );

		ar & Width & Height & MipLevels;
		ar & (int&)Format;
	}

	void LoadFromXMLNode( CXMLNodeReader& reader );
};


class CMeshResourceDesc : public CGraphicsResourceDesc
{
	/// Set after the mesh archive is loaded from the desc
	/// or an empty mesh is created.
	/// Used in asynchronous loading.
	int NumVertices;
	int NumIndices;
	int VertexFormatFlags;

public:

	CMeshType::Name MeshType;	///< used by mesh object

	U32 LoadOptionFlags;

public:

	inline CMeshResourceDesc();

	virtual GraphicsResourceType::Name GetResourceType() const { return GraphicsResourceType::Mesh; }

	virtual boost::shared_ptr<CGraphicsResourceDesc> GetCopy() const { return boost::shared_ptr<CMeshResourceDesc>( new CMeshResourceDesc(*this) ); }

	bool CanBeSharedAsSameMeshResource( const CMeshResourceDesc& desc ) const
	{
		if( MeshType        == desc.MeshType
		 && LoadOptionFlags == desc.LoadOptionFlags )
		{
			if( ResourcePath == desc.ResourcePath )
				return true;
			else
				return false;
		}
		else
			return false;
	}

	int CanBeUsedAsMeshCache( const CMeshResourceDesc& desc ) const
	{
		if( MeshType        != desc.MeshType
		 || LoadOptionFlags != desc.LoadOptionFlags )
			return 0;

		return 0;
	}

	void UpdateCachedTextureResourceDesc( CMeshResourceDesc& desc ) const
	{
		desc.ResourcePath = ResourcePath;
	}

	void Serialize( IArchive& ar, const unsigned int version )
	{
		CGraphicsResourceDesc::Serialize( ar, version );

		ar & (int&)MeshType;
		ar & LoadOptionFlags;
		ar & NumVertices & NumIndices & VertexFormatFlags;
	}

	void LoadFromXMLNode( CXMLNodeReader& reader );
};


class CShaderResourceDesc : public CGraphicsResourceDesc
{
public:

	virtual GraphicsResourceType::Name GetResourceType() const { return GraphicsResourceType::Shader; }

	virtual boost::shared_ptr<CGraphicsResourceDesc> GetCopy() const { return boost::shared_ptr<CShaderResourceDesc>( new CShaderResourceDesc(*this) ); }

	int CanBeUsedAsShaderCache( const CShaderResourceDesc& desc ) const { return 0; }

	void UpdateCachedTextureResourceDesc( CMeshResourceDesc& desc ) const
	{
		desc.ResourcePath = ResourcePath;
	}

	void Serialize( IArchive& ar, const unsigned int version )
	{
		CGraphicsResourceDesc::Serialize( ar, version );
	}
};


//-------------------------------------- inline implementations --------------------------------------

//==================================================================================================
// CGraphicsResourceDesc and its derived classes
//==================================================================================================

inline CGraphicsResourceDesc::CGraphicsResourceDesc()
:
m_IsCachedResource(false),
LoadingMode(CResourceLoadingMode::SYNCHRONOUS),
LoadingPriority(0)
{}


inline CMeshResourceDesc::CMeshResourceDesc()
:
MeshType(CMeshType::BASIC),
NumVertices(0),
NumIndices(0),
VertexFormatFlags(0),
LoadOptionFlags(0)
{}


#endif  __GraphicsResourceDescs_H__
