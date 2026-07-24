#pragma once

#include "imui/imui.h"
#include "imui/imui_toolbox.h"

#include <new>

namespace imui
{
	class UiInput;
	class UiInputState;
	struct UiSize;

	struct UiBorder : public ImuiBorder
	{
						UiBorder();
						UiBorder( float all );
						UiBorder( float horizontal, float vertical );
						UiBorder( float _top, float _left, float _bottom, float _right );
						UiBorder( const ImuiBorder& value );

		UiSize			getMinSize() const;
	};

	struct UiPos : public ImuiPos
	{
						UiPos();
						UiPos( float all );
						UiPos( float _x, float _y );
						UiPos( const ImuiPos& value );
		explicit		UiPos( const ImuiSize& value );

		UiPos			add( float _x, float _y ) const;
		UiPos			add( UiPos add ) const;
		UiPos			sub( float _x, float _y ) const;
		UiPos			sub( UiPos sub ) const;
		UiPos			scale( float factor ) const;

		UiPos			operator+( const UiPos& rhs ) const;
		UiPos			operator-( const UiPos& rhs ) const;
		UiPos			operator*( float rhs ) const;
		UiPos&			operator+=( const UiPos& rhs );
		UiPos&			operator-=( const UiPos& rhs );
		UiPos&			operator*=( float rhs );

		static UiPos	Zero;
	};

	struct UiRect : public ImuiRect
	{
						UiRect();
						UiRect( UiPos _pos, UiSize _size );
						UiRect( float x, float y, float width, float height );
						UiRect( const ImuiRect& value );

		UiRect			shrinkBorder( const ImuiBorder& border ) const;

		bool			includesPos( ImuiPos _pos ) const;
		bool			intersectsRect( const ImuiRect& rect2 ) const;

		float			getRight() const;
		float			getBottom() const;

		UiPos			getTopLeft() const;
		UiPos			getTopRight() const;
		UiPos			getBottomLeft() const;
		UiPos			getBottomRight() const;

		UiSize			getSize() const;
	};

	struct UiSize : public ImuiSize
	{
						UiSize();
						UiSize( float all );
						UiSize( float _width, float _height );
						UiSize( const ImuiSize& value );
						UiSize( const ImuiSkin& value );
						UiSize( const ImuiImage& value );

		UiSize			add( float _width, float _height ) const;
		UiSize			add( UiSize add ) const;
		UiSize			sub( float _width, float _height ) const;
		UiSize			sub( UiSize sub ) const;
		UiSize			scale( float factor ) const;
		UiSize			shrinkBorder( const UiBorder& border ) const;
		UiSize			expandBorder( const UiBorder& border ) const;

		UiSize			operator+( const UiSize& rhs ) const;
		UiSize			operator-( const UiSize& rhs ) const;
		UiSize			operator*( float rhs ) const;
		UiSize&			operator+=( const UiSize& rhs );
		UiSize&			operator-=( const UiSize& rhs );
		UiSize&			operator*=( float rhs );

		static UiSize	lerp( UiSize a, UiSize b, float t );
		static UiSize	lerp( UiSize a, UiSize b, float widthT, float heightT );
		static UiSize	min( UiSize a, UiSize b );
		static UiSize	max( UiSize a, UiSize b );

		static UiSize	Zero;
		static UiSize	One;
	};

	struct UiColor : public ImuiColor
	{
						UiColor();
						UiColor( uint8_t _red, uint8_t _green, uint8_t _blue );
						UiColor( uint8_t _red, uint8_t _green, uint8_t _blue, uint8_t _alpha );
						UiColor( float _red, float _green, float _blue );
						UiColor( float _red, float _green, float _blue, float _alpha );
		explicit		UiColor( ImuiColor value );

		static UiColor	createWhite( uint8_t _alpha );
		static UiColor	createBlack( uint8_t _alpha );
		static UiColor	createGray( uint8_t gray );
		static UiColor	createGray( uint8_t gray, uint8_t _alpha );

		static UiColor	White;
		static UiColor	Black;
		static UiColor	TransparentBlack;
	};

