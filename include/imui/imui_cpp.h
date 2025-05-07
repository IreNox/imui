#pragma once

#include "imui/imui.h"
#include "imui/imui_toolbox.h"

#include <new>

namespace imui
{
	class UiInput;
	class UiInputState;
	struct UiSize;

	struct UiBorder : public ImUiBorder
	{
						UiBorder();
						UiBorder( float all );
						UiBorder( float horizontal, float vertical );
						UiBorder( float _top, float _left, float _bottom, float _right );
						UiBorder( const ImUiBorder& value );

		UiSize			getMinSize() const;
	};

	struct UiPos : public ImUiPos
	{
						UiPos();
						UiPos( float all );
						UiPos( float _x, float _y );
						UiPos( const ImUiPos& value );
		explicit		UiPos( const ImUiSize& value );

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

	struct UiRect : public ImUiRect
	{
						UiRect();
						UiRect( UiPos _pos, UiSize _size );
						UiRect( float x, float y, float width, float height );
						UiRect( const ImUiRect& value );

		UiRect			shrinkBorder( const ImUiBorder& border ) const;

		bool			includesPos( ImUiPos _pos ) const;
		bool			intersectsRect( const ImUiRect& rect2 ) const;

		float			getRight() const;
		float			getBottom() const;

		UiPos			getTopLeft() const;
		UiPos			getTopRight() const;
		UiPos			getBottomLeft() const;
		UiPos			getBottomRight() const;

		UiSize			getSize() const;
	};

	struct UiSize : public ImUiSize
	{
						UiSize();
						UiSize( float all );
						UiSize( float _width, float _height );
						UiSize( const ImUiSize& value );
						UiSize( const ImUiSkin& value );
						UiSize( const ImUiImage& value );

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

	struct UiColor : public ImUiColor
	{
						UiColor();
						UiColor( uint8_t _red, uint8_t _green, uint8_t _blue );
						UiColor( uint8_t _red, uint8_t _green, uint8_t _blue, uint8_t _alpha );
						UiColor( float _red, float _green, float _blue );
						UiColor( float _red, float _green, float _blue, float _alpha );
		explicit		UiColor( ImUiColor value );

		static UiColor	CreateWhite( uint8_t _alpha );
		static UiColor	CreateBlack( uint8_t _alpha );
		static UiColor	CreateGray( uint8_t gray );
		static UiColor	CreateGray( uint8_t gray, uint8_t _alpha );

		static UiColor	White;
		static UiColor	Black;
		static UiColor	TransparentBlack;
	};

	struct UiTexCoord : public ImUiTexCoord
	{
							UiTexCoord( float _u0, float _v0, float _u1, float _v1 );

		static UiTexCoord	ZeroToOne;
	};

	struct UiContextParameters : public ImUiParameters
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
						UiContext( ImUiContext* context );
						UiContext( const UiContextParameters& parameters );
						~UiContext();

		bool			isValid() const;
		ImUiContext*	getInternal() const;

		UiInput&		beginInput();
		void			endInput();

		UiInputState	getInput() const;
		void			setMouseCursor( ImUiInputMouseCursor cursor );

	private:

		bool			m_owner;
		ImUiContext*	m_context;
	};

	class UiInputState
	{
		friend class UiContext;

	public:

		uint32_t			getKeyModifiers() const;	// returns ImUiInputModifier
		bool				isKeyDown( ImUiInputKey key ) const;
		bool				isKeyUp( ImUiInputKey key ) const;
		bool				hasKeyPressed( ImUiInputKey key ) const;
		bool				hasKeyReleased( ImUiInputKey key ) const;

		const char*			getText() const;

		UiPos				getMousePos() const;
		bool				isMouseInRect( UiRect rect ) const;
		bool				isMouseButtonDown( ImUiInputMouseButton button ) const;
		bool				isMouseButtonUp( ImUiInputMouseButton button ) const;
		bool				hasMouseButtonPressed( ImUiInputMouseButton button ) const;
		bool				hasMouseButtonReleased( ImUiInputMouseButton button ) const;
		UiPos				getMouseScrollDelta() const;

	private:

							UiInputState( const ImUiContext* imui );

		const ImUiContext*	m_context;
	};

	class UiFrame
	{
	public:

						UiFrame();
						UiFrame( ImUiFrame* frame );
						UiFrame( UiContext& context, float timeInSeconds );
						~UiFrame();

		void			beginFrame( UiContext& context, float timeInSeconds );
		void			endFrame();

		bool			isValid() const;
		ImUiFrame*		getInternal() const;

	private:

		bool			m_owner;
		ImUiFrame*		m_frame;
	};

	class UiSurface
	{
	public:

