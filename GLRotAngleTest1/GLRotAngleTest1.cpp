#include <GL/freeglut.h>
#include <cstdio>
#if 0
// DirectXMath
#include <DirectXMath.h>
#else
// GLM
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#endif

// D3DX Math, XNA Math を使う場合は適宜読み替えること。

// 簡単のため、特異点（視点と注視点が一致するときや、ジンバルロックなど）の回避は行なっていない。

namespace
{
#ifdef DIRECTX_MATH_VERSION
	using namespace DirectX;

	using MyVector2F = XMFLOAT2;
	using MyVector3F = XMFLOAT3;
	using MyVector4F = XMFLOAT4;

	using MyMatrix4x4F = XMFLOAT4X4;

	using MyVector2I = XMINT2;
#else
	using MyVector2F = glm::vec2;
	using MyVector3F = glm::vec3;
	using MyVector4F = glm::vec4;

	using MyMatrix4x4F = glm::mat4x4;

	using MyVector2I = glm::ivec2;
#endif

	const double D_PI = 3.14159265358979323846264338327;
	const float  F_PI = 3.14159265358979323846264338327f;

	inline float ToDegrees(float radians)
	{
#ifdef DIRECTX_MATH_VERSION
		return XMConvertToDegrees(radians);
#else
		return glm::degrees(radians);
#endif
	}

	inline MyVector3F RotateVector3ByRollPitchYaw(const MyVector3F& vIn, float pitch, float yaw, float roll)
	{
#ifdef DIRECTX_MATH_VERSION
		const XMMATRIX matRotation = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

		XMFLOAT3 vOut;
		XMStoreFloat3(&vOut, XMVector3TransformCoord(XMLoadFloat3(&vIn), matRotation));
		return vOut;
#else
		const MyMatrix4x4F mat = glm::yawPitchRoll(yaw, pitch, roll);
		MyVector4F vOut(vIn.x, vIn.y, vIn.z, 1);

		vOut = mat * vOut;
		vOut /= vOut.w; // w = 1 に射影。

		return MyVector3F(vOut);
#endif
	}

	// 単位ベクトル u = (0, 0, -1) に対する入力ベクトルの角度（Y 軸周り）を求める。
	inline float CalcAngleYOfVector3(const MyVector3F& vIn)
	{
		return F_PI + atan2f(vIn.x, vIn.z);
	}

	inline float CalcLengthProjectedZXPlaneOfVector3(const MyVector3F& vIn)
	{
		return sqrtf(vIn.z * vIn.z + vIn.x * vIn.x);
	}

	// 単位ベクトル u = (0, 0, -1) に対する入力ベクトルの角度（X, Y 軸周り）を求める。
	inline float CalcAngleXYOfVector3(MyVector2F& vOutAngle, const MyVector3F& vIn)
	{
		vOutAngle.y = CalcAngleYOfVector3(vIn);
		const float lenzx = CalcLengthProjectedZXPlaneOfVector3(vIn);
		vOutAngle.x = atan2f(vIn.y, lenzx);
		return lenzx;
	}

	MyVector3F g_vTargetPoint(1, 1, -1);
	MyVector3F g_vCameraEye(3, 4, 5);
	MyVector2I g_vScreenSize;

	void PrintTargetPoint()
	{
		printf("TargetPoint = (%+.1f, %+.1f, %+.1f)\n", g_vTargetPoint.x, g_vTargetPoint.y, g_vTargetPoint.z);
	}

	void PrintCameraEye()
	{
		printf("CameraEye = (%+.1f, %+.1f, %+.1f)\n", g_vCameraEye.x, g_vCameraEye.y, g_vCameraEye.z);
	}
}

#define USES_EXPLICITLY_ROTATED_VECTOR

