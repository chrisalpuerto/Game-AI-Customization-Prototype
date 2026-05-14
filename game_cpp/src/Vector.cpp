#include "Vector.h"
#include <cmath>

Vector::Vector(float x, float y, float z)
{
	tuple[0] = x;
	tuple[1] = y;
	tuple[2] = z;
}
Vector::Vector()
{
	tuple[0] = tuple[1] = tuple[2] = 0;
}
Vector::~Vector() {}

void Vector::RotateVector_alg(Vector axis, float angle)
{
	//use axis vector to create new coordinant system
	Vector u = axis.DivideVectorByScalar(axis.get_size()); //now it's a unit vector
	Vector v;
	//make v ortho to u
	v.tuple[0] = u.tuple[1] * u.tuple[2];
	v.tuple[1] = -1 * u.tuple[0] * u.tuple[2] * 0.5;
	v.tuple[2] = -1 * u.tuple[0] * u.tuple[1] * 0.5;
	v = v.DivideVectorByScalar(v.get_size());
	//find the final dimension
	Vector w = v.CrossProduct(&u);
	w = w.DivideVectorByScalar(w.get_size());

	//Now find the values A, B, and C that will construct the original vector along our new unit vectors u, v, and w.

	float D, Da, Db, Dc; //using Cramer's Rule for a system of equations.

	float vals_temp[9] = { u.tuple[0], v.tuple[0], w.tuple[0], u.tuple[1], v.tuple[1], w.tuple[1],u.tuple[2], v.tuple[2], w.tuple[2] };
	float vals[9] = { u.tuple[0], v.tuple[0], w.tuple[0], u.tuple[1], v.tuple[1], w.tuple[1],u.tuple[2], v.tuple[2], w.tuple[2] };

	myMatrix mat;
	mat.set_Det_3x3(&vals[0]);
	D = mat.solve_Det_3x3();

	vals[0] = tuple[0];
	vals[3] = tuple[1];
	vals[6] = tuple[2];
	mat.set_Det_3x3(&vals[0]);
	Da = mat.solve_Det_3x3();

	for (int i = 0; i < 9; i++)
	{
		vals[i] = vals_temp[i];
	}

	vals[1] = tuple[0];
	vals[4] = tuple[1];
	vals[7] = tuple[2];
	mat.set_Det_3x3(&vals[0]);
	Db = mat.solve_Det_3x3();

	for (int i = 0; i < 9; i++)
	{
		vals[i] = vals_temp[i];
	}

	vals[2] = tuple[0];
	vals[5] = tuple[1];
	vals[8] = tuple[2];
	mat.set_Det_3x3(&vals[0]);
	Dc = mat.solve_Det_3x3();

	float A = Da / D;
	float B = Db / D;
	float C = Dc / D;

	//Now use basic trigonometry to rotate the vector around the given axis, which is u.
	float mag = sqrt(pow(B, 2) + pow(C, 2)); //magnitude of the new projected vector along v and w
	float alpha = atan(C / B);
	float phi = alpha + angle;
	Vector rotated;
	rotated.tuple[0] = A * u.tuple[0] + mag * cos(phi) * v.tuple[0] + mag * sin(phi) * w.tuple[0];
	rotated.tuple[1] = A * u.tuple[1] + mag * cos(phi) * v.tuple[1] + mag * sin(phi) * w.tuple[1];
	rotated.tuple[2] = A * u.tuple[2] + mag * cos(phi) * v.tuple[2] + mag * sin(phi) * w.tuple[2];

	//Now rotate this vector!
	tuple[0] = rotated.tuple[0];
	tuple[1] = rotated.tuple[1];
	tuple[2] = rotated.tuple[2];

}

void Vector::RotateVector_mat(Vector* axis, float angle)
{
	//Rotate this vector and the new axis vector about either the z axis until the new axis vector is in the ZX plane.
	//Then, rotate them about the y axis until the new axis vector is parallel with the x axis.
	//Then, perform the rotation of this vector as if it were being rotated about the x axis.
	//Finally, reverse the transformations of both vectors.
	float tanXY = axis->tuple[1] / axis->tuple[0];
	float XYoff_angle = atan(tanXY);
	myMatrix mat;
	mat.init_RotXY_3x3(-XYoff_angle);
	mat.Rotate_XY_3x3(axis, axis);
	mat.Rotate_XY_3x3(this, this);

	float tanXZ = axis->tuple[2] / axis->tuple[0];
	float XZoff_angle = atan(tanXZ);
	mat.init_RotXZ_3x3(-XZoff_angle);
	mat.Rotate_XZ_3x3(axis, axis);
	mat.Rotate_XZ_3x3(this, this);

	//Now, the vectors are in place. Perform the desired transformation.
	mat.init_RotYZ_3x3(angle);
	mat.Rotate_YZ_3x3(this, this);

	//Undue the transformations to return the vectors to their original state, but keeping the transformation of this vector.
	mat.init_RotXZ_3x3(XZoff_angle);
	mat.Rotate_XZ_3x3(axis, axis);
	mat.Rotate_XZ_3x3(this, this);

	mat.init_RotXY_3x3(XYoff_angle);
	mat.Rotate_XY_3x3(axis, axis);
	mat.Rotate_XY_3x3(this, this);
}

