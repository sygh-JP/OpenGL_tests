#pragma once

#include "MyMath.hpp"

// 古い GLM の角度単位は昔の OpenGL 固定機能を踏襲しているらしく、既定で Degrees だが、GLM_FORCE_RADIANS を定義すると Radians になるらしい。
// GLM_FORCE_RADIANS は 0.9.6.0 で削除された模様。既定で Radians になったらしい。
// また、古い GLM では glm::mat4 のデフォルト コンストラクタで単位行列になっていたが、
// 新しい GLM のデフォルト コンストラクタは未初期化となるように仕様変更されたらしい。

namespace MyGLHelper
{
	//! @brief  Direct3D ライクのビューポート構造体。<br>
	struct Viewport
	{
		int X, Y, Width, Height;
		float MinZ, MaxZ; //!< 通常、0 と 1 を指定する。<br>
	};

	struct PerspectiveParam
	{
		float Fov; //!< 視野角[Degrees]。<br>
		float Near, Far;
	};

	class CameraParam
	{
	public:
		MyVector3F Eye;
		MyVector3F At;
		MyVector3F Up;

	public:
		CameraParam(MyVector3F vEye, MyVector3F vAt, MyVector3F vUp)
			: Eye(vEye)
			, At(vAt)
			, Up(vUp)
		{}
	};


	inline MyMatrix4x4F CreateMatrixLookAt(const CameraParam& camera)
	{
		return glm::lookAt(camera.Eye, camera.At, camera.Up);
	}

	inline MyMatrix4x4F CreateMatrixPerspectiveFov(const Viewport& vp, const PerspectiveParam& pers)
	{
		return glm::perspectiveFov(glm::radians(pers.Fov), float(vp.Width), float(vp.Height), pers.Near, pers.Far);
	}

	inline MyMatrix4x4F CreateMatrixOrotho2D(float left, float right, float bottom, float top)
	{
		return glm::ortho(left, right, bottom, top);
	}

	inline MyMatrix4x4F CreateMatrixOrotho(float left, float right, float bottom, float top, float zNear, float zFar)
	{
		return glm::ortho(left, right, bottom, top, zNear, zFar);
	}


	//! @brief  指定された行列により 3D ベクトルをトランスフォームし、その結果を w = 1 に射影する。<br>
	//! Direct3D の D3DXVec3TransformCoord() 類似。<br>
	inline MyVector3F TransformVector3Coord(const MyMatrix4x4F& mat, MyVector3F vec)
	{
		MyVector4F vOut(vec.x, vec.y, vec.z, 1);

		vOut = mat * vOut;
		vOut /= vOut.w; // w = 1 に射影。

		return MyVector3F(vOut);
	}


	//! @brief  ビューポート行列を作成。<br>
	//! 
	//! なお、OpenGL や Direct3D では固定機能でもプログラマブル シェーダーでも、<br>
	//! この行列の詳細だけは隠ぺいされ、通常は計算で求めることはなく、ビューポートのパラメータを API で指定する方式になっている。<br>
	inline MyMatrix4x4F CreateViewportMatrix(
		const Viewport& vp)
	{
		MyMatrix4x4F matVP(1); // 単位行列。

		matVP[0][0] = +vp.Width * 0.5f;
		matVP[1][1] = -vp.Height * 0.5f;
		matVP[2][2] = vp.MaxZ - vp.MinZ;
		matVP[3][0] = float(vp.X) + vp.Width * 0.5f; // D3D では [0][3] 要素。
		matVP[3][1] = float(vp.Y) + vp.Height * 0.5f; // D3D では [1][3] 要素。
		matVP[3][2] = vp.MinZ; // D3D では [2][3] 要素。

		return matVP;
	}

	//! @brief  ワールド座標をスクリーン位置へ変換する行列を作成する。<br>
	inline MyMatrix4x4F CreateMatrixTransformWorldCoordToScreenCoord(
		const MyMatrix4x4F& matView,
		const MyMatrix4x4F& matProj,
		const Viewport& vp)
	{
		return CreateViewportMatrix(vp) * matProj * matView;
	}

	//! @brief  スクリーン位置をワールド座標へ変換する行列を作成する。<br>
	inline void CreateMatrixUnProjectionScreenCoordToWorldCoord(
		MyMatrix4x4F& outMatrix,
		const MyMatrix4x4F& matView,
		const MyMatrix4x4F& matProj,
		const Viewport& vp)
	{

		// 逆行列の計算は負荷が高いので、(Mview^-1 * Mproj^-1 * Mviewport^-1) を求めるよりも、
		// (Mviewport * Mproj * Mview)^-1 を求めたほうがよい。
#if 0
		MyMatrix4x4F matVP = CreateViewportMatrix(vp);

		const MyMatrix4x4F invVP = glm::inverse(matVP);

		const MyMatrix4x4F invProjView = glm::inverse(matProj * matView);

		outMatrix = invProjView * invVP;
#else
		outMatrix = glm::inverse(CreateMatrixTransformWorldCoordToScreenCoord(matView, matProj, vp));
#endif
	}

	//! @brief  スクリーン位置をワールド座標へ直接変換。<br>
	//! 何度も同様の変換を繰り返し行なう場合は、一度 CreateMatrixUnProjectionScreenCoordToWorldCoord() で変換行列を計算したほうが効率的。<br>
	inline MyVector3F UnProjectScreenCoordToWorldCoord(
		int screenX,
		int screenY,
		float screenZ,
		const MyMatrix4x4F& matView,
		const MyMatrix4x4F& matProj,
		const Viewport& vp)
	{
		const MyVector3F vScreenCoord(float(screenX), float(screenY), screenZ);
#if 1
		return glm::unProject(vScreenCoord, matView, matProj, MyVector4I(vp.X, vp.Y, vp.Width, vp.Height));
#else
		MyMatrix4x4F matUnproj;
		CreateMatrixUnProjectionScreenCoordToWorldCoord(matUnproj, matView, matProj, vp);

		return TransformVector3Coord(matUnproj, vScreenCoord);
#endif
	}
}
