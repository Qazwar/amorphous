#include "ScriptManager.hpp"
#include "ScriptArchive.hpp"
#include "Support/StringAux.hpp"
#include "Support/Log/DefaultLog.hpp"
#include "Support/Profile.hpp"
#include "Support/fnop.hpp"
#include <boost/python.hpp>


using namespace std;
using namespace boost;


bool CScriptManager::ms_UseBoostPythonModules = true;


//=========================================================================
// CPythonUserCount
//=========================================================================

CSingleton<CPythonUserCount> CPythonUserCount::m_obj;



//=========================================================================
// Global Functions
//=========================================================================


static CScriptManager *g_pScriptManager = NULL;

//static CScriptManager::CEventScript *gs_pTargetScript = NULL;


static PyObject *SetScriptUpdateCallback( PyObject *dummy, PyObject *args )
{
    PyObject *result = NULL;
    PyObject *temp;

	if( PyArg_ParseTuple(args, "O:set_callback", &temp) )
	{
		if (!PyCallable_Check(temp))
		{
			// PyCallable_Check does not necessarily return 0 for an invalid script
			PyErr_SetString(PyExc_TypeError, "parameter must be callable");
			PrintLog( "an invalid event callback" );
//			return NULL;
			Py_INCREF(Py_None);
			result = Py_None;
			return result;
		}

		// Add a reference to new callback
		Py_XINCREF(temp);

//		Py_XDECREF(g_PythonCallback);  // Dispose of previous callback

		// Remember new callback
		g_pScriptManager->AddEventCallback( temp );

		// Boilerplate to return "None"
		Py_INCREF(Py_None);
		result = Py_None;
	}
	return result;
}


static PyObject *SetScriptInitCallback( PyObject *dummy, PyObject *args )
{
    PyObject *result = NULL;
    PyObject *temp;
	try
	{
		if( PyArg_ParseTuple(args, "0:set_callback", &temp) )
		{
			if (!PyCallable_Check(temp))
			{
				// PyCallable_Check does not necessarily return 0 for an invalid script
				PyErr_SetString(PyExc_TypeError, "parameter must be callable");
				PrintLog( "an invalid init callback" );
//				return NULL;
				Py_INCREF(Py_None);
				result = Py_None;
				return result;
			}

			// Add a reference to new callback
			Py_XINCREF(temp);

//			Py_XDECREF(g_PythonCallback);  // Dispose of previous callback

			// Remember new callback
			g_pScriptManager->AddInitCallback( temp );

			// Boilerplate to return "None"
			Py_INCREF(Py_None);
			result = Py_None;
		}
	}
	catch( std::exception& e )
	{
		g_Log.Print( WL_ERROR, "SetInitCallback() failed - exception: %s", e.what() );
	}

	return result;
}


PyMethodDef g_ScriptBaseMethod[] =
{
    { "SetCallback",		SetScriptUpdateCallback,	METH_VARARGS, "sets an event callback" },
    { "SetInitCallback",	SetScriptInitCallback,		METH_VARARGS, "sets an init callback" },
//	{ "SetInitCallback",	SetInitCallback,		METH_VARARGS, "sets an init callback" },
    {NULL, NULL}
};


static string GetExtraErrorInfo()
{
	// failed attempt - exception returned by PyErr_Occurred() cannot be retrieved as a string
/*
	PyObject *pException = PyErr_Occurred();
	char *exception_str = NULL;
	if( pException )
	{
		exception_str = PyString_AsString( pException );
	}
*/

	PyObject *pType, *pValue, *pTraceback;
	PyErr_Fetch( &pType, &pValue, &pTraceback );

	char *err_type = NULL, *err_value = NULL, *err_traceback = NULL;

	if( pType )
		err_type = PyString_AsString( pType );

	if( pValue )
		err_value = PyString_AsString( pValue );

	if( pTraceback )
		err_traceback = PyString_AsString( pTraceback );

	if( err_value )
		return string(err_value);
	else
		return string();
}



//===========================================================================
// CScriptManager
//===========================================================================

CScriptManager::CScriptManager()
:
m_pTargetScript(NULL),
m_bLoadFromNonArchivedFiles(false)
{
	g_pScriptManager = this;

	PythonUserCount().IncRefCount();

	if( ms_UseBoostPythonModules )
	{
		// add script_base module
	}
	else
	{
		// make a module for setting callbacks
		AddModule( "ScriptBase", g_ScriptBaseMethod );
//		PyObject *pBaseModule0 = PyImport_AddModule( "ScriptBase" );
//		PyObject *pBaseModule = Py_InitModule( "ScriptBase", g_ScriptBaseMethod );
	}
}

CScriptManager::~CScriptManager()
{
	PythonUserCount().DecRefCount();

	g_pScriptManager = NULL;
}


