#include "SceneObjectTransform.h"

std::ostream &operator<<(std::ostream &out, glm::mat4 matrix)
{
    out << matrix[0][0] << ' ' << matrix[1][0] << ' ' << matrix[2][0] << ' ' << matrix[3][0] << "\n";
    out << matrix[0][1] << ' ' << matrix[1][1] << ' ' << matrix[2][1] << ' ' << matrix[3][1] << "\n";
    out << matrix[0][2] << ' ' << matrix[1][2] << ' ' << matrix[2][2] << ' ' << matrix[3][2] << "\n";
    out << matrix[0][3] << ' ' << matrix[1][3] << ' ' << matrix[2][3] << ' ' << matrix[3][3] << "\n";
    return out;
}

und::SceneObjectTransform::SceneObjectTransform()
    : Rotation(glm::quat(1.f, 0.f, 0.f, 0.f))
    , Translation(glm::vec3(0.f, 0.f, 0.f))
    , Scale3D(glm::vec3(1.f, 1.f, 1.f))
{

}

und::SceneObjectTransform::SceneObjectTransform(const glm::quat &rotation)
    : Rotation(rotation)
    , Translation(glm::vec3(0.f, 0.f, 0.f))
    , Scale3D(glm::vec3(1.f, 1.f, 1.f))
{

}
und::SceneObjectTransform::SceneObjectTransform(const glm::vec3 &translation)
    : Rotation(glm::quat(1.f, 0.f, 0.f, 0.f))
    , Translation(translation)
    , Scale3D(glm::vec3(1.f, 1.f, 1.f))
{

}
und::SceneObjectTransform::SceneObjectTransform(const glm::vec3 &scale3d, const glm::quat &rotation, const glm::vec3 &translation)
    : Rotation(rotation)
    , Translation(translation)
    , Scale3D(scale3d)
{
}
und::SceneObjectTransform::SceneObjectTransform(const glm::mat4 &matrix)
{
    SetFromMatrix(matrix);
}
void und::SceneObjectTransform::SetFromMatrix(const glm::mat4 &matrix)
{
    glm::vec3 translation{ matrix[3][0], matrix[3][1], matrix[3][2] };
    glm::vec3 scale3d;
    scale3d.x = sqrt(matrix[0][0]*matrix[0][0] + matrix[0][1]*matrix[0][1] + matrix[0][2]*matrix[0][2]);
    scale3d.y = sqrt(matrix[1][0]*matrix[1][0] + matrix[1][1]*matrix[1][1] + matrix[1][2]*matrix[1][2]);
    scale3d.z = sqrt(matrix[2][0]*matrix[2][0] + matrix[2][1]*matrix[2][1] + matrix[2][2]*matrix[2][2]);

    glm::mat3 rotation_mat{
        matrix[0][0] / scale3d.x, matrix[0][1] / scale3d.x, matrix[0][2] / scale3d.x,
        matrix[1][0] / scale3d.y, matrix[1][1] / scale3d.y, matrix[1][2] / scale3d.y,
        matrix[2][0] / scale3d.z, matrix[2][1] / scale3d.z, matrix[2][2] / scale3d.z };
    glm::quat rotation(rotation_mat);        // 矩阵转四元数算法见《3D数学基础：图形与游戏开发》P168

    Rotation = rotation;
    Translation = translation;
    Scale3D = scale3d;
}
glm::vec3 und::SceneObjectTransform::TransformVectorNoScale(const glm::vec3 &v)
{
    //std::cout << Rotation.x << " " << Rotation.y << " " << Rotation.z << " " << Rotation.w << std::endl;
    return Rotation * v;
}
glm::vec3 und::SceneObjectTransform::GetUnitAxis(CoordAxis axis)
{
    if (axis == CoordAxis::X)
        return TransformVectorNoScale(glm::vec3(1.f, 0.f, 0.f));
    else if (axis == CoordAxis::Y)
        return TransformVectorNoScale(glm::vec3(0.f, 1.f, 0.f));
    return TransformVectorNoScale(glm::vec3(0.f, 0.f, 1.f));
}
glm::vec3 und::SceneObjectTransform::GetTranslation()
{
    return Translation;
}
glm::vec3 und::SceneObjectTransform::GetScale3D()
{
    return Scale3D;
}
glm::quat und::SceneObjectTransform::GetRotation()
{
    return Rotation;
}
void und::SceneObjectTransform::SetScale3D(const glm::vec3 &scale)
{
    Scale3D = scale;
}
void und::SceneObjectTransform::SetRotation(const glm::quat &rotation)
{
    Rotation = rotation;
}
void und::SceneObjectTransform::SetTranslation(const glm::vec3 &translation)
{
    Translation = translation;
}
und::SceneObjectTransform und::SceneObjectTransform::operator*(const SceneObjectTransform &Other) const
{
    // 令Q = quaternion，S = scale，T = translation
    // 令QST(A) = Q(A), S(A), T(A)表达一个变换，P代表一个位置向量
    // 根据SceneObjectTransform规定的变换顺序（Scale -> Rotate -> Translate）可以得到
    // QST(A) = Q(A)*S(A)*P*-Q(A) + T(A), 其中-Q(A)是四元数Q(A)的逆
    // QST(A×B) = Q(B)*S(B)*QST(A)*-Q(B) + T(B)
    // QST(A×B) = Q(B)*S(B)*[Q(A)*S(A)*P*-Q(A) + T(A)]*-Q(B) + T(B)
    // QST(A×B) = Q(B)*S(B)*Q(A)*S(A)*P*-Q(A)*-Q(B) + Q(B)*S(B)*T(A)*-Q(B) + T(B)
    // 整理得
    // QST(A×B) = [Q(B)*Q(A)] * [S(B)*S(A)] * P * [-Q(A)*-Q(B)] + [Q(B)*S(B)*T(A)*-Q(B) + T(B)]
    // 所以
    // Q(A×B) = Q(B)*Q(A)，Q(B)和Q(A)不可以交换，四元数乘积的逆等于各个四元数的逆以相反顺序相乘
    // S(A×B) = S(B)*S(A)，S(B)和S(A)可以交换
    // T(A×B) = Q(B)*S(B)*T(A)*-Q(B) + T(B)
    
    SceneObjectTransform output;
    output.Rotation = Other.Rotation * this->Rotation;
    output.Scale3D = Other.Scale3D * this->Scale3D;
    output.Translation = Other.Rotation * (Other.Scale3D * this->Translation) + Other.Translation;
    return output;
}
und::SceneObjectTransform und::SceneObjectTransform::CalcRelativeTransform(const SceneObjectTransform &Other)
{
    // this = output * Other，output = this * Other(-1)
    // 比如child_worldtrans = child_relative * parent_worldtrans，要求child_relative，
    // 那就child_worldtrans.CalcRelativeTransform(parent_worldtrans)
    // 令A = this, B = Other
    // QST(A) = Q(A)*S(A)*P*Q(A)(-1) + T(A)
    // QST(B)(-1) = Q(B)(-1)*S(B)(-1)*(P-TB)*Q(B)
    // QST(output) = QST(A) * QST(B)(-1) = Q(B)(-1)*Q(A)*S(A)*S(B)(-1)*P*Q(A)(-1)*Q(B) + Q(B)(-1)*S(B)(-1)*[T(A)-T(B)]*Q(B)
    // 所以
    // Q(output) = Q(B)(-1)*Q(A)
    // S(output) = S(A)*S(B)(-1)
    // T(output) = Q(B)(-1)*S(B)(-1)*[T(A)-T(B)]*Q(B)
    glm::quat InverseQB = glm::inverse(Other.Rotation);
    glm::vec3 InverseSB = glm::vec3(1.f, 1.f, 1.f) / Other.Scale3D;
    SceneObjectTransform output;
    output.Rotation = InverseQB * this->Rotation;
    output.Scale3D = InverseSB * this->Scale3D;
    output.Translation = InverseQB * (InverseSB * (this->Translation - Other.Translation));
    return output;
}
glm::mat4 und::SceneObjectTransform::ToMatrix()
{
    glm::mat4 scale_mat{
        Scale3D.x, 0, 0, 0,
        0, Scale3D.y, 0, 0,
        0, 0, Scale3D.z, 0,
        0, 0, 0, 1
    };
    glm::mat4 rotation_mat = glm::mat4_cast(Rotation);
    glm::mat4 tranlation_mat{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        Translation.x, Translation.y, Translation.z, 1
    };

    return tranlation_mat * rotation_mat * scale_mat;
}
//void und::SceneObjectTransform::Multiply(SceneObjectTransform *Output, SceneObjectTransform *A, SceneObjectTransform *B)
//{
//}
//und::SceneObjectTransform::SceneObjectTransform(const glm::mat4 &matrix)
//{
//    SetFromMatrix(matrix);
//}
//void und::SceneObjectTransform::SetFromMatrix(const glm::mat4 &matrix)
//{
//
//    Rotation = glm::quat(matrix);
//
//}
und::Rotator::Rotator() : Pitch(0.f), Yaw(0.f), Roll(0.f) {}
und::Rotator::Rotator(const glm::vec3 &eulerAngles)
{
    Pitch = eulerAngles.x;
    Yaw = eulerAngles.y;
    Roll = eulerAngles.z;
}
und::Rotator::Rotator(float pitch, float yaw, float roll) : Pitch(pitch), Yaw(yaw), Roll(roll) {}

