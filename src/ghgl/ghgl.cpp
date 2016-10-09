#include "ghgl.hpp"

namespace ghgl
{
  GhGL::GhGL(const std::string & name /*= "GhGL"*/)
    : m_AppTitle(name)
    , m_Width(640)
    , m_Height(480)
    , m_Font(0)
    , m_Dpy(0)
    , m_ModeSwitch(0)
    , m_Active(0)
    , m_Run(0)
    , m_Info(1)
    , m_Frames(0)
    , m_FPS(0)
  {
  }


  int GhGL::Run()
  {
    int fullscreen;
    XEvent event;

    if(!OnInitializeData())
    {
      fprintf( stderr, "OnInitializeData failed\n" );
      return( 1 );
    }

    fullscreen = 1;

    do
    {
      m_ModeSwitch  = 0;
      fullscreen ^= 1;

      if( InitializeGLX( fullscreen ) )
      {
        fprintf( stderr, "InitializeGLX failed\n" );
        return( 1 );
      }

      if(!OnInitialize())
      {
        fprintf( stderr, "OnInitialize failed\n" );
        return( 1 );
      }

      HideMouse();
      HandleResize();

      m_Run = 1;

      while( m_Run )
      {
        if( m_Active )
        {
          Draw();
          glXSwapBuffers( m_Dpy, m_Win );
        }
        else
        {
          XPeekEvent( m_Dpy, &event );
        }

        while( XPending( m_Dpy ) )
        {
          XNextEvent( m_Dpy, &event );

          switch( event.type )
          {
            case ButtonPress:
            {
            int x = event.xmotion.x,
              y = event.xmotion.y;

              switch( event.xbutton.button )
              {
                case Button1: OnMouseDown(0, x, y); break;
                case Button3: OnMouseDown(1, x, y); break;
              }

              break;
            }

            case ButtonRelease:
            {
              int x = event.xmotion.x,
                y = event.xmotion.y;

              switch( event.xbutton.button )
              {
                case Button1: OnMouseUp(0, x, y); break;
                case Button3: OnMouseUp(1, x, y); break;
              }

              break;
            }

            case MotionNotify:
            {
              int x = event.xmotion.x,
                y = event.xmotion.y;

              switch( event.xbutton.button )
              {
                case Button1: OnMouseMove(0, x, y); break;
                case Button3: OnMouseMove(1, x, y); break;
                default: OnMouseMove(-1, x, y); break;
              }

              break;
            }

            case KeyPress:
            {
              OnKeyDown(XLookupKeysym( &event.xkey, 0 ));
              break;
            }

            case KeyRelease:
            {
              switch( XLookupKeysym( &event.xkey, 0 ) )
              {
                //case XK_space:
                case 'f':
                  m_Info ^= 1;
                  break;
                case XK_Tab:
                  m_ModeSwitch = 1;
                case XK_Escape:
                  m_Run = 0;
                  break;
                default:
                  OnKeyUp(XLookupKeysym( &event.xkey, 0 ));
                  break;
              }
              break;
            }

            case UnmapNotify: m_Active = 0; break;
            case   MapNotify: m_Active = 1; break;

            case ConfigureNotify:
            {
              m_Width  = event.xconfigure.width;
              m_Height = event.xconfigure.height;
              HandleResize();
              break;
            }

            case ClientMessage:
            {
              if( event.xclient.data.l[0] == (int) m_wmDelete )
              {
                m_Active = m_Run = 0;
              }
              break;
            }

            case ReparentNotify: break;

            default:
            {
              printf( "caught unknown event, type %d\n",
                  event.type );
              break;
            }
          }
        }
      }

      glXMakeCurrent( m_Dpy, None, NULL );
      glXDestroyContext( m_Dpy, m_Ctx );
      XDestroyWindow( m_Dpy, m_Win );
      XCloseDisplay( m_Dpy );
    }
    while( m_ModeSwitch );
    return( 0 );
  }

