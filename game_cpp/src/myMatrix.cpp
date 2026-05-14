#include "myMatrix.h"
#include "myPoint.h"

#include <cmath>
myMatrix::myMatrix() {}
myMatrix::~myMatrix() {}







void myMatrix::set_Det_3x3(float* arr)
{
	//input array should be indexed so that each element will correspond to the location that cycles first through the columns then resets and goes down a row.
	//so arr[0] ~ m[0][0], arr[2] ~ m[0][2], arr[3] ~ m[1][0] and so on

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			m_Det_3x3.m[i][j] = *arr;
			arr++;
		}
	}
}

float myMatrix::solve_Det_3x3()
{
	float s = m_Det_3x3.m[0][0] * (m_Det_3x3.m[1][1] * m_Det_3x3.m[2][2] - m_Det_3x3.m[1][2] * m_Det_3x3.m[2][1])
		-
		m_Det_3x3.m[0][1] * (m_Det_3x3.m[1][0] * m_Det_3x3.m[2][2] - m_Det_3x3.m[1][2] * m_Det_3x3.m[2][0])
		+
		m_Det_3x3.m[0][2] * (m_Det_3x3.m[1][0] * m_Det_3x3.m[2][1] - m_Det_3x3.m[1][1] * m_Det_3x3.m[2][0]);

	return s;
}

void myMatrix::init_normal_CTM(float* pValue)
{
	m_normal.m00 = pValue[0];
	m_normal.m10 = pValue[1];
	m_normal.m01 = pValue[2];
	m_normal.m11 = pValue[3];
	m_normal.dx = pValue[4];
	m_normal.dy = pValue[5];
}
void myMatrix::convert_normal_pt(myPoint* i, myPoint* o)
{
	myPoint in = *i;
	o->tuple[0] = in.tuple[0] * m_normal.m00 + in.tuple[1] * m_normal.m10 + m_normal.dx;
	o->tuple[1] = in.tuple[0] * m_normal.m01 + in.tuple[1] * m_normal.m11 + m_normal.dy;
}

void myMatrix::convert_normal_vec(Vector* i, Vector* o)
{
	o->tuple[0] = i->tuple[0] * m_normal.m00 + i->tuple[1] * m_normal.m10;
	o->tuple[1] = i->tuple[0] * m_normal.m01 + i->tuple[1] * m_normal.m11;
}

void myMatrix::init_RotXY_3x3(float angle)
{
	//converts degrees to rads
	angle = angle * 3.14159f / 180.0f;
	m_RotXY_3x3.m[0][0] = cos(angle);
	m_RotXY_3x3.m[1][0] = -sin(angle);
	m_RotXY_3x3.m[0][1] = sin(angle);
	m_RotXY_3x3.m[1][1] = cos(angle);
	m_RotXY_3x3.m[2][2] = 1.0f;
}
void myMatrix::init_RotXZ_3x3(float angle)
{
	//converts degrees to rads
	angle = angle * 3.14159f / 180.0f;
	m_RotXZ_3x3.m[0][0] = cos(angle);
	m_RotXZ_3x3.m[2][0] = -sin(angle);
	m_RotXZ_3x3.m[1][1] = 1.0f;
	m_RotXZ_3x3.m[0][2] = sin(angle);
	m_RotXZ_3x3.m[2][2] = cos(angle);
}
void myMatrix::init_RotYZ_3x3(float angle)
{
	//converts degrees to rads
	angle = angle * 3.14159f / 180.0f;
	m_RotYZ_3x3.m[0][0] = 1.0f;
	m_RotYZ_3x3.m[1][1] = cos(angle);
	m_RotYZ_3x3.m[2][1] = -sin(angle);
	m_RotYZ_3x3.m[1][2] = sin(angle);
	m_RotYZ_3x3.m[2][2] = cos(angle);
}

void myMatrix::Rotate_XY_3x3(Vector* i, Vector* o)
{
	Vector in = *i;
	o->tuple[0] = in.tuple[0] * m_RotXY_3x3.m[0][0] + in.tuple[1] * m_RotXY_3x3.m[1][0] + in.tuple[2] * m_RotXY_3x3.m[2][0];
	o->tuple[1] = in.tuple[0] * m_RotXY_3x3.m[0][1] + in.tuple[1] * m_RotXY_3x3.m[1][1] + in.tuple[2] * m_RotXY_3x3.m[2][1];
	o->tuple[2] = in.tuple[0] * m_RotXY_3x3.m[0][2] + in.tuple[1] * m_RotXY_3x3.m[1][2] + in.tuple[2] * m_RotXY_3x3.m[2][2];

}

