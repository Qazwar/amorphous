#ifndef  __GLCustomMeshRenderer_HPP__
#define  __GLCustomMeshRenderer_HPP__


#include "amorphous/Graphics/Mesh/CustomMeshRenderer.hpp"


namespace amorphous
{


class CGLCustomMeshRenderer : public CustomMeshRenderer
{
	void RenderMeshWithCurrentProgram( CustomMesh& mesh );

public:

	void RenderMesh( CustomMesh& mesh );

	void RenderSubset( CustomMesh& mesh, int subset_index );

	void RenderZSortedMesh( CustomMesh& mesh ) { RenderMesh( mesh ); }

	void RenderMesh( CustomMesh& mesh, ShaderManager& shader_mgr );

	void RenderSubset( CustomMesh& mesh, ShaderManager& shader_mgr, int subset_index );

	void RenderZSortedMesh( CustomMesh& mesh, ShaderManager& shader_mgr ) { RenderMesh( mesh, shader_mgr ); }

	static CGLCustomMeshRenderer ms_Instance;
};

} // namespace amorphous



#endif /* __GLCustomMeshRenderer_HPP__ */