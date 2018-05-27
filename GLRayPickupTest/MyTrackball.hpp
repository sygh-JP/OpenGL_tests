#pragma once

#include "MyMath.hpp"


//! @brief  簡易トラックボール処理クラス。<br>
class MyTrackball
{
	MyVector2I m_dragStartPos; //!< ドラッグ開始位置。<br>
	MyVector2F m_scalingVal; //!< マウスの絶対位置→ウィンドウ内での相対位置の換算係数。<br>
	MyQuaternion4F m_cq; //!< 回転の初期値（クォータニオン）。<br>
	MyQuaternion4F m_tq; //!< ドラッグ中の回転（クォータニオン）。<br>
	MyMatrix4x4F m_rotMatrix; //!< 回転の変換行列。<br>
	bool m_isDragging; //!< ドラッグ中か否か。<br>
public:
	MyTrackball();
	void OnResize(int w, int h); //!< トラックボール処理の範囲指定。<br>
	void OnMouseDragStart(int x, int y); //!< トラックボール処理の開始。<br>
	void OnMouseDragging(int x, int y); //!< 回転の変換行列の計算。<br>
	void OnMouseDragStop(int x, int y); //!< トラックボール処理の停止。<br>

	//! @brief  回転行列を取得する。<br>
	//! 戻り値を glMultMatrixf() などで使用してカメラもしくはオブジェクトを回転する。<br>
	const float* GetRotationMatrixAsArray() const
	{ return &m_rotMatrix[0].x; }
	const MyMatrix4x4F& GetRotationMatrix() const
	{ return m_rotMatrix; }
};