void myMatrix::Rotate_XZ_3x3(Vector* i, Vector* o)
{
	Vector in = *i;
	o->tuple[0] = in.tuple[0] * m_RotXZ_3x3.m[0][0] + in.tuple[1] * m_RotXZ_3x3.m[1][0] + in.tuple[2] * m_RotXZ_3x3.m[2][0];
	o->tuple[1] = in.tuple[0] * m_RotXZ_3x3.m[0][1] + in.tuple[1] * m_RotXZ_3x3.m[1][1] + in.tuple[2] * m_RotXZ_3x3.m[2][1];
	o->tuple[2] = in.tuple[0] * m_RotXZ_3x3.m[0][2] + in.tuple[1] * m_RotXZ_3x3.m[1][2] + in.tuple[2] * m_RotXZ_3x3.m[2][2];

}

void myMatrix::Rotate_YZ_3x3(Vector* i, Vector* o)
{
	Vector in = *i;
	o->tuple[0] = in.tuple[0] * m_RotYZ_3x3.m[0][0] + in.tuple[1] * m_RotYZ_3x3.m[1][0] + in.tuple[2] * m_RotYZ_3x3.m[2][0];
	o->tuple[1] = in.tuple[0] * m_RotYZ_3x3.m[0][1] + in.tuple[1] * m_RotYZ_3x3.m[1][1] + in.tuple[2] * m_RotYZ_3x3.m[2][1];
	o->tuple[2] = in.tuple[0] * m_RotYZ_3x3.m[0][2] + in.tuple[1] * m_RotYZ_3x3.m[1][2] + in.tuple[2] * m_RotYZ_3x3.m[2][2];

}


void myMatrix::init_RotXY_4x4(float angle)
{
	//converts degrees to rads
	angle = angle * 3.14159f / 180.0f;
	m_RotXY_4x4.m[0][0] = cos(angle);
	m_RotXY_4x4.m[1][0] = -sin(angle);
	m_RotXY_4x4.m[0][1] = sin(angle);
	m_RotXY_4x4.m[1][1] = cos(angle);
	m_RotXY_4x4.m[2][2] = 1.0f;
	m_RotXY_4x4.m[3][3] = 1.0f;
}
void myMatrix::init_RotXZ_4x4(float angle)
{
	//converts degrees to rads
	angle = angle * 3.14159f / 180.0f;
	m_RotXY_4x4.m[0][0] = cos(angle);
	m_RotXY_4x4.m[2][0] = sin(angle);
	m_RotXY_4x4.m[1][1] = 1.0f;
	m_RotXY_4x4.m[0][2] = -sin(angle);
	m_RotXY_4x4.m[2][2] = cos(angle);
	m_RotXY_4x4.m[3][3] = 1.0f;
}
void myMatrix::init_RotYZ_4x4(float angle)
{
	//converts degrees to rads
	angle = angle * 3.14159f / 180.0f;
	m_RotYZ_4x4.m[0][0] = 1;
	m_RotYZ_4x4.m[1][1] = cos(angle);
	m_RotYZ_4x4.m[2][1] = -sin(angle);
	m_RotYZ_4x4.m[1][2] = sin(angle);
	m_RotYZ_4x4.m[2][2] = cos(angle);
	m_RotYZ_4x4.m[3][3] = 1.0f;
}

void myMatrix::init_Tran_4x4(float dx, float dy, float dz)
{
	m_Translate_4x4.m[0][0] = 1.0f;
	m_Translate_4x4.m[3][0] = dx;
	m_Translate_4x4.m[1][1] = 1.0f;
	m_Translate_4x4.m[3][1] = dy;
	m_Translate_4x4.m[2][2] = 1.0f;
	m_Translate_4x4.m[3][2] = dz;
	m_Translate_4x4.m[3][3] = 1.0f;
}
void myMatrix::Translate_4x4(myPoint* in, myPoint* out)
{
	myPoint d = *in;
	out->tuple[0] = d.tuple[0] * m_Translate_4x4.m[0][0] + d.tuple[1] * m_Translate_4x4.m[1][0] + d.tuple[2] * m_Translate_4x4.m[2][0] + 1 * m_Translate_4x4.m[3][0];
	out->tuple[1] = d.tuple[0] * m_Translate_4x4.m[0][1] + d.tuple[1] * m_Translate_4x4.m[1][1] + d.tuple[2] * m_Translate_4x4.m[2][1] + 1 * m_Translate_4x4.m[3][1];
	out->tuple[2] = d.tuple[0] * m_Translate_4x4.m[0][2] + d.tuple[1] * m_Translate_4x4.m[1][2] + d.tuple[2] * m_Translate_4x4.m[2][2] + 1 * m_Translate_4x4.m[3][2];
}

void myMatrix::init_Scale_4x4(float dx, float dy, float dz)
{
	m_Scale_4x4.m[0][0] = dx;
	m_Scale_4x4.m[1][1] = dy;
	m_Scale_4x4.m[2][2] = dz;
	m_Scale_4x4.m[3][3] = 1.0f;
}
void myMatrix::Scale_4x4(myPoint* in, myPoint* out)
{
	myPoint d = *in;
	out->tuple[0] = d.tuple[0] * m_Scale_4x4.m[0][0] + d.tuple[1] * m_Scale_4x4.m[1][0] + d.tuple[2] * m_Scale_4x4.m[2][0] + 1 * m_Scale_4x4.m[3][0];
	out->tuple[1] = d.tuple[0] * m_Scale_4x4.m[0][1] + d.tuple[1] * m_Scale_4x4.m[1][1] + d.tuple[2] * m_Scale_4x4.m[2][1] + 1 * m_Scale_4x4.m[3][1];
	out->tuple[2] = d.tuple[0] * m_Scale_4x4.m[0][2] + d.tuple[1] * m_Scale_4x4.m[1][2] + d.tuple[2] * m_Scale_4x4.m[2][2] + 1 * m_Scale_4x4.m[3][2];
}