	struct UiTexCoord : public ImuiTexCoord
	{
							UiTexCoord( float _u0, float _v0, float _u1, float _v1 );

		static UiTexCoord	ZeroToOne;
	};

	struct UiContextParameters : public ImuiParameters
	{
						UiContextParameters();
	};

	class UiNonCopyable
	{
	public:

		UiNonCopyable() {}

	private:

		UiNonCopyable( const UiNonCopyable& ) {}
	};

	class UiContext
	{
	public:

						UiContext();
						UiContext( ImuiContext* context );
						UiContext( const UiContextParameters& parameters );
						~UiContext();

		bool			isValid() const;
		ImuiContext*	getInternal() const;

		UiInput&		beginInput( const ImuiInputState* previousInput );
		void			endInput();

		void			setMouseCursor( ImuiInputMouseCursor cursor );

	private:

		bool			m_owner;
		ImuiContext*	m_context;
	};

	class UiInputState
	{
	public:

								UiInputState( const ImuiInputState* input );

		uint32_t				getKeyModifiers() const;	// returns imuiInputModifier
		bool					isKeyDown( ImuiInputKey key ) const;
		bool					isKeyUp( ImuiInputKey key ) const;
		bool					hasKeyPressed( ImuiInputKey key ) const;
		bool					hasKeyReleased( ImuiInputKey key ) const;

		const char*				getText() const;

		UiPos					getMousePos() const;
		bool					isMouseInRect( UiRect rect ) const;
		bool					isMouseButtonDown( ImuiInputMouseButton button ) const;
		bool					isMouseButtonUp( ImuiInputMouseButton button ) const;
		bool					hasMouseButtonPressed( ImuiInputMouseButton button ) const;
		bool					hasMouseButtonReleased( ImuiInputMouseButton button ) const;
		UiPos					getMouseScrollDelta() const;

	private:

		const ImuiInputState*	m_state;
	};

	class UiFrame
	{
	public:

						UiFrame();
						UiFrame( ImuiFrame* frame );
						UiFrame( UiContext& context, float timeInSeconds );
						~UiFrame();

		void			beginFrame( UiContext& context, float timeInSeconds );
		void			endFrame();

		bool			isValid() const;
		ImuiFrame*		getInternal() const;

	private:

		bool			m_owner;
		ImuiFrame*		m_frame;
	};

	class UiSurface
	{
	public:

						UiSurface();
						UiSurface( ImuiSurface* surface );
						UiSurface( ImuiFrame* frame, const char* name, const ImuiSize& size, const ImuiInputState* input, float dpiScale );
						UiSurface( UiFrame& frame, const char* name, const ImuiSize& size, const ImuiInputState* input, float dpiScale );
						~UiSurface();

		void			beginSurface( ImuiFrame* frame, const char* name, const ImuiSize& size, const ImuiInputState* input, float dpiScale );
		void			beginSurface( UiFrame& frame, const char* name, const ImuiSize& size, const ImuiInputState* input, float dpiScale );
		void			endSurface();

		bool			isValid() const;
		ImuiSurface*	getInternal() const;
		UiContext		getContext() const;
		UiInputState	getInput() const;

		double			getTime() const;
		UiRect			getRect() const;
		UiSize			getSize() const;
		float			getDpiScale() const;

	private:

		bool			m_owner;
		ImuiSurface*	m_surface;
	};

	class UiWindow
	{
	public:

						UiWindow();
						UiWindow( ImuiWindow* window );
						UiWindow( ImuiSurface* surface, const char* name, const ImuiRect& rect, uint32_t zOrder );
						UiWindow( UiSurface& surface, const char* name, const ImuiRect& rect, uint32_t zOrder );
						~UiWindow();

		void			beginWindow( ImuiSurface* surface, const char* name, const ImuiRect& rect, uint32_t zOrder );
		void			beginWindow( UiSurface& surface, const char* name, const ImuiRect& rect, uint32_t zOrder );
		void			endWindow();

		bool			isValid() const;
		ImuiWindow*		getInternal() const;
		UiContext		getContext() const;
		UiSurface		getSurface() const;
		UiInputState	getInput() const;

