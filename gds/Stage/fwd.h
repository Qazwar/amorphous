#ifndef __FWD_Stage_H__
#define __FWD_Stage_H__


class CStage;
class CCopyEntity;
class CCopyEntityDesc;
class CEntityFactory;
class CCopyEntityDescFileData;
class CBaseEntity;
class CBaseEntityFactory;
class CBaseEntityHandle;
class CEntitySet;
class CStaticGeometryBase;
class CStaticGeometryArchiveFG;
struct STrace;
class CTrace;
class CViewFrustumTest;
class CScreenEffectManager;
class CSurfaceMaterial;
class CSurfaceMaterialManager;
class CTextMessageManager;
struct SGameMessage;
class CBEC_MotionPath;

class CScriptManager;

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
typedef boost::shared_ptr<CStage> CStageSharedPtr;
typedef boost::weak_ptr<CStage> CStageWeakPtr;
typedef boost::shared_ptr<CBaseEntityFactory> CBaseEntityFactorySharedPtr;


#endif /* __FWD_Stage_H__ */
