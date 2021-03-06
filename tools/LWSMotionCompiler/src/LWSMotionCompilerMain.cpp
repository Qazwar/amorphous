#include "LWSMotionCompiler.hpp"
#include "amorphous/MotionSynthesis/BVHMotionDatabaseCompiler.hpp"
#include "amorphous/base.hpp"
#include "amorphous/Support/lfs.hpp"
#include "amorphous/Support/ParamLoader.hpp"
#include "amorphous/Support/FileOpenDialog_Win32.hpp"
#include "amorphous/Support/MiscAux.hpp"
#include "amorphous/Support/Log/DefaultLogAux.hpp"

using namespace std;
using namespace boost::filesystem;
using namespace amorphous::msynth;






/*
bool ends_with( const std::string& target, const std::string& end )
{
	if( target.substr( target.length() - end.length(), end.length() );
}
*/

int main( int argc, char *argv[] )
{
	using namespace filesystem;

	path init_wd = lfs::get_cwd();

	if( init_wd.leaf() != "app" )
		lfs::set_wd( "../../app" );

	path app_wd = lfs::get_cwd();

	// Search an input filepath
	// 1. command line argument
	// 2. text file named "params.txt"
	// 3. File Open dialog
	string filepath;
	if( 2 <= argc )
		filepath  = argv[1];
	else
	{
		filepath = LoadParamFromFile<string>( "params.txt", "MotionDescFile" );

		if( filepath.length() == 0 )
			GetFilename( filepath );
	}

	if( filepath.length() == 0 )
		return 0;

	// Open a file for logging
	path dirpath = path(filepath).parent_path();
	string log_filename = "log_" + string(GetBuildInfo()) + "-" + path(filepath).leaf().string() + ".html";
	path html_log_filepath = dirpath / log_filename;
	InitHTMLLog( html_log_filepath.string() );

//	shared_ptr<CMotionPrimitiveCompilerCreator> pBVHCompilerCreator( new CBVHMotionPrimitiveCompilerCreator );
//	RegisterMotionPrimitiveCompilerCreator( pBVHCompilerCreator );

	shared_ptr<MotionPrimitiveCompilerCreator> pLWSCompilerCreator( new CLWSMotionPrimitiveCompilerCreator );
	RegisterMotionPrimitiveCompilerCreator( pLWSCompilerCreator );

	/// init the xml parser (calls Initialize() in ctor)
//	CXMLParserInitReleaseManager xml_parser_mgr;

	msynth::MotionDatabaseBuilder mdb;
	mdb.Build( filepath );

//	CLWSMotionCompiler test;
//	test.BuildFromDescFile( filepath );

	return 0;
}
