#pragma once

#include <ostream>
#include "Maths.h"
#include "Archive.h"

namespace nilou {

    class NClass;
    
	const FVector WORLD_UP(0.0, 0.0, 1.0);
	const FVector WORLD_FORWARD(1.0, 0.0, 0.0);
	const FVector WORLD_RIGHT(0.0, 1.0, 0.0);

    template <typename T>
    inline bool Equals(const TVector<T> &A, const TVector<T> &B, T Tolerance=KINDA_SMALL_NUMBER)
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
    private:
        static NClass* Z_StaticClass;
    public:
        virtual NClass *GetClass() const { return Z_StaticClass; }
        static NClass *StaticClass() { return Z_StaticClass; }
        /** Rotation around the right axis (around Y axis), Looking up and down (0=Straight Ahead, +Up, -Down) */
	    T Pitch;

	    /** Rotation around the up axis (around Z axis), Running in circles 0=East (positive X axis) , +North, -South. */
	    T Yaw;

	    /** Rotation around the forward axis (around X axis), Tilting your head, 0=Straight, +Clockwise, -CCW. */
	    T Roll;

        TRotator();
        TRotator(T pitch, T yaw, T roll);
        explicit TRotator(const TVector<T> &eulerAngles);
        explicit TRotator(const TQuat<T> &rotation);

        static T NormalizeAxis(T Angle);
        static T ClampAxis(T Angle);

        TQuat<T> ToQuat() const;
        bool Equals(const TRotator &B, T Tolerance=KINDA_SMALL_NUMBER) const
        {
            return glm::abs(Pitch-B.Pitch) <= Tolerance && glm::abs(Yaw-B.Yaw) <= Tolerance && glm::abs(Roll-B.Roll) <= Tolerance;
        }

        static const TRotator ZeroRotator;

        friend void Serialize(FArchive& Ar, TRotator<T>& Rotator);
        template <typename U>
        friend class TClassRegistry;
    };

    template <typename T>
    TVector<T> RotateVector(const TQuat<T> &rotation, const TVector<T> &V);

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
        TQuat<T> Rotation;
        TVector<T> Translation;
        TVector<T> Scale3D;
    private:
        static NClass* Z_StaticClass;
    public:
        virtual NClass *GetClass() const { return Z_StaticClass; }
        static NClass *StaticClass() { return Z_StaticClass; }
        template <typename U>
        friend class TClassRegistry;

        TTransform();
        TTransform(const TQuat<T> &rotation);
        TTransform(const TVector<T> &translation);
        TTransform(const TVector<T> &scale3d, const TQuat<T> &rotation, const TVector<T> &translation);
        TTransform(const TMatrix<T> &matrix);

        void SetFromMatrix(const TMatrix<T> &matrix);
        TVector<T> GetUnitAxis(ECoordAxis axis) const;
        TVector<T> GetScale3D() const;
        TQuat<T> GetRotation() const;
        TRotator<T> GetRotator() const;
        TVector<T> GetTranslation() const;
        TVector<T> GetLocation() const;

        TVector<T> TransformPosition(const TVector<T> &v) const;
        TVector<T> TransformPositionNoScale(const TVector<T> &v) const;
        TVector<T> TransformVector(const TVector<T> &v) const;
        TVector<T> TransformVectorNoScale(const TVector<T> &v) const;
        TVector<T> InverseTransformPosition(const TVector<T> &v) const;
        TVector<T> InverseTransformPositionNoScale(const TVector<T> &v) const;

        void SetScale3D(const TVector<T> &scale);
        void SetRotation(const TQuat<T> &rotation);
        void SetRotator(const TRotator<T> &rotator);
        void SetTranslation(const TVector<T> &translation);
        TTransform operator*(const TTransform &Other) const;
        TTransform GetRelativeTransform(const TTransform &Other) const;
        T GetMinimumAxisScale() const;
        TVector<T> GetSafeScaleReciprocal(const TVector<T> &InScale, T Tolerance = SMALL_NUMBER);

        static bool AnyHasNegativeScale(const TVector<T> &InScale3D, const TVector<T> &InOtherScale3D);

        TMatrix<T> ToMatrix() const;

        static const TTransform<T> Identity;

        friend void Serialize(FArchive& Ar, TTransform<T>& Transform);
        template <typename U>
        friend std::ostream &operator<<(std::ostream &out, const TTransform<U> &obj);
    };

    template<typename T> 
    const TTransform<T> TTransform<T>::Identity = TTransform<T>();
    
    template<typename T> 
    const TRotator<T> TRotator<T>::ZeroRotator = TRotator<T>();

    using FTransform = TTransform<double>;
    using FTransform3f = TTransform<float>;

    template <typename T>
    void Serialize(FArchive& Ar, TRotator<T>& Rotator)
    {
        nlohmann::json& Node = Ar.GetNode();
        if (Ar.IsLoading())
        {
            Rotator.Pitch = Node["Pitch"];
            Rotator.Yaw = Node["Yaw"];
            Rotator.Roll = Node["Roll"];
        }
        else
        {
            Node["Pitch"] = Rotator.Pitch;
            Node["Yaw"] = Rotator.Yaw;
            Node["Roll"] = Rotator.Roll;
        }
    }

    template <typename T>
    void Serialize(FArchive& Ar, TTransform<T>& Transform)
    {
        Serialize(Ar["Translation"], Transform.Translation);
        Serialize(Ar["Rotation"], Transform.Rotation);
        Serialize(Ar["Scale3D"], Transform.Scale3D);
    }
}

