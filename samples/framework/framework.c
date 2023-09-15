#include "framework.h"

#include "imui/imui.h"

#include <GL/glew.h>
#include <SDL.h>
#include <crtdbg.h>
#include <stdio.h>
#include <string.h>

#include "../../src/imui_types.h"

static ImUiInputKey s_inputKeyMapping[ SDL_NUM_SCANCODES ];
static const ImUiInputMouseButton s_inputMouseButtonMapping[] =
{
	ImUiInputMouseButton_MAX,		// Invalid 0
	ImUiInputMouseButton_Left,		// SDL_BUTTON_LEFT     1
	ImUiInputMouseButton_Middle,	// SDL_BUTTON_MIDDLE   2
	ImUiInputMouseButton_Right,		// SDL_BUTTON_RIGHT    3
	ImUiInputMouseButton_X1,		// SDL_BUTTON_X1       4
	ImUiInputMouseButton_X2			// SDL_BUTTON_X2       5
};

typedef struct ImFrameworkContext ImFrameworkContext;
struct ImFrameworkContext
{
	SDL_Window*					window;
	int							windowWidth;
	int							windowHeight;
	SDL_GLContext				glContext;

	GLuint						vertexShader;
	GLuint						fragmentShader;
	GLuint						fragmentShaderFont;

	GLuint						program;
	GLuint						programFont;
	GLint						programUniformProjection;
	GLint						programUniformTexture;

	GLuint						vertexArray;
	GLuint						vertexBuffer;
	GLuint						elementBuffer;

	GLuint						whiteTexture;
};

static bool ImFrameworkRendererInitialize( ImFrameworkContext* context );
static bool ImFrameworkRendererCompileShader( GLuint shader, const char* pShaderCode );
static void ImFrameworkRendererShutdown( ImFrameworkContext* context );
static void ImFrameworkRendererDraw( ImFrameworkContext* context, const ImUiDrawData* drawData );

int main( int argc, char* argv[] )
{
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

	// Turn on leak-checking bit.
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

	// Turn off CRT block checking bit.
	tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;

	// Set flag to the new value.
	_CrtSetDbgFlag( tmpFlag );
	//_CrtSetBreakAlloc( 100 );

	ImFrameworkContext context;

	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "I'm Ui", "Failed to initialize SDL.", NULL);
		return 1;
	}

	context.window = SDL_CreateWindow( "I'm Ui", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE );
	if( context.window == NULL)
	{
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "I'm Ui", "Failed to create Window.", NULL );
		return 1;
	}

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );

	context.glContext = SDL_GL_CreateContext( context.window );
	if( context.glContext == NULL )
	{
		return 1;
	}

	SDL_GL_SetSwapInterval( 1 );

	if( glewInit() != GLEW_OK )
	{
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "I'm Ui", "Failed to initialize GLEW.\n", NULL );
		return 1;
	}

	if( !ImFrameworkRendererInitialize( &context ) )
	{
		ImFrameworkRendererShutdown( &context );
		return 1;
	}

	ImUiParameters parameters = { 0 };
	ImUiContext* imui = ImUiCreate( &parameters );

	const bool init = ImUiFrameworkInitialize( imui );
	bool running = init;
	while( running )
	{
		ImUiInput* input = ImUiInputBegin( imui );
		SDL_Event sdlEvent;
		while( SDL_PollEvent( &sdlEvent ) )
		{
			switch( sdlEvent.type )
			{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				{
					const SDL_KeyboardEvent* pKeyEvent = &sdlEvent.key;

					const ImUiInputKey mappedKey = s_inputKeyMapping[ pKeyEvent->keysym.scancode ];
					if( mappedKey != ImUiInputKey_None )
					{
						(pKeyEvent->type == SDL_KEYDOWN ? ImUiInputPushKeyDown : ImUiInputPushKeyUp)( input, mappedKey );
					}
				}
				break;

			case SDL_MOUSEMOTION:
				{
					const SDL_MouseMotionEvent* pMouseEvent = &sdlEvent.motion;
					ImUiInputPushMouseMove( input, (float)pMouseEvent->x, (float)pMouseEvent->y );
				}
				break;

			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
				{
					const SDL_MouseButtonEvent* pMouseEvent = &sdlEvent.button;
					(pMouseEvent->type == SDL_MOUSEBUTTONDOWN ? ImUiInputPushMouseDown : ImUiInputPushMouseUp)( input, s_inputMouseButtonMapping[ pMouseEvent->button ] );
				}
				break;

			case SDL_MOUSEWHEEL:
				{
					const SDL_MouseWheelEvent* pMouseEvent = &sdlEvent.wheel;
					ImUiInputPushMouseScroll( input, (float)pMouseEvent->x, (float)pMouseEvent->y );
				}
				break;

			case SDL_QUIT:
				running = false;
				break;

			default:
				break;
			}
		}
		ImUiInputEnd( imui );

		SDL_GetWindowSize( context.window, &context.windowWidth, &context.windowHeight );

		ImUiFrame* frame = ImUiBegin( imui );
		ImUiSurface* surface = ImUiSurfaceBegin( frame, ImUiStringViewCreate( "main" ), ImUiSizeCreate( (float)context.windowWidth, (float)context.windowHeight ), 1.0f );

		ImUiFrameworkTick( surface );

		const ImUiDrawData* drawData = ImUiSurfaceEnd( surface );

		ImFrameworkRendererDraw( &context, drawData );

		ImUiEnd( frame );

		SDL_GL_SwapWindow( context.window );
	}

	if( init )
	{
		ImUiFrameworkShutdown( imui );
	}

	ImUiDestroy( imui );
	ImFrameworkRendererShutdown( &context );
	SDL_GL_DeleteContext( context.glContext );
	SDL_DestroyWindow( context.window );
	SDL_Quit();

	return 0;
}