void myMatrix::Rotate_XY_4x4(Vector* in, Vector* out)
{
	Vector d = *in;
	out->tuple[0] = d.tuple[0] * m_RotXY_4x4.m[0][0] + d.tuple[1] * m_RotXY_4x4.m[1][0] + d.tuple[2] * m_RotXY_4x4.m[2][0] + 1 * m_RotXY_4x4.m[3][0];
	out->tuple[1] = d.tuple[0] * m_RotXY_4x4.m[0][1] + d.tuple[1] * m_RotXY_4x4.m[1][1] + d.tuple[2] * m_RotXY_4x4.m[2][1] + 1 * m_RotXY_4x4.m[3][1];
	out->tuple[2] = d.tuple[0] * m_RotXY_4x4.m[0][2] + d.tuple[1] * m_RotXY_4x4.m[1][2] + d.tuple[2] * m_RotXY_4x4.m[2][2] + 1 * m_RotXY_4x4.m[3][2];
}
void myMatrix::Rotate_XZ_4x4(Vector* in, Vector* out)
{
	Vector d = *in;
	out->tuple[0] = d.tuple[0] * m_RotXZ_4x4.m[0][0] + d.tuple[1] * m_RotXZ_4x4.m[1][0] + d.tuple[2] * m_RotXZ_4x4.m[2][0] + 1 * m_RotXZ_4x4.m[3][0];
	out->tuple[1] = d.tuple[0] * m_RotXZ_4x4.m[0][1] + d.tuple[1] * m_RotXZ_4x4.m[1][1] + d.tuple[2] * m_RotXZ_4x4.m[2][1] + 1 * m_RotXZ_4x4.m[3][1];
	out->tuple[2] = d.tuple[0] * m_RotXZ_4x4.m[0][2] + d.tuple[1] * m_RotXZ_4x4.m[1][2] + d.tuple[2] * m_RotXZ_4x4.m[2][2] + 1 * m_RotXZ_4x4.m[3][2];
}
void myMatrix::Rotate_YZ_4x4(Vector* in, Vector* out)
{
	Vector d = *in;
	out->tuple[0] = d.tuple[0] * m_RotYZ_4x4.m[0][0] + d.tuple[1] * m_RotYZ_4x4.m[1][0] + d.tuple[2] * m_RotYZ_4x4.m[2][0] + 1 * m_RotYZ_4x4.m[3][0];
	out->tuple[1] = d.tuple[0] * m_RotYZ_4x4.m[0][1] + d.tuple[1] * m_RotYZ_4x4.m[1][1] + d.tuple[2] * m_RotYZ_4x4.m[2][1] + 1 * m_RotYZ_4x4.m[3][1];
	out->tuple[2] = d.tuple[0] * m_RotYZ_4x4.m[0][2] + d.tuple[1] * m_RotYZ_4x4.m[1][2] + d.tuple[2] * m_RotYZ_4x4.m[2][2] + 1 * m_RotYZ_4x4.m[3][2];
}


/*void myMatrix::ProjectPoint(myPoint* in, myPoint* out)
{
	myPoint d = *in;
	out->tuple[0] = d.tuple[0] * m_ViewTransform_4x4.m[0][0] + d.tuple[1] * m_ViewTransform_4x4.m[1][0] + d.tuple[2] * m_ViewTransform_4x4.m[2][0] + 1 * m_ViewTransform_4x4.m[3][0];
	out->tuple[1] = d.tuple[0] * m_ViewTransform_4x4.m[0][1] + d.tuple[1] * m_ViewTransform_4x4.m[1][1] + d.tuple[2] * m_ViewTransform_4x4.m[2][1] + 1 * m_ViewTransform_4x4.m[3][1];
	out->tuple[2] = d.tuple[0] * m_ViewTransform_4x4.m[0][2] + d.tuple[1] * m_ViewTransform_4x4.m[1][2] + d.tuple[2] * m_ViewTransform_4x4.m[2][2] + 1 * m_ViewTransform_4x4.m[3][2];
	float tup_4 = d.tuple[0] * m_ViewTransform_4x4.m[0][3] + d.tuple[1] * m_ViewTransform_4x4.m[1][3] + d.tuple[2] * m_ViewTransform_4x4.m[2][3] + 1 * m_ViewTransform_4x4.m[3][3];
	out->tuple[0] /= tup_4;
	out->tuple[1] /= tup_4;
	out->tuple[2] /= tup_4;
}*/