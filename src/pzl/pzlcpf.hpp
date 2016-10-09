#ifndef PZLCPF_H
#define PZLCPF_H

#include "ghgl/ghgl.hpp"
#include "pzlcommon.hpp"

class CorrectPositionFinder
{
public:
  typedef BlockField Field;
  typedef boost::array<BlockEntity, 6> BlockEntities;
  struct BlockPosition
  {
    BlockPosition()
      : x(0)
      , y(0)
      , z(0)
      , rx(0)
      , ry(0)
      , rz(0)
    {}

    BlockPosition(int vx, int vy, int vz, int vrx, int vry, int vrz)
      : x(vx)
      , y(vy)
      , z(vz)
      , rx(vrx)
      , ry(vry)
      , rz(vrz)
    {}

    int x;
    int y;
    int z;
    int rx;
    int ry;
    int rz;
  };
  typedef std::vector<BlockPosition> BlockPositions;

  CorrectPositionFinder()
    : m_NumIt(0)
    , m_CurIt(0)
    , m_IsFinished(false)
  {
    m_Entities[0].block = Block(Blockdata[0], 1.f, 0.f, 0.f);
    m_Entities[1].block = Block(Blockdata[1], 0.f, 1.f, 0.f);
    m_Entities[2].block = Block(Blockdata[2], 0.f, 0.f, 1.f);
    m_Entities[3].block = Block(Blockdata[3], 0.5f, 0.5f, 0.f);
    m_Entities[4].block = Block(Blockdata[4], 1.f, 0.f, 1.f);
    m_Entities[5].block = Block(Blockdata[5], 0.0f, 0.f, 0.0f);
    m_EntitiesCopy = m_Entities;
    workField.assign(0);
  }

  void TreshEntity(size_t i)
  {
    m_Entities[i].assigned = false;
    m_Entities[i].cached = false;
  }
  
  void AssignEntity(size_t i)
  {
    if(!m_Entities[i].assigned)
    {
      size_t pos = brute[i];
      m_Entities[i].x = m_BlockPositions[pos].x;
      m_Entities[i].y = m_BlockPositions[pos].y;
      m_Entities[i].z = m_BlockPositions[pos].z;
      m_Entities[i].rx = m_BlockPositions[pos].rx;
      m_Entities[i].ry = m_BlockPositions[pos].ry;
      m_Entities[i].rz = m_BlockPositions[pos].rz;
      m_Entities[i].assigned = true;
    }
  }

  void CacheEntity(size_t i)
  {
    if(!m_Entities[i].cached)
    {
      m_Entities[i].Cache(i+1);
    }
  }

  bool CheckColision(const Field & f1, const Field & f2)
  {
    for(size_t i = 0; i < f1.size(); ++i)
    {
      if(f1[i] && f2[i])
        return true;
    }
    return false;
  }

  void MergeField(Field & f1, const Field & f2)
  {
    for(size_t i = 0; i < f1.size(); ++i)
    {
      f1[i] = std::max(f1[i], f2[i]);
    }
  }

  void CleanupField(Field & f1, unsigned char value)
  {
    for(size_t i = 0; i < f1.size(); ++i)
    {
      if(f1[i] <= value)
        f1[i] = 0;
    }
  }

  bool FixBrut(Field & field, int h)
  {
    if(h < 0)
      return true;
    boost::this_thread::interruption_point();
    //std::cout << "fix brut "<<h<<"\n";
    //DumpField(field);
    bool result = false;
    for(size_t it = 0; it < m_NumIt; ++it)
    {
      CleanupField(field, h + 1);
      brute[h] = it;
      TreshEntity(h);
      AssignEntity(h);
      CacheEntity(h);
      Field f = m_Entities[h].field;
      if(!CheckColision(field, f))
      {
        MergeField(field, f);
        //std::cout << "FixBrut "<<h<<" found "<<it<<"\n";
        if(FixBrut(field, h - 1))
        {
          result = true;
          break;
        }
      }
    }
    //std::cout << "FixBrut "<<h<<" "<<brute[h]<<" result "<<result<<"\n";
    return result;
  }