Vector Vector::AddVectorToVector(Vector* vec_in)
{
	Vector out;
	out.tuple[0] = tuple[0] + vec_in->tuple[0];
	out.tuple[1] = tuple[1] + vec_in->tuple[1];
	out.tuple[2] = tuple[2] + vec_in->tuple[2];
	return out;
}
Vector Vector::SubtractVectorFromVector(Vector* vec_in)
{
	Vector out;
	out.tuple[0] = tuple[0] - vec_in->tuple[0];
	out.tuple[1] = tuple[1] - vec_in->tuple[1];
	out.tuple[2] = tuple[2] - vec_in->tuple[2];
	return out;
}

Vector Vector::DivideVectorByScalar(float in_scalar)
{
	Vector out;

	out.tuple[0] = tuple[0] / in_scalar;
	out.tuple[1] = tuple[1] / in_scalar;
	out.tuple[2] = tuple[2] / in_scalar;
	return out;
}

Vector Vector::MultiplyVectorByScalar(float in_scalar)
{
	Vector out;

	out.tuple[0] = tuple[0] * in_scalar;
	out.tuple[1] = tuple[1] * in_scalar;
	out.tuple[2] = tuple[2] * in_scalar;
	return out;
}

Vector Vector::CrossProduct(Vector* vec_in)
{
	//note: should use a determinant matrix whenever I've figured that out...
	Vector out;
	out.tuple[0] = (tuple[1] * vec_in->tuple[2]) - (tuple[2] * vec_in->tuple[1]);
	out.tuple[1] = (tuple[2] * vec_in->tuple[0]) - (tuple[0] * vec_in->tuple[2]);
	out.tuple[2] = (tuple[0] * vec_in->tuple[1]) - (tuple[1] * vec_in->tuple[0]);



	return out;
}
float Vector::DotProduct(Vector* vec_in)
{
	float d = 0.0;
	d += tuple[0] * vec_in->tuple[0];
	d += tuple[1] * vec_in->tuple[1];
	d += tuple[2] * vec_in->tuple[2];

	return d;
}

void Vector::make_unit()
{
	float size = get_size();
	if (size > 0)
		*this = DivideVectorByScalar(size);
}

bool Vector::isOpp(Vector* vec_in)
{
	//This will check the signs of each tuple of this vector and the given vector. If there is any opposition, then returns true;
	bool opp = false;
	int c = 0;
	for (int i = 0; i < 3; i++)
	{
		if ((tuple[i] < 0 && vec_in->tuple[i] > 0) || (tuple[i] > 0 && vec_in->tuple[i] < 0))
		{
			c++;
		}
	}
	if (c >= 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

float Vector::get_size()
{
	float size = sqrt(pow(tuple[0], 2) + pow(tuple[1], 2) + pow(tuple[2], 2));
	return size;
}

float Vector::get_angle()
{
	//Return value in radians.

	//Solution by cases because mathematical purity is a little lacking at the moment. Which is sad, but perhaps this function will be updated one day to what it should be.
	float x = tuple[0];
	float y = tuple[1];
	float pi = 3.14159;

	if (x > 0)
	{
		if (y > 0)
		{
			//quadrant 1. angle is arctan.
			return atan(y / x);
		}
		if (y < 0)
		{
			//quadrant 4. angle is 2pi + atan.
			return 2 * pi + atan(y / x);
		}
		if (y == 0)
		{
			return 0;
		}
	}
	if (x < 0)
	{
		if (y > 0)
		{
			//quadrant 2. angle is 180 + atan.
			return pi + atan(y / x);
		}
		if (y < 0)
		{
			//quadrant 3. angle is 180 + atan.
			return pi + atan(y / x);
		}
		if (y == 0)
		{
			return pi;
		}
	}
	if (x == 0)
	{
		if (y > 0)
		{
			return pi / 2;
		}
		if (y < 0)
		{
			return 3 * pi / 2;
		}
	}

	return -1000; //this means x and y are both 0.
}