#include "stdafx.h"
#include "MyTrackball.hpp"


MyTrackball::MyTrackball()
	: m_dragStartPos()
	, m_scalingVal()
	, m_cq(1, 0, 0, 0)
	, m_tq(1, 0, 0, 0)
	, m_rotMatrix(1)
	, m_isDragging()
{
}

void MyTrackball::OnResize(int w, int h)
{
	m_scalingVal.x = 1.0f / w;
	m_scalingVal.y = 1.0f / h;
}

void MyTrackball::OnMouseDragStart(int x, int y)
{
	m_isDragging = true;

	m_dragStartPos.x = x;
	m_dragStartPos.y = y;
}

void MyTrackball::OnMouseDragging(int x, int y)
{
	if (m_isDragging)
	{
		const MyVector2F diff(
			(x - m_dragStartPos.x) * m_scalingVal.x,
			(y - m_dragStartPos.y) * m_scalingVal.y);

		const float a = MyMath::GetVectorLength(diff);

		if (a > glm::epsilon<float>())
		{
			const float ar = a * MyMath::F_PI;
			const float as = sin(ar) / a;
			const MyQuaternion4F dq(cos(ar), diff.y * as, diff.x * as, 0);

			m_tq = glm::cross(dq, m_cq);

			m_rotMatrix = glm::mat4_cast(m_tq);
		}
	}
}

void MyTrackball::OnMouseDragStop(int x, int y)
{
	OnMouseDragging(x, y);

	m_cq = m_tq;

	m_isDragging = false;
}