  bool PrepareBrut()
  {
    brute.assign(0);
    Field f;
    f.assign(0);
    return FixBrut(f, 5);
  }

  void DumpField(const Field & field)
  {
    std::cout << "==============TOP==================\n";
    {
      for(int y = 0; y < 6; ++y)
      {
        for(int z = 0; z < 6; ++z)
        {
          for(int x = 0; x < 6; ++x)
            std::cout << (int)field[z*6*6+y*6+x];

          std::cout << "    ";
        }
        std::cout << "\n";
      }
    }
  }

  void CalcField(Field & field, BlockEntities & entities)
  {
    field.assign(0);
    for(int i = 0; i < 6; ++i)
    {
      entities[i].Cache(i+1);
      MergeField(field, entities[i].field);
    }
  }

  bool IsConstructionIsStable(const Field & field, BlockEntities & entities, bool withLog = false)
  {
    /*int dir_check[3]={0, 0, 0};
    boost::array<std::vector<int>, 3> dir_check_idx;
    for(int i = 0; i < 6; ++i)
    {
      if(entities[i].rx == 0 && entities[i].rz == 0)
      {
        ++dir_check[0];
        dir_check_idx[0].push_back(i);
      }
      if(entities[i].rx == 1 && entities[i].rz == 0)
      {
        ++dir_check[1];
        dir_check_idx[1].push_back(i);
      }
      if(entities[i].ry == 0 && entities[i].rz == 1)
      {
        ++dir_check[2];
        dir_check_idx[2].push_back(i);
      }
    }

    if(dir_check[0] != 2 || dir_check[1] != 2 || dir_check[2] != 2)
      return false;

    if(entities[dir_check_idx[0][0]].x != entities[dir_check_idx[0][1]].x &&
       entities[dir_check_idx[0][0]].z != entities[dir_check_idx[0][1]].z)
       return false;
    if(entities[dir_check_idx[0][0]].x == entities[dir_check_idx[0][1]].x &&
       entities[dir_check_idx[0][0]].x != 0)
       return false;
    else if(entities[dir_check_idx[0][0]].z != 0)
       return false;

    if(entities[dir_check_idx[1][0]].x != entities[dir_check_idx[1][1]].x &&
       entities[dir_check_idx[1][0]].y != entities[dir_check_idx[1][1]].y)
       return false;

    if(entities[dir_check_idx[1][0]].x == entities[dir_check_idx[1][1]].x &&
       entities[dir_check_idx[1][0]].x != 0)
       return false;
    else if (entities[dir_check_idx[1][0]].y != 0)
       return false;
       
    if(entities[dir_check_idx[2][0]].y != entities[dir_check_idx[2][1]].y &&
       entities[dir_check_idx[2][0]].z != entities[dir_check_idx[2][1]].z)
       return false;
      */

    /*Field fieldEmpty;
    fieldEmpty.assign(0);
    for(size_t j = 0; j < field.size(); ++j)
    {
      if(!field[j])
      {
        int z = j / 6 / 6;
        int y = (j / 6) % 6;
        int x = j % 6;

        int numnei = 0;

        if(x==0||!field[j-1])
          ++numnei;
        if(x==5||!field[j+1])
          ++numnei;
        if(y==0||!field[j-6])
          ++numnei;
        if(y==5||!field[j+6])
          ++numnei;
        if(z==0||!field[j-6*6])
          ++numnei;
        if(z==5||!field[j+6*6])
          ++numnei;

        fieldEmpty[j] = 1 + numnei;
      }
    }
    size_t sum = 0;
    size_t sum2 = 0;
    for(size_t j = 0; j < fieldEmpty.size(); ++j)
    {
      if(fieldEmpty[j] && fieldEmpty[j] >=5)
        ++sum;
      if(fieldEmpty[j] && fieldEmpty[j] > 1 && fieldEmpty[j] <=4)
        ++sum2;
      
    }
    std::cout<<sum<<" "<<sum2<<std::endl;
    //sum > 107 && 
    return sum2 < 8;*/

    const int dirs[6][3] = {
      { 1,  0,  0},
      {-1,  0,  0},
      { 0,  1,  0},
      { 0, -1,  0},
      { 0,  0,  1},
      { 0,  0, -1}
    };
    if(withLog)
    {
      std::cout << "===================================\n";
      DumpField(field);
      std::cout << "-----------------------------------\n";
    }
    bool result = true;
    for(int i = 0; i < 6 && result; ++i)
    {
      unsigned char value = i + 1;
      for(int dir = 0; dir < 6 && result; ++dir)
      {
        if(withLog){
          std::cout<<"idx "<<(int)value<<" dir "<<dir<<"\n";
        }
        int dx = dirs[dir][0];
        int dy = dirs[dir][1];
        int dz = dirs[dir][2];
        if(dx || dy || dz)
        {
          bool haveOne = false;
          for(size_t j = 0; j < field.size() && !haveOne; ++j)
          {
            if(field[j] == value)
            {
              int z = j / 6 / 6;
              int y = (j / 6) % 6;
              int x = j % 6;
              if(withLog){
                std::cout<<"found ("<<x<<","<<y<<","<<z<<")\n";
              }
              if(dx)
              {
                if(dx>0)
                {
                  for(int nx = x; nx < 6 && !haveOne; ++nx)
                  {
                    unsigned char checkValue = field[z*6*6+y*6+nx];
                    if(withLog){
                      std::cout<<"("<<nx<<","<<y<<","<<z<<")="<<(int)checkValue<<"\n";
                    }
                    if(checkValue && checkValue != value)
                    {
                      haveOne = true;
                      break;
                    }
                  }
                }
                else
                {
                  for(int nx = x; nx >= 0 && !haveOne; --nx)
                  {
                    unsigned char checkValue = field[z*6*6+y*6+nx];
                    if(withLog){
                      std::cout<<"("<<nx<<","<<y<<","<<z<<")="<<(int)checkValue<<"\n";
                    }
                    if(checkValue && checkValue != value)
                    {
                      haveOne = true;
                      break;
                    }
                  }
                }
              }
              if(dy)
              {
                if(dy>0)
                {
                  for(int ny = y; ny < 6 && !haveOne; ++ny)
                  {
                    unsigned char checkValue = field[z*6*6+ny*6+x];
                    if(checkValue && checkValue != value)
                    {
                      haveOne = true;
                      break;
                    }
                  }
                }
                else
                {
                  for(int ny = y; ny >= 0 && !haveOne; --ny)
                  {
                    unsigned char checkValue = field[z*6*6+ny*6+x];
                    if(checkValue && checkValue != value)
                    {
                      haveOne = true;
                      break;
                    }
                  }
                }
              }
              if(dz)
              {
                if(dz>0)
                {
                  for(int nz = z; nz < 6 && !haveOne; ++nz)
                  {
                    unsigned char checkValue = field[nz*6*6+y*6+x];
                    if(checkValue && checkValue != value)
                    {
                      haveOne = true;
                      break;
                    }
                  }
                }
                else
                {
                  for(int nz = z; nz >= 0 && !haveOne; --nz)
                  {
                    unsigned char checkValue = field[nz*6*6+y*6+x];
                    if(checkValue && checkValue != value)
                    {
                      haveOne = true;
                      break;
                    }
                  }
                }
              }
            }
          }

          if(!haveOne)
          {
            result = false;
            break;
          }
        }
      }
    }
    return result;
  }


