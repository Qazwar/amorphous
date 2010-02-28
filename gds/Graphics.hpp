// 2D primitive
#include "Graphics/2DPrimitive/2DRect.hpp"
#include "Graphics/2DPrimitive/2DFrameRect.hpp"
#include "Graphics/2DPrimitive/2DRoundRect.hpp"
#include "Graphics/2DPrimitive/2DRectSet.hpp"
#include "Graphics/2DPrimitive/2DTriangle.hpp"

// font
#include "Graphics/Font/Font.hpp"
#include "Graphics/Font/FontBase.hpp"
#include "Graphics/Font/TextureFont.hpp"
#include "Graphics/Font/TrueTypeTextureFont.hpp"
#include "Graphics/Font/BuiltinFonts.hpp"

// mesh
#include "Graphics/MeshModel/3DMeshModelArchive.hpp"
#include "Graphics/Mesh/BasicMesh.hpp"
#include "Graphics/Mesh/ProgressiveMesh.hpp"
#include "Graphics/Mesh/SkeletalMesh.hpp"
#include "Graphics/MeshGenerators.hpp"

// others
#include "Graphics/3DGameMath.hpp"
#include "Graphics/3DRect.hpp"
#include "Graphics/Camera.hpp"
#include "Graphics/D3DMisc.hpp"
#include "Graphics/Direct3D/Mesh/D3DXMeshObject.hpp"
#include "Graphics/Direct3D/Mesh/D3DXMeshObjectBase.hpp"
#include "Graphics/Direct3D/Mesh/D3DXPMeshObject.hpp"
#include "Graphics/Direct3D/Mesh/D3DXSMeshObject.hpp"
#include "Graphics/D3DXVec3Copy.hpp"
#include "Graphics/Direct3D9.hpp"
#include "Graphics/FloatRGBAColor.hpp"
#include "Graphics/FloatRGBColor.hpp"

#include "Graphics/FVF_Vertices.h"
#include "Graphics/General3DVertex.hpp"
#include "Graphics/GraphicsComponentCollector.hpp"
#include "Graphics/HemisphericLight.hpp"
#include "Graphics/IndexedPolygon.hpp"
#include "Graphics/LensFlare.hpp"
#include "Graphics/LightStructs.hpp"
#include "Graphics/LogOutput_OnScreen.hpp"
#include "Graphics/MeshObjectContainer.hpp"
//#include "Graphics/PointLight.hpp"
#include "Graphics/PostProcessManager.hpp"
#include "Graphics/RectTriListIndex.hpp"
#include "Graphics/SimpleMotionBlur.hpp"
#include "Graphics/TextureCoord.hpp"
#include "Graphics/GraphicsResourceManager.hpp"
#include "Graphics/GraphicsResourceCacheManager.hpp"
#include "Graphics/GraphicsResourceHandle.hpp"
#include "Graphics/TextureRenderTarget.hpp"

#include "Graphics/LinePrimitives.hpp"

#include "Graphics/GraphicsElementManager.hpp"
#include "Graphics/GraphicsEffectManager.hpp"


#include "Graphics/Shader/Serialization_ShaderTechniqueHandle.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Graphics/Shader/ShaderLightManager.hpp"
#include "Graphics/Shader/ShaderManager.hpp"
#include "Graphics/Shader/ShaderManagerHub.hpp"
#include "Graphics/Shader/ShaderTechniqueHandle.hpp"
