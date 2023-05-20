#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/epsilon.hpp>

#include <reflection/Class.h>

#define SMALL_NUMBER		(1.e-8f)
#define KINDA_SMALL_NUMBER	(1.e-4f)
#define BIG_NUMBER			(3.4e+38f)

using uvec2 = glm::uvec2;
using uvec3 = glm::uvec3;
using uvec4 = glm::uvec4;
using bvec2 = glm::bvec2;
using bvec3 = glm::bvec3;
using bvec4 = glm::bvec4;
using ivec2 = glm::ivec2;
using ivec3 = glm::ivec3;
using ivec4 = glm::ivec4;
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using dvec2 = glm::dvec2;
using dvec3 = glm::dvec3;
using dvec4 = glm::dvec4;
using mat2 = glm::mat2;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using dmat2 = glm::dmat2;
using dmat3 = glm::dmat3;
using dmat4 = glm::dmat4;

using quat = glm::quat;
using dquat = glm::dquat;

#define SERIALIZE_HELPER_VEC(Vec) \
    template<> \
    class TStaticSerializer<Vec> \
    { \
    public: \
        static void Serialize(Vec& Object, FArchive& Ar) \
        { \
            for (int i = 0; i < Object.length(); i++) \
                Ar.Node.push_back(Object[i]); \
        } \
        static void Deserialize(Vec& Object, FArchive& Ar) \
        { \
            if (Ar.Node.is_array()) \
            { \
                for (int i = 0; i < Ar.Node.size(); i++) \
                    Object[i] = Ar.Node[i].get<Vec::value_type>(); \
            } \
        } \
    };

#define SERIALIZE_HELPER_MAT(Mat) \
    template<> \
    class TStaticSerializer<Mat> \
    { \
    public: \
        static void Serialize(Mat& Object, FArchive& Ar) \
        { \
            int i = 0; \
            int len = Mat::length(); \
            for (; i < len*len; i++) \
            { \
                Ar.Node.push_back(Object[i/len][i%len]); \
            } \
        } \
        static void Deserialize(Mat& Object, FArchive& Ar) \
        { \
            if (Ar.Node.is_array()) \
            { \
                int i = 0; \
                int len = Mat::length(); \
                for (auto &element : Ar.Node) \
                { \
                    if (element.is_number()) \
                        Object[i/len][i%len] = element.get<Mat::value_type>(); \
                    i++; \
                } \
            } \
        } \
    }; \

SERIALIZE_HELPER_VEC(uvec2)
SERIALIZE_HELPER_VEC(uvec3)
SERIALIZE_HELPER_VEC(uvec4)
SERIALIZE_HELPER_VEC(bvec2)
SERIALIZE_HELPER_VEC(bvec3)
SERIALIZE_HELPER_VEC(bvec4)
SERIALIZE_HELPER_VEC(ivec2)
SERIALIZE_HELPER_VEC(ivec3)
SERIALIZE_HELPER_VEC(ivec4)
SERIALIZE_HELPER_VEC(vec2)
SERIALIZE_HELPER_VEC(vec3)
SERIALIZE_HELPER_VEC(vec4)
SERIALIZE_HELPER_VEC(dvec2)
SERIALIZE_HELPER_VEC(dvec3)
SERIALIZE_HELPER_VEC(dvec4)
SERIALIZE_HELPER_MAT(mat2)
SERIALIZE_HELPER_MAT(mat3)
SERIALIZE_HELPER_MAT(mat4)
SERIALIZE_HELPER_MAT(dmat2)
SERIALIZE_HELPER_MAT(dmat3)
SERIALIZE_HELPER_MAT(dmat4)

SERIALIZE_HELPER_VEC(quat)
SERIALIZE_HELPER_VEC(dquat)

namespace nilou {

    const double PI = glm::pi<double>();
    const double TWO_PI = glm::two_pi<double>();
    const double HALF_PI = glm::half_pi<double>();
    const double INV_PI = glm::one_over_pi<double>();

    class Math
    {
    public:
        /**
        * @brief Converts a relative to an absolute epsilon, for the epsilon-equality
        * check between two values.
        *
        * @tparam T The value type.
        * @tparam P The GLM precision type.
        *
        * @param a The first value.
        * @param b The the second value.
        * @param relativeEpsilon The relative epsilon.
        * @return The absolute epsilon.
        */
	    template <typename T, glm::precision P, template <typename, glm::precision> class vecType>
        static vecType<T, P> relativeEpsilonToAbsolute(
            const vecType<T, P>& a,
            const vecType<T, P>& b,
            double relativeEpsilon) noexcept 
        {
            return relativeEpsilon * glm::max(glm::abs(a), glm::abs(b));
        }

