#ifndef PZLCOMMON_H
#define PZLCOMMON_H

#include "ghgl/ghgl.hpp"

struct Block
{
  typedef unsigned char elem;
  Block(){}
  Block(const elem * v,
        float vr, float vg, float vb)
    : r(vr)
    , g(vg)
    , b(vb)
  {
    for(int z = 0; z < 2; ++z)
    {
        for(int y = 0; y < 6; ++y)
        {
        for(int x = 0; x < 2; ++x)
        {
          data[z][y][x] = *v++;
        }
      }
    }
  }
  elem data[2][6][2];
  float r, g, b;
};
const unsigned char Blockdata[6][24] = {
  { // fig 1
    1, 1, // level 1
    1, 1,
    1, 1,
    1, 1,
    1, 1,
    1, 1,
    1, 1, // level 2
    0, 0,
    0, 0,
    0, 0,
    0, 0,
    1, 1
  },
  { // fig 2
    1, 1, // level 1
    1, 1,
    1, 1,
    1, 1,
    1, 1,
    1, 1,
    1, 1, // level 2
    0, 0,
    0, 0,
    0, 0,
    1, 1,
    1, 1
  },
  { // fig 3
    1, 1, // level 1
    1, 1,
    1, 1,
    1, 1,
    1, 1,
    1, 1,
    1, 1, // level 2
    0, 0,
    0, 1,
    0, 1,
    0, 0,
    1, 1
  },
  { // fig 4
    1, 1, // level 1
    1, 1,
    0, 1,
    1, 1,
    1, 1,
    1, 1,
    1, 1, // level 2
    0, 0,
    0, 0,
    0, 0,
    0, 0,
    1, 1
  },
  { // fig 5
    1, 1, // level 1
    1, 1,
    0, 1,
    0, 1,
    1, 1,
    1, 1,
    1, 1, // level 2
    0, 1,
    0, 0,
    0, 0,
    0, 1,
    1, 1
  },
  { // fig 6
    1, 1, // level 1
    1, 1,
    0, 1,
    0, 1,
    1, 1,
    1, 1,
    1, 1, // level 2
    0, 0,
    0, 0,
    0, 1,
    0, 0,
    1, 1
  }
};

typedef boost::array<unsigned char, 216> BlockField; // 216 = 6*6*6

struct BlockEntity
{
  BlockEntity()
    : x(0)
    , y(0)
    , z(0)
    , rx(0)
    , ry(0)
    , rz(0)
    , assigned(false)
    , cached(false)
  {
    field.assign(0);
  }

  Block block;
  int x;
  int y;
  int z;
  int rx;
  int ry;
  int rz;

  bool assigned;
  BlockField field;
  bool cached;

  unsigned char GetField(int x, int y, int z)
  {
    int offset = z*6*6 + y*6 + x;
    if(offset >= 0 && offset < (int)field.size())
      return field[offset];
    return 0;
  }

  void SetField(int x, int y, int z, unsigned char value)
  {
    int offset = z*6*6 + y*6 + x;
    if(offset >= 0 && offset < (int)field.size())
    {
      field[offset] = value;
    }
  }

  void Cache(unsigned char value)
  {
    cached = true;
    field.assign(0);

    if(rx == 0 && rz == 0)
    {
      for(int lz = 0; lz < 2; ++lz)
        for(int ly = 0; ly < 6; ++ly)
          for(int lx = 0; lx < 2; ++lx)
          {
            unsigned char check = block.data[lz][ly][lx];
            if(check)
            {
              int px = lx;
              int pz = lz;
              if(ry==1){pz=1-lx; px=lz;}
              if(ry==2){pz=1-lz; px=1-lx;}
              if(ry==3){pz=lx; px=1-lz;}
              SetField(px+x+2, ly+y, pz+z+2, check?value:0);
            }
          }
    }
    if(rx == 1 && rz == 0)
    {
      for(int lz = 0; lz < 2; ++lz)
        for(int ly = 0; ly < 6; ++ly)
          for(int lx = 0; lx < 2; ++lx)
          {
            unsigned char check = block.data[lz][ly][lx];
            if(check)
            {
              int px = lx;
              int pz = 1-lz;
              if(ry==1){pz=lx; px=lz;}
              if(ry==2){pz=lz; px=1-lx;}
              if(ry==3){pz=1-lx; px=1-lz;}
              SetField(px+x+2, pz+y+2, ly+z, check?value:0);
            }
          }
    }
    if(ry == 0 && rz == 1)
    {
      for(int lz = 0; lz < 2; ++lz)
        for(int ly = 0; ly < 6; ++ly)
          for(int lx = 0; lx < 2; ++lx)
          {
            unsigned char check = block.data[lz][ly][lx];
            if(check)
            {
              int px = lx;
              int pz = lz;
              if(rx==1){pz=lx; px=1-lz;}
              if(rx==2){pz=1-lz; px=1-lx;}
              if(rx==3){pz=1-lx; px=lz;}
              SetField(5-ly+x, px+y+2, pz+z+2, check?value:0);
            }
          }
    }
  }
};
#endif // PZLCOMMON_H