void OnRender()
{
	glClearColor(0.0, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, g_vScreenSize.x, g_vScreenSize.y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, static_cast<double>(g_vScreenSize.x) / static_cast<double>(g_vScreenSize.y), 1.0, 100.0);

	static const MyVector3F vCameraAt(0, 0, 0);
	static const MyVector3F vCameraUp(0, 1, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		g_vCameraEye.x, g_vCameraEye.y, g_vCameraEye.z,
		vCameraAt.x, vCameraAt.y, vCameraAt.z,
		vCameraUp.x, vCameraUp.y, vCameraUp.z);

	static const MyVector3F vColorRed(1, 0, 0);
	static const MyVector3F vColorGreen(0, 1, 0);
	static const MyVector3F vColorBlue(0, 0, 1);
	static const MyVector3F vColorCyan(0, 1, 1);
	static const MyVector3F vColorMagenta(1, 0, 1);
	static const MyVector3F vColorYellow(1, 1, 0);

	static const MyVector3F vOrigin(0, 0, 0);
	static const MyVector3F vUnitX(1, 0, 0);
	static const MyVector3F vUnitY(0, 1, 0);
	static const MyVector3F vUnitZ(0, 0, 1);
	// 基準方向の単位ベクトル。右手座標系の画面奥方向。Pitch, Yaw ともにゼロの場合、Z 軸の負方向を向く。
	static const MyVector3F vBaseDir(0, 0, -1);

	MyVector2F vRotAngle;
	CalcAngleXYOfVector3(vRotAngle, g_vTargetPoint);

	// 簡単のため、原点を基準とした方向ベクトル u = (0, 0, -1) を回転させる Euler 角 (Pitch, Yaw) を算出し、
	// それをもとに回転行列を生成するだけにしている。
	// X 軸周りの回転角 (Pitch) と Y 軸周りの回転角 (Yaw) を求める場合であり、Z 軸周りの回転角 (Roll) は使っていない。
	// また、Roll-Pitch-Yaw の順に回転を行なうことを前提にしている。
	// 3次元以上の空間において回転（姿勢）を表現する方法は1つではなく、回転の順序や回転の軸の選び方に左右されるので、
	// 始点と終点が与えられたからといって一意に定まるものではない。
	const MyVector3F vRotated = RotateVector3ByRollPitchYaw(vBaseDir, vRotAngle.x, vRotAngle.y, 0);

	glBegin(GL_LINES);
	{
		glColor3fv(&vColorRed.x);
		glVertex3fv(&vOrigin.x);
		glVertex3fv(&vUnitX.x);

		glColor3fv(&vColorGreen.x);
		glVertex3fv(&vOrigin.x);
		glVertex3fv(&vUnitY.x);

		glColor3fv(&vColorBlue.x);
		glVertex3fv(&vOrigin.x);
		glVertex3fv(&vUnitZ.x);

#ifdef USES_EXPLICITLY_ROTATED_VECTOR
		glColor3fv(&vColorYellow.x);
		glVertex3fv(&vOrigin.x);
		glVertex3fv(&vRotated.x);
#endif
	}
	glEnd();

	glPointSize(2);

	glBegin(GL_POINTS);
	{
		glColor3fv(&vColorCyan.x);
		glVertex3fv(&g_vTargetPoint.x);
	}
	glEnd();

#ifndef USES_EXPLICITLY_ROTATED_VECTOR
	// OpenGL は左から乗算していくので、回転の順序とは逆の順序で呼び出す必要がある。
	glRotatef(ToDegrees(vRotAngle.y), 0, 1, 0);
	glRotatef(ToDegrees(vRotAngle.x), 1, 0, 0);

	glBegin(GL_LINES);
	{
		glColor3fv(&vColorYellow.x);
		glVertex3fv(&vOrigin.x);
		glVertex3fv(&vBaseDir.x);
	}
	glEnd();
#endif

	glutSwapBuffers();
}

void OnResize(int w, int h)
{
	g_vScreenSize.x = w;
	g_vScreenSize.y = h;

	glutPostRedisplay();
}

void OnKeyboard(unsigned char key, int x, int y)
{
	static const float step = 0.5f;
	switch (key)
	{
	case 'X':
		g_vTargetPoint.x -= step;
		PrintTargetPoint();
		glutPostRedisplay();
		break;
	case 'x':
		g_vTargetPoint.x += step;
		PrintTargetPoint();
		glutPostRedisplay();
		break;
	case 'Y':
		g_vTargetPoint.y -= step;
		PrintTargetPoint();
		glutPostRedisplay();
		break;
	case 'y':
		g_vTargetPoint.y += step;
		PrintTargetPoint();
		glutPostRedisplay();
		break;
	case 'Z':
		g_vTargetPoint.z -= step;
		PrintTargetPoint();
		glutPostRedisplay();
		break;
	case 'z':
		g_vTargetPoint.z += step;
		PrintTargetPoint();
		glutPostRedisplay();
		break;

	case 'Q':
	case 'q':
	case 0x1B: // Esc
		//exit(0);
		//::PostQuitMessage(key);
		glutLeaveMainLoop();
		break;
	default:
		break;
	}
}

void OnSpecial(int key, int x, int y)
{
	static const float step = 0.5f;
	switch (key)
	{
	case GLUT_KEY_LEFT:
		g_vCameraEye.x -= step;
		PrintCameraEye();
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		g_vCameraEye.x += step;
		PrintCameraEye();
		glutPostRedisplay();
		break;
	case GLUT_KEY_UP:
		g_vCameraEye.y += step;
		PrintCameraEye();
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		g_vCameraEye.y -= step;
		PrintCameraEye();
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

void OnMouseWheel(int wheelNumber, int direction, int x, int y)
{
	static const float step = 0.5f;
	if (direction > 0)
	{
		g_vCameraEye.z -= step;
		const float LimitDistance = 1;
		if (g_vCameraEye.z <= LimitDistance)
		{
			g_vCameraEye.z = LimitDistance;
		}
	}
	else if (direction < 0)
	{
		g_vCameraEye.z += step;
	}
	PrintCameraEye();
	glutPostRedisplay();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow(argv[0]);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutKeyboardFunc(OnKeyboard);
	glutSpecialFunc(OnSpecial);
	glutMouseWheelFunc(OnMouseWheel);
	glutMainLoop();
	return 0;
}
