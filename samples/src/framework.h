#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct ImUiContext ImUiContext;
typedef struct ImUiFont ImUiFont;
typedef struct ImUiSkin ImUiSkin;
typedef struct ImUiSurface ImUiSurface;
typedef struct ImUiImage ImUiImage;

typedef struct ImUiFrameworkTexture ImUiFrameworkTexture;

bool					ImUiFrameworkInitialize( ImUiContext* imui );
void					ImUiFrameworkShutdown( ImUiContext* imui );
void					ImUiFrameworkTick( ImUiSurface* surface );

void					ImUiFrameworkSetDpiScale( float dpiScale );

ImUiFrameworkTexture*	ImUiFrameworkTextureCreate( void* textureData, uint32_t width, uint32_t height, bool isFont );
void					ImUiFrameworkTextureDestroy( ImUiFrameworkTexture* texture );

bool					ImUiFrameworkFontCreate( ImUiFont** font, ImUiImage* image, const char* fontFilename, float fontSize );
void					ImUiFrameworkFontDestroy( ImUiFont** font, ImUiImage* image );

bool					ImUiFrameworkSkinCreate( ImUiSkin* skin, ImUiImage* image, uint32_t size, float radius, float factor, bool horizontal );
void					ImUiFrameworkSkinDestroy( ImUiSkin* skin, ImUiImage* image );

#ifdef __cplusplus
}
#endif
