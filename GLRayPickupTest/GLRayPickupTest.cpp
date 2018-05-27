#include "stdafx.h"
#include "MyTrackball.hpp"
#include "MyCollisionHelper.hpp"
#include "MyGLHelper.hpp"


#pragma comment(lib, "glew32.lib")


namespace
{
	const char WindowTitle[] = "OpenGL Ray Pickup Test";

	const int InitialWindowPositionX = 100;
	const int InitialWindowPositionY = 100;

	// ワールド座標系での交差判定のマージン[Length]。
	const float IntersectMarginInWolrd = 0.2f;
	// スクリーン座標系での交差判定のマージン[Pixels]。
	const float IntersectMarginInScreen = 2.0f;


#pragma region // グローバル変数。//

	MyGLHelper::Viewport g_viewport = { 0, 0, 720, 720, 0.0f, 1.0f };
	const MyGLHelper::PerspectiveParam g_persParam = { 45.0f, 0.1f, 1000.0f };
	MyGLHelper::CameraParam g_camera(MyVector3F(0, 0, 80), MyVector3F(0, 0, 0), MyVector3F(0, 1, 0));

	class MouseData
	{
	public:
		bool IsLeftButtonPressed;
		bool IsMiddleButtonPressed;
		bool IsRightButtonPressed;
		MyVector2I DragStartPosL;
		MyVector2I DragStartPosM;
		MyVector2I DragStartPosR;
		MyVector2I CurrentPos;
	public:
		MouseData()
		{}
	public:
		void GetNormalizedLDraggingRect(int& rectL, int& rectT, int& rectR, int& rectB) const
		{
			// 左上座標＜右下座標となるように正規化する。
			if (this->CurrentPos.x < this->DragStartPosL.x)
			{
				rectL = this->CurrentPos.x;
				rectR = this->DragStartPosL.x;
			}
			else
			{
				rectL = this->DragStartPosL.x;
				rectR = this->CurrentPos.x;
			}
			if (this->CurrentPos.y < this->DragStartPosL.y)
			{
				rectT = this->CurrentPos.y;
				rectB = this->DragStartPosL.y;
			}
			else
			{
				rectT = this->DragStartPosL.y;
				rectB = this->CurrentPos.y;
			}
		}
	};
	MouseData g_mouseData;

	MyTrackball g_myMeshTrackball;

	struct MyPointData
	{
		MyVector3F Position; // ワールド位置座標。
		MyVector4F Color;
		bool IsSelected; // マウス クリックなどにより選択されているかどうか。
	};

	std::vector<MyPointData> g_pointCloudVertices;

	bool g_rendersCoordAxes = true;
	bool g_usesWorldUnitAsIntersectMargin = false;

#pragma endregion


	void WaitForUserInput()
	{
		puts("Press any...");
		_getch();
	}

	void MyGLDrawString(void* pGlutFontType, const char* pStr)
	{
		while (*pStr)
		{
			glutBitmapCharacter(pGlutFontType, *pStr);
			++pStr;
		}
	}
}

// 関数プロトタイプ。
void InitializeApp();
void Display();
void Idle();
void Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void Special(int key, int x, int y);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void PassiveMotion(int x, int y);
void OnMouseWheel(int wheelNumber, int direction, int x, int y);

// エントリーポイント。
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowPosition(InitialWindowPositionX, InitialWindowPositionY);
	glutInitWindowSize(g_viewport.Width, g_viewport.Height);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow(WindowTitle);
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutIdleFunc(Idle);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutPassiveMotionFunc(PassiveMotion);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(Special);
	glutMouseWheelFunc(OnMouseWheel);
	// メイン ウィンドウにクローズ メッセージが投げられたときに、メイン ループを抜ける。
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	InitializeApp();

	glutMainLoop();

	return 0;
}

