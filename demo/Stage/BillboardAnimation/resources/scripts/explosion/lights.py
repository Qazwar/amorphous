import math3d
import gfx
import stage
import stage_util

light_util = stage_util.CreateStageLightUtility()

dir_light = light_util.CreateHSDirectionalLightEntity( upper_color = gfx.Color(1.0, 1.0, 1.0, 1.0), lower_color = gfx.Color(0.1,0.1,0.1,1.0), dir = math3d.Vector3(2,-3,1) )
pnt_light = light_util.CreateHSPointLightEntity( upper_color = gfx.Color(1.00,0.86,0.51,1.0), lower_color = gfx.Color(0.1,0.1,0.1,1.0), pos = math3d.Vector3(0.0,3.5,8.0), attenu0=0.1, attenu1=0.1, attenu2=0.1 )
#dir_light = light_util.CreateHSDirectionalLightEntity( upper_color = gfx.Color(1.0,0.0,0.0,1.0), lower_color = gfx.Color(0.0,1.0,0.0,1.0), dir = math3d.Vector3(2,-3,1) )