		double			getTime() const;

		void*			allocState( size_t size, ImuiId stateId );
		void*			allocState( size_t size, ImuiId stateId, bool& isNew );
		template< class T >
		T*				newState();
		template< class T >
		T*				newState( bool& isNew );

		UiRect			getRect() const;
		uint32_t		getZOrder() const;

	protected:

		bool			m_owner;
		ImuiWindow*		m_window;
	};

	class UiWidget : public UiNonCopyable
	{
	public:

						UiWidget();
		explicit		UiWidget( UiWindow& window );
						UiWidget( UiWindow& window, ImuiId id );
						UiWidget( UiWindow& window, const char* name );
		explicit		UiWidget( ImuiWidget* widget );
						~UiWidget();

		void			beginWidget( UiWindow& window );
		void			beginWidget( UiWindow& window, ImuiId id );
		void			beginWidget( UiWindow& window, const char* name );
		void			beginWidget( ImuiWidget* widget );
		void			endWidget();

		bool			isValid() const;
		ImuiWidget*		getInternal() const;
		UiContext		getContext() const;
		UiSurface		getSurface() const;
		UiWindow		getWindow() const;
		UiInputState	getInput() const;

		double			getTime();

		void*			allocState( size_t size, ImuiId stateId );
		void*			allocState( size_t size, ImuiId stateId, bool& isNew );
		template< class T >
		T*				newState();
		template< class T >
		T*				newState( bool& isNew );

		ImuiLayout		getLayout() const;
		void			setLayoutStack();							// default
		void			setLayoutScroll( float offsetX, float offsetY );
		void			setLayoutHorizontal( float spacing = 0.0f );
		void			setLayoutVertical( float spacing = 0.0f );
		void			setLayoutGrid( uint32_t columnCount, float colSpacing = 0.0f, float rowSpacing = 0.0f );

		UiBorder		getMargin() const;
		void			setMargin( const ImuiBorder& margin );
		UiBorder		getPadding() const;
		void			setPadding( const ImuiBorder& padding );

		UiSize			getMinSize() const;
		void			setMinWidth( float value );
		void			setMinHeight( float value );
		void			setMinSize( UiSize size );
		UiSize			getMaxSize() const;
		void			setMaxWidth( float value );
		void			setMaxHeight( float value );
		void			setMaxSize( UiSize size );

		void			setFixedWidth( float value );
		void			setFixedHeight( float value );
		void			setFixedSize( UiSize size );

		void			setStretch( float horizontal, float vertical );
		void			setStretchOne();
		float			getHStretch() const;
		void			setHStretch( float stretch );
		float			getVStretch() const;
		void			setVStretch( float stretch );

		void			setAlign( float horizontal, float vertical );
		float			getHAlign() const;
		void			setHAlign( float align );
		float			getVAlign() const;
		void			setVAlign( float align );

		UiPos			getPos() const;
		UiSize			getSize() const;
		UiRect			getRect() const;

		void			getInputState( ImuiWidgetInputState& inputState ) const;

		void			drawLine( ImuiPos p0, ImuiPos p1, ImuiColor color );
		void			drawTriangle( ImuiPos p0, ImuiPos p1, ImuiPos p2, ImuiColor color );
		void			drawColor( ImuiColor color );
		void			drawImage( const ImuiImage& image );
		void			drawImage( const ImuiImage& image, ImuiColor color );
		void			drawSkin( const ImuiSkin& skin, ImuiColor color );
		void			drawText( ImuiTextLayout* layout, ImuiColor color );
		void			drawPartialColor( const ImuiRect& rect, ImuiColor color );
		void			drawPartialImage( const ImuiRect& rect, const ImuiImage& image );
		void			drawPartialImage( const ImuiRect& rect, const ImuiImage& image, ImuiColor color );
		void			drawPartialSkin( const ImuiRect& rect, const ImuiSkin& skin, ImuiColor color );
		void			drawPositionText( ImuiPos pos, ImuiTextLayout* layout, ImuiColor color );

	protected:

		bool			m_owner;
		ImuiWidget*		m_widget;
	};

