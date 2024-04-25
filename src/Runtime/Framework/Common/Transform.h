#pragma once

#include <ostream>
#include "Common/Maths.h"

namespace nilou {
    
	const vec3 WORLD_UP(0.0, 0.0, 1.0);
	const vec3 WORLD_FORWARD(1.0, 0.0, 0.0);
	const vec3 WORLD_RIGHT(0.0, 1.0, 0.0);

    template <typename T>
    inline bool Equals(const glm::tvec3<T> &A, const glm::tvec3<T> &B, T Tolerance=KINDA_SMALL_NUMBER)
    {
        return glm::abs(A.x-B.x) <= Tolerance && glm::abs(A.y-B.y) <= Tolerance && glm::abs(A.z-B.z) <= Tolerance;
    }

    enum ECoordAxis
    {
        CA_X,
        CA_Y,
        CA_Z
    };

    template <typename T>
    struct TRotator
    {
        /** Rotation around the right axis (around Y axis), Looking up and down (0=Straight Ahead, +Up, -Down) */
	    T Pitch;

	    /** Rotation around the up axis (around Z axis), Running in circles 0=East (positive X axis) , +North, -South. */
	    T Yaw;

	    /** Rotation around the forward axis (around X axis), Tilting your head, 0=Straight, +Clockwise, -CCW. */
	    T Roll;

        TRotator();
        TRotator(T pitch, T yaw, T roll);
        explicit TRotator(const glm::tvec3<T> &eulerAngles);
        explicit TRotator(const glm::tquat<T> &rotation);

        static T NormalizeAxis(T Angle);
        static T ClampAxis(T Angle);

        quat ToQuat() const;
        bool Equals(const TRotator &B, T Tolerance=KINDA_SMALL_NUMBER) const
        {
            return glm::abs(Pitch-B.Pitch) <= Tolerance && glm::abs(Yaw-B.Yaw) <= Tolerance && glm::abs(Roll-B.Roll) <= Tolerance;
        }

        static const TRotator ZeroRotator;
    };

    template <typename T>
    glm::tvec3<T> RotateVector(const glm::tquat<T> &rotation, const glm::tvec3<T> &V);

    using FRotator = TRotator<double>;
    using FRotator3f = TRotator<float>;

    // 变换
    // 对于位置向量，变换是Scale -> Rotate -> Translate
    // 对于方向向量，变换是Scale -> Rotate
    // 当transform连乘时，比如C = B * A，C代表先应用A，再应用B后的结果，和opengl的matrix相同
    template <typename T>
    class TTransform
    {
    protected:
        glm::tquat<T> Rotation;
        glm::tvec3<T> Translation;
        glm::tvec3<T> Scale3D;

    public:

        template <typename U>
        friend std::ostream &operator<<(std::ostream &out, const TTransform<U> &obj);
        friend class TClassRegistry<TTransform>;

        TTransform();
        TTransform(const glm::tquat<T> &rotation);
        TTransform(const glm::tvec3<T> &translation);
        TTransform(const glm::tvec3<T> &scale3d, const glm::tquat<T> &rotation, const glm::tvec3<T> &translation);
        TTransform(const glm::tmat4x4<T> &matrix);

        void SetFromMatrix(const glm::tmat4x4<T> &matrix);
        glm::tvec3<T> GetUnitAxis(ECoordAxis axis) const;
        glm::tvec3<T> GetScale3D() const;
        glm::tquat<T> GetRotation() const;
        TRotator<T> GetRotator() const;
        glm::tvec3<T> GetTranslation() const;
        glm::tvec3<T> GetLocation() const;

        glm::tvec3<T> TransformPosition(const glm::tvec3<T> &v) const;
        glm::tvec3<T> TransformPositionNoScale(const glm::tvec3<T> &v) const;
        glm::tvec3<T> TransformVector(const glm::tvec3<T> &v) const;
        glm::tvec3<T> TransformVectorNoScale(const glm::tvec3<T> &v) const;
        glm::tvec3<T> InverseTransformPosition(const glm::tvec3<T> &v) const;
        glm::tvec3<T> InverseTransformPositionNoScale(const glm::tvec3<T> &v) const;