						UiSurface();
						UiSurface( ImUiSurface* surface );
						UiSurface( ImUiFrame* frame, const char* name, const ImUiSize& size, float dpiScale );
						UiSurface( UiFrame& frame, const char* name, const ImUiSize& size, float dpiScale );
						~UiSurface();

		void			beginSurface( ImUiFrame* frame, const char* name, const ImUiSize& size, float dpiScale );
		void			beginSurface( UiFrame& frame, const char* name, const ImUiSize& size, float dpiScale );
		void			endSurface();

		bool			isValid() const;
		ImUiSurface*	getInternal() const;
		UiContext		getContext() const;

		double			getTime() const;
		UiRect			getRect() const;
		UiSize			getSize() const;
		float			getDpiScale() const;

	private:

		bool			m_owner;
		ImUiSurface*	m_surface;
	};

	class UiWindow
	{
	public:

						UiWindow();
						UiWindow( ImUiWindow* window );
						UiWindow( ImUiSurface* surface, const char* name, const ImUiRect& rect, uint32_t zOrder );
						UiWindow( UiSurface& surface, const char* name, const ImUiRect& rect, uint32_t zOrder );
						~UiWindow();

		void			beginWindow( ImUiSurface* surface, const char* name, const ImUiRect& rect, uint32_t zOrder );
		void			beginWindow( UiSurface& surface, const char* name, const ImUiRect& rect, uint32_t zOrder );
		void			endWindow();

		bool			isValid() const;
		ImUiWindow*		getInternal() const;
		UiContext		getContext() const;
		UiSurface		getSurface() const;

		double			getTime() const;

		void*			allocState( size_t size, ImUiId stateId );
		void*			allocState( size_t size, ImUiId stateId, bool& isNew );
		template< class T >
		T*				newState();
		template< class T >
		T*				newState( bool& isNew );

		UiRect			getRect() const;
		uint32_t		getZOrder() const;

	protected:

		bool			m_owner;
		ImUiWindow*		m_window;
	};

	class UiWidget : public UiNonCopyable
	{
	public:

						UiWidget();
		explicit		UiWidget( UiWindow& window );
						UiWidget( UiWindow& window, ImUiId id );
						UiWidget( UiWindow& window, const char* name );
		explicit		UiWidget( ImUiWidget* widget );
						~UiWidget();

		void			beginWidget( UiWindow& window );
		void			beginWidget( UiWindow& window, ImUiId id );
		void			beginWidget( UiWindow& window, const char* name );
		void			beginWidget( ImUiWidget* widget );
		void			endWidget();

		bool			isValid() const;
		ImUiWidget*		getInternal() const;
		UiContext		getContext() const;
		UiSurface		getSurface() const;
		UiWindow		getWindow() const;

		double			getTime();

		void*			allocState( size_t size, ImUiId stateId );
		void*			allocState( size_t size, ImUiId stateId, bool& isNew );
		template< class T >
		T*				newState();
		template< class T >
		T*				newState( bool& isNew );

		ImUiLayout		getLayout() const;
		void			setLayoutStack();							// default
		void			setLayoutScroll( float offsetX, float offsetY );
		void			setLayoutHorizontal( float spacing = 0.0f );
		void			setLayoutVertical( float spacing = 0.0f );
		void			setLayoutGrid( uint32_t columnCount, float colSpacing = 0.0f, float rowSpacing = 0.0f );

		UiBorder		getMargin() const;
		void			setMargin( const ImUiBorder& margin );
		UiBorder		getPadding() const;
		void			setPadding( const ImUiBorder& padding );

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

		void			getInputState( ImUiWidgetInputState& inputState ) const;

		void			drawLine( ImUiPos p0, ImUiPos p1, ImUiColor color );
		void			drawTriangle( ImUiPos p0, ImUiPos p1, ImUiPos p2, ImUiColor color );
		void			drawColor( ImUiColor color );
		void			drawImage( const ImUiImage& image );
		void			drawImage( const ImUiImage& image, ImUiColor color );
		void			drawSkin( const ImUiSkin& skin, ImUiColor color );
		void			drawText( ImUiTextLayout* layout, ImUiColor color );
		void			drawPartialColor( const ImUiRect& rect, ImUiColor color );
		void			drawPartialImage( const ImUiRect& rect, const ImUiImage& image );
		void			drawPartialImage( const ImUiRect& rect, const ImUiImage& image, ImUiColor color );
		void			drawPartialSkin( const ImUiRect& rect, const ImUiSkin& skin, ImUiColor color );
		void			drawPositionText( ImUiPos pos, ImUiTextLayout* layout, ImUiColor color );

	protected:

		bool			m_owner;
		ImUiWidget*		m_widget;
	};

	class UiWidgetLayoutHorizontal : public UiWidget
	{
	public:

						UiWidgetLayoutHorizontal( UiWindow& window, float spacing = 0.0f );
	};

	class UiWidgetLayoutVertical : public UiWidget
	{
	public:

						UiWidgetLayoutVertical( UiWindow& window, float spacing = 0.0f );
	};

	class UiWidgetLayoutGrid : public UiWidget
	{
	public:

		UiWidgetLayoutGrid( UiWindow& window, uint32_t columnCount, float colSpacing = 0.0f, float rowSpacing = 0.0f );
	};

	namespace toolbox
	{
		struct UiToolboxTheme : public ImUiToolboxTheme, public UiNonCopyable
		{
						UiToolboxTheme();
						UiToolboxTheme( ImUiFont* inFont );

			void		setDefault( ImUiFont* inFont );

			void		applyConfig();

			static const UiColor&			getColor( ImUiToolboxColor color );
			static const ImUiSkin&			getSkin( ImUiToolboxSkin skin );
			static const ImUiImage&			getIcon( ImUiToolboxIcon icon );

			static const ImUiToolboxTheme& getTheme();
		};

		class UiToolboxConfigFloatScope : public UiNonCopyable
		{
		public:

			UiToolboxConfigFloatScope( const float& value, float newValue );
			~UiToolboxConfigFloatScope();

		private:

			float&				m_value;
			float				m_oldValue;
		};

		class UiToolboxConfigColorScope : public UiNonCopyable
		{
		public:

			UiToolboxConfigColorScope( ImUiToolboxColor color, const ImUiColor& newValue );
			~UiToolboxConfigColorScope();

		private:

			ImUiToolboxColor	m_color;
			ImUiColor			m_oldValue;
		};

		class UiToolboxConfigSkinScope : public UiNonCopyable
		{
		public:

			UiToolboxConfigSkinScope( ImUiToolboxSkin skin, const ImUiSkin& newValue );
			~UiToolboxConfigSkinScope();

		private:

			ImUiToolboxSkin		m_skin;
			ImUiSkin			m_oldValue;
		};

		class UiToolboxConfigIconScope : public UiNonCopyable
		{
		public:

			UiToolboxConfigIconScope( ImUiToolboxIcon icon, const ImUiImage& newValue );
			~UiToolboxConfigIconScope();

		private:

			ImUiToolboxIcon		m_icon;
			ImUiImage			m_oldValue;
		};

		class UiToolboxWindow : public UiWindow
		{
		public:

							UiToolboxWindow();
							UiToolboxWindow( UiWindow& window );
							UiToolboxWindow( ImUiWindow* window );
							UiToolboxWindow( ImUiSurface* surface, const char* name, const ImUiRect& rect, uint32_t zOrder );
							UiToolboxWindow( UiSurface& surface, const char* name, const ImUiRect& rect, uint32_t zOrder );

			void			spacer( float width, float height );
			void			strecher( float horizontal, float vertical );

			bool			buttonLabel( const char* text );
			bool			buttonLabelFormat( const char* format, ... );
			bool			buttonIcon( const ImUiImage& icon );
			bool			buttonIcon( const ImUiImage& icon, ImUiSize iconSize );

			bool			checkBox( bool& checked, const char* text );
			bool			checkBoxState( const char* text, bool defaultValue = false );

			void			label( const char* text );
			void			labelFormat( const char* format, ... );
			void			image( const ImUiImage& img );
			void			image( const ImUiImage& img, const ImUiSize& size );

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
						~UiToolboxLabel();

			void		begin( UiWindow& window, const char* text );
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

			void		setBuffer( char* buffer, size_t bufferSize );

			bool		end( size_t* textLength = nullptr );

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

			ImUiToolboxScrollAreaContext m_scrollArea;
		};

		class UiToolboxList : public UiWidget
		{
		public:

						UiToolboxList( UiWindow& window, float itemSize, size_t itemCount, bool selection );
						~UiToolboxList();

			size_t		getBeginIndex() const;
			size_t		getEndIndex() const;
			size_t		getSelectedIndex() const;
			void		setSelectedIndex( size_t index );

			void		nextItem( UiWidget* widget = nullptr );

		private:

			ImUiToolboxListContext	m_list;
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

			ImUiToolboxDropDownContext	m_dropDown;
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

			bool		header( const char* text );
			void		beginHeader( UiWidget& header );
			bool		endHeader( UiWidget& header );

			void		beginBody( UiWidget& body );
			void		endBody( UiWidget& body );

		private:

			ImUiToolboxTabViewContext	m_context;
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
		const ImUiId stateId = (ImUiId)(size_t)destructFunc;
		void* memory = ImUiWindowAllocStateNewDestruct( m_window, sizeof( T ), stateId, &isNew, destructFunc );
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
		const ImUiId stateId = (ImUiId)(size_t)destructFunc;
		void* memory = ImUiWidgetAllocStateNewDestruct( m_widget, sizeof( T ), stateId, &isNew, destructFunc );
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