	class UiWidgetLayoutHorizontal : public UiWidget
	{
	public:

						UiWidgetLayoutHorizontal( UiWindow& window, float spacing = 0.0f );
						UiWidgetLayoutHorizontal( UiWindow& window, ImuiId id, float spacing = 0.0f );
						UiWidgetLayoutHorizontal( UiWindow& window, const char* name, float spacing = 0.0f );
	};

	class UiWidgetLayoutVertical : public UiWidget
	{
	public:

						UiWidgetLayoutVertical( UiWindow& window, float spacing = 0.0f );
						UiWidgetLayoutVertical( UiWindow& window, ImuiId id, float spacing = 0.0f );
						UiWidgetLayoutVertical( UiWindow& window, const char* name, float spacing = 0.0f );
	};

	class UiWidgetLayoutGrid : public UiWidget
	{
	public:

						UiWidgetLayoutGrid( UiWindow& window, uint32_t columnCount, float colSpacing = 0.0f, float rowSpacing = 0.0f );
						UiWidgetLayoutGrid( UiWindow& window, ImuiId id, uint32_t columnCount, float colSpacing = 0.0f, float rowSpacing = 0.0f );
						UiWidgetLayoutGrid( UiWindow& window, const char* name, uint32_t columnCount, float colSpacing = 0.0f, float rowSpacing = 0.0f );
	};

	namespace toolbox
	{
		struct UiToolboxTheme : public ImuiToolboxTheme, public UiNonCopyable
		{
										UiToolboxTheme();
										UiToolboxTheme( ImuiFont* inFont );

			void		setDefault( ImuiFont* inFont );

			void		applyConfig();

			static const UiColor&		getColor( ImuiToolboxColor color );
			static const ImuiSkin&		getSkin( ImuiToolboxSkin skin );
			static const ImuiImage&		getIcon( ImuiToolboxIcon icon );

			static ImuiToolboxTheme&	getTheme();
		};

		class UiToolboxConfigFloatScope : public UiNonCopyable
		{
		public:

								UiToolboxConfigFloatScope( float& value, float newValue, bool active = true );
								~UiToolboxConfigFloatScope();

		private:

			float&				m_value;
			float				m_oldValue;
		};

		class UiToolboxConfigColorScope : public UiNonCopyable
		{
		public:

								UiToolboxConfigColorScope( ImuiToolboxColor color, const ImuiColor& newValue, bool active = true );
								~UiToolboxConfigColorScope();

		private:

			ImuiToolboxColor	m_color;
			ImuiColor			m_oldValue;
		};

		class UiToolboxConfigSkinScope : public UiNonCopyable
		{
		public:

								UiToolboxConfigSkinScope( ImuiToolboxSkin skin, const ImuiSkin& newValue, bool active = true );
								~UiToolboxConfigSkinScope();

		private:

			ImuiToolboxSkin		m_skin;
			ImuiSkin			m_oldValue;
		};

		class UiToolboxConfigIconScope : public UiNonCopyable
		{
		public:

								UiToolboxConfigIconScope( ImuiToolboxIcon icon, const ImuiImage& newValue, bool active = true );
								~UiToolboxConfigIconScope();

		private:

			ImuiToolboxIcon		m_icon;
			ImuiImage			m_oldValue;
		};

		class UiToolboxConfigBorderScope : public UiNonCopyable
		{
		public:

								UiToolboxConfigBorderScope( ImuiBorder& value, UiBorder newValue, bool active = true );
								~UiToolboxConfigBorderScope();

		private:

			ImuiBorder&			m_value;
			UiBorder			m_oldValue;
		};

		class UiToolboxWindow : public UiWindow
		{
		public:

							UiToolboxWindow();
							UiToolboxWindow( UiWindow& window );
							UiToolboxWindow( ImuiWindow* window );
							UiToolboxWindow( ImuiSurface* surface, const char* name, const ImuiRect& rect, uint32_t zOrder );
							UiToolboxWindow( UiSurface& surface, const char* name, const ImuiRect& rect, uint32_t zOrder );

