#include <iostream>
#include <iomanip>
#include <cassert>

struct X_Z
{
  float x;
  float z;
};

struct L3
{
  L3(const float *v1, const float *v2)
  {
    assert(v1[1] != v2[1]);

    n[0] = v2[0]-v1[0];
    n[1] = 1.f / (v2[1]-v1[1]); //store 1/n.y
    n[2] = v2[2]-v1[2];
    M0[0] = v1[0];
    M0[1] = v1[1];
    M0[2] = v1[2];

    std::cerr<<"L3 n["<<std::setprecision(9)<<n[0]<<", "<<std::setprecision(9)<<n[1]<<", "<<std::setprecision(9)<<n[2]<<"]\n";

  }

  X_Z getIntersectionPoint(float y) const
  {
    X_Z xz;
    const float t = (y-M0[1])*n[1];

    std::cerr<<"L3 t="<<std::setprecision(9)<<t<<"\n";

    xz.x = M0[0] + n[0]*t;
    xz.z = M0[2] + n[2]*t;
    return xz;
  }

  float n[3];
  float M0[3];
};

struct L3C
{
  L3C(const float *v1, const float *v2)
  {
    assert(v1[1] != v2[1]);

    //const float inv_n1 = 1.f/(v2[1]-v1[1]);
    const double inv_n1 = 1./(v2[1]-v1[1]);

    a0 = (v2[0]-v1[0])*inv_n1;
    b0 = v1[0] - a0*v1[1];
    
    a2 = (v2[2]-v1[2])*inv_n1;
    b2 = v1[2] - a2*v1[1];


    std::cerr<<"L3C  a0="<<std::setprecision(9)<<a0<<" b0="<<std::setprecision(9)<<b0<<" a2="<<std::setprecision(9)<<a2<<" b2="<<std::setprecision(9)<<b2<<"\n";

  }

  X_Z getIntersectionPoint(float y) const
  {
    X_Z xz;
    xz.x = a0*y + b0;
    xz.z = a2*y + b2;
    return xz;
  }

  float a0, b0;
  float a2, b2;
};


struct L3Cd
{
  L3Cd(const float *v1, const float *v2)
  {
    assert(v1[1] != v2[1]);

    double n0 = v2[0]-v1[0];
    double inv_n1 = 1.f/(v2[1]-v1[1]);
    double n2 = v2[2]-v1[2];

    b0 = v1[0] - n0*inv_n1*v1[1];
    a0 = n0*inv_n1;
    
    b2 = v1[2] - n2*inv_n1*v1[1];
    a2 = n2*inv_n1;



    std::cerr<<"L3Cd a0="<<std::setprecision(9)<<a0<<" b0="<<std::setprecision(9)<<b0<<" a2="<<std::setprecision(9)<<a2<<" b2="<<std::setprecision(9)<<b2<<"\n";

  }

  X_Z getIntersectionPoint(float y) const
  {
    X_Z xz;
    xz.x = a0*y + b0;
    xz.z = a2*y + b2;
    return xz;
  }

  double a0, b0;
  double a2, b2;
};


struct L3d
{
  L3d(const float *v1, const float *v2)
  {
    assert(v1[1] != v2[1]);

    n[0] = v2[0]-v1[0];
    n[1] = 1.0 / (v2[1]-v1[1]); //store 1/n.y
    n[2] = v2[2]-v1[2];
    M0[0] = v1[0];
    M0[1] = v1[1];
    M0[2] = v1[2];

    std::cerr<<"L3d n["<<std::setprecision(9)<<n[0]<<", "<<std::setprecision(9)<<n[1]<<", "<<std::setprecision(9)<<n[2]<<"]\n";
  }

  X_Z getIntersectionPoint(float y) const
  {
    X_Z xz;
    const double t = (y-M0[1])*n[1];

    std::cerr<<"L3d t="<<std::setprecision(9)<<t<<"\n";

    xz.x = M0[0] + n[0]*t;
    xz.z = M0[2] + n[2]*t;
    return xz;
  }

  double n[3];
  double M0[3];
};

