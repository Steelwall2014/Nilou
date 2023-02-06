#include "Transform.h"


namespace nilou {

    FTransform FTransform::Identity;
    FRotator FRotator::ZeroRotator;

    FTransform::FTransform()
        : Rotation(quat(1.f, 0.f, 0.f, 0.f))
        , Translation(vec3(0.f, 0.f, 0.f))
        , Scale3D(vec3(1.f, 1.f, 1.f))
    {

    }

    FTransform::FTransform(const quat &rotation)
        : Rotation(rotation)
        , Translation(vec3(0.f, 0.f, 0.f))
        , Scale3D(vec3(1.f, 1.f, 1.f))
    {

    }
    FTransform::FTransform(const vec3 &translation)
        : Rotation(quat(1.f, 0.f, 0.f, 0.f))
        , Translation(translation)
        , Scale3D(vec3(1.f, 1.f, 1.f))
    {

    }
    FTransform::FTransform(const vec3 &scale3d, const quat &rotation, const vec3 &translation)
        : Rotation(rotation)
        , Translation(translation)
        , Scale3D(scale3d)
    {
    }
    FTransform::FTransform(const mat4 &matrix)
    {
        SetFromMatrix(matrix);
    }
    void FTransform::SetFromMatrix(const mat4 &matrix)
    {
        vec3 translation{ matrix[3][0], matrix[3][1], matrix[3][2] };
        vec3 scale3d;
        scale3d.x = sqrt(matrix[0][0]*matrix[0][0] + matrix[0][1]*matrix[0][1] + matrix[0][2]*matrix[0][2]);
        scale3d.y = sqrt(matrix[1][0]*matrix[1][0] + matrix[1][1]*matrix[1][1] + matrix[1][2]*matrix[1][2]);
        scale3d.z = sqrt(matrix[2][0]*matrix[2][0] + matrix[2][1]*matrix[2][1] + matrix[2][2]*matrix[2][2]);

        mat3 rotation_mat{
            matrix[0][0] / scale3d.x, matrix[0][1] / scale3d.x, matrix[0][2] / scale3d.x,
            matrix[1][0] / scale3d.y, matrix[1][1] / scale3d.y, matrix[1][2] / scale3d.y,
            matrix[2][0] / scale3d.z, matrix[2][1] / scale3d.z, matrix[2][2] / scale3d.z };
        quat rotation(rotation_mat);        // 矩阵转四元数算法见《3D数学基础：图形与游戏开发》P168

        Rotation = rotation;
        Translation = translation;
        Scale3D = scale3d;
    }
    vec3 FTransform::TransformPosition(const vec3 &v) const
    {
        return RotateVector(Rotation, (Scale3D * v)) + Translation;
    }
    vec3 FTransform::TransformPositionNoScale(const vec3 &v) const
    {
        return RotateVector(Rotation, v + Translation);
    }
    vec3 FTransform::TransformVector(const vec3 &v) const
    {
        return RotateVector(Rotation, (Scale3D * v));
    }
    vec3 FTransform::TransformVectorNoScale(const vec3 &v) const
    {
        //std::cout << Rotation.x << " " << Rotation.y << " " << Rotation.z << " " << Rotation.w << std::endl;
        return RotateVector(Rotation, v);
    }
    vec3 FTransform::InverseTransformPosition(const vec3 &v) const
    {
        return (glm::inverse(Rotation) * (v - Translation)) / Scale3D;
    }
    vec3 FTransform::InverseTransformPositionNoScale(const vec3 &v) const
    {
        return glm::inverse(Rotation) * (v - Translation);
    }
    vec3 FTransform::GetUnitAxis(ECoordAxis axis) const
    {
        if (axis == ECoordAxis::CA_X)
            return TransformVectorNoScale(vec3(1, 0, 0));
        else if (axis == ECoordAxis::CA_Y)
            return TransformVectorNoScale(vec3(0, 1, 0));
        return TransformVectorNoScale(vec3(0, 0, 1));
    }
    vec3 FTransform::GetTranslation() const
    {
        return Translation;
    }
    vec3 FTransform::GetScale3D() const
    {
        return Scale3D;
    }
    quat FTransform::GetRotation() const
    {
        return Rotation;
    }
    FRotator FTransform::GetRotator() const
    {
        return FRotator(Rotation);
    }
    void FTransform::SetScale3D(const vec3 &scale)
    {
        Scale3D = scale;
    }
    void FTransform::SetRotation(const quat &rotation)
    {
        Rotation = rotation;
    }
    void FTransform::SetRotator(const FRotator &rotator)
    {
        Rotation = rotator.ToQuat();
    }
    void FTransform::SetTranslation(const vec3 &translation)
    {
        Translation = translation;
    }
    FTransform FTransform::operator*(const FTransform &Other) const
    {
        // 令Q = quaternion, S = scale, T = translation
        // 令QST(A) = Q(A), S(A), T(A)表达一个变换, P代表一个位置向量
        // 根据FTransform规定的变换顺序（Scale -> Rotate -> Translate）可以得到
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
        
        FTransform output;
        output.Rotation = Other.Rotation * this->Rotation;
        output.Scale3D = Other.Scale3D * this->Scale3D;
        output.Translation = Other.Rotation * (Other.Scale3D * this->Translation) + Other.Translation;
        return output;
    }
    FTransform FTransform::GetRelativeTransform(const FTransform &Other) const
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
        quat InverseQB = inverse(Other.Rotation);
        vec3 InverseSB = vec3(1.f, 1.f, 1.f) / Other.Scale3D;
        FTransform output;
        output.Rotation = InverseQB * this->Rotation;
        output.Scale3D = InverseSB * this->Scale3D;
        output.Translation = InverseQB * (InverseSB * (this->Translation - Other.Translation));
        return output;

