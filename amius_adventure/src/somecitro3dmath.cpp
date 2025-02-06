#include <c3d/maths.h>
void Mtx_Scale(C3D_Mtx* mtx, float x, float y, float z)
{
	int i;
	for (i = 0; i < 4; ++i)
	{
		mtx->r[i].x *= x;
		mtx->r[i].y *= y;
		mtx->r[i].z *= z;
	}
}

void Mtx_RotateX(C3D_Mtx* mtx, float angle, bool bRightSide)
{
	float  a, b;
	float  cosAngle = cosf(angle);
	float  sinAngle = sinf(angle);
	size_t i;

	if (bRightSide)
	{
		for (i = 0; i < 4; ++i)
		{
			a = mtx->r[i].y*cosAngle + mtx->r[i].z*sinAngle;
			b = mtx->r[i].z*cosAngle - mtx->r[i].y*sinAngle;
			mtx->r[i].y = a;
			mtx->r[i].z = b;
		}
	}
	else
	{
		for (i = 0; i < 4; ++i)
		{
			a = mtx->r[1].c[i]*cosAngle - mtx->r[2].c[i]*sinAngle;
			b = mtx->r[2].c[i]*cosAngle + mtx->r[1].c[i]*sinAngle;
			mtx->r[1].c[i] = a;
			mtx->r[2].c[i] = b;
		}
	}
}

void Mtx_RotateY(C3D_Mtx* mtx, float angle, bool bRightSide)
{
	float  a, b;
	float  cosAngle = cosf(angle);
	float  sinAngle = sinf(angle);
	size_t i;

	if (bRightSide)
	{
		for (i = 0; i < 4; ++i)
		{
			a = mtx->r[i].x*cosAngle - mtx->r[i].z*sinAngle;
			b = mtx->r[i].z*cosAngle + mtx->r[i].x*sinAngle;
			mtx->r[i].x = a;
			mtx->r[i].z = b;
		}
	}
	else
	{
		for (i = 0; i < 4; ++i)
		{
			a = mtx->r[0].c[i]*cosAngle + mtx->r[2].c[i]*sinAngle;
			b = mtx->r[2].c[i]*cosAngle - mtx->r[0].c[i]*sinAngle;
			mtx->r[0].c[i] = a;
			mtx->r[2].c[i] = b;
		}
	}
}

void Mtx_RotateZ(C3D_Mtx* mtx, float angle, bool bRightSide)
{
	float  a, b;
	float  cosAngle = cosf(angle);
	float  sinAngle = sinf(angle);
	size_t i;

	if (bRightSide)
	{
		for (i = 0; i < 4; ++i)
		{
			a = mtx->r[i].x*cosAngle + mtx->r[i].y*sinAngle;
			b = mtx->r[i].y*cosAngle - mtx->r[i].x*sinAngle;
			mtx->r[i].x = a;
			mtx->r[i].y = b;
		}
	}
	else
	{
		for (i = 0; i < 4; ++i)
		{
			a = mtx->r[0].c[i]*cosAngle - mtx->r[1].c[i]*sinAngle;
			b = mtx->r[1].c[i]*cosAngle + mtx->r[0].c[i]*sinAngle;
			mtx->r[0].c[i] = a;
			mtx->r[1].c[i] = b;
		}
	}
}

void Mtx_Translate(C3D_Mtx* mtx, float x, float y, float z, bool bRightSide)
{

	C3D_FVec v = FVec4_New(x, y, z, 1.0f);
	int i, j;

	if (bRightSide)
	{
		for (i = 0; i < 4; ++i)
			mtx->r[i].w = FVec4_Dot(mtx->r[i], v);
	}
	else
	{
		for (j = 0; j < 3; ++j)
			for (i = 0; i < 4; ++i)
				mtx->r[j].c[i] += mtx->r[3].c[i] * v.c[3-j];
	}

}