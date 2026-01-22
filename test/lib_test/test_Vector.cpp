

#include "../TestMainHelper.h"
#include "core/maths/vectors.h"

TEST(mathVector, constructorDefault) {
	evl::math::Vector<float, 3> v;
	EXPECT_FLOAT_EQ(v[0], 0.0f);
	EXPECT_FLOAT_EQ(v[1], 0.0f);
	EXPECT_FLOAT_EQ(v[2], 0.0f);
}

TEST(mathVector, constructorInitList) {
	constexpr evl::math::vec2f v2{1.0f, 2.0f};
	EXPECT_FLOAT_EQ(v2.x(), 1.0f);
	EXPECT_FLOAT_EQ(v2.y(), 2.0f);
	constexpr evl::math::vec3f v3{1.0f, 2.0f, 3.0f};
	EXPECT_FLOAT_EQ(v3.x(), 1.0f);
	EXPECT_FLOAT_EQ(v3.y(), 2.0f);
	EXPECT_FLOAT_EQ(v3.z(), 3.0f);
	constexpr evl::math::vec4f v4{1.0f, 2.0f, 3.0f, 4.0f};
	EXPECT_FLOAT_EQ(v4.r(), 1.0f);
	EXPECT_FLOAT_EQ(v4.g(), 2.0f);
	EXPECT_FLOAT_EQ(v4.b(), 3.0f);
	EXPECT_FLOAT_EQ(v4.a(), 4.0f);
	// components operations
	evl::math::vec4f v4_2{1.0f, 2.0f, 3.0f, 4.0f};
	v4_2.x() = 4.0f;
	v4_2.y() = 3.0f;
	v4_2.z() = 2.0f;
	v4_2.w() = 1.0f;
	EXPECT_FLOAT_EQ(v4_2.r(), 4.0f);
	EXPECT_FLOAT_EQ(v4_2.g(), 3.0f);
	EXPECT_FLOAT_EQ(v4_2.b(), 2.0f);
	EXPECT_FLOAT_EQ(v4_2.a(), 1.0f);
	v4_2.r() = 1.0f;
	v4_2.g() = 2.0f;
	v4_2.b() = 3.0f;
	v4_2.a() = 4.0f;
	EXPECT_FLOAT_EQ(v4_2.x(), 1.0f);
	EXPECT_FLOAT_EQ(v4_2.y(), 2.0f);
	EXPECT_FLOAT_EQ(v4_2.z(), 3.0f);
	EXPECT_FLOAT_EQ(v4_2.w(), 4.0f);
}

TEST(mathVector, affectation) {
	constexpr evl::math::vec3f v1{1.0f, 2.0f, 3.0f};
	evl::math::vec3f v2{v1};
	EXPECT_FLOAT_EQ(v2.x(), 1.0f);
	EXPECT_FLOAT_EQ(v2.y(), 2.0f);
	EXPECT_FLOAT_EQ(v2.z(), 3.0f);
	constexpr evl::math::vec4f v4{4.0f, 5.0f, 6.0f, 7.0f};
	evl::math::vec3f v3{v4};
	EXPECT_FLOAT_EQ(v3.x(), 4.0f);
	EXPECT_FLOAT_EQ(v3.y(), 5.0f);
	EXPECT_FLOAT_EQ(v3.z(), 6.0f);
	v2 = v4;
	EXPECT_FLOAT_EQ(v2.x(), 4.0f);
	EXPECT_FLOAT_EQ(v2.y(), 5.0f);
	EXPECT_FLOAT_EQ(v2.z(), 6.0f);
}

TEST(mathVector, equality) {
	constexpr evl::math::vec3f v1{1.0f, 2.0f, 3.0f};
	constexpr evl::math::vec3f v2{1.0f, 2.0f, 3.0f};
	constexpr evl::math::vec3f v3{4.0f, 5.0f, 6.0f};
	constexpr evl::math::vec3f v4{1.0f, 5.0f, 6.0f};
	EXPECT_TRUE(v1 == v2);
	EXPECT_FALSE(v1 != v2);
	EXPECT_FALSE(v1 == v3);
	EXPECT_TRUE(v1 != v3);
	EXPECT_FALSE(v1 == v4);
	EXPECT_TRUE(v1 != v4);
}