        void SetScale3D(const glm::tvec3<T> &scale);
        void SetRotation(const glm::tquat<T> &rotation);
        void SetRotator(const TRotator<T> &rotator);
        void SetTranslation(const glm::tvec3<T> &translation);
        TTransform operator*(const TTransform &Other) const;
        TTransform GetRelativeTransform(const TTransform &Other) const;
        T GetMinimumAxisScale() const;
        glm::tvec3<T> GetSafeScaleReciprocal(const glm::tvec3<T> &InScale, T Tolerance = SMALL_NUMBER);

        static bool AnyHasNegativeScale(const glm::tvec3<T> &InScale3D, const glm::tvec3<T> &InOtherScale3D);

        glm::tmat4x4<T> ToMatrix() const;

        static const TTransform<T> Identity;
    };

    using FTransform = TTransform<double>;
    using FTransform3f = TTransform<float>;
}

template<typename T>
class TStaticSerializer<nilou::TRotator<T>>
{
public:
    static void Serialize(nilou::TRotator<T>& Object, FArchive& Ar)
    {
        {
            FArchive local_Ar(Ar.Node["Pitch"], Ar);
            TStaticSerializer<decltype(Object.Pitch)>::Serialize(Object.Pitch, local_Ar);
        }
        {
            FArchive local_Ar(Ar.Node["Yaw"], Ar);
            TStaticSerializer<decltype(Object.Yaw)>::Serialize(Object.Yaw, local_Ar);
        }
        {
            FArchive local_Ar(Ar.Node["Roll"], Ar);
            TStaticSerializer<decltype(Object.Roll)>::Serialize(Object.Roll, local_Ar);
        }
    }
    static void Deserialize(nilou::TRotator<T>& Object, FArchive& Ar)
    {
        {
            FArchive local_Ar(Ar.Node["Pitch"], Ar);
            TStaticSerializer<decltype(Object.Pitch)>::Deserialize(Object.Pitch, local_Ar);
        }
        {
            FArchive local_Ar(Ar.Node["Yaw"], Ar);
            TStaticSerializer<decltype(Object.Yaw)>::Deserialize(Object.Yaw, local_Ar);
        }
        {
            FArchive local_Ar(Ar.Node["Roll"], Ar);
            TStaticSerializer<decltype(Object.Roll)>::Deserialize(Object.Roll, local_Ar);
        }
    }
};

template<typename T>
class TStaticSerializer<nilou::TTransform<T>>
{
public:
    static void Serialize(nilou::TTransform<T>& Object, FArchive& Ar)
    {
        auto Rotation = Object.GetRotation();
        auto Translation = Object.GetTranslation();
        auto Scale = Object.GetScale3D();
        {
            FArchive local_Ar(Ar.Node["Rotation"], Ar);
            TStaticSerializer<decltype(Rotation)>::Serialize(Rotation, local_Ar);
        }
        {
            FArchive local_Ar(Ar.Node["Translation"], Ar);
            TStaticSerializer<decltype(Translation)>::Serialize(Translation, local_Ar);
        }
        {
            FArchive local_Ar(Ar.Node["Scale"], Ar);
            TStaticSerializer<decltype(Scale)>::Serialize(Scale, local_Ar);
        }
    }
    static void Deserialize(nilou::TTransform<T>& Object, FArchive& Ar)
    {
        auto Rotation = Object.GetRotation();
        auto Translation = Object.GetTranslation();
        auto Scale = Object.GetScale3D();
        {
            FArchive local_Ar(Ar.Node["Rotation"], Ar);
            TStaticSerializer<decltype(Rotation)>::Deserialize(Rotation, local_Ar);
        }
        {
            FArchive local_Ar(Ar.Node["Translation"], Ar);
            TStaticSerializer<decltype(Translation)>::Deserialize(Translation, local_Ar);
        }
        {
            FArchive local_Ar(Ar.Node["Scale"], Ar);
            TStaticSerializer<decltype(Scale)>::Deserialize(Scale, local_Ar);
        }
        Object.SetRotation(Rotation);
        Object.SetTranslation(Translation);
        Object.SetScale3D(Scale);
    }
};

namespace nilou {

    template<typename T>
    TRotator<T>::TRotator() : Pitch(0), Yaw(0), Roll(0) {}
    template<typename T>
    TRotator<T>::TRotator(const glm::tvec3<T> &eulerAngles)
        : Pitch(eulerAngles.x), Yaw(eulerAngles.y), Roll(eulerAngles.z) {}
        
