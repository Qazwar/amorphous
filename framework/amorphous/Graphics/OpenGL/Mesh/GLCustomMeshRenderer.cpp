#include "GLCustomMeshRenderer.hpp"
#include "amorphous/Graphics/ShaderHandle.hpp"
#include "amorphous/Graphics/Mesh/CustomMesh.hpp"
#include "amorphous/Graphics/Shader/ShaderManager.hpp"
#include "amorphous/Graphics/Shader/GenericShaderHelpers.hpp"
#include "amorphous/Graphics/OpenGL/GLGraphicsDevice.hpp"
//#include "amorphous/Graphics/OpenGL/GLTextureResourceVisitor.hpp"
#include "amorphous/Support/Log/DefaultLog.hpp"
#include "amorphous/Support/Profile.hpp"
#include "amorphous/Support/Macro.h"


namespace amorphous
{


using namespace std;


GLCustomMeshRenderer GLCustomMeshRenderer::ms_Instance;


void GLCustomMeshRenderer::RenderMeshWithSpecifiedProgram( CustomMesh& mesh, ShaderManager& shader_mgr )
{
	PERIODICAL( 500, LOG_PRINT_VERBOSE("entered") );

	LOG_GL_ERROR( " Clearing OpenGL errors..." );

	// NOTE: it seems that the fixed function pipeline does not work
	// with glEnableVertexAttribArray() & glVertexAttribPointer().

	PROFILE_FUNCTION();

	uchar *pV = mesh.GetVertexBufferPtr();
	if( !pV )
		return;

	uchar *pI = mesh.GetIndexBufferPtr();
	if( !pI )
		return;

//	const uint num_indices   = mesh.GetNumIndices();
//	const uint num_triangles = num_indices / 3;

	const uint num_verts = mesh.GetNumVertices();
	const int vertex_size = mesh.GetVertexSize();

	const U32 vert_flags = mesh.GetVertexFormatFlags();

	static const bool use_vertex_attrib_array_and_ptr = true;

	// Unbind GL_ARRAY_BUFFER and GL_ELEMENT_ARRAY_BUFFER to source a standard memory location (RAM).
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	LOG_GL_ERROR( " glBindBuffer() failed." );

	if( vert_flags & VFF::POSITION )
	{
		uchar *pPos = pV + mesh.GetVertexElementOffset( VEE::POSITION );

		if( use_vertex_attrib_array_and_ptr )
		{
			//LOG_GL_ERROR( "glEnableVertexAttribArray( 0 )" );
			glEnableVertexAttribArray( 0 );
			//LOG_GL_ERROR( "glVertexAttribPointer( 0 )" );
			glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, vertex_size, pPos );
		}
		else
		{
			//glEnableClientState(GL_VERTEX_ARRAY);
			//glVertexPointer( 3, GL_FLOAT, vertex_size, pPos );
		}
	}
	else
	{
		LOG_PRINT_ERROR( " A vertex must at least have a position." );
		return;
	}


	if( vert_flags & VFF::NORMAL )
	{
		uchar *pNormal = pV + mesh.GetVertexElementOffset( VEE::NORMAL );

		if( use_vertex_attrib_array_and_ptr )
		{
			//LOG_GL_ERROR( "glEnableVertexAttribArray( 1 )" );
			glEnableVertexAttribArray( 1 );
			glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, vertex_size, pNormal );
		}
		else
		{
			//glDisableClientState(GL_NORMAL_ARRAY);
//			glEnableClientState(GL_NORMAL_ARRAY);
//			glNormalPointer( GL_FLOAT, vertex_size, pNormal );
		}
	}

	if( vert_flags & VFF::DIFFUSE_COLOR )
	{
		uchar *pDiffuseColor = pV + mesh.GetVertexElementOffset( VEE::DIFFUSE_COLOR );

		if( use_vertex_attrib_array_and_ptr )
		{
			//LOG_GL_ERROR( "glEnableVertexAttribArray( 2 )" );
			glEnableVertexAttribArray( 2 );
			glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE, vertex_size, pDiffuseColor );
		}
		else
		{
			//glDisableClientState(GL_COLOR_ARRAY);
//			glEnableClientState(GL_COLOR_ARRAY);
//			glColorPointer( 4, GL_FLOAT, vertex_size, pDiffuseColor );
		}
	}