static const char s_vertexShader[] =
	"#version 150\n"
	"uniform mat4 ProjectionMatrix;\n"
	"in vec2 Position;\n"
	"in vec2 TexCoord;\n"
	"in vec4 Color;\n"
	"out vec2 vtfUV;\n"
	"out vec4 vtfColor;\n"
	"void main() {\n"
	"	vtfUV		= TexCoord;\n"
	"	vtfColor	= Color;\n"
	"	gl_Position	= ProjectionMatrix * vec4(Position.xy, 0, 1);\n"
	"}\n";

static const char s_fragmentShader[] =
	"#version 150\n"
	"precision mediump float;\n"
	"uniform sampler2D Texture;\n"
	"in vec2 vtfUV;\n"
	"in vec4 vtfColor;\n"
	"out vec4 OutColor;\n"
	//"float rand(float n){ return fract(sin(n) * 43758.5453123); }\n"
	//"float noise( float p ){ float fl = floor( p ); float fc = fract( p ); return mix( rand( fl ), rand( fl + 1.0 ), fc ); }\n"
	"void main(){\n"
	"	vec4 texColor = texture(Texture, vtfUV.xy);\n"
	"	OutColor = vtfColor * texColor;\n"
	//"	OutColor.xyz += vec3( noise( gl_FragCoord.x ) + noise( gl_FragCoord.y ) ) / 2;\n"
	"}\n";

static const char s_fragmentShaderFont[] =
	"#version 150\n"
	"precision mediump float;\n"
	"uniform sampler2D Texture;\n"
	"in vec2 vtfUV;\n"
	"in vec4 vtfColor;\n"
	"out vec4 OutColor;\n"
	"void main(){\n"
	"	float charColor = texture(Texture, vtfUV.xy).r;\n"
	"	OutColor = vtfColor * charColor;\n"
	"}\n";

