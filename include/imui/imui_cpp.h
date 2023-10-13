#pragma once

#include "imui/imui.h"
#include "imui/imui_toolbox.h"

#include <new>

namespace imui
{
	class UiInput;
	class UiInputState;
	struct UiSize;

	struct UiStringView : public ImUiStringView
	{
						UiStringView();
						UiStringView( const char* str, size_t _length );
		template< size_t TLen >
						UiStringView( const char (&str)[ TLen ] );
		explicit		UiStringView( const ImUiStringView& value );

		const char*		getData() const;
		size_t			getLength() const;

		bool			isEmpty() const;

		bool			operator==( const UiStringView& rhs ) const;
		bool			operator!=( const UiStringView& rhs ) const;
	};

	struct UiAlign : public ImUiAlign
	{
						UiAlign();
						UiAlign( float hAlign, float vAlign );

		static UiAlign	Center;
	};

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
		static UiSize	Horizontal;
		static UiSize	Vertical;
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

		UiStringView		getText() const;

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
						UiSurface( ImUiFrame* frame, const UiStringView& name, const UiSize& size, float dpiScale );
						UiSurface( UiFrame& frame, const UiStringView& name, const UiSize& size, float dpiScale );
						~UiSurface();

		void			beginSurface( ImUiFrame* frame, const UiStringView& name, const UiSize& size, float dpiScale );
		void			beginSurface( UiFrame& frame, const UiStringView& name, const UiSize& size, float dpiScale );
		void			endSurface();

		bool			isValid() const;
		ImUiSurface*	getInternal() const;
		UiContext		getContext() const;

		float			getTime() const;
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
						UiWindow( ImUiSurface* surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder );
						UiWindow( UiSurface& surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder );
						~UiWindow();

		void			beginWindow( ImUiSurface* surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder );
		void			beginWindow( UiSurface& surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder );
		void			endWindow();

		bool			isValid() const;
		ImUiWindow*		getInternal() const;
		UiContext		getContext() const;
		UiSurface		getSurface() const;

		float			getTime() const;
		UiRect			getRect() const;
		uint32_t		getZOrder() const;

	protected:

		bool			m_owner;
		ImUiWindow*		m_window;
	};

	class UiWidget
	{
	public:

						UiWidget();
						UiWidget( UiWindow& window );
						UiWidget( UiWindow& window, ImUiId id );
						UiWidget( UiWindow& window, const UiStringView& name );
						~UiWidget();

		void			beginWidget( UiWindow& window );
		void			beginWidget( UiWindow& window, ImUiId id );
		void			beginWidget( UiWindow& window, const UiStringView& name );
		void			endWidget();

		bool			isValid() const;
		ImUiWidget*		getInternal() const;
		UiContext		getContext() const;
		UiSurface		getSurface() const;
		UiWindow		getWindow() const;

		void*			allocState( size_t size );
		void*			allocState( size_t size, bool& isNew );
		template< class T >
		T*				newState();
		template< class T >
		T*				newState( bool& isNew );

		ImUiLayout		getLayout() const;
		void			setLayoutStack();							// default
		void			setLayoutScroll( UiPos offset );
		void			setLayoutHorizontal();
		void			setLayoutHorizontalSpacing( float spacing );
		void			setLayoutVertical();
		void			setLayoutVerticalSpacing( float spacing );
		void			setLayoutGrid( size_t columnCount );

		float			getTime();

		UiBorder		getMargin() const;
		void			setMargin( const UiBorder& margin );
		UiBorder		getPadding() const;
		void			setPadding( const UiBorder& padding );

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

		UiSize			getStretch() const;
		void			setStretch( UiSize stretch );

		UiAlign			getAlign() const;
		void			setAlign( UiAlign align );
		void			setHAlign( float align );
		void			setVAlign( float align );

		UiPos			getPos() const;
		UiSize			getSize() const;
		UiRect			getRect() const;
		UiSize			getInnerSize();
		UiRect			getInnerRect();

		void			getInputState( ImUiWidgetInputState& inputState ) const;

		void			drawLine( UiPos p0, UiPos p1, UiColor color );
		void			drawWidgetColor( UiColor color );
		void			drawWidgetTexture( const ImUiTexture& texture );
		void			drawWidgetTextureColor( const ImUiTexture& texture, UiColor color );
		void			drawWidgetSkin( const ImUiSkin& skin );
		void			drawWidgetSkinColor( const ImUiSkin& skin, UiColor color );
		void			drawWidgetText( ImUiTextLayout* layout );
		void			drawWidgetTextColor( ImUiTextLayout* layout, UiColor color );
		void			drawRectColor( const UiRect& rect, ImUiColor color );
		void			drawRectTexture( const UiRect& rect, const ImUiTexture& texture );
		void			drawRectTextureUv( const UiRect& rect, const ImUiTexture& texture, const UiTexCoord& uv );
		void			drawRectTextureColor( const UiRect& rect, const ImUiTexture& texture, UiColor color );
		void			drawRectTextureColorUv( const UiRect& rect, const ImUiTexture& texture, UiColor color, const UiTexCoord& uv );
		void			drawSkin( const UiRect& rect, const ImUiSkin& skin );
		void			drawSkinColor( const UiRect& rect, const ImUiSkin& skin, UiColor color );
		void			drawText( UiPos pos, ImUiTextLayout* layout );
		void			drawTextColor( UiPos pos, ImUiTextLayout* layout, UiColor color );

	protected:

		ImUiWidget*		m_widget;
	};

	class UiWidgetLayoutHorizontal : public UiWidget
	{
	public:

						UiWidgetLayoutHorizontal( UiWindow& window );
						UiWidgetLayoutHorizontal( UiWindow& window, float spacing );
	};

	class UiWidgetLayoutVertical : public UiWidget
	{
	public:

						UiWidgetLayoutVertical( UiWindow& window );
						UiWidgetLayoutVertical( UiWindow& window, float spacing );
	};

	namespace toolbox
	{
		struct UiToolboxConfig : public ImUiToolboxConfig
		{
						UiToolboxConfig();
						UiToolboxConfig( ImUiFont* font );

			void		setDefault( ImUiFont* font );
		};

		class UiToolboxWindow : public UiWindow
		{
		public:

							UiToolboxWindow();
							UiToolboxWindow( UiWindow& window );
							UiToolboxWindow( ImUiWindow* window );
							UiToolboxWindow( ImUiSurface* surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder );
							UiToolboxWindow( UiSurface& surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder );

			void			spacer( float width, float height );
			void			strecher( float horizontal, float vertical );

			bool			buttonLabel( const UiStringView& text );
			bool			buttonLabelFormat( const char* format, ... );
			bool			buttonIcon( const ImUiTexture& icon );
			bool			buttonIcon( const ImUiTexture& icon, UiSize iconSize );

			bool			checkBox( bool& checked, const UiStringView& text );
			bool			checkBoxState( const UiStringView& text, bool defaultValue = false );

			void			label( const UiStringView& text );
			void			labelFormat( const char* format, ... );

			bool			slider( float& value, float min = 0.0f, float max = 1.0f );
			float			sliderState( float min = 0.0f, float max = 1.0f );
			float			sliderState( float min, float max, float defaultValue );

			bool			textEdit( char* buffer, size_t bufferSize, size_t* textLength = nullptr );
			UiStringView	textEditState( size_t bufferSize, UiStringView defaultValue = UiStringView() );

			void			progressBar( float value, float min = 0.0f, float max = 1.0f );

			size_t			dropDown( const UiStringView* items, size_t itemCount );
		};

		class UiToolboxButtonLabel : public UiWidget
		{
		public:

						UiToolboxButtonLabel();
						UiToolboxButtonLabel( UiWindow& window, const UiStringView& text );
						~UiToolboxButtonLabel();

			void		begin( UiWindow& window, const UiStringView& text );
			void		beginFormat( UiWindow& window, const char* format, ... );
			bool		end();
		};

		class UiToolboxLabel : public UiWidget
		{
		public:

						UiToolboxLabel();
						UiToolboxLabel( UiWindow& window, const UiStringView& text );
						~UiToolboxLabel();

			void		begin( UiWindow& window, const UiStringView& text );
			void		beginFormat( UiWindow& window, const char* format, ... );
			void		end();
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

		protected:

						UiToolboxScrollArea();
		};

		class UiToolboxList : public UiToolboxScrollArea
		{
		public:

						UiToolboxList( UiWindow& window, float itemSize, size_t itemCount );
						~UiToolboxList();

			size_t		getBeginIndex() const;
			size_t		getEndIndex() const;
			size_t		getSelectedIndex() const;
			void		setSelectedIndex( size_t index );

			ImUiWidget*	nextItem();

		private:

			ImUiToolboxListContext	m_list;
		};

		class UiToolboxDropdown : public UiWidget
		{
		public:

						UiToolboxDropdown( UiWindow& window, const UiStringView* items, size_t itemCount );
						//UiToolboxDropdown( UiWindow& window, const UiStringView& selectedItem, size_t itemCount );
						~UiToolboxDropdown();

			//bool		isOpen() const;

			//size_t		getBeginIndex() const;
			//size_t		getEndIndex() const;
			size_t		getSelectedIndex() const;
			void		setSelectedIndex( size_t index );

			//void		nextItem( const UiStringView& item );

		private:

			ImUiToolboxDropDownContext	m_dropDown;
		};

		class UiToolboxPopup : public UiToolboxWindow
		{
		public:

						UiToolboxPopup( UiSurface& surface );
						UiToolboxPopup( UiWindow& window );
						~UiToolboxPopup();

			size_t 		end( const UiStringView* buttons, size_t buttonCount );
			void		end();
		};

		void			setConfig( const UiToolboxConfig& config );
	}

	template< size_t TLen >
	UiStringView::UiStringView( const char( &str )[ TLen ] )
	{
		data	= str;
		length	= TLen - 1u;
	}

	template< class T >
	T* UiWidget::newState()
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
	T* UiWidget::newState( bool& isNew )
	{
		void* memory = ImUiWidgetAllocStateNewDestruct( m_widget, sizeof( T ), &isNew, &callDestructor< T > );
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
}