  void operator()()
  {
    m_IsFinished = false;
    try {
      m_BlockPositions.clear();
      m_SolutionCount = 0;
      m_Solutions.clear();

      int limit1 = -1; // -1; // -2;
      int limit2 = 2; //2; // 3;
      int limit_inc = 1;

      for(int r = 0; r < 4; ++r)
      {
        for(int j = limit1; j <limit2; j+=limit_inc)
        {
          for(int i = limit1; i <limit2; i+=limit_inc)
          {
            m_BlockPositions.push_back(BlockPosition(i, 0, j, 0, r, 0));
          }
        }
      }

      for(int r = 0; r < 4; ++r)
      {
        for(int j = limit1; j <limit2; j+=limit_inc)
        {
          for(int i = limit1; i <limit2; i+=limit_inc)
          {
            m_BlockPositions.push_back(BlockPosition(i, j, 0, 1, r, 0));
          }
        }
      }

      for(int r = 0; r < 4; ++r)
      {
        for(int j = limit1; j <limit2; j+=limit_inc)
        {
          for(int i = limit1; i <limit2; i+=limit_inc)
          {
            m_BlockPositions.push_back(BlockPosition(0, i, j, r, 0, 1));
          }
        }
      }

      m_NumIt = m_BlockPositions.size();
      m_CurIt = 0;

      //std::cout << "m_BlockPositions.size() =" << m_BlockPositions.size() << " " << m_NumIt << "\n";

      if(!PrepareBrut())
      {
        m_CurIt = m_NumIt;
      }
      else
      {
        m_CurIt = brute[5];
        {
          boost::mutex::scoped_lock lock(m_Mutex);
          m_EntitiesCopy = m_Entities;
        }
        workField.assign(0);
        for(int i = 0; i < 6; ++i)
        {
          MergeField(workField, m_Entities[i].field);
        }
        bool run = true;
        int counter = 0;
        while(run)
        {
//           ++counter%=1000;
//           if(!counter){
//             boost::this_thread::interruption_point();
//           }
          //boost::this_thread::sleep(boost::posix_time::milliseconds(10));
          boost::this_thread::interruption_point();

          m_CurIt = brute[5];

          // check field
          if(IsConstructionIsStable(workField, m_Entities))
          {
            //IsConstructionIsStable(workField, m_Entities, true);
            m_Solutions.push_back(m_Entities);
            ++m_SolutionCount;
          }

          if(!counter)
          {
            boost::mutex::scoped_lock lock(m_Mutex);
            m_EntitiesCopy = m_Entities;
          }

          bool haveProgress = false;
          for(size_t i = 0; i < 6; ++i)
          {
            TreshEntity(i);
            CleanupField(workField, i+1);
            while(brute[i] < m_NumIt)
            {
              boost::this_thread::interruption_point();
              ++brute[i];
              TreshEntity(i);
              if(brute[i] >= m_NumIt)
              {
                break;
              }
              AssignEntity(i);
              CacheEntity(i);
              if(!CheckColision(workField, m_Entities[i].field))
              {
                MergeField(workField, m_Entities[i].field);
                if(FixBrut(workField, i-1))
                {
                  haveProgress = true;
                  break;
                }
                else
                {
                  CleanupField(workField, i+1);
                }
              }
            }
            m_CurIt = brute[5];
            if(haveProgress)
              break;
          }

          if(!haveProgress)
          {
            run = false;
          }
        }
      }
    }
    catch(boost::thread_interrupted const&)
    {}
    m_IsFinished = true;
  }

  void GetEntities(BlockEntities & copyto)
  {
    boost::mutex::scoped_lock lock(m_Mutex);
    copyto = m_EntitiesCopy;
  }

  size_t GetNumIt() { return m_NumIt; }
  size_t GetCurIt() { return m_CurIt; }
  const boost::array<size_t, 6> & GetBrute() const { return brute; }
  Field GetWorkField() const { return workField; }
  const std::vector<BlockEntities> & GetSolutions() const { return m_Solutions; }
  bool IsFinished() const { return m_IsFinished; }
  size_t GetSolutionCount() const { return m_SolutionCount; }

private:
  BlockEntities m_Entities;
  BlockEntities m_EntitiesCopy;
  std::vector<BlockEntities> m_Solutions;
  boost::mutex m_Mutex;
  BlockPositions m_BlockPositions;
  size_t m_NumIt;
  size_t m_CurIt;
  boost::array<size_t, 6> brute;
  Field workField;
  bool m_IsFinished;
  size_t m_SolutionCount;
};

#endif // PZLCPF_H
