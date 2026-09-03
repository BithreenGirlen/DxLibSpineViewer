#ifndef DXLIB_SPINE_TEXTURE_LOADER_C_H_
#define DXLIB_SPINE_TEXTURE_LOADER_C_H_

/*
* Namespace is avoided here because firstly, some of the functions which are declared in <Spine/extension.h>,
* namely _spAtlasPage_createTexture(), _spAtlasPage_disposeTexture(), and _spUtil_readFile()
* must be implemented here globally.
* Secondly, the names of functions here should be same regardless of rendering backend,
* as is done in spine-cpp version player like :
* using CTextureLoader = CDxLibTextureLoader;
*/

void SpineTextureLoader_enableConversionToPma(bool toEnable);
bool SpineTextureLoader_isConversionToPmaEnabled();

#endif // !DXLIB_SPINE_TEXTURE_LOADER_C_H_
