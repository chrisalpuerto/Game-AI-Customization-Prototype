#pragma once
#include "myMatrix.h"
//
class Vector
{
public:
	Vector(float x, float y, float z);
	Vector();
	~Vector();
	float tuple[3]; //(x, y, z)

	float loc[3]; //(x, y, z) of the vector's tail

	void RotateVector_alg(Vector axis, float angle); //This one is not working as expected.
	void RotateVector_mat(Vector* axis, float angle);


	Vector AddVectorToVector(Vector* vec_in);
	Vector SubtractVectorFromVector(Vector* vec_in);
	Vector DivideVectorByScalar(float in_scalar);
	Vector MultiplyVectorByScalar(float in_scalar);
	Vector CrossProduct(Vector* vec_in);
	float DotProduct(Vector* vec_in);
	float get_size();
	float get_angle(); //angle it makes with the x axist in a standard 2D frame.
	bool isOpp(Vector* vec_in);
	//Makes this vector a unit vector
	void make_unit();		


	Vector operator * (float const& obj)
	{
		Vector out;

		out.tuple[0] = tuple[0] * obj;
		out.tuple[1] = tuple[1] * obj;
		out.tuple[2] = tuple[2] * obj;
		return out;
	}

	Vector operator / (float const& obj)
	{
		Vector out;

		out.tuple[0] = tuple[0] / obj;
		out.tuple[1] = tuple[1] / obj;
		out.tuple[2] = tuple[2] / obj;
		return out;
	}

	void operator += (Vector const& obj) {

		for (int i = 0; i < 3; i++)
		{
			tuple[i] += obj.tuple[i];
		}
	}

	void operator -= (Vector const& obj) {

		for (int i = 0; i < 3; i++)
		{
			tuple[i] -= obj.tuple[i];
		}
	}

	Vector operator - (Vector const& obj)
	{
		Vector out;
		out.tuple[0] = tuple[0] - obj.tuple[0];
		out.tuple[1] = tuple[1] - obj.tuple[1];
		out.tuple[2] = tuple[2] - obj.tuple[2];
		return out;
	}

	float operator [] (int)
	{
		return get_size();
	}

};