			void			spacer( float width, float height );
			void			strecher( float horizontal, float vertical );

			bool			buttonLabel( const char* text );
			bool			buttonLabelFormat( const char* format, ... );
			bool			buttonIcon( const ImuiImage& icon );
			bool			buttonIcon( const ImuiImage& icon, ImuiSize iconSize );

			bool			checkBox( bool& checked, const char* text );
			bool			checkBoxState( const char* text, bool defaultValue = false );

			void			label( const char* text );
			void			label( const char* text, size_t length );
			void			labelFormat( const char* format, ... );
			void			image( const ImuiImage& img );
			void			image( const ImuiImage& img, const ImuiSize& size );

			bool			slider( float& value, float min = 0.0f, float max = 1.0f );
			float			sliderState( float min = 0.0f, float max = 1.0f );
			float			sliderState( float min, float max, float defaultValue );

			bool			textEdit( char* buffer, size_t bufferSize, size_t* textLength = nullptr );
			const char*		textEditState( size_t bufferSize, const char* defaultValue = nullptr );

			void			progressBar( float value, float min = 0.0f, float max = 1.0f );

			size_t			dropDown( const char* const* items, size_t itemCount, size_t itemStride = sizeof( const char* ) );
		};

		class UiToolboxButton : public UiWidget
		{
		public:

						UiToolboxButton();
						UiToolboxButton( UiWindow& window );
						~UiToolboxButton();

			void		begin( UiWindow& window );
			bool		end();
		};

		class UiToolboxButtonLabel : public UiWidget
		{
		public:

						UiToolboxButtonLabel();
						UiToolboxButtonLabel( UiWindow& window, const char* text );
						~UiToolboxButtonLabel();

			void		begin( UiWindow& window, const char* text );
			void		beginFormat( UiWindow& window, const char* format, ... );
			bool		end();
		};

		class UiToolboxLabel : public UiWidget
		{
		public:

						UiToolboxLabel();
						UiToolboxLabel( UiWindow& window, const char* text );
						UiToolboxLabel( UiWindow& window, const char* text, size_t length );
						~UiToolboxLabel();

			void		begin( UiWindow& window, const char* text );
			void		begin( UiWindow& window, const char* text, size_t length );
			void		beginFormat( UiWindow& window, const char* format, ... );
			void		end();
		};

		class UiToolboxSlider : public UiWidget
		{
		public:

						UiToolboxSlider();
						UiToolboxSlider( UiWindow& window, float& value, float min = 0.0f, float max = 1.0f );
						~UiToolboxSlider();

			void		begin( UiWindow& window, float& value, float min = 0.0f, float max = 1.0f );
			bool		end();

		private:

			float*		m_value;
			float		m_min;
			float		m_max;
		};

		class UiToolboxTextEdit : public UiWidget
		{
		public:

						UiToolboxTextEdit( UiWindow& window );
						UiToolboxTextEdit( UiWindow& window, size_t bufferSize );
						UiToolboxTextEdit( UiWindow& window, char* buffer, size_t bufferSize );
						~UiToolboxTextEdit();

			bool		end( size_t* textLength = nullptr );

			void		setBuffer( char* buffer, size_t bufferSize );

		private:

			char*		m_buffer;
			size_t		m_bufferSize;
		};


		class UiToolboxScrollArea : public UiWidget
		{
		public:

						UiToolboxScrollArea( UiWindow& window );
						~UiToolboxScrollArea();

			void		enableSpacing( bool horizontal, bool vertical );

		private:

			ImuiToolboxScrollAreaContext m_scrollArea;
		};

		class UiToolboxList : public UiWidget
		{
		public:

						UiToolboxList( UiWindow& window, float itemSize, size_t itemCount, bool selection );
						~UiToolboxList();

			bool		end();

			size_t		getBeginIndex() const;
			size_t		getEndIndex() const;
			size_t		getSelectedIndex() const;
			void		setSelectedIndex( size_t index );

			void		nextItem( UiWidget* widget = nullptr, ImuiId id = IMUI_ID_DEFAULT );

		private:

			ImuiToolboxListContext	m_list;
		};

