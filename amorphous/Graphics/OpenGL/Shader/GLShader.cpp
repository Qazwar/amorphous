#include "GLShader.hpp"
#include "amorphous/Graphics/OpenGL/Shader/GLSLShaderLightManager.hpp"
#include "amorphous/Graphics/OpenGL/GLTextureResourceVisitor.hpp"
#include "amorphous/Support/lfs.hpp"
#include "amorphous/Support/SafeDelete.hpp"


#define GL_GEOMETRY_SHADER                0x8DD9


namespace amorphous
{

using namespace std;
using namespace boost;

/*

{
	ShaderHandle m_Shader;
	CMeshHandle m_Mesh;
}
{
	bool res = m_Shader.Load( "shaer.vert|shaer.frag" );

	m_Mesh.Load( "mesh.msh" );

	// ...

	m_Mesh.Render( m_Shader );
}

*/

int read_text_file( const std::string& filepath, std::vector<char>& buffer )
{
    if (filepath.length() == 0)
		return -1;

	if( !lfs::path_exists(filepath) )
	{
	//	fprintf(stderr,"Cannot open \"%s\" for stat read!\n", filename);
		return -1;
	}

	long size = lfs::get_filesize(filepath);

//	char * buf = new char[size+1];
	buffer.resize( size + 1, 0 );

	FILE *fp = fopen( filepath.c_str(), "rb" ); // Check if the file should be opened with "rb" or "r" option
	if ( !fp )
	{
//		fprintf(stderr,"Cannot open \"%s\" for read!\n", filepath.c_str());
		return -1;
	}

	size_t bytes = fread( &(buffer[0]), 1, size, fp );

	buffer[bytes] = 0;

	fclose(fp);
	return 0;
}

/*
// OpenGL 2.0 syntax:

void setShaders()
{

	char *vs,*fs;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);	

	vs = textFileRead("toon.vert");
	fs = textFileRead("toon.frag");

	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(v, 1, &vv,NULL);
	glShaderSource(f, 1, &ff,NULL);

	free(vs);free(fs);

	glCompileShader(v);
	glCompileShader(f);

	p = glCreateProgram();
	
	glAttachShader(p,v);
	glAttachShader(p,f);

	glLinkProgram(p);
	glUseProgram(p);
}
*/

// ARB extension syntax:
#if 0
void setShaders()
{

	char *vs,*fs;

	v = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	f = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);	

	vs = textFileRead("toon.vert");
	fs = textFileRead("toon.frag");

	const char * vv = vs;
	const char * ff = fs;

	glShaderSourceARB(v, 1, &vv,NULL);
	glShaderSourceARB(f, 1, &ff,NULL);

	free(vs);free(fs);

	glCompileShaderARB(v);
	glCompileShaderARB(f);

	p = glCreateProgramObjectARB();
	
	glAttachObjectARB(p,v);
	glAttachObjectARB(p,f);

	glLinkProgramARB(p);
	glUseProgramObjectARB(p);
}
#endif /* 0 */



static const char *sg_MinimalVertexShader =
"#version 330\n"\
"layout(location = 0) in vec4 position;\n"\
"uniform mat4 ProjViewWorld;\n"\
"void main(){gl_Position = ProjViewWorld * position;}\n";


static const char *sg_MinimalFragmentShader =
"#version 330\n"\
"out vec4 o_color;\n"\
"void main(){o_color = vec4(1,1,1,1);}\n";


CGLShader::CGLShader()
:
m_Shader(0)
{
}


CGLShader::~CGLShader()
{
	Release();
}


bool CGLShader::LoadFromFile( const std::string& filepath )
{
	Release();

	if( glCreateShader )
		m_Shader = glCreateShader( GetShaderType() );
	else if( glCreateShaderObjectARB )
		m_Shader = glCreateShaderObjectARB( GetShaderType() );
	else
		return false;

	vector<char> buffer;
	int ret = read_text_file( filepath, buffer ); // load .vert or .frag file here
	if( ret != 0 || buffer.empty() )
	{
		return false;
	}

	return CreateShader( &buffer[0] );
}


static void GetCompileStatus( GLenum shader_type, GLhandleARB shader, std::string& error_info )
{
	GLint status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	LOG_GL_ERROR( " glGetShaderiv() failed." );
	if (status == GL_FALSE)
	{
		GLint infoLogLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		if( infoLogLength == 0 )
			return;

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
		const char *strShaderType = "";
		switch(shader_type)
		{
		case GL_VERTEX_SHADER:   strShaderType = "vertex";   break;
		case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
		case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		}

		error_info = fmt_string( "Compile failure in %s shader:\n", strShaderType );
		error_info += strInfoLog;

		delete[] strInfoLog;
	}
}


