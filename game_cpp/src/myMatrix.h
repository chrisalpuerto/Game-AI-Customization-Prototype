#pragma once
#include "Vector.h"
class myPoint;
class Vector;

class myMatrix
{
public:
	myMatrix();
	~myMatrix();

	struct mat3x3
	{
		float m[3][3];
	};

	struct mat4x4
	{
		float m[4][4];
	};

	struct mat_CTM_normal
	{
		float m00;
		float m01;
		float m10;
		float m11;
		float dx;
		float dy;
	};

	mat3x3 m_RotXY_3x3 = { 0 };
	mat3x3 m_RotXZ_3x3 = { 0 };
	mat3x3 m_RotYZ_3x3 = { 0 };

	mat_CTM_normal m_normal;

	mat4x4 m_RotXY_4x4 = { 0 };
	mat4x4 m_RotXZ_4x4 = { 0 };
	mat4x4 m_RotYZ_4x4 = { 0 };
	mat4x4 m_Translate_4x4 = { 0 };
	mat4x4 m_Scale_4x4 = { 0 };
	mat4x4 m_ViewTransform_4x4 = { 0 };

	mat3x3 m_Det_3x3 = { 0 };

	void set_Det_3x3(float* arr);
	float solve_Det_3x3();

	void init_normal_CTM(float*);
	void convert_normal_pt(myPoint* i, myPoint* o);
	void convert_normal_vec(Vector* i, Vector* o);


	void init_RotXY_4x4(float angle);
	void init_RotXZ_4x4(float angle);
	void init_RotYZ_4x4(float angle);

	void init_Tran_4x4(float dx, float dy, float dz);
	void Translate_4x4(myPoint* in, myPoint* out);

	void init_Scale_4x4(float dx, float dy, float dz);
	void Scale_4x4(myPoint* in, myPoint* out);

	void Rotate_XY_4x4(Vector* in, Vector* out); //z axis
	void Rotate_XZ_4x4(Vector* in, Vector* out); //y axis
	void Rotate_YZ_4x4(Vector* in, Vector* out); //x axis

//	void init_View_Transform(Camera* cam);
//	void ProjectPoint(myPoint* in, myPoint* out);


	void init_RotXY_3x3(float angle);
	void init_RotXZ_3x3(float angle);
	void init_RotYZ_3x3(float angle);

	void Rotate_XY_3x3(Vector* in, Vector* out);
	void Rotate_XZ_3x3(Vector* in, Vector* out);
	void Rotate_YZ_3x3(Vector* i, Vector* o);


};

