#pragma once

#include "MyMath.hpp"

namespace MyCollision
{
	// 点と直線との距離の平方（3D）。
	template<typename T> T GetLengthSquaredBetweenLineAndPoint(const glm::detail::tvec3<T>& q1, const glm::detail::tvec3<T>& q2, const glm::detail::tvec3<T>& point)
	{
		const glm::detail::tvec3<T> vPQ1 = q1 - point, vQ2Q1 = q1 - q2, vCrossPQ1toQ2Q1 = glm::cross(vPQ1, vQ2Q1);
		return MyMath::GetVectorLengthSquared(vCrossPQ1toQ2Q1) / MyMath::GetVectorLengthSquared(vQ2Q1);
	}

	// 直線と球との交差判定。
	template<typename T> bool CheckLineIntersectWithSphere(const glm::detail::tvec3<T>& linePos1, const glm::detail::tvec3<T>& linePos2, const glm::detail::tvec3<T>& sphereCenter, T sphereRadius)
	{
		return (sphereRadius * sphereRadius) >= GetLengthSquaredBetweenLineAndPoint(linePos1, linePos2, sphereCenter);
	}

	// 指定された点と、最大値・最小値を指定された軸平行境界ボックス（AABB）との交差判定を行なう（2D）。
	template<typename T> bool CheckIntersectWithAABBparameterizedMinMax2D(T targetX, T targetY, T aabbMinX, T aabbMinY, T aabbMaxX, T aabbMaxY)
	{
		return
			(targetX > aabbMinX) &&
			(targetX < aabbMaxX) &&
			(targetY > aabbMinY) &&
			(targetY < aabbMaxY);
	}
}