namespace nilou {

    template<typename T>
    TRotator<T>::TRotator() : Pitch(0), Yaw(0), Roll(0) {}
    template<typename T>
    TRotator<T>::TRotator(const TVector<T> &eulerAngles)
        : Pitch(eulerAngles.x), Yaw(eulerAngles.y), Roll(eulerAngles.z) {}
        
    template<typename T>
    TRotator<T>::TRotator(T pitch, T yaw, T roll) : Pitch(pitch), Yaw(yaw), Roll(roll) {}

    template<typename T>
    TRotator<T>::TRotator(const TQuat<T> &rotation)
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
    TQuat<T> TRotator<T>::ToQuat() const
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

        TQuat<T> RotationQuat;
        RotationQuat.x =  CR*SP*SY - SR*CP*CY;
        RotationQuat.y = -CR*SP*CY - SR*CP*SY;
        RotationQuat.z =  CR*CP*SY - SR*SP*CY;
        RotationQuat.w =  CR*CP*CY + SR*SP*SY;
        return RotationQuat;
        // return quat(vec3(Pitch, Yaw, Roll));
    }

    template <typename T>
    TVector<T> RotateVector(const TQuat<T> &rotation, const TVector<T> &V)
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

        const TVector<T> Q(X, Y, Z);
        const TVector<T> TT = 2.0 * glm::cross(Q, V);
        const TVector<T> Result = V + (W * TT) + glm::cross(Q, TT);
        return Result;
        // return rotation * V;
    }

    template <typename T>
    TTransform<T>::TTransform()
        : Rotation(TQuat<T>(1.0, 0.0, 0.0, 0.0))
        , Translation(TVector<T>(0.0, 0.0, 0.0))
        , Scale3D(TVector<T>(1.0, 1.0, 1.0))
    {

    }

    template <typename T>
    TTransform<T>::TTransform(const TQuat<T> &rotation)
        : Rotation(rotation)
        , Translation(TVector<T>(0.0, 0.0, 0.0))
        , Scale3D(TVector<T>(1.0, 1.0, 1.0))
    {

    }

    template <typename T>
    TTransform<T>::TTransform(const TVector<T> &translation)
        : Rotation(TQuat<T>(1.0, 0.0, 0.0, 0.0))
        , Translation(translation)
        , Scale3D(TVector<T>(1.0, 1.0, 1.0))
    {

    }
    
    template <typename T>
    TTransform<T>::TTransform(const TVector<T> &scale3d, const TQuat<T> &rotation, const TVector<T> &translation)
        : Rotation(rotation)
        , Translation(translation)
        , Scale3D(scale3d)
    {
    }
    
    template <typename T>
    TTransform<T>::TTransform(const TMatrix<T> &matrix)
    {
        SetFromMatrix(matrix);
    }

    template <typename T>
    static TQuat<T> UEMatrixToQuat(glm::tmat3x3<T> M)
    {
        TQuat<T> out;
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
    void TTransform<T>::SetFromMatrix(const TMatrix<T> &matrix)
    {
        TVector<T> translation{ matrix[3][0], matrix[3][1], matrix[3][2] };
        TVector<T> scale3d;
        scale3d.x = sqrt(matrix[0][0]*matrix[0][0] + matrix[0][1]*matrix[0][1] + matrix[0][2]*matrix[0][2]);
        scale3d.y = sqrt(matrix[1][0]*matrix[1][0] + matrix[1][1]*matrix[1][1] + matrix[1][2]*matrix[1][2]);
        scale3d.z = sqrt(matrix[2][0]*matrix[2][0] + matrix[2][1]*matrix[2][1] + matrix[2][2]*matrix[2][2]);

        glm::tmat3x3<T> rotation_mat{
            matrix[0][0] / scale3d.x, matrix[0][1] / scale3d.x, matrix[0][2] / scale3d.x,
            matrix[1][0] / scale3d.y, matrix[1][1] / scale3d.y, matrix[1][2] / scale3d.y,
            matrix[2][0] / scale3d.z, matrix[2][1] / scale3d.z, matrix[2][2] / scale3d.z };
        if (glm::determinant(rotation_mat) < 0.f)
        {
            scale3d[0] *= -1;
            rotation_mat[0] = -rotation_mat[0];
        }
        TQuat<T> rotation = UEMatrixToQuat(rotation_mat);

        Rotation = rotation;
        Translation = translation;
        Scale3D = scale3d;
    }
    
    template <typename T>
    TVector<T> TTransform<T>::TransformPosition(const TVector<T> &v) const
    {
        return RotateVector(Rotation, (Scale3D * v)) + Translation;
    }
    
    template <typename T>
    TVector<T> TTransform<T>::TransformPositionNoScale(const TVector<T> &v) const
    {
        return RotateVector(Rotation, v + Translation);
    }
    
    template <typename T>
    TVector<T> TTransform<T>::TransformVector(const TVector<T> &v) const
    {
        return RotateVector(Rotation, (Scale3D * v));
    }
    
    template <typename T>
    TVector<T> TTransform<T>::TransformVectorNoScale(const TVector<T> &v) const
    {
        return RotateVector(Rotation, v);
    }
    
    template <typename T>
    TVector<T> TTransform<T>::InverseTransformPosition(const TVector<T> &v) const
    {
        return (glm::inverse(Rotation) * (v - Translation)) / Scale3D;
    }
    
    template <typename T>
    TVector<T> TTransform<T>::InverseTransformPositionNoScale(const TVector<T> &v) const
    {
        return glm::inverse(Rotation) * (v - Translation);
    }
    
    template <typename T>
    TVector<T> TTransform<T>::GetUnitAxis(ECoordAxis axis) const
    {
        if (axis == ECoordAxis::CA_X)
            return TransformVectorNoScale(TVector<T>(1, 0, 0));
        else if (axis == ECoordAxis::CA_Y)
            return TransformVectorNoScale(TVector<T>(0, 1, 0));
        return TransformVectorNoScale(TVector<T>(0, 0, 1));
    }
    
    template <typename T>
    TVector<T> TTransform<T>::GetTranslation() const
    {
        return Translation;
    }
    
    template <typename T>
    TVector<T> TTransform<T>::GetLocation() const
    {
        return Translation;
    }
    
    template <typename T>
    TVector<T> TTransform<T>::GetScale3D() const
    {
        return Scale3D;
    }
    
    template <typename T>
    TQuat<T> TTransform<T>::GetRotation() const
    {
        return Rotation;
    }
    
    template <typename T>
    TRotator<T> TTransform<T>::GetRotator() const
    {
        return TRotator<T>(Rotation);
    }
    
    template <typename T>
    void TTransform<T>::SetScale3D(const TVector<T> &scale)
    {
        Scale3D = scale;
    }
    
    template <typename T>
    void TTransform<T>::SetRotation(const TQuat<T> &rotation)
    {
        Rotation = rotation;
    }
    
    template <typename T>
    void TTransform<T>::SetRotator(const TRotator<T> &rotator)
    {
        Rotation = rotator.ToQuat();
    }
    
    template <typename T>
    void TTransform<T>::SetTranslation(const TVector<T> &translation)
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
        TQuat<T> InverseQB = inverse(Other.Rotation);
        TVector<T> InverseSB = TVector<T>(1.0, 1.0, 1.0) / Other.Scale3D;
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
    TVector<T> TTransform<T>::GetSafeScaleReciprocal(const TVector<T> &InScale, T Tolerance)
    {
        TVector<T> SafeReciprocalScale;
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
    bool TTransform<T>::AnyHasNegativeScale(const TVector<T> &InScale3D, const TVector<T> &InOtherScale3D)
    {
        TVector<T> min_scale = glm::min(InScale3D, InOtherScale3D);
        return min_scale.x < 0.0 || min_scale.y < 0.0 || min_scale.z < 0.0;
    }

    template <typename T>
    TMatrix<T> TTransform<T>::ToMatrix() const
    {
        TMatrix<T> scale_mat{
            Scale3D.x, 0, 0, 0,
            0, Scale3D.y, 0, 0,
            0, 0, Scale3D.z, 0,
            0, 0, 0, 1
        };
        TMatrix<T> rotation_mat = mat4_cast(Rotation);
        TMatrix<T> tranlation_mat{
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
    inline std::ostream &operator<<(std::ostream &out, const TVector<T> &obj)
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
    inline std::ostream &operator<<(std::ostream &out, const TQuat<T> &obj)
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
    inline std::ostream &operator<<(std::ostream &out, TMatrix<T> matrix)
    {
        out << matrix[0][0] << ' ' << matrix[1][0] << ' ' << matrix[2][0] << ' ' << matrix[3][0] << "\n";
        out << matrix[0][1] << ' ' << matrix[1][1] << ' ' << matrix[2][1] << ' ' << matrix[3][1] << "\n";
        out << matrix[0][2] << ' ' << matrix[1][2] << ' ' << matrix[2][2] << ' ' << matrix[3][2] << "\n";
        out << matrix[0][3] << ' ' << matrix[1][3] << ' ' << matrix[2][3] << ' ' << matrix[3][3] << "\n";
        return out;
    }
}