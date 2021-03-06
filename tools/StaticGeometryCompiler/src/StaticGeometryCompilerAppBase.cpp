//-----------------------------------------------------------------------------
// File: StaticGeometryCompiler_Base.cpp
//-----------------------------------------------------------------------------

//#include <vld.h>

#include "amorphous/Graphics.hpp"
#include "amorphous/Graphics/LogOutput_OnScreen.hpp"
#include "amorphous/Support.hpp"
#include "amorphous/Support/FileOpenDialog_Win32.hpp"
#include "amorphous/Support/MiscAux.hpp"
#include "amorphous/Support/lfs.hpp"
#include "amorphous/Support/Log/LogOutput.hpp"
#include "amorphous/Stage/StaticGeometry.hpp"
#include "amorphous/XML/XMLDocumentLoader.hpp"

#include "StaticGeometryCompilerFG.h"
#include "StaticGeometryCompiler.h"
#include "StaticGeometryCompiler_Main.h"

using namespace std;


bool CompileStaticGeometry( const string& filename )
{
	if( filename.substr( filename.length() - 3 ) == "xml" )
	{
		// compile a static geometry archive

		CXMLParserInitReleaseManager xml_parser_mgr;

		// compile static geometry
		LOG_SCOPE( "- static geomery compiler test." );
		CStaticGeometryCompiler compiler;
		bool compiled = compiler.CompileFromXMLDescFile( filename );

		if( compiled )
		{
/*			// go to the directory of the input xml file
			lfs::set_wd(lfs::get_path(filename));

			// go to the directory where a static geometry file was saved
			string output_filepath = compiler.GetDesc().m_OutputFilepath;
			lfs::set_wd( lfs::get_path(output_filepath) );

			m_OutputFilepath = output_filepath;

//			g_pTest = std::shared_ptr<CStaticGeometryViewer>( new CStaticGeometryViewer() );
//			g_pTest->LoadFromFile( lfs::get_nopath(output_filepath) );*/
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		// compile a static geometry archive of the previous version
		// - create from a .rd (resource desc) file
		CStaticGeometryCompilerFG compiler_fg;
		return compiler_fg.Build( filename );
	}

	return true;
}


/// Returns true upon successful completion of a static geometry
bool RunStaticGeometryCompiler( const std::string& cmd_line,
							   const std::string& initial_working_directory )
{
	// output log text on the screen
//	g_pLogOutput = new LogOutput_ScrolledTextBuffer( "Arial", 6, 12, 24, 120 );
//	GlobalLog().AddLogOutput( g_pLogOutput );
//	g_pLogOutput->SetTopLeftPos( Vector2(8,16) );

//	SetDefaultSkyboxMesh( compiler_fg );


//	if( initial_working_directory.find( "StaticGeometryCompiler\\vsp" ) != string::npos )
	if( initial_working_directory.find( "StaticGeometryBuilder\\vsp\\VC8" ) != string::npos )
	{
		// the working directory is the same with the project file directory
		// - Happens when the app is run from the Visual Studio
		// - Change the working directory to the directory of the application binary.
		lfs::set_wd( "../../app" );
	}

	// File open dialog is not working. Why???
	bool win32_file_open_dialog_is_available = false;

	string filename;
	if( 0 < cmd_line.length() )
	{
		// a filename is given as an argument

		if( cmd_line[0] == '"' )
		{
			// trim the double quotation characters at the beginning and the end of the string
			filename = cmd_line.substr( 1, cmd_line.length() - 2 );
		}
		else
		{
			filename = cmd_line;
		}

		// set the working directory to the one immediately under the desc file

        lfs::set_wd( lfs::get_parent_path(filename) );

		filename = lfs::get_leaf( filename );
	}
	else
	{
		// no command line arguments
		if( win32_file_open_dialog_is_available )
		{
			// select a desc file from OpenFile dialog
			if( !GetFilename(filename, NULL) )
				return 0;
		}
		else
		{
			// select a desc file written in "./default_input"
			// - see "D:\R&D\Project\App2\StaticGeometryCompiler"
			ParamLoader loader( "default_input.txt" );
			if( loader.IsReady() )
			{
				loader.LoadParam( "input", filename );
			}

			lfs::set_wd( lfs::get_parent_path(filename) );

			filename = lfs::get_leaf( filename );
		}
	}

	// set log output device
	// - GetFilename() has changed working directory to the directory where the selected file exists.
	//   Open the log file in the same directory.
	//   
	string log_filename = string("log_") + GetBuildInfo();
	LogOutput_HTML html_log( log_filename + ".html" );
	GlobalLog().AddLogOutput( &html_log );

	LogOutput_TextFile textfile_log( log_filename + ".txt" );
	GlobalLog().AddLogOutput( &textfile_log );

	LOG_PRINT( "initial working directory: " + initial_working_directory );

	LOG_PRINT( "current working directory: " + lfs::get_cwd() );

	LOG_PRINT( "command line argument string: " + cmd_line );

	LOG_PRINTF(( "Compiling a static geometry from the following desc file: '%s'", filename.c_str() ));

//	GlobalLog().Print( "directory path of exe file: " + lfs::get_path(get_exe_filepath()) );

//	GlobalLog().Print( "_fullpath( dest, '.', MAX_PATH ) > dest: '%s'", full_path );

//	filename = "./testdata/static_geometry_desc_draft.xml";

	bool compiled = false;

	// build static geometry
	if( 0 < filename.length() )
	{
		try
		{
			compiled = CompileStaticGeometry( filename );
		}
		catch( std::exception& e )
		{
			GlobalLog().Print( WL_ERROR, "exception: %s", e.what() );
		}
	}

	bool ret = false;
	if( compiled )
	{
		LOG_PRINT( "Compiled a static geometry from the following file: " + filename );
		ret = true;
	}
	else
	{
		ret = false;
	}

	GlobalLog().RemoveLogOutput( &html_log );
	GlobalLog().RemoveLogOutput( &textfile_log );

	return ret;
}
