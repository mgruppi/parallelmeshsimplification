class Vector3f
{

public:
  double x; //x axis direction
  double y; //y axis direction
  double z; //z axis direction


  Vector3f();
  Vector3f(double ni, double nj, double nk)
  {
    x = ni;
    y = nj;
    z = nk;
  }
  Vector3f(Vector3f& v)
  {
    x = v.x;
    y = v.y;
    z = v.z;
  }

  double getMag();
  Vector3f* cross(Vector3f* v);
  double dot(Vector3f& v);
  void normalize();
  void reverse();

};