void InitializeApp()
{
	// GLEW の初期化。
	// これにより、Windows で OpenGL 1.2 以上を利用するのが楽になる。
	// 実際に OpenGL 1.2 以上の Features (Extensions) にハードウェアおよびドライバーが対応しているかどうか不明な環境で実行する際は、
	// バージョン情報やエントリーポイントをつぶさに調べる必要がある。
	if (glewInit() != GLEW_OK)
	{
		puts("Error : Failed to initialize GLEW!!");
		WaitForUserInput();
		exit(-1);
	}

	// 点群の頂点データを設定。
	const int pointsNum = 1000;
	const float radius = 10;
	g_pointCloudVertices.resize(pointsNum);
	for (int i = 0; i < pointsNum; ++i)
	{
		MyPointData& point = g_pointCloudVertices[i];
		MyVector3F& pos = point.Position;
		// ランダム位置ベクトルを正規化して、点群を球面分布させる。
		// ただし、もし偶然にも (x, y, z) = (0, 0, 0) だった場合は、原点を指す点が混ざることになる。
		pos.x = MyMath::GetMyNormalRandF();
		pos.y = MyMath::GetMyNormalRandF();
		pos.z = MyMath::GetMyNormalRandF();
		pos = glm::normalize(pos);
		pos *= radius;
		// 色は象限ごとに分けてみる。X, Y, Z 成分がすべて正ならば黄色、すべて負ならばシアン、さもなくば白。
		if (pos.x > 0 && pos.y > 0 && pos.z > 0)
		{
			point.Color = MyColorFYellow;
		}
		else if (pos.x < 0 && pos.y < 0 && pos.z < 0)
		{
			point.Color = MyColorFCyan;
		}
		else
		{
			point.Color = MyColorFWhite;
		}
		point.IsSelected = false;
	}
}

void Idle()
{
	glutPostRedisplay();
}

void Reshape(int w, int h)
{
	g_viewport.Width = (w > 0) ? w : 1;
	g_viewport.Height = (h > 0) ? h : 1;

	g_myMeshTrackball.OnResize(g_viewport.Width, g_viewport.Height);
}

namespace
{
	void CalcUnProjectedRayPositions(MyVector3F& vWCoord0, MyVector3F& vWCoord1, const MyMatrix4x4F& matView, const MyMatrix4x4F& matProj)
	{
		// マウス位置 vS をワールド座標 vW に変換するには、
		// vS = Mviewport * Mproj * Mview * vW であることから、
		// vW = Mview^-1 * Mproj^-1 * Mviewport^-1 * vS を計算してやればよい。
		// 逆プロジェクション変換を行なうユーティリティは、GLU の gluUnProject() や GLM の glm::unProject() が存在する。
		// ここではビューポート設定値などをもとに逆プロジェクション行列そのものを計算してみる。
		// なお、一般的な各種 3D 変換行列の計算アルゴリズムは、GLM のヘッダーにすべて書かれてある。

		// 変換行列の計算を含めて、CPU プログラム側で完全にオフスクリーン実行すれば、OpenGL ES のバージョンにも依存しないし、Direct3D でも使える。
		// なお、変換結果はここで入力する仮想深度値に左右されるので注意。
		// 変換結果の座標値をそのまま利用するのではなく、交差判定に使う無限直線を算出するために Near 側と Far 側をペアで使う。
		// 
		// もし最前面にある物体のみを取得したい場合、OpenGL レンダリングを実行した後で glReadPixels() を使って、
		// あるピクセル位置に書き込まれた深度情報を取得して利用することもできる。
		// もしくは glSelectBuffer(), glRenderMode(GL_SELECT), gluPickMatrix() を使う方法もある。
		// ただしこれらの方法が OpenGL ES でも利用できるとは限らないので注意。
		MyMatrix4x4F matUnproj;
		MyGLHelper::CreateMatrixUnProjectionScreenCoordToWorldCoord(matUnproj, matView, matProj, g_viewport);
		vWCoord0 = MyGLHelper::TransformVector3Coord(matUnproj, MyVector3F(g_mouseData.CurrentPos.x, g_mouseData.CurrentPos.y, 0));
		vWCoord1 = MyGLHelper::TransformVector3Coord(matUnproj, MyVector3F(g_mouseData.CurrentPos.x, g_mouseData.CurrentPos.y, 1));
	}

	MyMatrix4x4F CalcViewMatrix()
	{
		const MyMatrix4x4F matLookAt = MyGLHelper::CreateMatrixLookAt(g_camera);

		const MyMatrix4x4F matTrackball = g_myMeshTrackball.GetRotationMatrix();

		return matLookAt * matTrackball;
	}