TEST(mathVector, addition_subtraction) {
	evl::math::vec3f v1{1.0f, 2.0f, 3.0f};
	constexpr evl::math::vec3f v2{4.0f, 5.0f, 6.0f};
	evl::math::vec3f v3 = v1 + v2;
	EXPECT_FLOAT_EQ(v3.x(), 5.0f);
	EXPECT_FLOAT_EQ(v3.y(), 7.0f);
	EXPECT_FLOAT_EQ(v3.z(), 9.0f);
	v1 += v2;
	EXPECT_FLOAT_EQ(v1.x(), 5.0f);
	EXPECT_FLOAT_EQ(v1.y(), 7.0f);
	EXPECT_FLOAT_EQ(v1.z(), 9.0f);
	v1 = evl::math::vec3f{1.0f, 2.0f, 3.0f};
	v3 = v1 - v2;
	EXPECT_FLOAT_EQ(v3.x(), -3.0f);
	EXPECT_FLOAT_EQ(v3.y(), -3.0f);
	EXPECT_FLOAT_EQ(v3.z(), -3.0f);
	v1 -= v2;
	EXPECT_FLOAT_EQ(v1.x(), -3.0f);
	EXPECT_FLOAT_EQ(v1.y(), -3.0f);
	EXPECT_FLOAT_EQ(v1.z(), -3.0f);
	evl::math::vec2f v5 = -evl::math::vec2f{2.0f, 3.0f};
	EXPECT_FLOAT_EQ(v5.x(), -2.0f);
	EXPECT_FLOAT_EQ(v5.y(), -3.0f);
}

TEST(mathVector, dot_cross) {
	constexpr evl::math::vec3f v1{1.0f, 2.0f, 3.0f};
	constexpr evl::math::vec3f v2{4.0f, 5.0f, 6.0f};
	constexpr float dot = v1 * v2;
	EXPECT_FLOAT_EQ(dot, 32.0f);
	evl::math::vec3f v3 = v1;
	v3 ^= v2;
	EXPECT_FLOAT_EQ(v3.x(), -3.0f);
	EXPECT_FLOAT_EQ(v3.y(), 6.0f);
	EXPECT_FLOAT_EQ(v3.z(), -3.0f);
	constexpr evl::math::vec3f v4 = v1 ^ v2;
	EXPECT_FLOAT_EQ(v4.x(), -3.0f);
	EXPECT_FLOAT_EQ(v4.y(), 6.0f);
	EXPECT_FLOAT_EQ(v4.z(), -3.0f);
}

TEST(mathVector, scalar_multiplication_division) {
	evl::math::vec3f v1{1.0f, 2.0f, 3.0f};
	constexpr float scalar = 2.0f;
	evl::math::vec3f v3 = v1 * scalar;
	EXPECT_FLOAT_EQ(v3.x(), 2.0f);
	EXPECT_FLOAT_EQ(v3.y(), 4.0f);
	EXPECT_FLOAT_EQ(v3.z(), 6.0f);
	v1 *= scalar;
	EXPECT_FLOAT_EQ(v1.x(), 2.0f);
	EXPECT_FLOAT_EQ(v1.y(), 4.0f);
	EXPECT_FLOAT_EQ(v1.z(), 6.0f);
	v1 = evl::math::vec3f{2.0f, 4.0f, 6.0f};
	v3 = v1 / scalar;
	EXPECT_FLOAT_EQ(v3.x(), 1.0f);
	EXPECT_FLOAT_EQ(v3.y(), 2.0f);
	EXPECT_FLOAT_EQ(v3.z(), 3.0f);
	v3 = scalar * v1;
	EXPECT_FLOAT_EQ(v3.x(), 4.0f);
	EXPECT_FLOAT_EQ(v3.y(), 8.0f);
	EXPECT_FLOAT_EQ(v3.z(), 12.0f);
	v1 /= scalar;
	EXPECT_FLOAT_EQ(v1.x(), 1.0f);
	EXPECT_FLOAT_EQ(v1.y(), 2.0f);
	EXPECT_FLOAT_EQ(v1.z(), 3.0f);
}

TEST(mathVector, surface_ratio) {
	constexpr evl::math::vec2f v1{4.0f, 5.0f};
	constexpr float surface = v1.surface();
	EXPECT_FLOAT_EQ(surface, 20.0f);
	constexpr float ratio = v1.ratio();
	EXPECT_FLOAT_EQ(ratio, 0.8f);
	auto bob = v1.surface()* v1.ratio();
	bob += 4.0f;
	EXPECT_FLOAT_EQ(bob, 20.0f);
}