bool CGLShader::CreateShader( const char *source )
{
	LOG_GL_ERROR( " Entered." );

	Release();

	m_Shader = glCreateShader( GetShaderType() );

	if( !source || strlen(source) == 0 )
		return false;

//	const char * vv = vs;

	string error_info;

	const char *pBuffer = source;
	glShaderSource( m_Shader, 1, &pBuffer, NULL );

	LOG_GL_ERROR( "glShaderSourceARB() failed." );

	glCompileShader( m_Shader );

	LOG_GL_ERROR( "glCompileShaderARB() failed. Failed to compile a shader" );

	GetCompileStatus( GetShaderType(), m_Shader, error_info );

	if( 0 < error_info.length() )
		LOG_PRINT_ERROR( error_info );

	return true;
}


bool CGLShader::LoadShaderFromText( const stream_buffer& buffer )
{
	return false;
}


void CGLShader::Release()
{
	if( 0 < m_Shader )
	{
		glDeleteShader( m_Shader );
		m_Shader = 0;
	}
}


void CGLShader::Reload()
{
}


CGLFragmentShader::CGLFragmentShader()
{
	for( int i=0; i<GLSL_NUM_MAX_TEXTURE_STAGES; i++ )
		m_aTextureLocation[i] = 0;
}


void CGLFragmentShader::InitGLShader()
{
//	if( !glGetUniformLocation ) glGetUniformLocation = glGetUniformLocationARB;
//	if( !glUniform1i )          glUniform1i          = glUniform1iARB;

	char sampler_name[16];
	memset( sampler_name, 0, sizeof(sampler_name) );
	for( int i=0; i<GLSL_NUM_MAX_TEXTURE_STAGES; i++ )
	{
		sprintf( sampler_name, "tex%d", i );
		m_aTextureLocation[i] = glGetUniformLocation( m_Shader, sampler_name );

		glUniform1i( m_aTextureLocation[i], i );
	}
}


//=============================================================================
// CGLProgram
//=============================================================================

bool sg_UseGRM = false;

CGLProgram::CGLProgram()
:
m_Program(0),
m_pVertexShader(NULL),
m_pFragmentShader(NULL)
{
	for( int i=0; i<NUM_PREDEFINED_MATRIX_UNIFORMS; i++ )
		m_PredefinedMatrixUniforms[i] = -1;

	m_pLightManager.reset( new CGLSLShaderLightManager );
}


CGLProgram::~CGLProgram()
{
	Release();
}


bool CGLProgram::LoadShaderFromFile( const std::string& filename )
{
	size_t separator_pos = filename.find( "|" );
	if( separator_pos == string::npos
	 || filename.length() - 1 <= separator_pos )
	{
		return false;
	}

	const string vs_path = filename.substr( 0, separator_pos );
	const string fs_path = filename.substr( separator_pos + 1 ); // range is checked by the code above

	bool vs_loaded = false, fs_loaded = false;
	if( sg_UseGRM )
	{
		// use handles

//		ShaderHandle m_VertexShader;
//		ShaderHandle m_FragmentShader;

		ShaderResourceDesc vs_desc, fs_desc;

		// vertex program
		vs_desc.ResourcePath = vs_path;
		vs_desc.ShaderType = ShaderType::VERTEX_SHADER;
		vs_loaded = m_VertexShader.Load( vs_desc );

		// fragment program
		fs_desc.ResourcePath = fs_path;
		vs_desc.ShaderType = ShaderType::PIXEL_SHADER;
		fs_loaded = m_FragmentShader.Load( fs_desc );

//		m_pVertexShader   = dynamic_pointer_cast<CGLVertexShader,ShaderManager>( m_VertexShader.GetShaderManager() );
//		m_pFragmentShader = dynamic_pointer_cast<CGLFragmentShader,ShaderManager>( m_FragmentShader.GetShaderManager() );
		m_pVertexShader   = dynamic_cast<CGLVertexShader *>( m_VertexShader.GetShaderManager() );
		m_pFragmentShader = dynamic_cast<CGLFragmentShader *>( m_FragmentShader.GetShaderManager() );

	}
	else
	{
		// These vertex and fragment shaders are managed by this shader manager
		// and not registered to graphics resource manager (GRM)
		// rationale: shader manager classes are modules that belongs to layer lower than that of GRM
		// but SetTexture function of shader manager takes texture handle as its argument...
//		m_pVertexShader   = shared_ptr<CGLVertexShader>( new CGLVertexShader );
//		m_pFragmentShader = shared_ptr<CGLFragmentShader>( new CGLFragmentShader );
		m_pVertexShader   = new CGLVertexShader;
		m_pFragmentShader = new CGLFragmentShader;

		// how to share vertex & fragment shaders between different shader managers

		vs_loaded = m_pVertexShader->LoadFromFile( vs_path );
		fs_loaded = m_pFragmentShader->LoadFromFile( fs_path );
	}

	if( !vs_loaded )
		LOG_PRINT_ERROR( "Failed to load a vertex shader: " + vs_path );

	if( !fs_loaded )
		LOG_PRINT_ERROR( "Failed to load a fragment shader: " + fs_path );

	if( !vs_loaded || !fs_loaded )
	{
		bool min_shaders_loaded = LoadMimalShaders();
		if( min_shaders_loaded )
			return false;
	}

	if( !m_pVertexShader || !m_pFragmentShader )
		return false;

	Result::Name res = InitProgram();

	return (res == Result::SUCCESS) ? true : false;
}