static bool ImFrameworkRendererInitialize( ImFrameworkContext* context )
{
	// Shader
	context->vertexShader		= glCreateShader( GL_VERTEX_SHADER );
	context->fragmentShader		= glCreateShader( GL_FRAGMENT_SHADER );
	context->fragmentShaderFont	= glCreateShader( GL_FRAGMENT_SHADER );
	if( context->vertexShader == 0u ||
		context->fragmentShader == 0u ||
		context->fragmentShaderFont == 0u )
	{
		printf( "[renderer] Failed to create GL Shader.\n" );
		return false;
	}

	if( !ImFrameworkRendererCompileShader( context->vertexShader, s_vertexShader ) ||
		!ImFrameworkRendererCompileShader( context->fragmentShader, s_fragmentShader ) ||
		!ImFrameworkRendererCompileShader( context->fragmentShaderFont, s_fragmentShaderFont ) )
	{
		printf( "[renderer] Failed to compile GL Shader.\n" );
		return false;
	}

	context->program = glCreateProgram();
	context->programFont = glCreateProgram();
	if( context->program == 0u ||
		context->programFont == 0u )
	{
		printf( "[renderer] Failed to create GL Program.\n" );
		return false;
	}

	glAttachShader( context->program, context->vertexShader );
	glAttachShader( context->program, context->fragmentShader );
	glLinkProgram( context->program );

	glAttachShader( context->programFont, context->vertexShader );
	glAttachShader( context->programFont, context->fragmentShaderFont );
	glLinkProgram( context->programFont );

	bool ok = true;
	GLint programStatus;
	glGetProgramiv( context->program, GL_LINK_STATUS, &programStatus );
	ok &= (programStatus == GL_TRUE);
	glGetProgramiv( context->programFont, GL_LINK_STATUS, &programStatus );
	ok &= (programStatus == GL_TRUE);

	if( !ok )
	{
		printf( "[renderer] Failed to link GL Program.\n" );
		return false;
	}

	context->programUniformProjection	= glGetUniformLocation( context->program, "ProjectionMatrix" );
	context->programUniformTexture		= glGetUniformLocation( context->program, "Texture" );

	// Buffer
	const GLuint attributePosition	= (GLuint)glGetAttribLocation( context->program, "Position" );
	const GLuint attributeTexCoord	= (GLuint)glGetAttribLocation( context->program, "TexCoord" );
	const GLuint attributeColor		= (GLuint)glGetAttribLocation( context->program, "Color" );

	glGenBuffers( 1, &context->vertexBuffer );
	glGenBuffers( 1, &context->elementBuffer );
	glGenVertexArrays( 1, &context->vertexArray );

	glBindVertexArray( context->vertexArray );
	glBindBuffer( GL_ARRAY_BUFFER, context->vertexBuffer );
	//glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, context->elementBuffer );

	glEnableVertexAttribArray( attributePosition );
	glEnableVertexAttribArray( attributeTexCoord );
	glEnableVertexAttribArray( attributeColor );

	const GLsizei vertexSize	= sizeof( float ) * 8;
	size_t vertexPositionOffset	= 0u;
	size_t vertexUvOffset		= sizeof( float ) * 2u;
	size_t vertexColorOffset	= sizeof( float ) * 4u;
	glVertexAttribPointer( attributePosition, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)vertexPositionOffset );
	glVertexAttribPointer( attributeTexCoord, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)vertexUvOffset );
	glVertexAttribPointer( attributeColor, 4, GL_FLOAT, GL_FALSE, vertexSize, (void*)vertexColorOffset );

	glGenTextures( 1, &context->whiteTexture );
	glBindTexture( GL_TEXTURE_2D, context->whiteTexture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	const uint32 data = 0xffffffffu;
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data );

	return true;
}

static bool ImFrameworkRendererCompileShader( GLuint shader, const char* pShaderCode )
{
	glShaderSource( shader, 1, &pShaderCode, 0 );
	glCompileShader( shader );

	GLint shaderStatus;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &shaderStatus );

	if( shaderStatus != GL_TRUE )
	{
		char buffer[ 2048u ];
		GLsizei infoLength;
		glGetShaderInfoLog( shader, 2048, &infoLength, buffer );

		printf( "[renderer] Failed to compile Shader. Error: %s\n", buffer );
		return false;
	}

	return true;
}

static void ImFrameworkRendererShutdown( ImFrameworkContext* context )
{
	if( context->whiteTexture != 0u )
	{
		glDeleteTextures( 1u, &context->whiteTexture );
		context->whiteTexture = 0u;
	}

	if( context->vertexArray != 0u )
	{
		glDeleteVertexArrays( 1, &context->vertexArray );
		context->vertexArray = 0u;
	}

	if( context->elementBuffer != 0u )
	{
		glDeleteBuffers( 1, &context->elementBuffer );
		context->elementBuffer = 0u;
	}

	if( context->vertexBuffer != 0u )
	{
		glDeleteBuffers( 1, &context->vertexBuffer );
		context->vertexBuffer = 0u;
	}

	if( context->program != 0u )
	{
		glDetachShader( context->program, context->fragmentShader );
		glDetachShader( context->program, context->vertexShader );
		glDeleteProgram( context->program );

		context->program = 0u;
	}

	if( context->programFont != 0u )
	{
		glDetachShader( context->programFont, context->fragmentShaderFont );
		glDetachShader( context->programFont, context->vertexShader );
		glDeleteProgram( context->programFont );

		context->programFont = 0u;
	}

	if( context->vertexShader != 0u )
	{
		glDeleteShader( context->vertexShader );
		context->vertexShader = 0u;
	}

	if( context->fragmentShader != 0u )
	{
		glDeleteShader( context->fragmentShader );
		context->fragmentShader = 0u;
	}

	if( context->fragmentShaderFont != 0u )
	{
		glDeleteShader( context->fragmentShaderFont );
		context->fragmentShaderFont = 0u;
	}
}