    template<typename T>
    TRotator<T>::TRotator(T pitch, T yaw, T roll) : Pitch(pitch), Yaw(yaw), Roll(roll) {}

    template<typename T>
    TRotator<T>::TRotator(const glm::tquat<T> &rotation)
    {
        T X = rotation.x;
        T Y = rotation.y;
        T Z = rotation.z;
        T W = rotation.w;


		const T SingularityTest = Z * X - W * Y;
		const T YawY = 2.0 * (W * Z + X * Y);
		const T YawX = (1.0 - 2.0 * (Y*Y + Z*Z));

		// reference 
		// http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
		// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/

		// this value was found from experience, the above websites recommend different values
		// but that isn't the case for us, so I went through different testing, and finally found the case 
		// where both of world lives happily. 
		const T SINGULARITY_THRESHOLD = 0.4999995;
		const T RAD_TO_DEG = (180.0 / PI);
		// float Pitch, Yaw, Roll;

		if (SingularityTest < -SINGULARITY_THRESHOLD)
		{
			Pitch = -90.0;
			Yaw = (std::atan2(YawY, YawX) * RAD_TO_DEG);
			Roll = TRotator<T>::NormalizeAxis(-Yaw - (2.0 * std::atan2(X, W) * RAD_TO_DEG));
		}
		else if (SingularityTest > SINGULARITY_THRESHOLD)
		{
			Pitch = 90.0;
			Yaw = (std::atan2(YawY, YawX) * RAD_TO_DEG);
			Roll = TRotator<T>::NormalizeAxis(Yaw - (2.0 * std::atan2(X, W) * RAD_TO_DEG));
		}
		else
		{
			Pitch = (std::asin(2.0 * SingularityTest) * RAD_TO_DEG);
			Yaw = (std::atan2(YawY, YawX) * RAD_TO_DEG);
			Roll = (std::atan2(-2.0 * (W*X + Y*Z), (1.0 - 2.0 * (X*X + Y*Y))) * RAD_TO_DEG);
		}
    }

    template<typename T>
    T TRotator<T>::NormalizeAxis(T Angle)
    {
        Angle = ClampAxis(Angle);

        if (Angle > 180.0)
        {
            // shift to (-180,180]
            Angle -= 360.0;
        }

        return Angle;
    }

    template<typename T>
    T TRotator<T>::ClampAxis(T Angle)
    {
        Angle = glm::mod(Angle, 360.0);

        if (Angle < 0.0)
        {
            // shift to [0,360) range
            Angle += 360.0;
        }

        return Angle;
    }

    template<typename T>
    void SinCos(T* ScalarSin, T* ScalarCos, T Value)
    {
        // Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
		T quotient = (INV_PI*0.5)*Value;
		if (Value >= 0.0)
		{
			quotient = (T)((glm::int64)(quotient + 0.5));
		}
		else
		{
			quotient = (T)((glm::int64)(quotient - 0.5));
		}
		T y = Value - TWO_PI * quotient;

		// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
		T sign;
		if (y > HALF_PI)
		{
			y = PI - y;
			sign = -1.0;
		}
		else if (y < -HALF_PI)
		{
			y = -PI - y;
			sign = -1.0;
		}
		else
		{
			sign = +1.0;
		}

		T y2 = y * y;

		// 11-degree minimax approximation
		*ScalarSin = ( ( ( ( (-2.3889859e-08 * y2 + 2.7525562e-06) * y2 - 0.00019840874 ) * y2 + 0.0083333310 ) * y2 - 0.16666667 ) * y2 + 1.0 ) * y;

		// 10-degree minimax approximation
		T p = ( ( ( ( -2.6051615e-07 * y2 + 2.4760495e-05 ) * y2 - 0.0013888378 ) * y2 + 0.041666638 ) * y2 - 0.5 ) * y2 + 1.0;
		*ScalarCos = sign*p;
    }

