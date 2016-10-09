#include "ghgl/ghgl.hpp"
#include "pzlcommon.hpp"
#include "pzlcpf.hpp"

class PazzleSolverGfx
  : public ghgl::GhGL
{
public:

  PazzleSolverGfx()
    : GhGL("PazzleSolver")
    , m_Spiral(0)
    , m_GridAngle(0)
    , m_RotateGrid(true)
    , m_RenderGrid(true)
    , m_RenderField(false)
    , m_RenderExploded(false)
    , m_RenderObjectMask(0x3f)
    , m_RenderAlpha(false)
    , m_RenderInfo(true)
    , m_ManualControl(false)
    , m_ManualRX(0)
    , m_ManualRY(0)
    , m_SolverStarted(false)
    , m_ProcessingSolutions(false)
    , m_SolutionIndex(0)
  {
    m_Blocks[0] = Block(Blockdata[0], 1.f, 0.f, 0.f);
    m_Blocks[1] = Block(Blockdata[1], 0.f, 1.f, 0.f);
    m_Blocks[2] = Block(Blockdata[2], 0.f, 0.f, 1.f);
    m_Blocks[3] = Block(Blockdata[3], 0.7f, 0.7f, 0.f);
    m_Blocks[4] = Block(Blockdata[4], 0.f, 0.7f, 0.7f);
    m_Blocks[5] = Block(Blockdata[5], 0.7f, 0.f, 0.7f);
    m_CorrectPositionFinder.GetEntities(m_Entities);
  }

  virtual ~PazzleSolverGfx()
  {
    m_CorrectPositionFinderThread.interrupt();
    m_CorrectPositionFinderThread.join();
  }

protected:

  virtual void OnKeyUp(int data)
  {
    if(data == (int)'s')
    {
      if(!m_SolverStarted)
      {
        m_SolverStarted = true;
        m_ProcessingSolutions = false;
        m_CorrectPositionFinderThread = boost::thread(boost::ref(m_CorrectPositionFinder));
      }
      else
      {
        m_CorrectPositionFinderThread.interrupt();
      }
    }
    if(data == (int)'r')
    {
      m_RotateGrid = !m_RotateGrid;
    }
    if(data == (int)'g')
    {
      m_RenderGrid = !m_RenderGrid;
    }
    if(data == (int)'h')
    {
      m_RenderField = !m_RenderField;
    }
    if(data == (int)'e')
    {
      m_RenderExploded = !m_RenderExploded;
    }
    if(data >= (int)'1' && data <= (int)'6')
    {
      m_RenderObjectMask ^= (1<<(data-(int)'1'));
    }
    if(data == (int)'n')
    {
      if(!m_Solutions.empty())
        ++m_SolutionIndex %= m_Solutions.size();
    }
    if(data == (int)'p')
    {
      if(!m_Solutions.empty())
      {
        if(m_SolutionIndex > 0)
        {
          --m_SolutionIndex;
        }
        else
        {
          m_SolutionIndex = m_Solutions.size() - 1;
        }
      }
    }
    if(data == (int)'w')
    {
      if(m_ProcessingSolutions)
        m_CorrectPositionFinder.IsConstructionIsStable(m_Field, m_Entities, true);
    }
    if(data == (int)'i')
    {
      m_RenderInfo = !m_RenderInfo;
    }
    if(data == (int)'a')
    {
      m_RenderAlpha = !m_RenderAlpha;
    }
 
    if(data == (int)'m')
    {
      m_ManualControl = !m_ManualControl;
    }
    if(data == (int)'k')
    {
      m_ManualRX = m_ManualRY = 0;
    }
    if(data == XK_Left)
    {
      m_ManualRX -= 5;
    }
    if(data == XK_Right)
    {
      m_ManualRX += 5;
    }
    if(data == XK_Up)
    {
      m_ManualRY -= 5;
      if(m_ManualRY < -90)
        m_ManualRY = -90;
    }
    if(data == XK_Down)
    {
      m_ManualRY += 5;
      if(m_ManualRY > 90)
        m_ManualRY = 90;
    }
  }

  bool OnInitializeData()
  {
    m_Spiral = 0.0f;
    srand( (unsigned int) time( NULL ) );
    return true;
  }

  bool OnInitialize()
  {
    return true;
  }

  void OnDraw()
  {
    

    if(m_SolverStarted)
    {
      if(m_CorrectPositionFinder.IsFinished())
      {
        m_SolverStarted = false;
        m_Solutions = m_CorrectPositionFinder.GetSolutions();
        m_SolutionIndex = 0;
        if(!m_Solutions.empty())
          m_ProcessingSolutions = true;
      }
    }

    if(m_SolverStarted)
    {
      time_t now = time(0);
      if(now - m_LastUpdate > 0)
      {
        m_LastUpdate = now;
        m_CorrectPositionFinder.GetEntities(m_Entities);
      }
    }
    if(m_ProcessingSolutions)
    {
      m_Entities = m_Solutions[m_SolutionIndex];
      m_CorrectPositionFinder.CalcField(m_Field, m_Entities);
    }

    if( GetFPS() )
    {
      m_Spiral += 40.0f / GetFPS();
      if(m_RotateGrid)
      {
        m_GridAngle += 40.0f / GetFPS();
      }
    }

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int drawFrom = 0;
    int drawTo = 6;

    int blockWidth = GetWidth()/6;
    double aspect = (double)blockWidth / 150.0;

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluPerspective( 50.0, aspect, 1.f, 100.0 );

    for(int i = drawFrom; i < drawTo; ++i)
    {
      glViewport( 0 + i * blockWidth, 0, blockWidth, 150);

      glMatrixMode( GL_MODELVIEW );
      glLoadIdentity();
      glTranslatef(0.0f, 0.0f, -22.0f);
      if(m_ManualControl)
      {
        glRotatef( m_ManualRY, 1.0f, 0.0f, 0.0f );
        glRotatef( m_ManualRX, 0.0f, 1.0f, 0.0f );
      }
      else
      {
        glRotatef( 30.f, 1.0f, 0.0f, 0.0f );
        glRotatef( m_GridAngle, 0.0f, 1.0f, 0.0f );
      }
      glRotatef( m_Entities[i].rx * 90.f, 1.0f, 0.0f, 0.0f );
      glRotatef( m_Entities[i].ry * 90.f, 0.0f, 1.0f, 0.0f );
      glRotatef( m_Entities[i].rz * 90.f, 0.0f, 0.0f, 1.0f );
      RenderBlock(m_Entities[i].block, 2.f, 0.25f, true, 0.5f);
    }

    glViewport( 0, 0, GetWidth(), GetHeight());
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    float gridscale=1.0f;
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -18.0f);
    if(m_ManualControl)
    {
      glRotatef( m_ManualRY, 1.0f, 0.0f, 0.0f );
      glRotatef( m_ManualRX, 0.0f, 1.0f, 0.0f );
    }
    else
    {
      glRotatef( 30.f, 1.0f, 0.0f, 0.0f );
      glRotatef( m_GridAngle, 0.0f, 1.0f, 0.0f );
    }
    glScalef(gridscale, gridscale, gridscale);

    if(!m_RenderAlpha)
    {
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LEQUAL);
    }

    for(int i = drawFrom; i < drawTo; ++i)
    {
      if((m_RenderObjectMask&(1<<i)) == 0)
        continue;
      glMatrixMode( GL_MODELVIEW );
      glPushMatrix();
      if(m_RenderExploded)
      {
        float scale = 2.5;
        glTranslatef(m_Entities[i].x*scale, m_Entities[i].y*scale, m_Entities[i].z*scale);
      }
      else
      {
        glTranslatef(m_Entities[i].x, m_Entities[i].y, m_Entities[i].z);
      }
      glRotatef( m_Entities[i].rx * 90.f, 1.0f, 0.0f, 0.0f );
      glRotatef( m_Entities[i].ry * 90.f, 0.0f, 1.0f, 0.0f );
      glRotatef( m_Entities[i].rz * 90.f, 0.0f, 0.0f, 1.0f );
      RenderBlock(m_Entities[i].block, 1.f, m_RenderAlpha?0.15f:1.f, true, m_RenderAlpha?0.35f:1.f);
      glPopMatrix();
    }

    if(m_RenderGrid)
    {
      RenderGrid();
    }

    if(!m_RenderAlpha)
    {
      glDisable(GL_DEPTH_TEST);
    }

    if(m_RenderField)
    {
      if(m_SolverStarted)
      {
        RenderField(m_CorrectPositionFinder.GetWorkField());
      }
      if(!m_SolverStarted && m_ProcessingSolutions)
      {
        RenderField(m_Field);
      }
    }

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluPerspective( 50.0, aspect, 1.f, 100.0 );

    for(int i = 0; i < 6; ++i)
    {
      glViewport( 0 + i * blockWidth, GetHeight() - 150, blockWidth, 150);

      glMatrixMode( GL_MODELVIEW );
      glLoadIdentity();
      glTranslatef(0.0f, 0.0f, -18.0f);
      glRotatef( 30.f, 1.0f, 0.0f, 0.0f );
      glRotatef( (i%2?-1:1)*m_Spiral*2 + i * 122, 0.0f, 1.0f, 0.0f );
      RenderBlock(m_Blocks[i], 2.f, 0.25f, true, 0.5f);
    }

    glViewport( 0, 0, GetWidth(), GetHeight());
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    if(m_SolverStarted)
    {
      int percentage = floor(100.0*(double)m_CorrectPositionFinder.GetCurIt()/(double)m_CorrectPositionFinder.GetNumIt()+0.5);
      Printf(0.7f, 0.7f, 0.7f, 1.0f, 5, GetHeight()/2, GetFont(),
            "%s %d%%", m_CorrectPositionFinder.IsFinished()?"finished":"search", percentage);

      const boost::array<size_t, 6> & brute = m_CorrectPositionFinder.GetBrute();
      Printf(0.7f, 0.7f, 0.7f, 1.0f, 5, GetHeight()/2 - 14, GetFont(),
            "[%03u,%03u,%03u,%03u,%03u,%03u]:%03u", brute[0], brute[1], brute[2], brute[3], brute[4], brute[5],
            m_CorrectPositionFinder.GetNumIt());
      Printf(0.7f, 0.7f, 0.7f, 1.0f, 5, GetHeight()/2 - 14*2, GetFont(),
            "solutions: %03u", m_CorrectPositionFinder.GetSolutionCount());
    }

    if(!m_SolverStarted && m_ProcessingSolutions)
    {
      Printf(0.7f, 0.7f, 0.7f, 1.0f, 5, GetHeight()/2, GetFont(),
            "solution %d/%d", (int)m_SolutionIndex, (int)m_Solutions.size());
    }

    if(m_RenderInfo)
    {

      Printf(1.0f, 1.0f, 1.0f, 1.0f, 5, GetHeight() - 15, GetFont(),
            "[i] - toggle help panel", (int)m_SolutionIndex, (int)m_Solutions.size());
      Printf(1.0f, 1.0f, 1.0f, 1.0f, 5, GetHeight() - 15 - 14*1, GetFont(),
            "[s] - start/stop solver", (int)m_SolutionIndex, (int)m_Solutions.size());
      Printf(1.0f, 1.0f, 1.0f, 1.0f, 5, GetHeight() - 15 - 14*2, GetFont(),
            "[r] - start/stop autorotation", (int)m_SolutionIndex, (int)m_Solutions.size());
      Printf(1.0f, 1.0f, 1.0f, 1.0f, 5, GetHeight() - 15 - 14*3, GetFont(),
            "[g] - toggle grid render", (int)m_SolutionIndex, (int)m_Solutions.size());
      Printf(1.0f, 1.0f, 1.0f, 1.0f, 5, GetHeight() - 15 - 14*4, GetFont(),
            "[h] - toggle field render", (int)m_SolutionIndex, (int)m_Solutions.size());
      Printf(1.0f, 1.0f, 1.0f, 1.0f, 5, GetHeight() - 15 - 14*5, GetFont(),
            "[e] - explode mode", (int)m_SolutionIndex, (int)m_Solutions.size());
      Printf(1.0f, 1.0f, 1.0f, 1.0f, 5, GetHeight() - 15 - 14*6, GetFont(),
            "[1-6] - toggle figure render", (int)m_SolutionIndex, (int)m_Solutions.size());
      Printf(1.0f, 1.0f, 1.0f, 1.0f, 5, GetHeight() - 15 - 14*7, GetFont(),
            "[n/p] - next/previous solution", (int)m_SolutionIndex, (int)m_Solutions.size());
      Printf(1.0f, 1.0f, 1.0f, 1.0f, 5, GetHeight() - 15 - 14*8, GetFont(),
            "[a] - render transparent figures", (int)m_SolutionIndex, (int)m_Solutions.size());
      Printf(1.0f, 1.0f, 1.0f, 1.0f, 5, GetHeight() - 15 - 14*9, GetFont(),
            "[m] - toggle manual rotation", (int)m_SolutionIndex, (int)m_Solutions.size());
      Printf(1.0f, 1.0f, 1.0f, 1.0f, 5, GetHeight() - 15 - 14*10, GetFont(),
            "[k] - reset rotation", (int)m_SolutionIndex, (int)m_Solutions.size());
      Printf(1.0f, 1.0f, 1.0f, 1.0f, 5, GetHeight() - 15 - 14*11, GetFont(),
            "[left/right/up/down] - manual rotation controls", (int)m_SolutionIndex, (int)m_Solutions.size());
    }

  }

  void RenderGrid()
  {
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.f);

    glColor4f(0.3f, 1.f, 0.3f, 0.15f);
    glBegin(GL_LINES);

    for(int x = 0; x <= 6; ++x)
    {
      for(int y = 0; y <= 6; ++y)
      {
        glVertex3f( -3.f + x,  -3.f + y,  -3.f );
        glVertex3f( -3.f + x,  -3.f + y,   3.f );

        glVertex3f( -3.f,  -3.f + x,  -3.f + y );
        glVertex3f(  3.f,  -3.f + x,  -3.f + y );

        glVertex3f( -3.f + x,  -3.f,  -3.f + y );
        glVertex3f( -3.f + x,   3.f,  -3.f + y );
      }
    }

    glColor4f(1.f, 0.f, 0.f, 1.f);
    glVertex3f( -4.f,  -4.f,  -4.f );
    glVertex3f( -3.f,  -4.f,  -4.f );
    glColor4f(0.f, 1.f, 0.f, 1.f);
    glVertex3f( -4.f,  -4.f,  -4.f );
    glVertex3f( -4.f,  -3.f,  -4.f );
    glColor4f(0.f, 0.f, 1.f, 1.f);
    glVertex3f( -4.f,  -4.f,  -4.f );
    glVertex3f( -4.f,  -4.f,  -3.f );
    glEnd();
    glDisable(GL_LINE_SMOOTH);
  }

  void RenderField(const CorrectPositionFinder::Field & field)
  {
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);
    glPointSize(4.f);

    glBegin(GL_POINTS);

    for(int i = 0; i < (int)field.size(); ++i)
    {
      if(field[i])
      {
        if(field[i] >= 1 && field[i]<= 6 && field[i]-1 < (int)m_Entities.size())
        {
          BlockEntity ent = m_Entities[field[i]-1];
          glColor4f(ent.block.r, ent.block.g, ent.block.b, 0.5f);
          int z = i / 6 / 6;
          int y = (i / 6) % 6;
          int x = i % 6;
          glVertex3f( -2.5f + x,  -2.5f + y,  -2.5f + z );
        }
      }
    }

    glEnd();
    glDisable(GL_POINT_SMOOTH);

  }

  void RenderBlock(const Block & block, float scale, float alpha, bool withLines=false, float linealpha=0.5f)
  {
    float sh = scale/2.f;

    for(int z = 0; z < 2; ++z)
    {
      float lz = (float(z) - 0.5f) * scale;
      for(int y = 0; y < 6; ++y)
      {
        float ly = (float(y) - 2.5f) * scale;
        for(int x = 0; x < 2; ++x)
        {
          float lx = (float(x) - 0.5f) * scale;
          if(block.data[z][y][x])
          {
            if(withLines)
            {
              glColor4f(1.f, 1.f, 1.f, linealpha);
              glBegin(GL_LINE_STRIP);
              glVertex3f(  sh + lx,  sh + ly,  sh + lz); // xy +z
              glVertex3f( -sh + lx,  sh + ly,  sh + lz );
              glVertex3f( -sh + lx, -sh + ly,  sh + lz );
              glVertex3f(  sh + lx, -sh + ly,  sh + lz );
              glVertex3f(  sh + lx,  sh + ly,  sh + lz );
              glVertex3f(  sh + lx,  sh + ly, -sh + lz ); // xz +y
              glVertex3f( -sh + lx,  sh + ly, -sh + lz );
              glVertex3f( -sh + lx,  sh + ly,  sh + lz );
              glVertex3f( -sh + lx, -sh + ly,  sh + lz ); // yz -x
              glVertex3f( -sh + lx, -sh + ly, -sh + lz );
              glVertex3f( -sh + lx,  sh + ly, -sh + lz );
              glVertex3f( -sh + lx, -sh + ly, -sh + lz ); // xy -z
              glVertex3f(  sh + lx, -sh + ly, -sh + lz );
              glVertex3f(  sh + lx,  sh + ly, -sh + lz );
              glVertex3f( sh + lx, -sh + ly, -sh + lz ); // last line
              glVertex3f( sh + lx, -sh + ly,  sh + lz );
              glEnd();
            }

            glColor4f(block.r, block.g, block.b, alpha);

            if(z == 0 || !block.data[z-1][y][x])
              RenderQuadXY(lx, ly, lz - sh, sh);
            if(z == 1 || !block.data[z+1][y][x])
              RenderQuadXY(lx, ly, lz + sh, sh);
            if(y == 0 || !block.data[z][y-1][x])
              RenderQuadXZ(lx, ly - sh, lz, sh);
            if(y == 5 || !block.data[z][y+1][x])
              RenderQuadXZ(lx, ly + sh, lz, sh);
            if(x == 0 || !block.data[z][y][x-1])
              RenderQuadYZ(lx - sh, ly, lz, sh);
            if(x == 1 || !block.data[z][y][x+1])
              RenderQuadYZ(lx + sh, ly, lz, sh);
          }
        }
      }
    }
  }

  void RenderQuadXY(float x, float y, float z, float s)
  {
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(  s + x,  s + y, z );
    glVertex3f( -s + x,  s + y, z );
    glVertex3f(  s + x, -s + y, z );
    glVertex3f( -s + x, -s + y, z );
    glEnd();
  }

  void RenderQuadXZ(float x, float y, float z, float s)
  {
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(  s + x, y,  s + z );
    glVertex3f( -s + x, y,  s + z );
    glVertex3f(  s + x, y, -s + z );
    glVertex3f( -s + x, y, -s + z );
    glEnd();
  }

  void RenderQuadYZ(float x, float y, float z, float s)
  {
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(  x,  s + y,  s + z);
    glVertex3f(  x, -s + y,  s + z);
    glVertex3f(  x,  s + y, -s + z);
    glVertex3f(  x, -s + y, -s + z);
    glEnd();
  }
private:
    GLfloat m_Spiral;
    GLfloat m_GridAngle;
    bool m_RotateGrid;
    bool m_RenderGrid;
    bool m_RenderField;
    bool m_RenderExploded;
    int  m_RenderObjectMask;
    bool m_RenderAlpha;
    bool m_RenderInfo;
    bool m_ManualControl;
    int  m_ManualRX;
    int  m_ManualRY;
    boost::array<Block, 6> m_Blocks;
    CorrectPositionFinder m_CorrectPositionFinder;
    boost::thread m_CorrectPositionFinderThread;
    bool m_SolverStarted;
    CorrectPositionFinder::BlockEntities m_Entities;
    CorrectPositionFinder::Field m_Field;
    time_t m_LastUpdate;
    bool m_ProcessingSolutions;
    std::vector<CorrectPositionFinder::BlockEntities> m_Solutions;
    size_t m_SolutionIndex;
};

int main(int /*argc*/, char** /*argv*/)
{
  PazzleSolverGfx app;
  return app.Run();
}
