#pragma once

#include "myMatrix.h"

class Vector {
public:
  Vector(float x, float y, float z);
  Vector();
  ~Vector();

  float tuple[3];
  float loc[3];

  void RotateVector_alg(Vector axis, float angle);
  void RotateVector_mat(Vector* axis, float angle);

  Vector AddVectorToVector(Vector* vec_in);
  Vector SubtractVectorFromVector(Vector* vec_in);
  Vector DivideVectorByScalar(float in_scalar);
  Vector MultiplyVectorByScalar(float in_scalar);
  Vector CrossProduct(Vector* vec_in);
  float DotProduct(Vector* vec_in);
  float get_size();
  float get_angle();
  bool isOpp(Vector* vec_in);
  void make_unit();

  Vector operator*(float const& obj) {
    Vector out;
    out.tuple[0] = tuple[0] * obj;
    out.tuple[1] = tuple[1] * obj;
    out.tuple[2] = tuple[2] * obj;
    return out;
  }

  Vector operator/(float const& obj) {
    Vector out;
    out.tuple[0] = tuple[0] / obj;
    out.tuple[1] = tuple[1] / obj;
    out.tuple[2] = tuple[2] / obj;
    return out;
  }

  void operator+=(Vector const& obj) {
    for (int i = 0; i < 3; i++) {
      tuple[i] += obj.tuple[i];
    }
  }

  void operator-=(Vector const& obj) {
    for (int i = 0; i < 3; i++) {
      tuple[i] -= obj.tuple[i];
    }
  }

  Vector operator-(Vector const& obj) {
    Vector out;
    out.tuple[0] = tuple[0] - obj.tuple[0];
    out.tuple[1] = tuple[1] - obj.tuple[1];
    out.tuple[2] = tuple[2] - obj.tuple[2];
    return out;
  }
};

