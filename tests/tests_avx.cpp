#include <gtest/gtest.h>
#include <immintrin.h>
#include <array>

// todo: not fastest way
float AvxExtractFloat(__m256 v, int i) {
   float r[8];
   _mm256_storeu_ps(r, v);
   return r[i];
}

TEST(Avx, Basic) {
   __m256 a = _mm256_set1_ps(1.f);
   __m256 b = _mm256_set1_ps(2.f);
   __m256 add = _mm256_add_ps(a, b);

   float r[8];
   _mm256_storeu_ps(r, add);
   ASSERT_EQ(r[0], 3.f);
}

TEST(Avx, Mask) {
   __m256 a = _mm256_loadu_ps(std::array<float, 8>{1, 2, 3}.data());
   __m256 b = _mm256_loadu_ps(std::array<float, 8>{3, 2, 1}.data());
   __m256 mask = _mm256_cmp_ps(a, b, _CMP_EQ_OQ);

   __m256 res = _mm256_blendv_ps(a, _mm256_add_ps(a, b), mask);
   ASSERT_EQ(AvxExtractFloat(res, 0), 1.f);
   ASSERT_EQ(AvxExtractFloat(res, 1), 4.f);
   ASSERT_EQ(AvxExtractFloat(res, 2), 3.f);
}

TEST(Avx, Permute) {
   __m256 a = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);

   __m256 res = _mm256_permute_ps(a, _MM_SHUFFLE(3, 2, 1, 0));
   ASSERT_EQ(AvxExtractFloat(res, 0), 0.f);
   ASSERT_EQ(AvxExtractFloat(res, 1), 1.f);
   ASSERT_EQ(AvxExtractFloat(res, 2), 2.f);
   ASSERT_EQ(AvxExtractFloat(res, 3), 3.f);

   res = _mm256_permute_ps(a, _MM_SHUFFLE(1, 3, 0, 2));
   ASSERT_EQ(AvxExtractFloat(res, 0), 2.f);
   ASSERT_EQ(AvxExtractFloat(res, 1), 0.f);
   ASSERT_EQ(AvxExtractFloat(res, 2), 3.f);
   ASSERT_EQ(AvxExtractFloat(res, 3), 1.f);
}