und::Rotator::Rotator(const glm::quat &rotation)
{
    glm::vec3 raw_angles = glm::eulerAngles(rotation);
    if (raw_angles.x < glm::radians(-90.f) || raw_angles.x > glm::radians(90.f))
    {
        raw_angles.y = glm::radians(180.f) - raw_angles.y;
    }
    else
    {
        if (raw_angles.y < 0.f)
            raw_angles.y = glm::radians(360.f) + raw_angles.y;
    }

    if (raw_angles.x < glm::radians(-90.f))
    {
        raw_angles.x += glm::radians(180.f);
    }
    else if (raw_angles.x > glm::radians(90.f))
    {
        raw_angles.x -= glm::radians(180.f);
    }

    Pitch = raw_angles.x;
    Yaw = raw_angles.y;
    Roll = raw_angles.z;
}
glm::quat und::Rotator::ToQuat() const

{
    //Rotator temp = Rotator(*this);
    //if (90.f < glm::degrees(temp.Yaw) && glm::degrees(temp.Yaw) < 270.f)
    //{
    //    if (temp.Pitch > 0.f)
    //        temp.Pitch -= glm::radians(90.f);
    //    else if (temp.Pitch < 0.f)
    //        temp.Pitch += glm::radians(180.f);
    //}
    //if (90.f < glm::degrees(temp.Yaw) && glm::degrees(temp.Yaw) < 270.f)
    //{
    //    temp.Yaw = glm::radians(180.f) - temp.Yaw;
    //}
    //else if (270.f < glm::degrees(temp.Yaw))
    //{
    //    temp.Yaw = temp.Yaw - glm::radians(360.f);
    //}
    return glm::quat(glm::vec3(Pitch, -Roll, Yaw));
}
namespace und {
    // 这里只能包在namespace里面，如果这样写std::ostream &und::operator<<(std::ostream &out, const SceneObjectTransform &obj)会报错
    std::ostream &operator<<(std::ostream &out, const SceneObjectTransform &obj)
    {
        out << "Scale: " << obj.Scale3D << std::endl;
        out << "Rotation: " << obj.Rotation << std::endl;
        out << "Translation: " << obj.Translation << std::endl;

        return out;
    }
}