void CScriptManager::AddModule( const string& module_name, PyMethodDef method[] )
{
	PyObject *pModule0 = PyImport_AddModule( module_name.c_str() );
	if( !pModule0 )
	{
		LOG_PRINT_ERROR( " - PyImport_AddModule() failed for module '" + module_name + "'. Error: " + GetExtraErrorInfo() );
	}

	PyObject *pModule  = Py_InitModule( module_name.c_str(), method );
	if( !pModule )
	{
		LOG_PRINT_ERROR( " - Py_InitModule() failed for module '" + module_name + "'. Error: " + GetExtraErrorInfo() );
	}
}


void CScriptManager::AddEventCallback( PyObject* pEventCallback )
{
	if( m_pTargetScript )
		m_pTargetScript->m_pEventCallback = pEventCallback;
}


void CScriptManager::AddInitCallback( PyObject* pEventCallback )
{
	if( m_pTargetScript )
		m_pTargetScript->m_pInitCallback = pEventCallback;
}


/// execute the script and register the callback from the script
bool CScriptManager::LoadScript( const stream_buffer& buffer, CEventScript& dest_script )
{
	boost::thread::id thread_id = boost::this_thread::get_id();

	bool res = false;
	if( ms_UseBoostPythonModules )
	{
		// Retrieve the main module
		python::object main = python::import("__main__");

		// Retrieve the main module's namespace
		python::object global(main.attr("__dict__"));

		// set script holder object that will be the target for registering callback functions
		m_pTargetScript = &dest_script;

		PyErr_Clear();

		// run  the script and register the callback function(s)
//		python::object result = python::exec( &(buffer.get_buffer()[0]), global, global );

		try
		{
			python::object result = python::exec( &(buffer.get_buffer()[0]), global, global );
		}
		catch( std::exception& e )
		{
			g_Log.Print( WL_WARNING, "std::exception: %s", e.what() );
		}
		catch( boost::exception& e )
		{
			// handle exception
			LOG_PRINT_ERROR( "boost::exception" );
		}
		catch( boost::python::error_already_set& e )
		{
			// handle exception
			LOG_PRINT_ERROR( "python::exec() threw an exception 'error_already_set'. Error: " + GetExtraErrorInfo() );
		}

		res = true;//result

		m_pTargetScript = NULL;
	}
	else
	{
		PyObject *pMainModule     = PyImport_AddModule( "__main__" );
		PyObject *pMainDictionary = PyModule_GetDict( pMainModule );

		// set script holder object that will be the target for registering callback functions
		m_pTargetScript = &dest_script;

		PyErr_Clear();

		// run  the script and register the callback function(s)
		PyObject* pRunResult = PyRun_String( (const char *)(&buffer.get_buffer()[0]),
											 Py_file_input, pMainDictionary, pMainDictionary );

		res = pRunResult ? true : false;

		m_pTargetScript = NULL;
	}

	if( !dest_script.m_pEventCallback )
	{
		dest_script.m_bIsDone = true;
		LOG_PRINT_ERROR( " No callback function has been found in the script or PyRun_String() returned NULL. Error: " + GetExtraErrorInfo() );
//		g_Log.Print( "no callback has been found set for script[%02d]", m_vecEventScript.size() - 1 );
		return false;
	}

//	if( pRunResult == NULL )
	if( res == false )
	{
		return false;
//		PrintLog( "an exception raised during the execution of the script, '" + filename + "'");
		LOG_PRINT_ERROR( " PyRun_String() returned NULL. Error: " + GetExtraErrorInfo() );
	}
	else
	{
		dest_script.m_bIsDone = false;
		return true;
//		PrintLog( "script file, '" + filename + "' loaded");
	}

	return true;
}


bool CScriptManager::LoadScriptArchiveFile( const string& filename )
{
	CScriptArchive script_archive;
	bool res = script_archive.LoadFromFile( filename );
	if( !res )
		return false;

	// script archive contains one or more script files
	// load one by one 
	size_t i, num_scripts = script_archive.m_vecBuffer.size();
	for( i=0; i<num_scripts; i++ )
	{
		string& script_filename = script_archive.m_vecSourceFilename[i];

		m_vecEventScript.push_back( CEventScript() );

		if( !LoadScript( script_archive.m_vecBuffer[i].m_Buffer, m_vecEventScript.back() ) )
		{
			// ReleaseScripts();
			LOG_PRINT_ERROR( "invalid script file: [" + to_string((int)i) + "] " + script_filename );
		}
		else
		{
			m_vecEventScript.back().filename = script_filename;
			LOG_PRINT( "loaded a script file: [" + to_string((int)i) + "] " + script_filename );

			// check for the non-archived file
			struct stat script_file_stat;
			if( stat(script_filename.c_str(), &script_file_stat) != 0 )
				continue;

			// non-archived script file was found
			// - record the last modified time of the file
			m_vecEventScript.back().m_LastModifiedTime = script_file_stat.st_mtime;

			// check for file updates later at runtime
			m_bLoadFromNonArchivedFiles = true;
		}
	}

	return true;
}