//	if(mTexId)
	if( vert_flags & VFF::TEXCOORD2_0 )
	{
		uchar *pTex = pV + mesh.GetVertexElementOffset( VEE::TEXCOORD2_0 );

		if( use_vertex_attrib_array_and_ptr )
		{
			//LOG_GL_ERROR( "glEnableVertexAttribArray( 3 )" );
			glEnableVertexAttribArray( 3 );
			glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, vertex_size, pTex );
		}
		else
		{
			//glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//			glTexCoordPointer( 2, GL_FLOAT, vertex_size, pTex );
		}

		// Commented out; we don't need glEnable(GL_TEXTURE_2D) unless we use FFP.
		// In fact, this causes the GL_INVALID_ENUM error on OpenGL ES so FFP simply seems to be non-option.
		//LOG_GL_ERROR( "Calling glEnable()" );
		//glEnable(GL_TEXTURE_2D);
		//LOG_GL_ERROR( "Did glEnable(GL_TEXTURE_2D) cause an error? (hint: it would if your app runs on GL ES)" );

//		glBindTexture(GL_TEXTURE_2D, mTexId);
		/*glColor4f(1.0f, 1.0f, 1.0f,1.0f);*/
	}

	const uint index_size = mesh.GetIndexSize();
	uint index_type = GL_UNSIGNED_BYTE;
	switch( index_size )
	{
	case 1: index_type = GL_UNSIGNED_BYTE;  break;
	case 2: index_type = GL_UNSIGNED_SHORT; break;
	case 4: index_type = GL_UNSIGNED_INT;   break;
	default:
		break;
	}

	const int num_mats = mesh.GetNumMaterials();
	for( int i=0; i<num_mats; i++ )
	{
		const MeshMaterial& mat = mesh.GetMaterial(i);
		for( size_t j=0; j<mat.Texture.size(); j++ )
		{
//			SetTextureGL( j, mat.Texture[j] );
			shader_mgr.SetTexture( j, mat.Texture[j] );
		}

		int index_offsets_in_bytes        = mesh.GetTriangleSets()[i].m_iStartIndex * index_size;
		int num_indices_of_current_subset = mesh.GetTriangleSets()[i].m_iNumTriangles * 3;

//		if( use_draw_range_elements )
//		{
//			glDrawRangeElements( GL_TRIANGLES, 0, num_verts-1, num_triangles, index_type, pI );
//		}
//		else
		{
			LOG_GL_ERROR( "calling glDrawElements().")
			PROFILE_SCOPE( "glDrawElements( GL_TRIANGLES, num_indices, index_type, pI )" );
//			glDrawElements( GL_TRIANGLES, num_indices, index_type, pI );
			glDrawElements( GL_TRIANGLES, num_indices_of_current_subset, index_type, pI + index_offsets_in_bytes );
		}
	}

	if( vert_flags & VFF::TEXCOORD2_0 )
	{
		//glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//		glDisable(GL_TEXTURE_2D);
	}

//	if( vert_flags & VFF::DIFFUSE_COLOR )
//		glDisableClientState(GL_COLOR_ARRAY);
//	if( vert_flags & VFF::NORMAL )
//		glDisableClientState(GL_NORMAL_ARRAY);
//	glDisableClientState(GL_VERTEX_ARRAY);

	LOG_GL_ERROR( "post-draw GL error clearing.")
}


void GLCustomMeshRenderer::RenderSubset( CustomMesh& mesh, int subset_index )
{
	LOG_PRINT_ERROR( " Not implemented." );
}


void GLCustomMeshRenderer::RenderMesh( CustomMesh& mesh )
{
	glUseProgram( 0 );

	ShaderHandle shader = CreateNoLightingShader();
	ShaderManager *pShaderMgr = shader.GetShaderManager();
	if( pShaderMgr )
		RenderMeshWithSpecifiedProgram( mesh, *pShaderMgr );
}


void GLCustomMeshRenderer::RenderMesh( CustomMesh& mesh, ShaderManager& shader_mgr )
{
//	LOG_PRINT_ERROR( " Not implemented." );

	shader_mgr.Begin();

	RenderMeshWithSpecifiedProgram( mesh, shader_mgr );
}


void GLCustomMeshRenderer::RenderSubset( CustomMesh& mesh, ShaderManager& shader_mgr, int subset_index )
{
	LOG_PRINT_ERROR( " Not implemented." );
}


} // namespace amorphous