		class UiToolboxDropdown : public UiWidget
		{
		public:

						UiToolboxDropdown( UiWindow& window, const char* const* items, size_t itemCount, size_t itemStride = sizeof( const char* ) );
						//UiToolboxDropdown( UiWindow& window, const char* selectedItem, size_t itemCount );
						~UiToolboxDropdown();

			//bool		isOpen() const;

			//size_t		getBeginIndex() const;
			//size_t		getEndIndex() const;
			size_t		getSelectedIndex() const;
			void		setSelectedIndex( size_t index );

			//void		nextItem( const char* item );

		private:

			ImuiToolboxDropDownContext	m_dropDown;
		};

		class UiToolboxPopup : public UiToolboxWindow
		{
		public:

						UiToolboxPopup( UiSurface& surface );
						UiToolboxPopup( UiWindow& window );
						~UiToolboxPopup();

			size_t 		end( const char** buttons, size_t buttonCount );
			void		end();
		};

		class UiToolboxTabView : public UiWidget
		{
		public:

						UiToolboxTabView();
						UiToolboxTabView( UiWindow& window );
						~UiToolboxTabView();

			size_t		getSelectedIndex() const;
			void		setSelectedIndex( size_t index );

			bool		header( const char* text );
			void		beginHeader( UiWidget& header );
			bool		endHeader( UiWidget& header );

			void		beginBody( UiWidget& body );
			void		endBody( UiWidget& body );

		private:

			ImuiToolboxTabViewContext	m_context;
		};
	}

	template< class T >
	T* UiWindow::newState()
	{
		bool isNew;
		return newState< T >( isNew );
	}

	template< class T >
	void callDestructor( void* ptr )
	{
		T* obj = (T*)ptr;
		obj->T::~T();
	}

	template< class T >
	T* UiWindow::newState( bool& isNew )
	{
		void (*destructFunc)(void*) = &callDestructor< T >;
		const ImuiId stateId = (ImuiId)(size_t)destructFunc;
		void* memory = imuiWindowAllocStateNewDestruct( m_window, sizeof( T ), stateId, &isNew, destructFunc );
		if( !memory )
		{
			return nullptr;
		}

		T* state = (T*)memory;
		if( isNew )
		{
			new (state) T();
		}

		return state;
	}

	template< class T >
	T* UiWidget::newState()
	{
		bool isNew;
		return newState< T >( isNew );
	}

	template< class T >
	T* UiWidget::newState( bool& isNew )
	{
		void (*destructFunc)( void* ) = &callDestructor< T >;
		const ImuiId stateId = (ImuiId)(size_t)destructFunc;
		void* memory = imuiWidgetAllocStateNewDestruct( m_widget, sizeof( T ), stateId, &isNew, destructFunc );
		if( !memory )
		{
			return nullptr;
		}

		T* state = (T*)memory;
		if( isNew )
		{
			new (state) T();
		}

		return state;
	}

	template< class T >
	class UiAnimation : public UiNonCopyable
	{
	public:

		UiAnimation( UiWidget& widget, T minValue, T maxValue, double timeSpan, bool backwards = false )
		{
			const double currentTime = widget.getTime();

			bool isNew;
			m_state = widget.newState< State >( isNew );

			if( isNew )
			{
				m_state->startTime = currentTime;
				m_state->backwards = backwards;
			}

			m_progress = float( (currentTime - m_state->startTime) / timeSpan );
			m_progress = m_progress > 1.0f ? 1.0f : m_progress;

			if( m_state->backwards != backwards )
			{
				m_state->startTime = currentTime - (timeSpan * (1.0 - m_progress));
				m_state->backwards = backwards;
			}
			else if( m_state->backwards )
			{
				m_progress = 1.0f - m_progress;
			}

			m_value = minValue + ((maxValue - minValue) * m_progress);
		}

		T getValue() const
		{
			return m_value;
		}

		float getProgrss() const
		{
			return m_progress;
		}

	private:

		struct State
		{
			double	startTime;
			bool	backwards;
		};

		T			m_value;
		float		m_progress;
		State*		m_state;
	};
}
