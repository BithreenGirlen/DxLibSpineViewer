#ifndef DXLIB_SPINE_TEXTURE_LOADER_C_H_
#define DXLIB_SPINE_TEXTURE_LOADER_C_H_


void SpineTextureLoader_enableConversionToPma(bool toEnable);
bool SpineTextureLoader_isConversionToPmaEnabled();
/// @brief Register a texture load callback; *pOutImage should be casted to SIHandle
void SpineTextureLoader_setTextureLoadCallback(void (*pFunc)(void* pUserDatum, const char* textureFilePath, size_t filePathLength, void* pOutImage), void* pUserDatum);

#endif // !DXLIB_SPINE_TEXTURE_LOADER_C_H_