struct L3dB
{
  L3dB(const float *v1, const float *v2)
  {
    assert(v1[1] != v2[1]);

    n[0] = v2[0]-v1[0];
    n[1] = (v2[1]-v1[1]);
    n[2] = v2[2]-v1[2];
    M0[0] = v1[0];
    M0[1] = v1[1];
    M0[2] = v1[2];
  }

  X_Z getIntersectionPoint(float y) const
  {
    X_Z xz;
    const double t = (y-M0[1])/n[1];
    xz.x = M0[0] + n[0]*t;
    xz.z = M0[2] + n[2]*t;
    return xz;
  }

  double n[3];
  double M0[3];
};

int
main()
{
  float v0[3], v1[3], v2[3];

  /*
  v0[0] = 0.366518617;
  v0[1] = 0.370499969;
  v0[2] = 0.0284997057;

  v1[0] = 0.363615572;
  v1[1] = 0.365600705;
  v1[2] = 0.0286619943;

  v2[0] = 0.36050427;
  v2[1] = 0.359580576;
  v2[2] = 0.0288111884;

  const float y = 0.366518319;
  */




  v0[0] = 1.94127965;
  v0[1] = -0.671975791; 
  v0[2] = -0.0151555855;

  v1[0] = 1.94204235;
  v1[1] =  -0.675811946;
  v1[2] =  -0.0140955634;

  v2[0] = 1.94170022;
  v2[1] =  -0.673834026;
  v2[2] =  -0.0146424621;

  const float y = -0.675810456;

  
  L3 l01(v0, v1);
  L3 l02(v2, v1); //(v0, v2);
  X_Z xz01 = l01.getIntersectionPoint(y);
  X_Z xz02 = l02.getIntersectionPoint(y);
  
  L3d l01d(v0, v1);
  L3d l02d(v2, v1); //(v0, v2);
  X_Z xz01d = l01d.getIntersectionPoint(y);
  X_Z xz02d = l02d.getIntersectionPoint(y);
  
  L3dB l01dB(v0, v1);
  L3dB l02dB(v2, v1); //(v0, v2);
  X_Z xz01dB = l01dB.getIntersectionPoint(y);
  X_Z xz02dB = l02dB.getIntersectionPoint(y);
  
  std::cerr<<"xz01  =["<<std::setprecision(9)<<xz01.x<<", "<<std::setprecision(9)<<xz01.z<<"] xz02  =["<<std::setprecision(9)<<xz02.x<<", "<<std::setprecision(9)<<xz02.z<<"]\n";
  
  std::cerr<<"xz01d =["<<std::setprecision(9)<<xz01d.x<<", "<<std::setprecision(9)<<xz01d.z<<"] xz02d =["<<std::setprecision(9)<<xz02d.x<<", "<<std::setprecision(9)<<xz02d.z<<"]\n";

  std::cerr<<"xz01dB=["<<std::setprecision(9)<<xz01dB.x<<", "<<std::setprecision(9)<<xz01dB.z<<"] xz02dB=["<<std::setprecision(9)<<xz02dB.x<<", "<<std::setprecision(9)<<xz02dB.z<<"]\n";


  L3C l01C(v0, v1);
  L3C l02C(v2, v1); //(v0, v2);
  X_Z xz01C = l01C.getIntersectionPoint(y);
  X_Z xz02C = l02C.getIntersectionPoint(y);
  std::cerr<<"xz01C =["<<std::setprecision(9)<<xz01C.x<<", "<<std::setprecision(9)<<xz01C.z<<"] xz02C =["<<std::setprecision(9)<<xz02C.x<<", "<<std::setprecision(9)<<xz02C.z<<"]\n";
  
  L3Cd l01Cd(v0, v1);
  L3Cd l02Cd(v2, v1); //(v0, v2);
  X_Z xz01Cd = l01Cd.getIntersectionPoint(y);
  X_Z xz02Cd = l02Cd.getIntersectionPoint(y);
  std::cerr<<"xz01Cd=["<<std::setprecision(9)<<xz01Cd.x<<", "<<std::setprecision(9)<<xz01Cd.z<<"] xz02Cd=["<<std::setprecision(9)<<xz02Cd.x<<", "<<std::setprecision(9)<<xz02Cd.z<<"]\n";
  


  return 0;
}