    template<typename T>
    quat TRotator<T>::ToQuat() const
    {
    	const T DEG_TO_RAD = PI/(180.0);
        const T RADS_DIVIDED_BY_2 = DEG_TO_RAD/2.0;
        T SP, SY, SR;
        T CP, CY, CR;

        const T PitchNoWinding = glm::mod(Pitch, 360.0);
        const T YawNoWinding = glm::mod(Yaw, 360.0);
        const T RollNoWinding = glm::mod(Roll, 360.0);

        SinCos(&SP, &CP, PitchNoWinding * RADS_DIVIDED_BY_2);
        SinCos(&SY, &CY, YawNoWinding * RADS_DIVIDED_BY_2);
        SinCos(&SR, &CR, RollNoWinding * RADS_DIVIDED_BY_2);

        glm::quat RotationQuat;
        RotationQuat.x =  CR*SP*SY - SR*CP*CY;
        RotationQuat.y = -CR*SP*CY - SR*CP*SY;
        RotationQuat.z =  CR*CP*SY - SR*SP*CY;
        RotationQuat.w =  CR*CP*CY + SR*SP*SY;
        return RotationQuat;
        // return quat(vec3(Pitch, Yaw, Roll));
    }

    template <typename T>
    glm::tvec3<T> RotateVector(const glm::tquat<T> &rotation, const glm::tvec3<T> &V)
    {
        // http://people.csail.mit.edu/bkph/articles/Quaternions.pdf
        // V' = V + 2w(Q x V) + (2Q x (Q x V))
        // refactor:
        // V' = V + w(2(Q x V)) + (Q x (2(Q x V)))
        // T = 2(Q x V);
        // V' = V + w*(T) + (Q x T)

        T X = rotation.x;
        T Y = rotation.y;
        T Z = rotation.z;
        T W = rotation.w;

        const glm::tvec3<T> Q(X, Y, Z);
        const glm::tvec3<T> TT = 2.0 * glm::cross(Q, V);
        const glm::tvec3<T> Result = V + (W * TT) + glm::cross(Q, TT);
        return Result;
        // return rotation * V;
    }

    template <typename T>
    TTransform<T>::TTransform()
        : Rotation(glm::tquat<T>(1.0, 0.0, 0.0, 0.0))
        , Translation(glm::tvec3<T>(0.0, 0.0, 0.0))
        , Scale3D(glm::tvec3<T>(1.0, 1.0, 1.0))
    {

    }

    template <typename T>
    TTransform<T>::TTransform(const glm::tquat<T> &rotation)
        : Rotation(rotation)
        , Translation(glm::tvec3<T>(0.0, 0.0, 0.0))
        , Scale3D(glm::tvec3<T>(1.0, 1.0, 1.0))
    {

    }

    template <typename T>
    TTransform<T>::TTransform(const glm::tvec3<T> &translation)
        : Rotation(glm::tquat<T>(1.0, 0.0, 0.0, 0.0))
        , Translation(translation)
        , Scale3D(glm::tvec3<T>(1.0, 1.0, 1.0))
    {

    }
    
    template <typename T>
    TTransform<T>::TTransform(const glm::tvec3<T> &scale3d, const glm::tquat<T> &rotation, const glm::tvec3<T> &translation)
        : Rotation(rotation)
        , Translation(translation)
        , Scale3D(scale3d)
    {
    }
    
    template <typename T>
    TTransform<T>::TTransform(const glm::tmat4x4<T> &matrix)
    {
        SetFromMatrix(matrix);
    }

    template <typename T>
    static glm::tquat<T> UEMatrixToQuat(glm::tmat3x3<T> M)
    {
        glm::tquat<T> out;
        T s;

        // Check diagonal (trace)
        const T tr = M[0][0] + M[1][1] + M[2][2];

        if (tr > 0.0f) 
        {
            T InvS = 1 / glm::sqrt(tr + T(1.f));
            out.w = T(T(0.5f) * (T(1.f) / InvS));
            s = T(0.5f) * InvS;

            out.x = ((M[1][2] - M[2][1]) * s);
            out.y = ((M[2][0] - M[0][2]) * s);
            out.z = ((M[0][1] - M[1][0]) * s);
        } 
        else 
        {
            // diagonal is negative
            int i = 0;

            if (M[1][1] > M[0][0])
                i = 1;

            if (M[2][2] > M[i][i])
                i = 2;

            static constexpr int nxt[3] = { 1, 2, 0 };
            const int j = nxt[i];
            const int k = nxt[j];
    
            s = M[i][i] - M[j][j] - M[k][k] + T(1.0f);

            T InvS = 1 / glm::sqrt(s);

            T qt[4];
            qt[i] = T(0.5f) * (T(1.f) / InvS);

            s = T(0.5f) * InvS;

            qt[3] = (M[j][k] - M[k][j]) * s;
            qt[j] = (M[i][j] + M[j][i]) * s;
            qt[k] = (M[i][k] + M[k][i]) * s;

            out.x = qt[0];
            out.y = qt[1];
            out.z = qt[2];
            out.w = qt[3];

        }
        return out;
    }
    
