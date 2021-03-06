#include "GLEmbeddedMiscGLSLShader.hpp"
#include "amorphous/Support/Log/DefaultLog.hpp"
#include <istream>
#include <fstream>


namespace amorphous
{


void GLEmbeddedMiscGLSLShader::GetSingleDiffuseColorVertexShader( std::string& shader )
{
	static const char *vs =
	"#version 330\n"\
	"layout(location = 0) in vec4 position;\n"\
	"layout(location = 2) in vec4 _dc;\n"\
	"out vec4 dc;\n"\
	"uniform mat4 ProjViewWorld;\n"\
	"void main(){gl_Position=ProjViewWorld*position;dc=_dc;}\n";

	shader = vs;
}


void GLEmbeddedMiscGLSLShader::GetPositionOnlyVertexShader( std::string& shader )
{
	static const char *vs =
	"#version 330\n"\
	"layout(location = 0) in vec4 position;\n"\
	"uniform mat4 ProjViewWorld;\n"\
	"void main(){gl_Position=ProjViewWorld*position;}\n";

	shader = vs;
}


void GLEmbeddedMiscGLSLShader::GetVertexWeightMapDisplayVertexShader( std::string& shader )
{
	// Adapted from GenerateNoLightingVertexShader()
	static const char *vs =
	"#version 330\n"\
	"#define NUM_COLORS 22\n"\
	"uniform vec4 colors[NUM_COLORS] ="\
	"{"\
		"vec4(1.00,0.25,0.25,1),"\
		"vec4(0.25,1.00,0.25,1),"\
		"vec4(0.25,0.25,1.00,1),"\
		"vec4(1.00,0.25,1.00,1),"\
		"vec4(0.25,1.00,1.00,1),"\
		"vec4(1.00,0.25,1.00,1),"\
		"vec4(0.25,0.25,0.25,1),"\
		"vec4(1.00,0.50,0.50,1),"\
		"vec4(0.50,1.00,0.50,1),"\
		"vec4(0.50,0.50,1.00,1),"\
		"vec4(1.00,0.50,1.00,1),"\
		"vec4(0.50,1.00,1.00,1),"\
		"vec4(1.00,0.50,1.00,1),"\
		"vec4(0.50,0.50,0.50,1),"\
		"vec4(1.00,0.75,0.75,1),"\
		"vec4(0.75,1.00,0.75,1),"\
		"vec4(0.75,0.75,1.00,1),"\
		"vec4(1.00,0.75,1.00,1),"\
		"vec4(0.75,1.00,1.00,1),"\
		"vec4(1.00,0.75,1.00,1),"\
		"vec4(0.75,0.75,0.75,1),"\
		"vec4(1,1,1,1)"\
	"};\n"\
	"layout(location = 0) in vec4 position;\n"\
	"layout(location = 1) in vec4 blend_weights;\n"\
	"layout(location = 2) in ivec4 blend_indices;\n"\
	"layout(location = 3) in vec3 normal;\n"\
	"layout(location = 4) in vec4 _dc;\n"\
	"out vec4 dc;\n"\
	"uniform mat4 ProjViewWorld;\n"\
	"void main()"\
	"{"\
		"gl_Position=ProjViewWorld*position;"\
		"dc=colors[ blend_indices[0] % NUM_COLORS ];"\
	"}\n";

	shader = vs;
}


void GLEmbeddedMiscGLSLShader::GetViewSpaceDepthRenderingVertexShader( std::string& shader )
{
	static const char *vs =
		"#version 330\n"\
		"layout(location = 0) in vec4 position;\n"\
		"out vec4 p;\n"\
		"uniform mat4 ViewWorld;\n"\
		"uniform mat4 ProjViewWorld;\n"\
		"void main(){vec4 pos=ProjViewWorld*position;gl_Position=pos;vec4 vp=ViewWorld*position;p.x=vp.z;p.y=vp.w;}\n";

	shader = vs;
}


void GLEmbeddedMiscGLSLShader::GetProjectionSpaceDepthRenderingVertexShader( std::string& shader )
{
	static const char *vs =
		"#version 330\n"\
		"layout(location = 0) in vec4 position;\n"\
		"out vec4 d;\n"\
		"uniform mat4 ProjViewWorld;\n"\
		"void main(){vec4 pos=ProjViewWorld*position;gl_Position=pos;d.x=pos.z;d.y=pos.w;}\n";

	shader = vs;
}


void GLEmbeddedMiscGLSLShader::GetSingleColorMembraneVertexShader( std::string& shader )
{
	static const char *vs =
	"#version 330\n"\
	"layout(location = 0) in vec4 position;\n"\
	"layout(location = 2) in vec4 _dc;\n"\
	"layout(location = 1) in vec3 normal;\n"\
	"out vec4 dc;\n"\
	"out vec3 vn;\n"\
	"uniform mat4 World;\n"\
	"uniform mat4 ViewWorld;\n"\
	"uniform mat4 ProjViewWorld;\n"\
	"void main(){gl_Position=ProjViewWorld*position;vn=mat3(ViewWorld)*normal;dc=_dc;}\n";
/*	"void main(){gl_Position=ProjViewWorld*position;vn=normal;dc=_dc;}\n";*/

	shader = vs;
}


void GLEmbeddedMiscGLSLShader::GetSingleDiffuseColorFragmentShader( std::string& shader )
{
	static const char *fs =
	"#version 330\n"\
	"in vec4 dc;\n"\
	"layout(location = 0) out vec4 fc;\n"\
	"void main(){"\
	"	fc = dc;\n"\
	"}\n";

	shader = fs;
}


void GLEmbeddedMiscGLSLShader::GetSingleFixedDiffuseColorFragmentShader( std::string& shader )
{
	static const char *fs =
	"#version 330\n"\
	"layout(location = 0) out vec4 fc;\n"\
	"void main(){fc=vec4(1.0,1.0,1.0,1.0);}\n";

	shader = fs;
}


void GLEmbeddedMiscGLSLShader::GetVertexWeightMapDisplayFragmentShader( std::string& shader )
{
	static const char *fs =
	"#version 330\n"\
	"in vec4 dc;\n"\
	"layout(location = 0) out vec4 fc;\n"\
	"void main(){"\
		"fc = dc;\n"\
	"}\n";

	shader = fs;
}


void GLEmbeddedMiscGLSLShader::GetViewSpaceDepthRenderingFragmentShader( std::string& shader )
{
	static const char *fs =
		"#version 330\n"\
		"in vec4 p;\n"\
		"layout(location = 0) out vec4 fc;\n"\
		"void main(){"\
		"fc.x = fc.y = fc.z = p.x / p.y;\n"\
		/*"fc.x = fc.y = fc.z = p.x / p.y * 0.3; // Scale the depth to see the results on the screen (for debugging)\n"\*/
		"fc.w = 1;\n"\
		"}\n";

	shader = fs;
}


void GLEmbeddedMiscGLSLShader::GetProjectionSpaceDepthRenderingFragmentShader( std::string& shader )
{
	static const char *fs =
		"#version 330\n"\
		"in vec4 d;\n"\
		"layout(location = 0) out vec4 fc;\n"\
		"void main(){"\
		"fc.x = fc.y = fc.z = d.x / d.y;\n"\
		/*"fc.x = fc.y = fc.z = d.x / d.y * 0.5; // Scale the depth to see the results on the screen (for debugging)\n"\*/
		"fc.w = 1;\n"\
		"}\n";

	shader = fs;
}


void GLEmbeddedMiscGLSLShader::GetSingleColorMembraneFragmentShader( std::string& shader )
{
	// Calculate the opacity based on the view space normal.
	// View space normal z = -1.0: facing directly toward the camera, thus opacity_factor = 1.0
	// View space normal z = 0 or higher: facing vertical or away from the camera, thus opacity_factor = 1.0
	static const char *fs =
	"#version 330\n"\
	// vn: view space normal
	"in vec3 vn;\n"\
	"in vec4 dc;\n"\
	"layout(location = 0) out vec4 fc;\n"\
	"uniform vec4 Color=vec4(1,1,0,1);\n"\
	"uniform float MinOpacity = 0.0;\n"\
	"uniform float MaxOpacity = 1.0;\n"\
	"void main(){"\
		"vec3 n = normalize(vn);"\
		"float f=1.0+clamp(n.z,-1.0,0.0);"\
		"float a = MinOpacity+f*(MaxOpacity-MinOpacity);\n"\
		/*"fc=vec4(vn.x,vn.y,vn.z,1);"\*/
		"fc=Color;"\
		"fc.a=a;"\
	"}\n";

	shader = fs;
}


std::string get_file_contents(const char *filename)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if( in.is_open() )
	{
		return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
	}

//	throw(errno);
	return "";
}