bool CScriptManager::ExecuteScript( const stream_buffer& buffer )
{
	CEventScript es;
	return LoadScript( buffer, es );
}


bool CScriptManager::LoadScriptFromFile( const std::string& filename )
{
/*	stream_buffer script_buffer;
	bool res = script_buffer.LoadTextFile( filename );
	if( !res )
		return false;
	
	LoadScript( script_buffer );

	return true;
*/

	return false;
}


void CScriptManager::InitScripts()
{
	size_t i, num_eventscripts = m_vecEventScript.size();
	for( i=0; i<num_eventscripts; i++ )
	{
		if( !m_vecEventScript[i].m_pInitCallback )
			continue;

		PyObject *pArgList;
		pArgList = Py_BuildValue( "()" );

		PyObject *pResult  = PyEval_CallObject( m_vecEventScript[i].m_pInitCallback, pArgList );

		Py_DECREF( pArgList );
/*
		if( pResult == NULL )
		{
			// an invalid script - discard the object
			PyErr_Clear();	// omitting this causes an error in Py_Finalize()
			m_vecEventScript[i].m_bIsDone = true;
			PrintLog( "invalid return value from init script" );
		}
		else
		{
			int result;
//			PyArg_ParseTuple( pResult, "i", &result );
			PyArg_Parse( pResult, "i", &result );

			if( result == CEventScript::EVENT_DONE )
			{
				CEventScript& script = m_vecEventScript[i];

//	necessary?	Py_DECREF( m_vecEventScript[i].m_pInitCallback );
				PrintLog( "script[" + to_string(int(i)) + "] '"+ script.name + "' has been initialized" );
			}
			Py_DECREF( pResult );
		}*/
	}
}


void CScriptManager::ReloadUpdatedScriptFiles()
{
	size_t i, num_eventscripts = m_vecEventScript.size();
	struct stat script_file_stat;
	stream_buffer script_buffer;

	for( i=0; i<num_eventscripts; i++ )
	{
		CEventScript& script = m_vecEventScript[i];

		if( script.m_LastModifiedTime == 0 )
			continue;	// last modified time is not recorded

		if( stat(script.filename.c_str(), &script_file_stat) != 0 )
			continue;

		if( script.m_LastModifiedTime < script_file_stat.st_mtime )
		{
			// script has been updated - try to reload it

			LOG_PRINT( "Reloading a script: " + script.filename );

			script.m_LastModifiedTime = script_file_stat.st_mtime;

			//ReleaseScript()?
			bool res = script_buffer.LoadTextFile( script.filename );
			if( !res )
				continue;

			if( script.m_pEventCallback )
			{
				Py_DECREF( script.m_pEventCallback ); 
				script.m_pEventCallback = NULL;
			}

			if( script.m_pInitCallback )
			{
				Py_DECREF( script.m_pInitCallback ); 
				script.m_pInitCallback = NULL;
			}

			if( !LoadScript( script_buffer, script ) )
			{
				LOG_PRINT_ERROR( string("invalid script: ") + script.filename );
			}
		}
	}
}


void CScriptManager::Update()
{
	PROFILE_FUNCTION();

	if( m_bLoadFromNonArchivedFiles )
		ReloadUpdatedScriptFiles();

	size_t i, num_eventscripts = m_vecEventScript.size();

	for( i=0; i<num_eventscripts; i++ )
	{
		if( m_vecEventScript[i].m_bIsDone )
			continue;

		PyObject *pArgList;
		pArgList = Py_BuildValue( "()" );

		PyObject *pResult  = PyEval_CallObject( m_vecEventScript[i].m_pEventCallback, pArgList );

		Py_DECREF( pArgList );

		if( pResult == NULL )
		{
			// an invalid script - discard the object

			// save the error info first
			string err_string_ex = GetExtraErrorInfo();

			// omitting this causes an error in Py_Finalize()
			PyErr_Clear();

			// mark the script as 'finished'
			m_vecEventScript[i].m_bIsDone = true;

			// log the error
			string err_string = "invalid return value from event script: " + m_vecEventScript[i].filename;
			if( 0 < err_string_ex.length() )
				err_string += " - " + err_string_ex;

			LOG_PRINT_ERROR( err_string );
		}
		else
		{
			int result;
//			PyArg_ParseTuple( pResult, "i", &result );
			PyArg_Parse( pResult, "i", &result );

			if( result == CEventScript::EVENT_DONE )
			{
				CEventScript& script = m_vecEventScript[i];

				script.m_bIsDone = true;
//	necessary?	Py_DECREF( m_vecEventScript[i].m_pEventCallback );
				LOG_PRINT( "script[" + to_string(int(i)) + "] '"+ script.filename + "' has been finished." );
			}
			Py_DECREF( pResult );
		}
	}
}
