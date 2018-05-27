#pragma once


typedef glm::dvec2 MyVector2D;
typedef glm::dvec3 MyVector3D;
typedef glm::dvec4 MyVector4D;
typedef glm::dquat MyQuaternion4D;
typedef glm::dmat4 MyMatrix4x4D;

typedef glm::vec2 MyVector2F;
typedef glm::vec3 MyVector3F;
typedef glm::vec4 MyVector4F;
typedef glm::quat MyQuaternion4F;
typedef glm::mat4 MyMatrix4x4F;

typedef glm::ivec2 MyVector2I;
typedef glm::ivec3 MyVector3I;
typedef glm::ivec4 MyVector4I;

namespace glm
{
	namespace detail
	{
		// 古い互換シンボル。
		template<typename T> using tvec2 = vec<2, T, glm::highp>;
		template<typename T> using tvec3 = vec<3, T, glm::highp>;
		template<typename T> using tvec4 = vec<4, T, glm::highp>;
	}
}

class MyRect
{
public:
	float X, Y, Width, Height;
public:
	MyRect()
		: X(), Y(), Width(), Height() {}

	MyRect(float x, float y, float width, float height)
		: X(x), Y(y), Width(width), Height(height) {}

	float GetLeft() const { return this->X; }
	float GetTop() const { return this->Y; }
	float GetRight() const { return this->X + this->Width; }
	float GetBottom() const { return this->Y + this->Height; }
};


namespace MyMath
{
	// glm::pi() を使ってもよい。

	const double D_PI = 3.14159265358979323846264338327;
	const float  F_PI = 3.14159265358979323846264338327f;

	const double D_2PI = 2.0 * D_PI;
	const float  F_2PI = 2.0f * F_PI;

	// glm::radians() と glm::degrees() に置き換える。
#if 0
	inline float ToRadians(float degrees)
	{ return degrees * F_PI / 180; }

	inline double ToRadians(double degrees)
	{ return degrees * D_PI / 180; }

	inline float ToDegrees(float radians)
	{ return radians * 180 / F_PI; }

	inline double ToDegrees(double radians)
	{ return radians * 180 / D_PI; }
#endif

	//! @brief  x が 2 のべき乗であるか否かを調べる。<br>
	template<typename T> bool IsModulo2(T x)
	{ return x != 0 && (x & (x-1)) == 0; }


	// 2D ベクトルの長さの平方。
	template<typename T> T GetVectorLengthSquared(const glm::detail::tvec2<T>& vec)
	{ return vec.x * vec.x + vec.y * vec.y; }

	// 2D ベクトルの長さ。
	template<typename T> double GetVectorLength(const glm::detail::tvec2<T>& vec)
	{ return sqrt(double(GetVectorLengthSquared(vec))); }
	// float 版だけは float で返すようオーバーロードを定義する。テンプレートの特殊化では対応不能。
	inline float GetVectorLength(const glm::detail::tvec2<float>& vec)
	{ return sqrt(GetVectorLengthSquared(vec)); }

	// 3D ベクトルの長さの平方。
	template<typename T> T GetVectorLengthSquared(const glm::detail::tvec3<T>& vec)
	{ return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z; }

	// 3D ベクトルの長さ。
	template<typename T> double GetVectorLength(const glm::detail::tvec3<T>& vec)
	{ return sqrt(double(GetVectorLengthSquared(vec))); }
	// float 版だけは float で返すようオーバーロードを定義する。テンプレートの特殊化では対応不能。
	inline float GetVectorLength(const glm::detail::tvec3<float>& vec)
	{ return sqrt(GetVectorLengthSquared(vec)); }