	MyMatrix4x4F CalcProjectionMatrix()
	{
#if 1
		return MyGLHelper::CreateMatrixPerspectiveFov(g_viewport, g_persParam);
#else
		return MyGLHelper::CreateMatrixOrotho(
			-g_viewport.Width * 0.5f, +g_viewport.Width * 0.5f, -g_viewport.Height * 0.5f, +g_viewport.Height * 0.5f,
			g_persParam.Near, g_persParam.Far);
#endif
	}

	MyMatrix4x4F CalcTransformMatrixWorldCoordToScreenCoord()
	{
		return MyGLHelper::CreateMatrixTransformWorldCoordToScreenCoord(
			CalcViewMatrix(), CalcProjectionMatrix(), g_viewport);
	}
} // end of namespace


void Display()
{
	const MyVector4F backColor = MyColorFDodgerBlue;
	glClearColor(backColor.r, backColor.g, backColor.b, backColor.a);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// プロジェクション行列の設定。
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#if 0
#if 1
	gluPerspective(g_persParam.Fov, static_cast<double>(g_viewport.Width) / g_viewport.Height, g_persParam.Near, g_persParam.Far);
#else
	glOrtho(
		-g_viewport.Width * 0.5f, +g_viewport.Width * 0.5f, -g_viewport.Height * 0.5f, +g_viewport.Height * 0.5f,
		g_persParam.Near, g_persParam.Far);
#endif
#else
	const MyMatrix4x4F matProj = CalcProjectionMatrix();
	glMultMatrixf(&matProj[0][0]);
#endif

	glMatrixMode(GL_MODELVIEW);
	glViewport(g_viewport.X, g_viewport.Y, g_viewport.Width, g_viewport.Height);

	// 視点の設定。
#if 0
	gluLookAt(
		g_camera.Eye.x, g_camera.Eye.y, g_camera.Eye.z,
		g_camera.At.x, g_camera.At.y, g_camera.At.z,
		g_camera.Up.x, g_camera.Up.y, g_camera.Up.z);

	glMultMatrixf(g_myMeshTrackball.GetRotationMatrixAsArray());
#else
	const MyMatrix4x4F matView = CalcViewMatrix();
	glMultMatrixf(&matView[0][0]);
#endif

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	// トラックボール中心を原点とした座標軸の描画。
	if (g_rendersCoordAxes)
	{
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		const float coordLength = 20.0f;

		glColor4fv(&MyColorFRed.r);
		glVertex3f(0, 0, 0);
		glVertex3f(coordLength, 0, 0);

		glColor4fv(&MyColorFLime.r);
		glVertex3f(0, 0, 0);
		glVertex3f(0, coordLength, 0);

		glColor4fv(&MyColorFBlue.r);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, coordLength);

		glEnd();
		glEnable(GL_LIGHTING);
	}

	MyVector3F vWCoord0, vWCoord1;
	// 点群の描画。
	// 同時に、マウス カーソル位置を通り画面に直交するレイと、点との交差判定を行なう。
	{
		// もし交差判定の結果を CPU 側で持つ必要がない場合、判定計算を GPU 側（GLSL 頂点シェーダー側）で行なうこともできなくはない。
		glDisable(GL_LIGHTING);
		glPointSize(2.0f);
		glBegin(GL_POINTS);

		// マウス位置をワールド座標に変換する。
		CalcUnProjectedRayPositions(vWCoord0, vWCoord1, matView, matProj);

		// 点群の交差判定と描画をまとめて行なう。
		if (g_usesWorldUnitAsIntersectMargin)
		{
			// レイをワールド座標へ射影して、点群の各点（小さな球）との交差判定を行なう。
			const size_t pointsNum = g_pointCloudVertices.size();
			for (size_t i = 0; i < pointsNum; ++i)
			{
				const MyPointData& point = g_pointCloudVertices[i];

				// 画面を貫く無限直線と、ターゲット位置を中心とし交差マージンを半径とする球の交差を調べる。
				const bool intersects = MyCollision::CheckLineIntersectWithSphere(
					vWCoord0, vWCoord1, point.Position, IntersectMarginInWolrd);

				const MyVector4F pointColor = intersects ? MyColorFMagenta : (point.IsSelected ? MyColorFBlack : point.Color);
				glColor4fv(&pointColor.r);
				glVertex3fv(&point.Position.x);
			}
		}
		else
		{
			// 点群の各点を CPU でスクリーン座標変換し、マウス位置と交差するかどうかを調べる。
			const MyMatrix4x4F matToScreen = CalcTransformMatrixWorldCoordToScreenCoord();

			const size_t pointsNum = g_pointCloudVertices.size();
			for (size_t i = 0; i < pointsNum; ++i)
			{
				MyPointData& point = g_pointCloudVertices[i];
				const MyVector3F vScreen = MyGLHelper::TransformVector3Coord(matToScreen, point.Position);
				const float mouseX = float(g_mouseData.CurrentPos.x);
				const float mouseY = float(g_mouseData.CurrentPos.y);
				const bool intersects = MyCollision::CheckIntersectWithAABBparameterizedMinMax2D(
					mouseX, mouseY,
					vScreen.x - IntersectMarginInScreen,
					vScreen.y - IntersectMarginInScreen,
					vScreen.x + IntersectMarginInScreen,
					vScreen.y + IntersectMarginInScreen);

				const MyVector4F pointColor = intersects ? MyColorFMagenta : (point.IsSelected ? MyColorFBlack : point.Color);
				glColor4fv(&pointColor.r);
				glVertex3fv(&point.Position.x);
			}
		}

		// もし、シーンの拡大縮小（カメラのズームイン・ズームアウト）に関わらず、
		// 点群の各点が常に一定サイズの OpenGL ポイント プリミティブで描画される場合、
		// スクリーン座標系での交差判定のほうが都合がよい。
		// 逆に、カメラが接近すると実サイズを模した多ポリゴンの球で描画されるなど、
		// 描画される点の大きさがシーンの拡大縮小に左右される場合、
		// ワールド座標系での交差判定のほうが都合がよい。
		// 各々のメリットは交差判定マージンをどの座標系で設定するのか、にもよるので、
		// それを加味してどの座標系での交差判定を行なうかを決定するとよい。

		glEnd();
		glEnable(GL_LIGHTING);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (g_mouseData.IsLeftButtonPressed)
	{

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		// 左上を原点とするマウスの 2D 座標を直接指定できるように、正射影する。
		gluOrtho2D(0, g_viewport.Width, g_viewport.Height, 0);

		// スクリーン選択矩形の描画（半透明）。
		// 深度テストを切って（無視して）最前面に描画する。深度バッファへの書き込みも禁止しておく。
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(false);

		{
			int rectL = 0;
			int rectT = 0;
			int rectR = 0;
			int rectB = 0;
			g_mouseData.GetNormalizedLDraggingRect(rectL, rectT, rectR, rectB);

			glColor4f(0.0f, 0.4f, 0.4f, 0.5f);
			glRecti(rectL, rectT, rectR, rectB);

			glColor4f(0.4f, 0.8f, 1.0f, 1.0f);
			glBegin(GL_LINE_LOOP);
			glVertex2d(rectL, rectT);
			glVertex2d(rectR, rectT);
			glVertex2d(rectR, rectB);
			glVertex2d(rectL, rectB);
			glEnd();
		}

		glEnable(GL_DEPTH_TEST);
		glDepthMask(true);
	}

	// メッセージの描画。
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glDisable(GL_BLEND);
		static char message[1024];
		const int fontSize = 18;
		const int offsetAmt = 2;
		// glRasterPos*() と違い、glWindowPos*() は OpenGL 1.4 の機能。

		glColor4fv(&MyColorFWhite.r);

		sprintf_s(message, "Mouse0(%4d, %4d, 0) = World0(%+8.2f, %+8.2f, %+8.2f)",
			g_mouseData.CurrentPos.x, g_mouseData.CurrentPos.y,
			vWCoord0.x, vWCoord0.y, vWCoord0.z);
		glWindowPos2i(offsetAmt, g_viewport.Height - (fontSize + offsetAmt) * 1);
		MyGLDrawString(GLUT_BITMAP_9_BY_15, message);

		sprintf_s(message, "Mouse1(%4d, %4d, 1) = World1(%+8.2f, %+8.2f, %+8.2f)",
			g_mouseData.CurrentPos.x, g_mouseData.CurrentPos.y,
			vWCoord1.x, vWCoord1.y, vWCoord1.z);
		glWindowPos2i(offsetAmt, g_viewport.Height - (fontSize + offsetAmt) * 2);
		MyGLDrawString(GLUT_BITMAP_9_BY_15, message);

		glColor4fv(&MyColorFLime.r);

		sprintf_s(message, "L-Click/L-Drag:Select, R-Drag:Rotation, Wheel:Zoom");
		glWindowPos2i(offsetAmt, g_viewport.Height - (fontSize + offsetAmt) * 3);
		MyGLDrawString(GLUT_BITMAP_9_BY_15, message);
	}

	glutSwapBuffers();
}

