#include "Vector.h"

#include <cmath>

Vector::Vector(float x, float y, float z) {
  tuple[0] = x;
  tuple[1] = y;
  tuple[2] = z;
}

Vector::Vector() {
  tuple[0] = tuple[1] = tuple[2] = 0;
}

Vector::~Vector() {}

void Vector::RotateVector_alg(Vector axis, float angle) {
  Vector u = axis.DivideVectorByScalar(axis.get_size());
  Vector v;
  v.tuple[0] = u.tuple[1] * u.tuple[2];
  v.tuple[1] = -1 * u.tuple[0] * u.tuple[2] * 0.5f;
  v.tuple[2] = -1 * u.tuple[0] * u.tuple[1] * 0.5f;
  v = v.DivideVectorByScalar(v.get_size());
  Vector w = v.CrossProduct(&u);
  w = w.DivideVectorByScalar(w.get_size());

  float vals_temp[9] = {
    u.tuple[0], v.tuple[0], w.tuple[0],
    u.tuple[1], v.tuple[1], w.tuple[1],
    u.tuple[2], v.tuple[2], w.tuple[2],
  };
  float vals[9] = {
    u.tuple[0], v.tuple[0], w.tuple[0],
    u.tuple[1], v.tuple[1], w.tuple[1],
    u.tuple[2], v.tuple[2], w.tuple[2],
  };

  myMatrix mat;
  mat.set_Det_3x3(&vals[0]);
  const float D = mat.solve_Det_3x3();

  vals[0] = tuple[0];
  vals[3] = tuple[1];
  vals[6] = tuple[2];
  mat.set_Det_3x3(&vals[0]);
  const float Da = mat.solve_Det_3x3();

  for (int i = 0; i < 9; i++) {
    vals[i] = vals_temp[i];
  }

  vals[1] = tuple[0];
  vals[4] = tuple[1];
  vals[7] = tuple[2];
  mat.set_Det_3x3(&vals[0]);
  const float Db = mat.solve_Det_3x3();

  for (int i = 0; i < 9; i++) {
    vals[i] = vals_temp[i];
  }

  vals[2] = tuple[0];
  vals[5] = tuple[1];
  vals[8] = tuple[2];
  mat.set_Det_3x3(&vals[0]);
  const float Dc = mat.solve_Det_3x3();

  const float A = Da / D;
  const float B = Db / D;
  const float C = Dc / D;
  const float mag = std::sqrt(std::pow(B, 2) + std::pow(C, 2));
  const float alpha = std::atan(C / B);
  const float phi = alpha + angle;

  Vector rotated;
  rotated.tuple[0] = A * u.tuple[0] + mag * std::cos(phi) * v.tuple[0] + mag * std::sin(phi) * w.tuple[0];
  rotated.tuple[1] = A * u.tuple[1] + mag * std::cos(phi) * v.tuple[1] + mag * std::sin(phi) * w.tuple[1];
  rotated.tuple[2] = A * u.tuple[2] + mag * std::cos(phi) * v.tuple[2] + mag * std::sin(phi) * w.tuple[2];

  tuple[0] = rotated.tuple[0];
  tuple[1] = rotated.tuple[1];
  tuple[2] = rotated.tuple[2];
}

void Vector::RotateVector_mat(Vector* axis, float angle) {
  float tanXY = axis->tuple[1] / axis->tuple[0];
  float XYoff_angle = std::atan(tanXY);
  myMatrix mat;
  mat.init_RotXY_3x3(-XYoff_angle);
  mat.Rotate_XY_3x3(axis, axis);
  mat.Rotate_XY_3x3(this, this);

  float tanXZ = axis->tuple[2] / axis->tuple[0];
  float XZoff_angle = std::atan(tanXZ);
  mat.init_RotXZ_3x3(-XZoff_angle);
  mat.Rotate_XZ_3x3(axis, axis);
  mat.Rotate_XZ_3x3(this, this);

  mat.init_RotYZ_3x3(angle);
  mat.Rotate_YZ_3x3(this, this);

  mat.init_RotXZ_3x3(XZoff_angle);
  mat.Rotate_XZ_3x3(axis, axis);
  mat.Rotate_XZ_3x3(this, this);

  mat.init_RotXY_3x3(XYoff_angle);
  mat.Rotate_XY_3x3(axis, axis);
  mat.Rotate_XY_3x3(this, this);
}

Vector Vector::AddVectorToVector(Vector* vec_in) {
  return Vector(tuple[0] + vec_in->tuple[0], tuple[1] + vec_in->tuple[1], tuple[2] + vec_in->tuple[2]);
}

Vector Vector::SubtractVectorFromVector(Vector* vec_in) {
  return Vector(tuple[0] - vec_in->tuple[0], tuple[1] - vec_in->tuple[1], tuple[2] - vec_in->tuple[2]);
}

Vector Vector::DivideVectorByScalar(float in_scalar) {
  return Vector(tuple[0] / in_scalar, tuple[1] / in_scalar, tuple[2] / in_scalar);
}

Vector Vector::MultiplyVectorByScalar(float in_scalar) {
  return Vector(tuple[0] * in_scalar, tuple[1] * in_scalar, tuple[2] * in_scalar);
}

Vector Vector::CrossProduct(Vector* vec_in) {
  return Vector(
    (tuple[1] * vec_in->tuple[2]) - (tuple[2] * vec_in->tuple[1]),
    (tuple[2] * vec_in->tuple[0]) - (tuple[0] * vec_in->tuple[2]),
    (tuple[0] * vec_in->tuple[1]) - (tuple[1] * vec_in->tuple[0]));
}

float Vector::DotProduct(Vector* vec_in) {
  return tuple[0] * vec_in->tuple[0] + tuple[1] * vec_in->tuple[1] + tuple[2] * vec_in->tuple[2];
}

void Vector::make_unit() {
  float size = get_size();
  if (size > 0) {
    *this = DivideVectorByScalar(size);
  }
}

bool Vector::isOpp(Vector* vec_in) {
  int c = 0;
  for (int i = 0; i < 3; i++) {
    if ((tuple[i] < 0 && vec_in->tuple[i] > 0) || (tuple[i] > 0 && vec_in->tuple[i] < 0)) {
      c++;
    }
  }
  return c >= 1;
}

float Vector::get_size() {
  return std::sqrt(std::pow(tuple[0], 2) + std::pow(tuple[1], 2) + std::pow(tuple[2], 2));
}

float Vector::get_angle() {
  const float x = tuple[0];
  const float y = tuple[1];
  const float pi = 3.14159f;

  if (x > 0) {
    if (y > 0) return std::atan(y / x);
    if (y < 0) return 2 * pi + std::atan(y / x);
    return 0;
  }
  if (x < 0) {
    if (y != 0) return pi + std::atan(y / x);
    return pi;
  }
  if (y > 0) return pi / 2;
  if (y < 0) return 3 * pi / 2;
  return -1000;
}