Result::Name CGLProgram::InitProgram()
{
	if( !m_pVertexShader || !m_pFragmentShader )
		return Result::UNKNOWN_ERROR;

	m_Program = glCreateProgramObjectARB();

	LOG_GL_ERROR( " glCreateProgramObjectARB() failed." );

	glAttachShader( m_Program, m_pVertexShader->GetGLHandle() );

	LOG_GL_ERROR( " glAttachShader() failed." );

	glAttachShader( m_Program, m_pFragmentShader->GetGLHandle() );

	LOG_GL_ERROR( " glAttachShader() failed." );

	glLinkProgram( m_Program );

	LOG_GL_ERROR( " glLinkProgram() failed." );

//	glUseProgramObjectARB( m_Program );

	InitUniforms();

	if( m_pLightManager )
		m_pLightManager->Init( m_Program );

	LOG_GL_ERROR( "Detected a GL error at the end of the function." );

	return Result::SUCCESS;
}


bool CGLProgram::LoadMimalShaders()
{
	SafeDelete( m_pVertexShader );
	SafeDelete( m_pFragmentShader );

	m_pVertexShader   = new CGLVertexShader;
	m_pFragmentShader = new CGLFragmentShader;

	bool min_vs_created = m_pVertexShader->CreateShader( sg_MinimalVertexShader );
	bool min_fs_created = m_pFragmentShader->CreateShader( sg_MinimalFragmentShader );

	return (min_vs_created && min_fs_created);
}


bool CGLProgram::LoadShaderFromText( const stream_buffer& buffer )
{
	return false;
}


bool CGLProgram::LoadShaderFromText( const char *vertex_shader, const char *fragment_shader )
{
	SafeDelete( m_pVertexShader );
	SafeDelete( m_pFragmentShader );

	m_pVertexShader   = new CGLVertexShader;
	m_pFragmentShader = new CGLFragmentShader;

	bool vs_created = m_pVertexShader->CreateShader( vertex_shader );
	bool fs_created = m_pFragmentShader->CreateShader( fragment_shader );

	if( !vs_created || !fs_created )
	{
	}

	Result::Name res = InitProgram();

	return (res == Result::SUCCESS) ? true : false;
}


void CGLProgram::Release()
{
	// release m_Program

	if( sg_UseGRM )
	{
		m_pVertexShader = NULL;
		m_pFragmentShader = NULL;
		m_VertexShader.Release();
		m_FragmentShader.Release();
	}
	else
	{
		SafeDelete( m_pVertexShader );
		SafeDelete( m_pFragmentShader );
	}
}


void CGLProgram::Reload()
{
}


void CGLProgram::SetViewerPosition( const Vector3& vEyePosition )
{
}


void CGLProgram::SetVertexBlendMatrix( int i, const Matrix34& mat )
{
//	if( m_pVertexShader )
//		m_pVertexShader
}


void CGLProgram::SetVertexBlendMatrix( int i, const Matrix44& mat )
{
//	if( m_pVertexShader )
//		m_pVertexShader
}


void CGLProgram::SetVertexBlendTransforms( const std::vector<Transform>& src_transforms )
{
//	if( m_pVertexShader )
//		m_pVertexShader
}



Result::Name CGLProgram::SetTexture( const int iStage, const TextureHandle& texture )
{
	LOG_GL_ERROR( " Entered." );

	if( glActiveTexture )
		glActiveTexture( GL_TEXTURE0 + iStage );
	else if( glActiveTextureARB )
		glActiveTextureARB( GL_TEXTURE0_ARB + iStage );

	string error_message = fmt_string( " glActiveTexture() failed (stage: %d).", iStage );
	LOG_GL_ERROR( error_message.c_str() );

	glEnable( GL_TEXTURE_2D );

	// glMultiTexCoord2fARB

//	glBindTexture( GL_TEXTURE_2D, texture.GetGLTextureID() );
	SetTextureGL_FFP( iStage, texture );

	return Result::UNKNOWN_ERROR;
}


