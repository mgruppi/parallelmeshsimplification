#include "Vector3f.h"
#include <cmath>

double Vector3f::getMag()
{
  return sqrt(x*x + y*y + z*z);
}

Vector3f& Vector3f::cross(Vector3f& v)
{
    double s1 = y*v.z - z*v.y; //x direction of cross product
    double s2 = z*v.x - x*v.z; //y direction
    double s3 = x*v.y - y*v.x;

    Vector3f res(s1,s2,s3);

    return res;
}

double Vector3f::dot(Vector3f& v)
{
    return x*v.x + y*v.y + z*v.z; //Return dot product
}

void Vector3f::normalize()
{
    double mag = getMag();
    x = x/mag;
    y = y/mag;
    z = z/mag;
}