    template <typename T>
    void TTransform<T>::SetFromMatrix(const glm::tmat4x4<T> &matrix)
    {
        glm::tvec3<T> translation{ matrix[3][0], matrix[3][1], matrix[3][2] };
        glm::tvec3<T> scale3d;
        scale3d.x = sqrt(matrix[0][0]*matrix[0][0] + matrix[0][1]*matrix[0][1] + matrix[0][2]*matrix[0][2]);
        scale3d.y = sqrt(matrix[1][0]*matrix[1][0] + matrix[1][1]*matrix[1][1] + matrix[1][2]*matrix[1][2]);
        scale3d.z = sqrt(matrix[2][0]*matrix[2][0] + matrix[2][1]*matrix[2][1] + matrix[2][2]*matrix[2][2]);

        mat3 rotation_mat{
            matrix[0][0] / scale3d.x, matrix[0][1] / scale3d.x, matrix[0][2] / scale3d.x,
            matrix[1][0] / scale3d.y, matrix[1][1] / scale3d.y, matrix[1][2] / scale3d.y,
            matrix[2][0] / scale3d.z, matrix[2][1] / scale3d.z, matrix[2][2] / scale3d.z };
        if (glm::determinant(rotation_mat) < 0.f)
        {
            scale3d[0] *= -1;
            rotation_mat[0] = -rotation_mat[0];
        }
        glm::tquat<T> rotation = UEMatrixToQuat(rotation_mat);

        Rotation = rotation;
        Translation = translation;
        Scale3D = scale3d;
    }
    
    template <typename T>
    glm::tvec3<T> TTransform<T>::TransformPosition(const glm::tvec3<T> &v) const
    {
        return RotateVector(Rotation, (Scale3D * v)) + Translation;
    }
    
    template <typename T>
    glm::tvec3<T> TTransform<T>::TransformPositionNoScale(const glm::tvec3<T> &v) const
    {
        return RotateVector(Rotation, v + Translation);
    }
    
    template <typename T>
    glm::tvec3<T> TTransform<T>::TransformVector(const glm::tvec3<T> &v) const
    {
        return RotateVector(Rotation, (Scale3D * v));
    }
    
    template <typename T>
    glm::tvec3<T> TTransform<T>::TransformVectorNoScale(const glm::tvec3<T> &v) const
    {
        return RotateVector(Rotation, v);
    }
    
    template <typename T>
    glm::tvec3<T> TTransform<T>::InverseTransformPosition(const glm::tvec3<T> &v) const
    {
        return (glm::inverse(Rotation) * (v - Translation)) / Scale3D;
    }
    
    template <typename T>
    glm::tvec3<T> TTransform<T>::InverseTransformPositionNoScale(const glm::tvec3<T> &v) const
    {
        return glm::inverse(Rotation) * (v - Translation);
    }
    
    template <typename T>
    glm::tvec3<T> TTransform<T>::GetUnitAxis(ECoordAxis axis) const
    {
        if (axis == ECoordAxis::CA_X)
            return TransformVectorNoScale(glm::tvec3<T>(1, 0, 0));
        else if (axis == ECoordAxis::CA_Y)
            return TransformVectorNoScale(glm::tvec3<T>(0, 1, 0));
        return TransformVectorNoScale(glm::tvec3<T>(0, 0, 1));
    }
    
    template <typename T>
    glm::tvec3<T> TTransform<T>::GetTranslation() const
    {
        return Translation;
    }
    
    template <typename T>
    glm::tvec3<T> TTransform<T>::GetLocation() const
    {
        return Translation;
    }
    
    template <typename T>
    glm::tvec3<T> TTransform<T>::GetScale3D() const
    {
        return Scale3D;
    }
    
    template <typename T>
    glm::tquat<T> TTransform<T>::GetRotation() const
    {
        return Rotation;
    }
    
