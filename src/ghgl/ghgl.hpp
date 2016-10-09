#ifndef GHGL_H
#define GHGL_H

#include "common.hpp"

namespace ghgl
{
  class GhGL
  {
  public:
    struct timer
    {
        struct timeval start;
    };

    GhGL(const std::string & name = "GhGL");
    virtual ~GhGL(){}

    int Run();

  protected:
    virtual bool OnInitialize() { return true; }
    virtual bool OnInitializeData() { return true; }
    virtual void OnClear();
    virtual void OnUpdateProjection();
    virtual void OnDraw() {}
    virtual void OnMouseDown(int data, int xpos,  int ypos ) {}
    virtual void OnMouseUp(int data, int xpos,  int ypos ) {}
    virtual void OnMouseMove(int data, int xpos,  int ypos ) {}
    virtual void OnKeyDown(int data) {}
    virtual void OnKeyUp(int data) {}

    float Timer( struct timer *t, int reset );
    void HideMouse();
    void MoveMouse(int x, int y);
    void Printf( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha,
                GLint x, GLint y, GLuint m_Font, const char *format, ... );

    GLfloat GetFPS() const { return m_FPS; }
    GLuint GetFont() const { return m_Font; }

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

  private:
    bool Initialize();
    void Draw();
    void HandleResize();

    int InitializeGLX( int fullscreen );

  private:
    std::string m_AppTitle;
    int m_Width;
    int m_Height;
    GLuint m_Font;

    Display * m_Dpy;
    Window m_Win;
    Atom m_wmDelete;
    GLXContext m_Ctx;
    Cursor m_NullCursor;

    int m_ModeSwitch;
    int m_Active;
    int m_Run;

    int m_Info;
    int m_Frames;
    GLfloat m_FPS;
    struct timer tv;

  };
} // namespace ghgl

#endif // GHGL_H