void Mouse(int button, int state, int x, int y)
{
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		g_mouseData.IsLeftButtonPressed = (state == GLUT_DOWN);
		if (g_mouseData.IsLeftButtonPressed)
		{
			g_mouseData.DragStartPosL = MyVector2I(x, y);
		}
		else
		{
			// ドラッグ開始位置と終了位置がほぼ同じ場合、小範囲ピッキングとみなす。
			// 異なる場合は広範囲矩形選択とみなす。
			// 交差判定の計算量は O(n) となる。
			const MyVector2I vDiff = g_mouseData.DragStartPosL - MyVector2I(x, y);
			if (MyMath::GetVectorLength(vDiff) < 2)
			{
				if (g_usesWorldUnitAsIntersectMargin)
				{
					// レイをワールド座標へ射影して、点群の各点（小さな球）との交差判定を行なう。
					const MyMatrix4x4F matView = CalcViewMatrix();
					const MyMatrix4x4F matProj = CalcProjectionMatrix();
					MyVector3F vWCoord0, vWCoord1;
					CalcUnProjectedRayPositions(vWCoord0, vWCoord1, matView, matProj);

					const size_t pointsNum = g_pointCloudVertices.size();
					for (size_t i = 0; i < pointsNum; ++i)
					{
						MyPointData& point = g_pointCloudVertices[i];

						// 画面を貫く無限直線と、ターゲット位置を中心とし交差マージンを半径とする球の交差を調べる。
						const bool intersects = MyCollision::CheckLineIntersectWithSphere(
							vWCoord0, vWCoord1, point.Position, IntersectMarginInWolrd);
						// 交差している場合、選択状態を反転。交差していない場合、変更しない。
						if (intersects)
						{
							point.IsSelected = !point.IsSelected;
						}
					}
				}
				else
				{
					// 点群の各点を CPU でスクリーン座標変換し、マウス位置と交差するかどうかを調べる。
					// 変換結果の深度値（スクリーン座標系における Z 座標）を使えば、画面手前のオブジェクトだけ選択する、ということもできる。
					const MyMatrix4x4F matToScreen = CalcTransformMatrixWorldCoordToScreenCoord();

					const size_t pointsNum = g_pointCloudVertices.size();
					for (size_t i = 0; i < pointsNum; ++i)
					{
						MyPointData& point = g_pointCloudVertices[i];
						const MyVector3F vScreen = MyGLHelper::TransformVector3Coord(matToScreen, point.Position);
						const float mouseX = float(g_mouseData.CurrentPos.x);
						const float mouseY = float(g_mouseData.CurrentPos.y);
						const bool intersects = MyCollision::CheckIntersectWithAABBparameterizedMinMax2D(
							mouseX, mouseY,
							vScreen.x - IntersectMarginInScreen,
							vScreen.y - IntersectMarginInScreen,
							vScreen.x + IntersectMarginInScreen,
							vScreen.y + IntersectMarginInScreen);
						// 交差している場合、選択状態を反転。交差していない場合、変更しない。
						if (intersects)
						{
							point.IsSelected = !point.IsSelected;
						}
					}
				}
			}
			else
			{
				// 点群の各点を CPU でスクリーン座標変換し、ドラッグによる選択矩形と交差するかどうかを調べる。
				// スクリーン選択矩形をワールド座標系に射影して交差判定をするのは非常に複雑になり、非効率。
				const MyMatrix4x4F matToScreen = CalcTransformMatrixWorldCoordToScreenCoord();
				int rectL = 0;
				int rectT = 0;
				int rectR = 0;
				int rectB = 0;
				g_mouseData.GetNormalizedLDraggingRect(rectL, rectT, rectR, rectB);
				const size_t pointsNum = g_pointCloudVertices.size();
				for (size_t i = 0; i < pointsNum; ++i)
				{
					MyPointData& point = g_pointCloudVertices[i];
					const MyVector3F vScreen = MyGLHelper::TransformVector3Coord(matToScreen, point.Position);
					const bool intersects = MyCollision::CheckIntersectWithAABBparameterizedMinMax2D(
						int(vScreen.x), int(vScreen.y), rectL, rectT, rectR, rectB);
					point.IsSelected = intersects;
				}
			}
		}
		break;
	case GLUT_RIGHT_BUTTON:
		g_mouseData.IsRightButtonPressed = (state == GLUT_DOWN);
		if (g_mouseData.IsRightButtonPressed)
		{
			g_mouseData.DragStartPosR = MyVector2I(x, y);
			// トラックボール開始。
			g_myMeshTrackball.OnMouseDragStart(x, y);
		}
		else
		{
			// トラックボール停止。
			g_myMeshTrackball.OnMouseDragStop(x, y);
		}
		break;
	default:
		break;
	}
}

