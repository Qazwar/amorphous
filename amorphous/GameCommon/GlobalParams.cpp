#include "GlobalParams.hpp"
#include "Support/Log/DefaultLog.hpp"
#include <string>


namespace amorphous
{

using namespace std;


// instance of the global params
GlobalParams g_GlobalParams;


void GlobalParams::LoadLogVerbosity( ParamLoader& loader )
{
	string verbosity_title;
	loader.LoadParam( "log_verbosity", verbosity_title );

	int verbosity = WL_WARNING;
	if( verbosity_title == "verbose" )      verbosity = WL_VERBOSE;
	else if( verbosity_title == "caution" ) verbosity = WL_CAUTION;
	else if( verbosity_title == "warning" ) verbosity = WL_WARNING;
	else if( verbosity_title == "error" )   verbosity = WL_ERROR;

	g_Log.SetRefVal( verbosity );
}


} // namespace amorphous