	template<typename T> glm::detail::tvec4<T> CreateColorFromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		return glm::detail::tvec4<T>(T(r) / 255, T(g) / 255, T(b) / 255, T(a) / 255);
	}

	template<typename T> glm::detail::tvec4<T> CreateColorFromARGB(uint32_t colorVal)
	{
		return CreateColorFromRGBA<T>(0xFF & (colorVal >> 16), 0xFF & (colorVal >> 8), 0xFF & colorVal, 0xFF & (colorVal >> 24));
	}


	inline double GetMyNormalRandD()
	{
		// [0, RAND_MAX] の整数を [-1, +1] にマッピングする。
		// C/C++ 標準ライブラリの乱数は一様ではなく、質が悪いがご容赦を。
		return 2.0 * (static_cast<double>(std::rand()) / RAND_MAX - 0.5);
	}

	inline float GetMyNormalRandF()
	{
		return static_cast<float>(GetMyNormalRandD());
	}

	// RGB - HSV (HSB) 相互変換。やや長いので、本来はインライン記述せずきちんと実装ファイルを用意したほうがよい。
	// RGB, HSV ともに正規化された値を使用する（正規数のほうが、GLSL や HLSL にもそのまま転用しやすい）。

	inline MyVector3F RgbToHsv(float r, float g, float b)
	{
		const float maxVal = std::max(r, std::max(g, b));
		const float minVal = std::min(r, std::min(g, b));
		const float delta = maxVal - minVal;
		const float v = maxVal;
		const float s = (maxVal != 0) ? (delta / maxVal) : 0;
		float h = 0;
		if (r == maxVal)
		{
			h = (g - b) / delta;
		}
		else if (g == maxVal)
		{
			h = 2 + (b - r) / delta;
		}
		else
		{
			h = 4 + (r - g) / delta;
		}
		h /= 6.0;
		if (h < 0)
		{
			h += 1.0;
		}
		return MyVector3F(h, s, v);
	}

	inline MyVector3F HsvToRgb(float h, float s, float v)
	{
		if (s == 0)
		{
			// Grayscale
			return MyVector3F(v, v, v);
		}
		else
		{
			if (1.0 <= h)
			{
				h -= 1.0;
			}
			h *= 6.0;
			const float i = std::floor(h);
			const float f = h - i;
			const float aa = v * (1 - s);
			const float bb = v * (1 - s * f);
			const float cc = v * (1 - s * (1 - f));
			if (i < 1)
			{
				return MyVector3F(v, cc, aa);
			}
			else if (i < 2)
			{
				return MyVector3F(bb, v, aa);
			}
			else if (i < 3)
			{
				return MyVector3F(aa, v, cc);
			}
			else if (i < 4)
			{
				return MyVector3F(aa, bb, v);
			}
			else if (i < 5)
			{
				return MyVector3F(cc, aa, v);
			}
			else
			{
				return MyVector3F(v, aa, bb);
			}
		}
	}
}


// RGBA
// 色名称は .NET Framework をベースにしている。
const MyVector4F MyColorFWhite(1, 1, 1, 1);
const MyVector4F MyColorFBlack(0, 0, 0, 0);
const MyVector4F MyColorFCyan(0, 1, 1, 1);
const MyVector4F MyColorFMagenta(1, 0, 1, 1);
const MyVector4F MyColorFYellow(1, 1, 0, 1);
const MyVector4F MyColorFRed(1, 0, 0, 1);
const MyVector4F MyColorFLime(0, 1, 0, 1);
const MyVector4F MyColorFBlue(0, 0, 1, 1);
const MyVector4F MyColorFTeal(0, 0.5f, 0.5f, 1);
const MyVector4F MyColorFCornflowerBlue = MyMath::CreateColorFromARGB<float>(0xFF6495ED);
const MyVector4F MyColorFDodgerBlue = MyMath::CreateColorFromARGB<float>(0xFF1E90FF);

#if 0
// C++17 では std::size() を使うべき。
#ifndef ARRAYSIZE
// Windows SDK の WinNT.h に定義されてある ARRAYSIZE() マクロとほぼ同様だが、完全互換ではない。
template <typename T, size_t N> char (*RtlpNumberOf(T (&)[N] ))[N];
#define ARRAYSIZE(A) (sizeof(*RtlpNumberOf(A)))
#endif
#endif