// マウス ドラッグ時。
void Motion(int x, int y)
{
	g_mouseData.CurrentPos = MyVector2I(x, y);
	if (g_mouseData.IsRightButtonPressed)
	{
		// トラックボール移動。
		g_myMeshTrackball.OnMouseDragging(x, y);
	}
}

// マウス移動時。ドラッグ中は呼ばれない。
void PassiveMotion(int x, int y)
{
	g_mouseData.CurrentPos = MyVector2I(x, y);
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case '\033':
		//exit(0);
		::PostQuitMessage(key);
		break;

	case 'f':
		break;

	case 'a':
		g_rendersCoordAxes = !g_rendersCoordAxes;
		printf("g_rendersCoordAxes = %d\n", g_rendersCoordAxes);
		break;

	case 's':
		g_usesWorldUnitAsIntersectMargin = false;
		printf("g_usesWorldUnitAsIntersectMargin = %d\n", g_usesWorldUnitAsIntersectMargin);
		break;

	case 't':
		break;

	case 'w':
		g_usesWorldUnitAsIntersectMargin = true;
		printf("g_usesWorldUnitAsIntersectMargin = %d\n", g_usesWorldUnitAsIntersectMargin);
		break;

	case 'm':
		break;

	default:
		break;
	}
}

void Special(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F1:
		break;

	case GLUT_KEY_F2:
		break;

	case GLUT_KEY_F3:
		break;

	case GLUT_KEY_F4:
		break;

	case GLUT_KEY_F5:
		break;

	case GLUT_KEY_F6:
		break;

	case GLUT_KEY_F7:
		break;

	case GLUT_KEY_F8:
		break;

	case GLUT_KEY_F9:
		break;

	case GLUT_KEY_F10:
		break;

	case GLUT_KEY_F11:
		break;

	case GLUT_KEY_F12:
		break;

	case GLUT_KEY_LEFT:
		break;

	case GLUT_KEY_RIGHT:
		break;

	case GLUT_KEY_UP:
		break;

	case GLUT_KEY_DOWN:
		break;

	case GLUT_KEY_PAGE_UP:
		break;

	case GLUT_KEY_PAGE_DOWN:
		break;

	case GLUT_KEY_HOME:
		break;

	case GLUT_KEY_END:
		break;

	case GLUT_KEY_INSERT:
		break;
	}
}


void OnMouseWheel(int wheelNumber, int direction, int x, int y)
{
	if (direction > 0)
	{
		g_camera.Eye.z -= 1;
		const float LimitDistance = 1;
		if (g_camera.Eye.z <= LimitDistance)
		{
			g_camera.Eye.z = LimitDistance;
		}
	}
	else if (direction < 0)
	{
		g_camera.Eye.z += 1;
	}
}
