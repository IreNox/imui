#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct ImUiContext ImUiContext;
typedef struct ImUiSurface ImUiSurface;

bool		ImUiFrameworkInitialize( ImUiContext* imui );
void		ImUiFrameworkShutdown( ImUiContext* imui );
void		ImUiFrameworkTick( ImUiSurface* surface );

uint32_t	ImUiFrameworkTextureCreate( void* textureData, uint32_t width, uint32_t height );
void		ImUiFrameworkTextureDestroy( uint32_t textureHandle );