TEST(mathVector, iterator) {
	constexpr evl::math::vec3f v1{1.0f, 2.0f, 3.0f};
	float sum = 0.0f;
	for (const auto& comp : v1) {
		sum += comp;
	}
	EXPECT_FLOAT_EQ(sum, 6.0f);
	evl::math::vec3f v2{0.0f, 0.0f, 0.0f};
	for (auto& comp : v2) {
		comp = 1.0f;
	}
	EXPECT_FLOAT_EQ(v2.x(), 1.0f);
	EXPECT_FLOAT_EQ(v2.y(), 1.0f);
	EXPECT_FLOAT_EQ(v2.z(), 1.0f);
}

TEST(mathVector, data_access) {
	constexpr evl::math::vec3f v1{1.0f, 2.0f, 3.0f};
	const float* data = v1.data();
	EXPECT_FLOAT_EQ(data[0], 1.0f);
	EXPECT_FLOAT_EQ(data[1], 2.0f);
	EXPECT_FLOAT_EQ(data[2], 3.0f);
	evl::math::vec3f v2{0.0f, 0.0f, 0.0f};
	float* data2 = v2.data();
	data2[0] = 4.0f;
	data2[1] = 5.0f;
	data2[2] = 6.0f;
	EXPECT_FLOAT_EQ(v2.x(), 4.0f);
	EXPECT_FLOAT_EQ(v2.y(), 5.0f);
	EXPECT_FLOAT_EQ(v2.z(), 6.0f);
}

TEST(mathVector, norm) {
	{
		constexpr evl::math::vec3f v0{0.0f, 0.0f, 0.0f};
		constexpr float normSq0 = v0.normSq();
		EXPECT_FLOAT_EQ(normSq0, 0.0f);
		const float norm0 = v0.norm();
		EXPECT_FLOAT_EQ(norm0, 0.0f);
		evl::math::vec3f v00{0.0f, 0.0f, 0.0f};
		v00.normalize();
		EXPECT_FLOAT_EQ(v00.x(), 0.0f);
		EXPECT_FLOAT_EQ(v00.y(), 0.0f);
		EXPECT_FLOAT_EQ(v00.z(), 0.0f);
	}
	{
		constexpr evl::math::vec3f v1{3.0f, 4.0f, 0.0f};
		constexpr float normSq = v1.normSq();
		EXPECT_FLOAT_EQ(normSq, 25.0f);
		const float norm = v1.norm();
		EXPECT_FLOAT_EQ(norm, 5.0f);
		constexpr evl::math::vec3i v2{1, 2, -2};
		const int norm2 = v2.norm();
		EXPECT_EQ(norm2, 5);
		constexpr evl::math::vec3ui v3{1, 2, 2};
		constexpr uint32_t norm3 = v3.norm();
		EXPECT_EQ(norm3, 5);
		auto v4 = v1.normalized();
		EXPECT_FLOAT_EQ(v4.x(), 0.6f);
		EXPECT_FLOAT_EQ(v4.y(), 0.8f);
		EXPECT_FLOAT_EQ(v4.z(), 0.0f);
	}
	{
		evl::math::vec3f v1{3.0f, 4.0f, 0.0f};
		evl::math::vec3i v2{1, 2, -2};
		evl::math::vec3ui v3{1, 2, 2};
		v1.normalize();
		EXPECT_FLOAT_EQ(v1.x(), 0.6f);
		EXPECT_FLOAT_EQ(v1.y(), 0.8f);
		EXPECT_FLOAT_EQ(v1.z(), 0.0f);
		v2.normalize();
		EXPECT_EQ(v2.x(), 0);
		EXPECT_EQ(v2.y(), 1);
		EXPECT_EQ(v2.z(), 0);
		v3.normalize();
		EXPECT_EQ(v3.x(), 0);
		EXPECT_EQ(v3.y(), 1);
		EXPECT_EQ(v3.z(), 0);
	}
}

TEST(mathVector, crossProduct) {
	constexpr evl::math::vec3f v1{1.0f, 0.0f, 0.0f};
	constexpr evl::math::vec3f v2{0.0f, 1.0f, 0.0f};
	evl::math::vec3f v3 = v1 ^ v2;
	EXPECT_FLOAT_EQ(v3.x(), 0.0f);
	EXPECT_FLOAT_EQ(v3.y(), 0.0f);
	EXPECT_FLOAT_EQ(v3.z(), 1.0f);
}