    template <typename T>
    TRotator<T> TTransform<T>::GetRotator() const
    {
        return TRotator<T>(Rotation);
    }
    
    template <typename T>
    void TTransform<T>::SetScale3D(const glm::tvec3<T> &scale)
    {
        Scale3D = scale;
    }
    
    template <typename T>
    void TTransform<T>::SetRotation(const glm::tquat<T> &rotation)
    {
        Rotation = rotation;
    }
    
    template <typename T>
    void TTransform<T>::SetRotator(const TRotator<T> &rotator)
    {
        Rotation = rotator.ToQuat();
    }
    
    template <typename T>
    void TTransform<T>::SetTranslation(const glm::tvec3<T> &translation)
    {
        Translation = translation;
    }
    
    template <typename T>
    TTransform<T> TTransform<T>::operator*(const TTransform &Other) const
    {
        // 令Q = quaternion, S = scale, T = translation
        // 令QST(A) = Q(A), S(A), T(A)表达一个变换, P代表一个位置向量
        // 根据TTransform规定的变换顺序（Scale -> Rotate -> Translate）可以得到
        // QST(A) = Q(A)*S(A)*P*-Q(A) + T(A), 其中-Q(A)是四元数Q(A)的逆
        // QST(A×B) = Q(B)*S(B)*QST(A)*-Q(B) + T(B)
        // QST(A×B) = Q(B)*S(B)*[Q(A)*S(A)*P*-Q(A) + T(A)]*-Q(B) + T(B)
        // QST(A×B) = Q(B)*S(B)*Q(A)*S(A)*P*-Q(A)*-Q(B) + Q(B)*S(B)*T(A)*-Q(B) + T(B)
        // 整理得
        // QST(A×B) = [Q(B)*Q(A)] * [S(B)*S(A)] * P * [-Q(A)*-Q(B)] + [Q(B)*S(B)*T(A)*-Q(B) + T(B)]
        // 所以
        // Q(A×B) = Q(B)*Q(A), Q(B)和Q(A)不可以交换, 四元数乘积的逆等于各个四元数的逆以相反顺序相乘
        // S(A×B) = S(B)*S(A), S(B)和S(A)可以交换
        // T(A×B) = Q(B)*S(B)*T(A)*-Q(B) + T(B)
        
        TTransform output;
        output.Rotation = Other.Rotation * this->Rotation;
        output.Scale3D = Other.Scale3D * this->Scale3D;
        output.Translation = Other.Rotation * (Other.Scale3D * this->Translation) + Other.Translation;
        return output;
    }
    
    template <typename T>
    TTransform<T> TTransform<T>::GetRelativeTransform(const TTransform &Other) const
    {
        // this = output * Other, output = this * Other(-1)
        // 比如child_worldtrans = child_relative * parent_worldtrans, 要求出child_relative, 
        // 那就child_worldtrans.CalcRelativeTransform(parent_worldtrans)
        // 令A = this, B = Other
        // QST(A) = Q(A)*S(A)*P*Q(A)(-1) + T(A)
        // QST(B)(-1) = Q(B)(-1)*S(B)(-1)*(P-TB)*Q(B)
        // QST(output) = QST(A) * QST(B)(-1) = Q(B)(-1)*Q(A)*S(A)*S(B)(-1)*P*Q(A)(-1)*Q(B) + Q(B)(-1)*S(B)(-1)*[T(A)-T(B)]*Q(B)
        // 所以
        // Q(output) = Q(B)(-1)*Q(A)
        // S(output) = S(A)*S(B)(-1)
        // T(output) = Q(B)(-1)*S(B)(-1)*[T(A)-T(B)]*Q(B)
        glm::tquat<T> InverseQB = inverse(Other.Rotation);
        glm::tvec3<T> InverseSB = glm::tvec3<T>(1.0, 1.0, 1.0) / Other.Scale3D;
        TTransform output;
        output.Rotation = InverseQB * this->Rotation;
        output.Scale3D = InverseSB * this->Scale3D;
        output.Translation = InverseQB * (InverseSB * (this->Translation - Other.Translation));
        return output;
    }

    template <typename T>
    T TTransform<T>::GetMinimumAxisScale() const
    {
        return glm::min(Scale3D.x, glm::min(Scale3D.y, Scale3D.z));
    }