  int GhGL::InitializeGLX( int fullscreen )
  {
    int vi_attr[] =
      {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RED_SIZE,     8,
        GLX_GREEN_SIZE,   8,
        GLX_BLUE_SIZE,    8,
        GLX_DEPTH_SIZE,  24,
        None
      };

    XVisualInfo *vi;
    Window root_win;
    XWindowAttributes win_attr;
    XSetWindowAttributes set_attr;
    XFontStruct *fixed;
    XColor black =
    {
      0, 0, 0, 0, 0, 0
    };

    if( ( m_Dpy = XOpenDisplay( NULL ) ) == NULL )
    {
      fprintf( stderr, "XOpenDisplay failed\n" );
      return( 1 );
    }

    if( ( vi = glXChooseVisual( m_Dpy, DefaultScreen( m_Dpy ),
                  vi_attr ) ) == NULL )
    {
      fprintf( stderr, "glXChooseVisual failed\n" );
      XCloseDisplay( m_Dpy );
      return( 1 );
    }

    root_win = RootWindow( m_Dpy, vi->screen );

    XGetWindowAttributes( m_Dpy, root_win, &win_attr );

    m_Width  = ( fullscreen ) ? win_attr.width  : 640;
    m_Height = ( fullscreen ) ? win_attr.height : 480;

    set_attr.border_pixel = 0;

    set_attr.colormap =
      XCreateColormap( m_Dpy, root_win, vi->visual, AllocNone );

    set_attr.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask |
      ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;

    set_attr.override_redirect = ( ( fullscreen ) ? True : False );

    m_Win =
      XCreateWindow(
          m_Dpy, root_win, 0, 0, m_Width, m_Height, 0, vi->depth,
          InputOutput, vi->visual, CWBorderPixel | CWColormap |
          CWEventMask | CWOverrideRedirect, &set_attr );

    XStoreName( m_Dpy, m_Win, m_AppTitle.c_str() );
    XMapWindow( m_Dpy, m_Win );

    if( fullscreen )
    {
      XGrabKeyboard(  m_Dpy, m_Win, True, GrabModeAsync,
              GrabModeAsync, CurrentTime );
    }
    else
    {
      m_wmDelete = XInternAtom( m_Dpy, "WM_DELETE_WINDOW", True );
      XSetWMProtocols( m_Dpy, m_Win, &m_wmDelete, 1 );
    }

    if( ( m_Ctx = glXCreateContext( m_Dpy, vi, NULL, True ) ) == NULL )
    {
      fprintf( stderr, "glXCreateContext failed\n" );
      XDestroyWindow( m_Dpy, m_Win );
      XCloseDisplay( m_Dpy );
      return( 1 );
    }

    if( glXMakeCurrent( m_Dpy, m_Win, m_Ctx ) == False )
    {
      fprintf( stderr, "glXMakeCurrent failed\n" );
      glXDestroyContext( m_Dpy, m_Ctx );
      XDestroyWindow( m_Dpy, m_Win );
      XCloseDisplay( m_Dpy );
      return( 1 );
    }

    m_Font = glGenLists( 256 );

    fixed = XLoadQueryFont(
      m_Dpy, "-misc-fixed-medium-r-*-*-12-*-*-*-*-*-*-*" );

    m_NullCursor = XCreateGlyphCursor(
      m_Dpy, fixed->fid, fixed->fid, ' ', ' ', &black, &black );

    glXUseXFont( fixed->fid, 0, 256, m_Font );

    XFreeFont( m_Dpy, fixed );

    return( 0 );
  }

  float GhGL::Timer( struct GhGL::timer *t, int reset )
  {
    float delta;
    struct timeval offset;

    gettimeofday( &offset, NULL );

    delta = (float) ( offset.tv_sec  - t->start.tv_sec  ) +
        (float) ( offset.tv_usec - t->start.tv_usec ) / 1e6;

    if( reset )
    {
      t->start.tv_sec  = offset.tv_sec;
      t->start.tv_usec = offset.tv_usec;
    }

    return( delta );
  }

  void GhGL::HideMouse()
  {
    XDefineCursor( m_Dpy, m_Win, m_NullCursor );
  }

  void GhGL::MoveMouse( int x, int y )
  {
    XWarpPointer( m_Dpy, None, m_Win, 0, 0, 0, 0, x, y );
  }

  void GhGL::OnClear()
  {
    glClearColor(0.0,0.0,0.0,1.0);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
  }

  void GhGL::Draw()
  {
    OnClear();
    OnDraw();

    if( m_FPS && m_Info )
    {
      Printf( 0.7f, 0.7f, 0.7f, 1.0f, m_Width - 64, m_Height - 20,
            m_Font, "%5.1f fps", m_FPS );
    }

    glFinish();

    m_Frames++;

    if( Timer( &tv, 0 ) >= 0.2f )
    {
      m_FPS = (GLfloat) m_Frames / Timer( &tv, 1 );
      m_Frames = 0;
    }
  }

  void GhGL::OnUpdateProjection()
  {
    glViewport( 0, 0, m_Width, m_Height );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 50.0, (GLdouble) m_Width / m_Height, 0.1, 100.0 );
  }

  void GhGL::HandleResize()
  {
    OnUpdateProjection();

    m_FPS = 0.0f;
    m_Frames = 0;
    Timer( &tv, 1 );
  }

  void GhGL::Printf( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha,
          GLint x, GLint y, GLuint m_Font, const char *format, ... )
  {
    va_list argp;
    char text[256];

    va_start( argp, format );
    vsprintf( text, format, argp );
    va_end( argp );

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();

      glLoadIdentity();
      gluOrtho2D( 0.0, (GLdouble) m_Width,
            0.0, (GLdouble) m_Height );

      glMatrixMode( GL_MODELVIEW );
      glLoadIdentity();

      glColor4f( red, green, blue, alpha );
      glRasterPos2i( x, y );
      glListBase( m_Font );
      glCallLists( strlen( text ), GL_UNSIGNED_BYTE, text );

    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
  }

} // namespace ghgl
