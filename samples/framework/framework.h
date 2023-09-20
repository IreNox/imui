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
typedef struct ImUiTexture ImUiTexture;

typedef struct ImUiFrameworkTexture ImUiFrameworkTexture;

bool					ImUiFrameworkInitialize( ImUiContext* imui );
void					ImUiFrameworkShutdown( ImUiContext* imui );
void					ImUiFrameworkTick( ImUiSurface* surface );

ImUiFrameworkTexture*	ImUiFrameworkTextureCreate( void* textureData, uint32_t width, uint32_t height, bool isFont );
void					ImUiFrameworkTextureDestroy( ImUiFrameworkTexture* texture );

bool					ImUiFrameworkFontCreate( ImUiFont** font, ImUiTexture* texture, const char* fontFilename, float fontSize );
void					ImUiFrameworkFontDestroy( ImUiFont** font, ImUiTexture* texture );

bool					ImUiFrameworkSkinCreate( ImUiSkin* skin, ImUiTexture* texture, uint32_t size, float radius, float factor, bool horizontal );
void					ImUiFrameworkSkinDestroy( ImUiSkin* skin, ImUiTexture* texture );

#ifdef __cplusplus
}
#endif