Result::Name GLEmbeddedMiscGLSLShader::GetVertexShader( MiscShader::ID shader_id, std::string& shader )
{
	switch( shader_id )
	{
	case MiscShader::SINGLE_DIFFUSE_COLOR:
		GetSingleDiffuseColorVertexShader(shader);
		break;
	case MiscShader::SINGLE_DIFFUSE_COLOR_WHITE:
		GetPositionOnlyVertexShader(shader);
		break;
	case MiscShader::SHADED_SINGLE_DIFFUSE_COLOR:
		GetSingleDiffuseColorVertexShader(shader);
		break;
	case MiscShader::VERTEX_WEIGHT_MAP_DISPLAY:
		GetVertexWeightMapDisplayVertexShader(shader);
		break;
	case MiscShader::DEPTH_RENDERING_IN_VIEW_SPACE:
		GetViewSpaceDepthRenderingVertexShader(shader);
		break;
	case MiscShader::DEPTH_RENDERING_IN_PROJECTION_SPACE:
		GetProjectionSpaceDepthRenderingVertexShader(shader);
		break;
	case MiscShader::SINGLE_COLOR_MEMBRANE:
		GetSingleColorMembraneVertexShader(shader);
		break;
	case MiscShader::ORTHO_SHADOW_MAP:
	case MiscShader::ORTHO_SHADOW_MAP_WITH_VERTEX_BLEND:
		shader = get_file_contents( "ShadowMapDemo/shaders/glsl/OrthoShadowMap.vert" );
		break;
	case MiscShader::SPOTLIGHT_SHADOW_MAP:
	case MiscShader::SPOTLIGHT_SHADOW_MAP_VERTEX_BLEND:
		shader = get_file_contents( "ShadowMapDemo/shaders/glsl/SpotlightShadowMap.vert" );
		break;
	case MiscShader::ORTHO_SHADOW_RECEIVER:
	case MiscShader::ORTHO_SHADOW_RECEIVER_WITH_VERTEX_BLEND:
	case MiscShader::SPOTLIGHT_SHADOW_RECEIVER:
	case MiscShader::SPOTLIGHT_SHADOW_RECEIVER_VERTEX_BLEND:
		shader = get_file_contents( "ShadowMapDemo/shaders/glsl/SpotlightShadowReceiver.vert" );
		break;
	default:
		LOG_PRINT_ERROR("Unsupported vertex shader type: " + to_string((int)shader_id));
		return Result::UNKNOWN_ERROR;
	}

	return Result::SUCCESS;
}