static void ImFrameworkRendererDraw( ImFrameworkContext* context, const ImUiDrawData* drawData )
{
	glViewport( 0, 0, context->windowWidth, context->windowHeight );

	const float color[ 4u ] = { 0.01f, 0.2f, 0.7f, 1.0f };
	glClearColor( color[ 0u ], color[ 1u ], color[ 2u ], color[ 3u ] );
	glClear( GL_COLOR_BUFFER_BIT );

	glEnable( GL_BLEND );
	glBlendEquation( GL_FUNC_ADD );

	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	//glEnable( GL_SCISSOR_TEST );

	glUseProgram( context->program );
	glUniform1i( context->programUniformTexture, 0 );

	const GLfloat projectionMatrix[ 4 ][ 4 ] ={
		{  2.0f / context->windowWidth,	 0.0f,							 0.0f,	0.0f },
		{  0.0f,						-2.0f / context->windowHeight,	 0.0f,	0.0f },
		{  0.0f,						 0.0f,							-1.0f,	0.0f },
		{ -1.0f,						 1.0f,							 0.0f,	1.0f }
	};
	glUniformMatrix4fv( context->programUniformProjection, 1, GL_FALSE, &projectionMatrix[ 0u ][ 0u ] );

	// bind buffers
	glBindVertexArray( context->vertexArray );
	glBindBuffer( GL_ARRAY_BUFFER, context->vertexBuffer );
	//glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, context->elementBuffer );

	glBufferData( GL_ARRAY_BUFFER, drawData->vertexDataSize, NULL, GL_STREAM_DRAW );
	//glBufferData( GL_ELEMENT_ARRAY_BUFFER, drawData->indexCount * 4u, NULL, GL_STREAM_DRAW );

	// upload
	{
		void* pVertexData = glMapBufferRange( GL_ARRAY_BUFFER, 0, drawData->vertexDataSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT );
		//void* pElementData = glMapBufferRange( GL_ELEMENT_ARRAY_BUFFER, 0, drawData->indexCount * 4u, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT );

		memcpy( pVertexData, drawData->vertexData, drawData->vertexDataSize );
		//memcpy( pElementData, drawData->indexData, drawData->indexCount * 4u );

		glUnmapBuffer( GL_ARRAY_BUFFER );
		//glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );
	}

	for( size_t i = 0u; i < drawData->commandCount; ++i )
	{
		const ImUiDrawCommand* pCommand = &drawData->commands[ i ];
		IMUI_ASSERT( pCommand->count >= 0u );

		GLuint texture = (GLuint)(size_t)pCommand->texture;
		if( texture == 0u )
		{
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

			glUseProgram( context->program );
			glBindTexture( GL_TEXTURE_2D, context->whiteTexture );
		}
		else
		{
			glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );

			glUseProgram( context->programFont );
			glUniform1i( context->programUniformTexture, 0 );
			glUniformMatrix4fv( context->programUniformProjection, 1, GL_FALSE, &projectionMatrix[ 0u ][ 0u ] );
			glBindTexture( GL_TEXTURE_2D, texture );
		}

		//glScissor(
		//	(GLint)(pCommand->clipRect.position.x),
		//	(GLint)((height - (GLint)(pCommand->clipRect.position.y + pCommand->clipRect.size.height))),
		//	(GLint)(pCommand->clipRect.size.width),
		//	(GLint)(pCommand->clipRect.size.height)
		//);

		glDrawArrays( GL_TRIANGLES, (GLint)pCommand->offset, (GLsizei)pCommand->count );
		//glDrawElements( GL_TRIANGLES, (GLsizei)pCommand->count, GL_UNSIGNED_SHORT, &pCommand->offset );
	}

	// reset OpenGL state
	glUseProgram( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );
	glDisable( GL_BLEND );
	glDisable( GL_SCISSOR_TEST );
}

uint32_t ImUiFrameworkTextureCreate( void* textureData, uint32_t width, uint32_t height )
{
	GLuint textureHandle = 0u;
	glGenTextures( 1, &textureHandle );
	glBindTexture( GL_TEXTURE_2D, textureHandle );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, textureData );

	return textureHandle;
}

void ImUiFrameworkTextureDestroy( uint32_t textureHandle )
{
	glDeleteTextures( 1u, &textureHandle );
}