        /**
        * @brief Converts a relative to an absolute epsilon, for the epsilon-equality
        * check between two values.
        *
        * @param a The first value.
        * @param b The the second value.
        * @param relativeEpsilon The relative epsilon.
        * @return The absolute epsilon.
        */
        static double relativeEpsilonToAbsolute(
            double a,
            double b,
            double relativeEpsilon) noexcept 
        {
            return relativeEpsilon * glm::max(glm::abs(a), glm::abs(b));
        }
        /**
        * @brief Checks whether two values are equal up to a given relative epsilon.
        *
        * @tparam T The value type.
        * @tparam P The GLM precision type.
        *
        * @param left The first value.
        * @param right The the second value.
        * @param relativeEpsilon The relative epsilon.
        * @return Whether the values are epsilon-equal
        */
	    template <typename T, glm::precision P, template <typename, glm::precision> class vecType>
        static bool equalsEpsilon(
            const vecType<T, P>& left,
            const vecType<T, P>& right,
            double relativeEpsilon) noexcept 
        {
            return Math::equalsEpsilon(left, right, relativeEpsilon, relativeEpsilon);
        }

        /**
        * @brief Checks whether two values are equal up to a given relative epsilon.
        *
        * @param left The first value.
        * @param right The the second value.
        * @param relativeEpsilon The relative epsilon.
        * @return Whether the values are epsilon-equal
        */
        static bool
        equalsEpsilon(double left, double right, double relativeEpsilon) noexcept 
        {
            return equalsEpsilon(left, right, relativeEpsilon, relativeEpsilon);
        }

        /**
        * @brief Determines if two values are equal using an absolute or relative
        * tolerance test.
        *
        * This is useful to avoid problems due to roundoff error when comparing
        * floating-point values directly. The values are first compared using an
        * absolute tolerance test. If that fails, a relative tolerance test is
        * performed. Use this test if you are unsure of the magnitudes of left and
        * right.
        *
        * @param left The first value to compare.
        * @param right The other value to compare.
        * @param relativeEpsilon The maximum inclusive delta between `left` and
        * `right` for the relative tolerance test.
        * @param absoluteEpsilon The maximum inclusive delta between `left` and
        * `right` for the absolute tolerance test.
        * @returns `true` if the values are equal within the epsilon; otherwise,
        * `false`.
        *
        * @snippet TestMath.cpp equalsEpsilon
        */
        static bool equalsEpsilon(
            double left,
            double right,
            double relativeEpsilon,
            double absoluteEpsilon) noexcept 
        {
            const double diff = glm::abs(left - right);
            return diff <= absoluteEpsilon ||
                diff <= relativeEpsilonToAbsolute(left, right, relativeEpsilon);
        }

        /**
        * @brief Determines if two values are equal using an absolute or relative
        * tolerance test.
        *
        * This is useful to avoid problems due to roundoff error when comparing
        * floating-point values directly. The values are first compared using an
        * absolute tolerance test. If that fails, a relative tolerance test is
        * performed. Use this test if you are unsure of the magnitudes of left and
        * right.
        *
        * @tparam T The value type.
        * @tparam P The GLM precision type.
        *
        * @param left The first value to compare.
        * @param right The other value to compare.
        * @param relativeEpsilon The maximum inclusive delta between `left` and
        * `right` for the relative tolerance test.
        * @param absoluteEpsilon The maximum inclusive delta between `left` and
        * `right` for the absolute tolerance test.
        * @returns `true` if the values are equal within the epsilon; otherwise,
        * `false`.
        */
	    template <typename T, glm::precision P, template <typename, glm::precision> class vecType>
        static bool equalsEpsilon(
            const vecType<T, P>& left,
            const vecType<T, P>& right,
            double relativeEpsilon,
            double absoluteEpsilon) noexcept 
        {
            const vecType<T, P> diff = glm::abs(left - right);
            return glm::lessThanEqual(diff, vecType<T, P>(absoluteEpsilon)) ==
                    vecType<bool, P>(true) ||
                glm::lessThanEqual(
                    diff,
                    relativeEpsilonToAbsolute(left, right, relativeEpsilon)) ==
                    vecType<bool, P>(true);
        }
    };
}