void CGLProgram::Begin()
{
	LOG_GL_ERROR( " Entered." );

	if( m_Program == 0 )
		return;

//	glLinkProgram( m_Program );

	glUseProgram( m_Program );

	LOG_GL_ERROR( " glUseProgram() failed." );
}


void CGLProgram::End()
{
}


void CGLProgram::SetParam( ShaderParameter<int>& int_param )
{
}


void CGLProgram::SetParam( ShaderParameter<float>& float_param )
{
}


void CGLProgram::SetParam( ShaderParameter<Vector3>& vec3_param )
{
}


void CGLProgram::SetParam( ShaderParameter<SFloatRGBAColor>& color_param )
{
}


void CGLProgram::SetParam( ShaderParameter< std::vector<float> >& float_param )
{
}


void CGLProgram::SetParam( ShaderParameter<TextureParam>& tex_param )
{
}


void CGLProgram::SetParam( ShaderParameter<Matrix44>& mat44_param )
{
}


void CGLProgram::SetParam( const char *parameter_name, int int_param )
{
}


void CGLProgram::SetParam( const char *parameter_name, float float_param )
{
	GLint location = glGetUniformLocation( m_Program, parameter_name );
	glUseProgram( m_Program );
	if( 0 <= location )
		glUniform1f( location, float_param );
}


void CGLProgram::SetParam( const char *parameter_name, const Vector3& vec3_param )
{
	GLint location = glGetUniformLocation( m_Program, parameter_name );
	glUseProgram( m_Program );
//	if( 0 <= location )
//		glUniform3fv( location, (const float *)&vec3_param );
}


void CGLProgram::SetParam( const char *parameter_name, const SFloatRGBAColor& color_param )
{
}


void CGLProgram::SetParam( const char *parameter_name, const float *float_param, uint num_float_values )
{
}


void CGLProgram::SetParam( const char *parameter_name, const Matrix44& mat44_param )
{
}


boost::shared_ptr<ShaderLightManager> CGLProgram::GetShaderLightManager()
{
	return m_pLightManager;
}


void CGLProgram::InitUniforms()
{
	glUseProgram( m_Program );

	// Return values of glGetUniformLocation() will be -1 if
	// 1. The matrix is not defined in the shader.
	// 2. The matrix is defined in the shader, but not used (true?).
	m_PredefinedMatrixUniforms[MATRIX_WORLD]           = glGetUniformLocation( m_Program, "World" );
	m_PredefinedMatrixUniforms[MATRIX_VIEW]            = glGetUniformLocation( m_Program, "View" );
	m_PredefinedMatrixUniforms[MATRIX_PROJ]            = glGetUniformLocation( m_Program, "Proj" );
	m_PredefinedMatrixUniforms[MATRIX_VIEW_WORLD]      = glGetUniformLocation( m_Program, "ViewWorld" );
	m_PredefinedMatrixUniforms[MATRIX_PROJ_VIEW_WORLD] = glGetUniformLocation( m_Program, "ProjViewWorld" );
}

/*
//==================================================================================
// 
//==================================================================================
void CGLProgramBase::LoadShaderFromFile( const std::string& filepath )
{
	vector<char> buffer;
	int ret = read_text_file( filepath, buffer );
	if( ret != 0 )
		return;

	GLenum program_type = GetProgramType();
	glGenProgramsARB(1, &m_ProgramID);
	glBindProgramARB(program_type, m_ProgramID);
//	glProgramStringARB(program_type, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei) strlen(code), (GLubyte *) code);
	glProgramStringARB(program_type, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei) buffer.size(), (GLubyte *) &(buffer[0]));

	GLint error_pos;
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_pos);
	if (error_pos != -1)
	{
		const GLubyte *error_string;
		error_string = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
		string error_msg = fmt_string( "%s program error at position: %d\n%s\n",
			(program_type == GL_VERTEX_PROGRAM_ARB) ? "vertex" : "fragment",
			error_pos, error_string);

		LOG_PRINT_ERROR( error_msg );
	}

//	return program_id;
}


GLenum CGLVertexProgram::GetProgramType() const { return GL_VERTEX_PROGRAM_ARB; }


GLenum CGLFragmentProgram::GetProgramType() const { return GL_FRAGMENT_PROGRAM_ARB; }
*/

/*
void InitVertexBlendTransforms(  )
{
}


void SetVertexBlendMatrices(  )
{
	char name[128];
	memset( name, 0, sizeof(name) );
	sprintf( name, "g_BlendTransforms" );

	GLint m_Locaiton = glGetUniformLocation( program, name );
}


void SetVertexBlendTransforms(  )
{
	glUniform4f( m_Locaiton, f0, f1, f2, f3 );

	float values[] = {...};
	int count = 1;
	glUniform4fv( m_Locaiton, count, values );
}
*/


} // namespace amorphous
