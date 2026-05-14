#pragma once

#include "Vector.h"

class myPoint {
public:
  myPoint(float x, float y, float z);
  myPoint();
  ~myPoint();

  float tuple[3];
  bool on = false;

  void toggle();
  bool get_on();

  myPoint AddVectorToPoint(const Vector* vec_in) const;
  myPoint SubtractVectorFromPoint(const Vector* vec_in) const;
  Vector SubtractPointFromPoint(const myPoint* pt_in) const;
};