Result::Name GLEmbeddedMiscGLSLShader::GetFragmentShader( MiscShader::ID shader_id, std::string& shader )
{
	switch( shader_id )
	{
	case MiscShader::SINGLE_DIFFUSE_COLOR:
		GetSingleDiffuseColorFragmentShader(shader);
		break;
	case MiscShader::SINGLE_DIFFUSE_COLOR_WHITE:
		GetSingleFixedDiffuseColorFragmentShader(shader);
		break;
	case MiscShader::SHADED_SINGLE_DIFFUSE_COLOR:
		GetSingleDiffuseColorFragmentShader(shader);
		break;
	case MiscShader::VERTEX_WEIGHT_MAP_DISPLAY:
		GetVertexWeightMapDisplayFragmentShader(shader);
		break;
	case MiscShader::DEPTH_RENDERING_IN_VIEW_SPACE:
		GetViewSpaceDepthRenderingFragmentShader(shader);
		break;
	case MiscShader::DEPTH_RENDERING_IN_PROJECTION_SPACE:
		GetProjectionSpaceDepthRenderingFragmentShader(shader);
		break;
	case MiscShader::SINGLE_COLOR_MEMBRANE:
		GetSingleColorMembraneFragmentShader(shader);
		break;
	case MiscShader::ORTHO_SHADOW_MAP:
		shader = get_file_contents( "ShadowMapDemo/shaders/glsl/OrthoShadowMap.frag" );
		break;
	case MiscShader::SPOTLIGHT_SHADOW_MAP:
		shader = get_file_contents( "ShadowMapDemo/shaders/glsl/SpotlightShadowMap.frag" );
		break;
	case MiscShader::SPOTLIGHT_SHADOW_RECEIVER:
		shader = get_file_contents( "ShadowMapDemo/shaders/glsl/SpotlightShadowReceiver.frag" );
		break;
	default:
		LOG_PRINT_ERROR("Unsupported fragment shader type: " + to_string((int)shader_id));
		return Result::UNKNOWN_ERROR;
		break;
	}

	return Result::SUCCESS;
}


} // namespace amorphous
