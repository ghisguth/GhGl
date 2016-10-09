#include "ghgl/ghgl.hpp"

class Spiral
  : public ghgl::GhGL
{
public:

  Spiral()
    : GhGL("spiral")
    , m_Texture(0)
    , m_Spiral(0)
    , m_Atoms(0)
  {}

protected:

  #define NB_ATOMS 2048

  #define CREATE_PARTICLE(p)                  \
  {                               \
    p[0] = 0.6f + (GLfloat) rand() / ( 2.5f * RAND_MAX );   \
    p[1] = 0.6f + (GLfloat) rand() / ( 2.5f * RAND_MAX );   \
    p[2] = 0.6f + (GLfloat) rand() / ( 2.5f * RAND_MAX );   \
    p[3] = 0.6f + (GLfloat) rand() / ( 2.5f * RAND_MAX );   \
    p[4] = 0.5f - 1.0f * (GLfloat) rand() / RAND_MAX;     \
    p[5] = 0.5f - 1.0f * (GLfloat) rand() / RAND_MAX;     \
    p[6] = 0.5f + 2.0f * (GLfloat) rand() / RAND_MAX;     \
    p[7] = 40.0f * (GLfloat) rand() / RAND_MAX;       \
  }

  bool OnInitializeData()
  {
    int i = 0;

    m_Spiral = 0.0f;

    srand( (unsigned int) time( NULL ) );

    m_Atoms = (GLfloat **) malloc( NB_ATOMS * sizeof( GLfloat * ) );

    if( m_Atoms == NULL )
    {
      perror( "malloc" );
      return false;
    }

    for( i = 0; i < NB_ATOMS; i++)
    {
      m_Atoms[i] = (GLfloat *) malloc( 8 * sizeof( GLfloat ) );

      if( m_Atoms[i] == NULL )
      {
        perror( "malloc" );
        return false;
      }

      CREATE_PARTICLE( m_Atoms[i] );
    }

    return true;
  }

  bool OnInitialize()
  {
    int i, j, k;
    double dx, dy, alpha;
    unsigned char *map, c;

    if( ! ( map = (unsigned char *) malloc( 128 * 128 * 3 ) ) )
    {
      perror( "malloc" );
      return false;
    }

    k = 0;
    for( i = 0; i < 128; i++ )
    {
      dx = 1.5 * M_PI * (double) ( i - 64 ) / 128.0;

      for( j = 0; j < 128; j++ )
      {
        dy = 1.5 * M_PI * (double) ( j - 64 ) / 128.0;
        alpha = 0.4 + cos( sqrt( dx * dx + dy * dy ) ) / 2;
        if( alpha < 0.0 ) alpha = 0.0;
        c = (unsigned char) ( alpha * 255.0 );
        map[k++] = c;
        map[k++] = c;
        map[k++] = c;
      }
    }

    glGenTextures( 1, &m_Texture );

    glBindTexture( GL_TEXTURE_2D, m_Texture );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    glTexImage2D( GL_TEXTURE_2D, 0, 3, 128, 128, 0,
            GL_RGB, GL_UNSIGNED_BYTE, map );

    free( map );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    return true;
  }

  void OnDraw()
  {
    int i;
    float t, x, y, z;

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glRotatef( m_Spiral, 0.0f, 0.0f, 1.0f );

    if( GetFPS() )
    {
      m_Spiral += 40.0f / GetFPS();
    }

    glEnable( GL_TEXTURE_2D );

    for( i = 0; i < NB_ATOMS; i++ )
    {
      glColor4f( m_Atoms[i][0], m_Atoms[i][1],
            m_Atoms[i][2], m_Atoms[i][3] );
      t = m_Atoms[i][7];

      x = t * m_Atoms[i][4];
      y = t * m_Atoms[i][5];
      z = t * m_Atoms[i][6] - 100.0f;

      if( GetFPS() )
      {
        m_Atoms[i][7] += 30.0f / GetFPS();
      }

      if( z >= 0.0f )
      {
        CREATE_PARTICLE( m_Atoms[i] );
      }

      glBegin(GL_TRIANGLE_STRIP);

      float ps = 0.1f; // particle scale
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  ps + x,  ps + y, z );
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -ps + x,  ps + y, z );
      glTexCoord2d( 1.0f, 0.0f ); glVertex3f(  ps + x, -ps + y, z );
      glTexCoord2d( 0.0f, 0.0f ); glVertex3f( -ps + x, -ps + y, z );

      glEnd();
    }

    glDisable( GL_TEXTURE_2D );
  }
private:
    GLuint m_Texture;
    GLfloat m_Spiral;
    GLfloat **m_Atoms;
};

int main(int /*argc*/, char** /*argv*/)
{
  Spiral app;
  return app.Run();
}

