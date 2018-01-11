amorphous
=========

A C++-based framework I have written to create basic real-time 3D graphics applications.
It is primarily intended for my personal use, but it's up here on GitHub now, so it is free for anyone to use for whatever peaceful purpose.
Please understand that many of them are experimental and not functional yet (i.e. don't get mad at me for putting this on GitHub).

----
## Modules (framework/amorphous)

|Module             |Status               |Description|
|-------------------|---------------------|---------------------|
|3DMath             |Functional           |Basic 3D mathmatics, i.e. Vector3, Matrix33, and so forth|
|App                |Functional           ||
|BSPMapCompiler     |Deprecated           ||
|GameCommon         |Partially functional ||
|GameTextSystem     |Deprecated           ||
|Graphics           |Functional           ||
|GUI                |Partially functional ||
|Input              |Functional           ||
|Item               |Partially functional ||
|LightWave          |Functional           ||
|MotionCapture      |Functional           ||
|MotionSynthesis    |Partially functional ||
|Physics            |Functional           ||
|Script             |Partially functional ||
|Sound              |Partially functional ||
|Stage              |Partially functional ||
|Support            |Partially functional ||
|Task               |Functional           ||
|Utilities          |Functional           ||
|XML                |Functional           ||
 

## External dependencies
---------
The framework depends on the following third party libraries:
- boost (filesystem)
- OpenGL
- The FreeType Project
- PhysX
- OpenAL
- TBB (Threading Building Blocks)



## Other third-party libraries Amorphous uses
---------
Along with the third-party libraries listed above, Amorphous also incorporates the following libraries. These are completely embedded inside the framework, i.e. their source code are compiled together with other Amourphous source code. Therefore, users of the framework do not have to be aware of them.
- zlib
- stb (stb_image*)