        // this = Other * output, output = Other(-1) * this 
        // 比如child_worldtrans = parent_worldtrans * child_relative, 要求出child_relative, 
        // 那就child_worldtrans.CalcRelativeTransform(parent_worldtrans)
        // 令A = this, B = Other
        // QST(A) = Q(A)*S(A)*P*Q(A)(-1) + T(A)
        // QST(B)(-1) = Q(B)(-1)*S(B)(-1)*(P-TB)*Q(B)
        // QST(output) = QST(B)(-1) * QST(A) = Q(A)*Q(B)(-1)*S(A)*S(B)(-1)*P*Q(B)*Q(A)(-1) + T(A)-Q(A)*Q(B)(-1)*S(A)*S(B)(-1)*T(B)*Q(B)*Q(A)(-1)
        // 所以
        // Q(output) = Q(A)*Q(B)(-1)
        // S(output) = S(A)*S(B)(-1)
        // T(output) = T(A)-Q(A)*Q(B)(-1)*S(A)*S(B)(-1)*T(B)*Q(B)*Q(A)(-1)
        // quat InverseQB = inverse(Other.Rotation);
        // vec3 InverseSB = vec3(1.f, 1.f, 1.f) / Other.Scale3D;
        // FTransform output;
        // output.Rotation = this->Rotation * InverseQB;
        // output.Scale3D = InverseSB * this->Scale3D;
        // output.Translation = this->Translation - this->Rotation*InverseQB * (this->Scale3D * InverseSB * Other.Translation);
        // return output;
    }

    float FTransform::GetMinimumAxisScale() const
    {
        return glm::min(Scale3D.x, glm::min(Scale3D.y, Scale3D.z));
    }

    vec3 FTransform::GetSafeScaleReciprocal(const vec3 &InScale, float Tolerance)
    {
        vec3 SafeReciprocalScale;
        if (glm::abs(InScale.x) <= Tolerance)
        {
            SafeReciprocalScale.x = 0.f;
        }
        else
        {
            SafeReciprocalScale.x = 1.f/InScale.x;
        }

        if (glm::abs(InScale.y) <= Tolerance)
        {
            SafeReciprocalScale.y = 0.f;
        }
        else
        {
            SafeReciprocalScale.y = 1.f/InScale.y;
        }

        if (glm::abs(InScale.z) <= Tolerance)
        {
            SafeReciprocalScale.z = 0.f;
        }
        else
        {
            SafeReciprocalScale.z = 1.f/InScale.z;
        }

        return SafeReciprocalScale;
    }
    
    bool FTransform::AnyHasNegativeScale(const vec3 &InScale3D, const vec3 &InOtherScale3D)
    {
        vec3 min_scale = glm::min(InScale3D, InOtherScale3D);
        return min_scale.x < 0 || min_scale.y < 0 || min_scale.z < 0;
    }

    mat4 FTransform::ToMatrix() const
    {
        mat4 scale_mat{
            Scale3D.x, 0, 0, 0,
            0, Scale3D.y, 0, 0,
            0, 0, Scale3D.z, 0,
            0, 0, 0, 1
        };
        mat4 rotation_mat = mat4_cast(Rotation);
        mat4 tranlation_mat{
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            Translation.x, Translation.y, Translation.z, 1
        };

        return tranlation_mat * rotation_mat * scale_mat;
    }
    //void FTransform::Multiply(FTransform *Output, FTransform *A, FTransform *B)
    //{
    //}
    //FTransform::FTransform(const mat4 &matrix)
    //{
    //    SetFromMatrix(matrix);
    //}
    //void FTransform::SetFromMatrix(const mat4 &matrix)
    //{
    //
    //    Rotation = quat(matrix);
    //
    //}
    FRotator::FRotator() : Pitch(0.f), Yaw(0.f), Roll(0.f) {}
    FRotator::FRotator(const vec3 &eulerAngles)
    {
        Pitch = eulerAngles.x;
        Yaw = eulerAngles.y;
        Roll = eulerAngles.z;
    }
    FRotator::FRotator(float pitch, float yaw, float roll) : Pitch(pitch), Yaw(yaw), Roll(roll) {}

    FRotator::FRotator(const quat &rotation)
    {
        float X = rotation.x;
        float Y = rotation.y;
        float Z = rotation.z;
        float W = rotation.w;


		const float SingularityTest = Z * X - W * Y;
		const float YawY = 2.f * (W * Z + X * Y);
		const float YawX = (1.f - 2.f * (Y*Y + Z*Z));

		// reference 
		// http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
		// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/

		// this value was found from experience, the above websites recommend different values
		// but that isn't the case for us, so I went through different testing, and finally found the case 
		// where both of world lives happily. 
		const float SINGULARITY_THRESHOLD = 0.4999995f;
		const float RAD_TO_DEG = (180.f / PI);
		// float Pitch, Yaw, Roll;

		if (SingularityTest < -SINGULARITY_THRESHOLD)
		{
			Pitch = -90.f;
			Yaw = (std::atan2(YawY, YawX) * RAD_TO_DEG);
			Roll = FRotator::NormalizeAxis(-Yaw - (2.f * std::atan2(X, W) * RAD_TO_DEG));
		}
		else if (SingularityTest > SINGULARITY_THRESHOLD)
		{
			Pitch = 90.f;
			Yaw = (std::atan2(YawY, YawX) * RAD_TO_DEG);
			Roll = FRotator::NormalizeAxis(Yaw - (2.f * std::atan2(X, W) * RAD_TO_DEG));
		}
		else
		{
			Pitch = (std::asin(2.f * SingularityTest) * RAD_TO_DEG);
			Yaw = (std::atan2(YawY, YawX) * RAD_TO_DEG);
			Roll = (std::atan2(-2.f * (W*X + Y*Z), (1.f - 2.f * (X*X + Y*Y))) * RAD_TO_DEG);
		}

		// *this = FRotator(Pitch, Yaw, Roll);

        // vec3 raw_angles = eulerAngles(rotation);
        // Pitch = raw_angles.x;
        // Yaw = raw_angles.y;
        // Roll = raw_angles.z;
        // if (raw_angles.x < glm::radians(-90.f) || raw_angles.x > glm::radians(90.f))
        // {
        //     raw_angles.y = glm::radians(180.f) - raw_angles.y;
        // }
        // else
        // {
        //     if (raw_angles.y < 0.f)
        //         raw_angles.y = glm::radians(360.f) + raw_angles.y;
        // }

        // if (raw_angles.x < glm::radians(-90.f))
        // {
        //     raw_angles.x += glm::radians(180.f);
        // }
        // else if (raw_angles.x > glm::radians(90.f))
        // {
        //     raw_angles.x -= glm::radians(180.f);
        // }

        // Pitch = raw_angles.x;
        // Yaw = raw_angles.y;
        // Roll = raw_angles.z;
    }

    float FRotator::NormalizeAxis(float Angle)
    {
        Angle = ClampAxis(Angle);

        if (Angle > 180.0)
        {
            // shift to (-180,180]
            Angle -= 360.0;
        }

        return Angle;
    }

    float FRotator::ClampAxis(float Angle)
    {
        Angle = glm::mod(Angle, 360.0f);

        if (Angle < 0.0)
        {
            // shift to [0,360) range
            Angle += 360.0;
        }

        return Angle;
    }

    void SinCos(float* ScalarSin, float* ScalarCos, float Value)
    {
        // Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
		float quotient = (INV_PI*0.5f)*Value;
		if (Value >= 0.0f)
		{
			quotient = (float)((glm::int64)(quotient + 0.5f));
		}
		else
		{
			quotient = (float)((glm::int64)(quotient - 0.5f));
		}
		float y = Value - TWO_PI * quotient;

		// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
		float sign;
		if (y > HALF_PI)
		{
			y = PI - y;
			sign = -1.0f;
		}
		else if (y < -HALF_PI)
		{
			y = -PI - y;
			sign = -1.0f;
		}
		else
		{
			sign = +1.0f;
		}

		float y2 = y * y;

		// 11-degree minimax approximation
		*ScalarSin = ( ( ( ( (-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f ) * y2 + 0.0083333310f ) * y2 - 0.16666667f ) * y2 + 1.0f ) * y;

		// 10-degree minimax approximation
		float p = ( ( ( ( -2.6051615e-07f * y2 + 2.4760495e-05f ) * y2 - 0.0013888378f ) * y2 + 0.041666638f ) * y2 - 0.5f ) * y2 + 1.0f;
		*ScalarCos = sign*p;

		// *ScalarSin = glm::sin(Value);
		// *ScalarCos = glm::cos(Value);
    }

    quat FRotator::ToQuat() const
    {
        //FRotator temp = FRotator(*this);
        //if (90.f < degrees(temp.Yaw) && degrees(temp.Yaw) < 270.f)
        //{
        //    if (temp.Pitch > 0.f)
        //        temp.Pitch -= radians(90.f);
        //    else if (temp.Pitch < 0.f)
        //        temp.Pitch += radians(180.f);
        //}
        //if (90.f < degrees(temp.Yaw) && degrees(temp.Yaw) < 270.f)
        //{
        //    temp.Yaw = radians(180.f) - temp.Yaw;
        //}
        //else if (270.f < degrees(temp.Yaw))
        //{
        //    temp.Yaw = temp.Yaw - radians(360.f);
        //}

    	const float DEG_TO_RAD = PI/(180.f);
        const float RADS_DIVIDED_BY_2 = DEG_TO_RAD/2.f;
        float SP, SY, SR;
        float CP, CY, CR;

        const float PitchNoWinding = glm::mod(Pitch, 360.0f);
        const float YawNoWinding = glm::mod(Yaw, 360.0f);
        const float RollNoWinding = glm::mod(Roll, 360.0f);

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

    glm::vec3 RotateVector(const quat &rotation, const vec3 &V)
    {
        // http://people.csail.mit.edu/bkph/articles/Quaternions.pdf
        // V' = V + 2w(Q x V) + (2Q x (Q x V))
        // refactor:
        // V' = V + w(2(Q x V)) + (Q x (2(Q x V)))
        // T = 2(Q x V);
        // V' = V + w*(T) + (Q x T)

        float X = rotation.x;
        float Y = rotation.y;
        float Z = rotation.z;
        float W = rotation.w;

        const vec3 Q(X, Y, Z);
        const vec3 TT = 2.f * glm::cross(Q, V);
        const vec3 Result = V + (W * TT) + glm::cross(Q, TT);
        return Result;
        // return rotation * V;
    }

}
std::ostream &operator<<(std::ostream &out, nilou::mat4 matrix)
{
    out << matrix[0][0] << ' ' << matrix[1][0] << ' ' << matrix[2][0] << ' ' << matrix[3][0] << "\n";
    out << matrix[0][1] << ' ' << matrix[1][1] << ' ' << matrix[2][1] << ' ' << matrix[3][1] << "\n";
    out << matrix[0][2] << ' ' << matrix[1][2] << ' ' << matrix[2][2] << ' ' << matrix[3][2] << "\n";
    out << matrix[0][3] << ' ' << matrix[1][3] << ' ' << matrix[2][3] << ' ' << matrix[3][3] << "\n";
    return out;
}

namespace nilou {
    // 这里只能包在namespace里面, 如果这样写std::ostream &operator<<(std::ostream &out, const FTransform &obj)会报错
    std::ostream &operator<<(std::ostream &out, const FTransform &obj)
    {
        out << "Scale: " << obj.Scale3D << std::endl;
        out << "Rotation: " << obj.Rotation << std::endl;
        out << "Translation: " << obj.Translation << std::endl;

        return out;
    }
}

