
#include "PyModule_HUD.h"

#include "3DMath/Vector3.h"
#include "PlayerInfo.h"
#include "HUD_PlayerBase.h"


using namespace std;


PyObject* ShowTime( PyObject* self, PyObject* args )
{
	HUD_PlayerBase *pHUD = PLAYERINFO.GetHUD();
	if( !pHUD )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	pHUD->TimerDisplay().Show();

    Py_INCREF( Py_None );
	return Py_None;
}


PyObject* HideTime( PyObject* self, PyObject* args )
{
	HUD_PlayerBase *pHUD = PLAYERINFO.GetHUD();
	if( !pHUD )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	pHUD->TimerDisplay().Hide();

    Py_INCREF( Py_None );
	return Py_None;
}


PyObject* SetTimeColor( PyObject* self, PyObject* args )
{
/*	HUD_PlayerBase *pHUD = PLAYERINFO.GetHUD();
	if( !pHUD )
		return Py_None;

	SFloatRGBAColor color;
	int result = PyArg_ParseTuple( args, "fff", &r, &g, &b );

	pHUD->TimerDisplay().m_Color = color.GetARGB32();
*/
    Py_INCREF( Py_None );
	return Py_None;
}


PyObject* UpdateTimeS( PyObject* self, PyObject* args )
{
	HUD_PlayerBase *pHUD = PLAYERINFO.GetHUD();
	if( !pHUD )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	float time_in_sec;
	int result = PyArg_ParseTuple( args, "f", &time_in_sec );

	pHUD->TimerDisplay().m_TimeMS = (unsigned long)time_in_sec * 1000;

    Py_INCREF( Py_None );
	return Py_None;
}



PyObject* LoadGlobalMap( PyObject* self, PyObject* args )
{
	HUD_PlayerBase *pHUD = PLAYERINFO.GetHUD();
	if( !pHUD )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	char* global_map_texture_filename;
	int result = PyArg_ParseTuple( args, "s", &global_map_texture_filename );

	pHUD->LoadGlobalMapTexture( global_map_texture_filename );

    Py_INCREF( Py_None );
	return Py_None;
}


/**
 * create entity from pos, dir, vel
 */
PyObject* _CreateEntity( PyObject* self, PyObject* args )
{
/*	if( !gs_pTargetStage )
		return Py_None;

	char *base_name;
	Vector3 pos;
	Vector3 dir = Vector3(0,0,1);
	Vector3 vel = Vector3(0,0,0);
	int result = PyArg_ParseTuple( args, "sfff|ffffff", &base_name,
		                                             &pos.x, &pos.y, &pos.z,
		                                             &dir.x, &dir.y, &dir.z,
		                                             &vel.x, &vel.y, &vel.z );

	CreateNamedEntity( "", base_name, pos, dir, vel );
*/
    Py_INCREF( Py_None );
	return Py_None;
}


PyMethodDef g_PyModuleHUDMethod[] =
{
	{ "HideTime",				HideTime,				METH_VARARGS, "" },
	{ "ShowTime",				ShowTime,				METH_VARARGS, "" },
	{ "UpdateTimeS",			UpdateTimeS,			METH_VARARGS, "" },
//	{ "UpdateTimeMS",			UpdateTimeMS,			METH_VARARGS, "" },
	{ "SetTimeColor",			SetTimeColor,			METH_VARARGS, "" },
	{ "LoadGlobalMap",			LoadGlobalMap,			METH_VARARGS, "" },
//	{ "CreateEntityHrz",		CreateEntityHrz,		METH_VARARGS, "creates an entity at a given position" },
	{ NULL, NULL }
};
