#define GL_GLEXT_PROTOTYPES
#include "ghgl/ghgl.hpp"
#include <GL/glext.h>

class Lines
  : public ghgl::GhGL
{
public:

  Lines()
    : GhGL("lines")
    , textureBlurWidth(256)
    , textureBlurHeight(256)
    , glVersion(0)
    , use_framebuffers_(false)
    , m_Texture(0)
    , m_CurrentTexture(0)
    , m_RenderTexture(0)
    , m_Spiral(0)
    , m_Atoms(0)
  {}

protected:

  bool OnInitializeData()
  {
    theta = 0.0f;
    return true;
  }

  bool OnInitialize()
  {
    glVersion = atof((char*)glGetString(GL_VERSION));
    if(glVersion >= 2.0)
    {
      use_framebuffers_ = true;
    }

    UpdateBlurTexSize();


    glGenTextures( 1, &m_Texture );
    glBindTexture( GL_TEXTURE_2D, m_Texture );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    OnUpdateProjection();

    if(use_framebuffers_)
    {
      m_TextureArray[0] = m_Texture;
      glGenTextures( 1, &m_Texture );
      glBindTexture( GL_TEXTURE_2D, m_Texture );
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      OnUpdateTexture();
      m_TextureArray[1] = m_Texture;

      glGenFramebuffers(1, &m_RenderTexture);
      glBindFramebuffer(GL_FRAMEBUFFER, m_RenderTexture);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture, 0);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glEnable( GL_BLEND );
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glInterleavedArrays(GL_C3F_V3F,0,stars);
    glMatrixMode(GL_MODELVIEW);

    for(int i = 0; i < starsCount; ++i)
    {
      stars[6*i+0] = 0.0f;
      stars[6*i+1] = 0.0f;
      stars[6*i+2] = 0.0f;
      stars[6*i+3] = (((float)rand() / ((float)RAND_MAX + 1)) * 2 - 1) * 4;
      stars[6*i+4] = (((float)rand() / ((float)RAND_MAX + 1)) * 2 - 1) * 4;
      stars[6*i+5] = (((float)rand() / ((float)RAND_MAX + 1)) * 2 - 1) * 4;
    }

    return true;
  }

  void UpdateBlurTexSize()
  {
    if(false && use_framebuffers_)
    {
      textureBlurWidth = GetWidth();
      textureBlurHeight = GetHeight();
    }
    else
    {
      textureBlurWidth = 1 << (int)(log2(GetWidth()));
      textureBlurHeight = 1 << (int)(log2(GetHeight()));
      if(textureBlurWidth == GetWidth())
        textureBlurWidth >>= 1;
      if(textureBlurHeight == GetHeight())
        textureBlurHeight >>= 1;
    }
  }

  void OnUpdateProjection()
  {
    glViewport( 0, 0, GetWidth(), GetHeight() );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum(-0.104f,0.104f,-0.058f,0.058f,0.1f,100.0f);

    UpdateBlurTexSize();

    OnUpdateTexture();
  }

  void OnUpdateTexture()
  {
    std::vector<unsigned char> data;
    data.resize(textureBlurWidth * textureBlurHeight * 3);
    glTexImage2D( GL_TEXTURE_2D, 0, 3, textureBlurWidth, textureBlurHeight, 0,
            GL_RGB, GL_UNSIGNED_BYTE, &data[0] );

  }

  void OnClear()
  {
    glClear(GL_COLOR_BUFFER_BIT);
  }

  void OnDraw()
  {
    glDisable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef(0,0,-4.0);
    glRotatef(theta,0,0,1.0);

    if(use_framebuffers_)
    {
      m_Texture = m_TextureArray[m_CurrentTexture];
      glBindTexture( GL_TEXTURE_2D, m_Texture );
      glBindFramebuffer(GL_FRAMEBUFFER, m_RenderTexture);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureArray[1 - m_CurrentTexture], 0);
      glClear(GL_COLOR_BUFFER_BIT);
    }

    glViewport(0,0,textureBlurWidth,textureBlurHeight);
    RenderMotionBlur();
    glInterleavedArrays(GL_C3F_V3F,0,stars);
    glColor4f(1,1,1,1);
    glDrawArrays(GL_LINES,0,starsCount);


    if(use_framebuffers_)
    {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      m_CurrentTexture = 1 - m_CurrentTexture;
      m_Texture = m_TextureArray[m_CurrentTexture];
      glBindTexture( GL_TEXTURE_2D, m_Texture );
    }
    else
    {
      glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGB,0,0,textureBlurWidth,textureBlurHeight,0);
    }
    glViewport(0,0,GetWidth(),GetHeight());
    glClear(GL_COLOR_BUFFER_BIT);
    RenderMotionBlur();

    float fps_mul = 0.0;
    if( GetFPS() )
    {
      fps_mul = 25.0f / GetFPS();
    }

    for(int i = 0; i < starsCount; ++i)
    {
      float value = stars[6*i+5] + zDelta * fps_mul;
      stars[6*i+5] = value;
      if(value > 4.0)
        stars[6*i+5] = -1;
      value *= colorMul;
      stars[6*i+0] = value;
      stars[6*i+1] = value;
      stars[6*i+2] = value;
    }

    theta += delta * fps_mul;



    /*glDisable(GL_BLEND);
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0,GetWidth(),GetHeight(),0.0,-1.0,1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1,1,1,1);
    glBegin(GL_QUADS);
    glTexCoord2f(0,1.0);
    glVertex2i(100,100);
    glTexCoord2f(0,0);
    glVertex2i(100,300);
    glTexCoord2f(1.0,0);
    glVertex2i(300, 300);
    glTexCoord2f(1,1.0);
    glVertex2i(300,100);
    glEnd();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    */
  }

  void RenderMotionBlur()
  {
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0,GetWidth(),GetHeight(),0.0,-1.0,1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1,1,1.0,0.86);
    glBegin(GL_QUADS);
    glTexCoord2f(0,1.0);
    glVertex2i(0,0);
    glTexCoord2f(0,0);
    glVertex2i(0,GetHeight());
    glTexCoord2f(1.0,0);
    glVertex2i(GetWidth(), GetHeight());
    glTexCoord2f(1,1.0);
    glVertex2i(GetWidth(),0);
    glEnd();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
  }

private:
  static const int starsCount = 2300;
  static const float delta = 0.3 * 0.5;
  static const float zDelta = 0.02 * 0.5;
  static const float colorMul = 0.02;

  float stars[starsCount * 6];
  float theta;
  int textureBlurWidth;
  int textureBlurHeight;

  float glVersion;

  bool use_framebuffers_;

  GLuint m_Texture;
  GLuint m_TextureArray[2];
  int m_CurrentTexture;
  GLuint m_RenderTexture;
  GLfloat m_Spiral;
  GLfloat **m_Atoms;
};

int main(int /*argc*/, char** /*argv*/)
{
  Lines app;
  return app.Run();
}

