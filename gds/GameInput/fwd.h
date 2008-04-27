#ifndef  __GAMEINPUT_FWD_H__
#define  __GAMEINPUT_FWD_H__


class CInputHandler;

class CStdKeyboard;        /// for Windows platform
class CStdMouseInput;      /// for Windows platform
class CDIKeyboard;         /// for Windows platform (DirectInput)
class CDirectInputMouse;   /// for Windows platform (DirectInput)
class CDirectInputGamepad; /// for Windows platform (DirectInput)

#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr<CInputHandler> CInputHandlerSharedPtr;


#endif /* __GAMEINPUT_FWD_H__ */
