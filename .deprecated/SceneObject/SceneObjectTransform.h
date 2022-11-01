#pragma once
#include "Common/BaseSceneObject.h"

namespace und {
    enum CoordAxis
    {
        X,
        Y,
        Z
    };
    struct Rotator
    {
        /** Rotation around the right axis (around X axis), Looking up and down (0=Straight Ahead, +Up, -Down) */
        float Pitch;

        /** Rotation around the up axis (around Z axis), Running in circles 0=East, +North, -South. */
        float Yaw;

        /** Rotation around the forward axis (around Y axis), Tilting your head, 0=Straight, +Clockwise, -CCW. */
        float Roll;
        Rotator();
        Rotator(const glm::vec3 &eulerAngles);
        Rotator(float pitch, float yaw, float roll);
        Rotator(const glm::quat &rotation);

        glm::quat ToQuat() const;
    };
    // 变换
    // 对于位置向量，变换是Scale -> Rotate -> Translate
    // 对于方向向量，变换是Scale -> Rotate
    // 当transform连乘时，比如C = A * B，C代表先应用A，再应用B后的结果，和opengl的matrix不同
    class SceneObjectTransform
    {
    protected:
        glm::quat Rotation;
        glm::vec3 Translation;
        glm::vec3 Scale3D;

    public:
        SceneObjectTransform();
        SceneObjectTransform(const glm::quat &rotation);
        SceneObjectTransform(const glm::vec3 &translation);
        SceneObjectTransform(const glm::vec3 &scale3d, const glm::quat &rotation, const glm::vec3 &translation);
        SceneObjectTransform(const glm::mat4 &matrix);
        void SetFromMatrix(const glm::mat4 &matrix);
        glm::vec3 TransformVectorNoScale(const glm::vec3 &v);
        glm::vec3 GetUnitAxis(CoordAxis axis);

        glm::vec3 GetScale3D();
        glm::quat GetRotation();
        glm::vec3 GetTranslation();

        void SetScale3D(const glm::vec3 &scale);
        void SetRotation(const glm::quat &rotation);
        void SetTranslation(const glm::vec3 &translation);
        SceneObjectTransform operator*(const SceneObjectTransform &Other) const;
        SceneObjectTransform CalcRelativeTransform(const SceneObjectTransform &Other);
        glm::mat4 ToMatrix();
        //static void Multiply(SceneObjectTransform *Output, SceneObjectTransform *A, SceneObjectTransform *B);
        friend std::ostream &operator<<(std::ostream &out, const SceneObjectTransform &obj);
    };
}