    template <typename T>
    glm::tvec3<T> TTransform<T>::GetSafeScaleReciprocal(const glm::tvec3<T> &InScale, T Tolerance)
    {
        glm::tvec3<T> SafeReciprocalScale;
        if (glm::abs(InScale.x) <= Tolerance)
        {
            SafeReciprocalScale.x = 0.0;
        }
        else
        {
            SafeReciprocalScale.x = 1.0/InScale.x;
        }

        if (glm::abs(InScale.y) <= Tolerance)
        {
            SafeReciprocalScale.y = 0.0;
        }
        else
        {
            SafeReciprocalScale.y = 1.0/InScale.y;
        }

        if (glm::abs(InScale.z) <= Tolerance)
        {
            SafeReciprocalScale.z = 0.0;
        }
        else
        {
            SafeReciprocalScale.z = 1.0/InScale.z;
        }

        return SafeReciprocalScale;
    }
    
    template <typename T>
    bool TTransform<T>::AnyHasNegativeScale(const glm::tvec3<T> &InScale3D, const glm::tvec3<T> &InOtherScale3D)
    {
        glm::tvec3<T> min_scale = glm::min(InScale3D, InOtherScale3D);
        return min_scale.x < 0.0 || min_scale.y < 0.0 || min_scale.z < 0.0;
    }

    template <typename T>
    glm::tmat4x4<T> TTransform<T>::ToMatrix() const
    {
        glm::tmat4x4<T> scale_mat{
            Scale3D.x, 0, 0, 0,
            0, Scale3D.y, 0, 0,
            0, 0, Scale3D.z, 0,
            0, 0, 0, 1
        };
        glm::tmat4x4<T> rotation_mat = mat4_cast(Rotation);
        glm::tmat4x4<T> tranlation_mat{
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            Translation.x, Translation.y, Translation.z, 1
        };

        return tranlation_mat * rotation_mat * scale_mat;
    }
}

namespace nilou {
    template <typename T>
    inline std::ostream &operator<<(std::ostream &out, const glm::tvec2<T> &obj)
    {
        out << "vec2: " << obj.x << " " << obj.y << " ";
        return out;
    }
    template <typename T>
    inline std::ostream &operator<<(std::ostream &out, const glm::tvec3<T> &obj)
    {
        out << "vec3: " << obj.x << " " << obj.y << " " << obj.z << " ";
        return out;
    }
    template <typename T>
    inline std::ostream &operator<<(std::ostream &out, const glm::tvec4<T> &obj)
    {
        out << "vec4: " << obj.x << " " << obj.y << " " << obj.z << " " << obj.z << " ";
        return out;
    }
    template <typename T>
    inline std::ostream &operator<<(std::ostream &out, const TRotator<T> &obj)
    {
        out << "FRotator: Yaw=" << obj.Yaw << " Pitch=" << obj.Pitch << " Roll=" << obj.Roll << " ";
        return out;
    }
    template <typename T>
    inline std::ostream &operator<<(std::ostream &out, const glm::tquat<T> &obj)
    {
        out << "quat: x=" << obj.x << " y=" << obj.y << " z=" << obj.z << " w=" << obj.w << " ";
        return out;
    }
    template <typename T>
    inline std::ostream &operator<<(std::ostream &out, const TTransform<T> &obj)
    {
        out << "Scale: " << obj.Scale3D << std::endl;
        out << "Rotation: " << obj.Rotation << std::endl;
        out << "Translation: " << obj.Translation << std::endl;

        return out;
    }
    template <typename T>
    inline std::ostream &operator<<(std::ostream &out, glm::tmat4x4<T> matrix)
    {
        out << matrix[0][0] << ' ' << matrix[1][0] << ' ' << matrix[2][0] << ' ' << matrix[3][0] << "\n";
        out << matrix[0][1] << ' ' << matrix[1][1] << ' ' << matrix[2][1] << ' ' << matrix[3][1] << "\n";
        out << matrix[0][2] << ' ' << matrix[1][2] << ' ' << matrix[2][2] << ' ' << matrix[3][2] << "\n";
        out << matrix[0][3] << ' ' << matrix[1][3] << ' ' << matrix[2][3] << ' ' << matrix[3][3] << "\n";
        return out;
    }
}