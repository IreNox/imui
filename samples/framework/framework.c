#include "framework.h"

#include "imui/imui.h"

#include <SDL.h>

static ImUiInputKey s_inputKeyMapping[ SDL_NUM_SCANCODES ];
static ImUiInputMouseButton s_inputMouseButtonMapping[] =
{
	ImUiInputMouseButton_MAX,		// Invalid 0
	ImUiInputMouseButton_Left,		// SDL_BUTTON_LEFT     1
	ImUiInputMouseButton_Middle,	// SDL_BUTTON_MIDDLE   2
	ImUiInputMouseButton_Right,		// SDL_BUTTON_RIGHT    3
	ImUiInputMouseButton_X1,		// SDL_BUTTON_X1       4
	ImUiInputMouseButton_X2			// SDL_BUTTON_X2       5
};

int main( int argc, char* argv[] )
{
	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "I'm Ui", "Failed to initialize SDL.", NULL);
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow( "I'm Ui", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL );
	if( window == NULL)
	{
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "I'm Ui", "Failed to create Window.", NULL );
		return 1;
	}

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );

	SDL_GLContext glContext = SDL_GL_CreateContext( window );
	if( glContext == NULL )
	{
		return 1;
	}

	SDL_GL_SetSwapInterval( 1 );

	ImUiParameters parameters = { 0 };
	ImUiContext* imui = ImUiCreate( &parameters );

	while( true )
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
					ImUiInputPushMouseMove( input, (float)pMouseEvent->x, (float)pMouseEvent->x );
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

			default:
				break;
			}
		}
		ImUiInputEnd( imui );

		ImUiFrame* frame = ImUiBegin( imui );
		ImUiSurface* surface = ImUiSurfaceBegin( frame, ImUiStringViewCreate( "main" ), ImUiSizeCreate( 1280, 720 ), 1.0f );

		ImUiFrameworkTick( surface );

		const ImUiDrawData* drawData = ImUiSurfaceEnd( surface );

		// TODO: draw

		ImUiEnd( frame );
	}

	ImUiDestroy( imui );
	SDL_GL_DeleteContext( glContext );
	SDL_DestroyWindow( window );
	SDL_Quit();
}
