#ifndef __PYMODULE_GAMEITEMSHOP_H__
#define __PYMODULE_GAMEITEMSHOP_H__


#include "amorphous/Script/fwd.hpp"
#include "amorphous/Script/PythonHeaders.hpp"


namespace amorphous
{


void RegisterPythonModule_Shop( PythonScriptManager& mgr );


class GameItemShop;
//extern bool AddShop( GameItemShop* pShop )
//extern bool ReleaseShop( const std::string& name )


} // namespace amorphous



#endif  /*  __PYMODULE_GAMEITEMSHOP_H__  */
