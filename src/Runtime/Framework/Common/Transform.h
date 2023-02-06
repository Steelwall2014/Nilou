#pragma once

#include <ostream>
#include "Common/Maths.h"

namespace nilou {
    
	const glm::vec3 WORLD_UP(0.f, 0.f, 1.f);
	const glm::vec3 WORLD_FORWARD(1.f, 0.f, 0.f);
	const glm::vec3 WORLD_RIGHT(0.f, 1.f, 0.f);

    inline bool Equals(const vec3 &A, const vec3 &B, float Tolerance=KINDA_SMALL_NUMBER)
    {
        return glm::abs(A.x-B.x) <= Tolerance && glm::abs(A.y-B.y) <= Tolerance && glm::abs(A.z-B.z) <= Tolerance;
    }

    enum ECoordAxis
    {
        CA_X,
        CA_Y,
        CA_Z
    };
    struct FRotator
    {
        // /** Rotation around the right axis (around X axis), Looking up and down (0=Straight Ahead, +Up, -Down) */
        // float Pitch;

        // /** Rotation around the up axis (around Z axis), Running in circles 0=East, +North, -South. */
        // float Yaw;

        // /** Rotation around the forward axis (around Y axis), Tilting your head, 0=Straight, +Clockwise, -CCW. */
        // float Roll;

        /** Rotation around the right axis (around Y axis), Looking up and down (0=Straight Ahead, +Up, -Down) */
	    float Pitch;

	    /** Rotation around the up axis (around Z axis), Running in circles 0=East (positive X axis) , +North, -South. */
	    float Yaw;

	    /** Rotation around the forward axis (around X axis), Tilting your head, 0=Straight, +Clockwise, -CCW. */
	    float Roll;

        FRotator();
        FRotator(float pitch, float yaw, float roll);
        explicit FRotator(const vec3 &eulerAngles);
        explicit FRotator(const quat &rotation);

        static float NormalizeAxis(float Angle);
        static float ClampAxis(float Angle);

        quat ToQuat() const;
        bool Equals(const FRotator &B, float Tolerance=KINDA_SMALL_NUMBER) const
        {
            return glm::abs(Pitch-B.Pitch) <= Tolerance && glm::abs(Yaw-B.Yaw) <= Tolerance && glm::abs(Roll-B.Roll) <= Tolerance;
        }

        static FRotator ZeroRotator;
    };

    glm::vec3 RotateVector(const quat &rotation, const vec3 &V);

    // 变换
    // 对于位置向量，变换是Scale -> Rotate -> Translate
    // 对于方向向量，变换是Scale -> Rotate
    // 当transform连乘时，比如C = B * A，C代表先应用A，再应用B后的结果，和opengl的matrix相同
    class FTransform
    {
    protected:
        quat Rotation;
        vec3 Translation;
        vec3 Scale3D;

    public:
        FTransform();
        FTransform(const quat &rotation);
        FTransform(const vec3 &translation);
        FTransform(const vec3 &scale3d, const quat &rotation, const vec3 &translation);
        FTransform(const mat4 &matrix);

        static FTransform Identity;

        void SetFromMatrix(const mat4 &matrix);
        vec3 GetUnitAxis(ECoordAxis axis) const;

        vec3 TransformPosition(const vec3 &v) const;
        vec3 TransformPositionNoScale(const vec3 &v) const;
        vec3 TransformVector(const vec3 &v) const;
        vec3 TransformVectorNoScale(const vec3 &v) const;
        vec3 InverseTransformPosition(const vec3 &v) const;
        vec3 InverseTransformPositionNoScale(const vec3 &v) const;

        vec3 GetScale3D() const;
        quat GetRotation() const;
        FRotator GetRotator() const;
        vec3 GetTranslation() const;
        inline vec3 GetLocation() const
        {
            return GetTranslation();
        }

        void SetScale3D(const vec3 &scale);
        void SetRotation(const quat &rotation);
        void SetRotator(const FRotator &rotator);
        void SetTranslation(const vec3 &translation);
        FTransform operator*(const FTransform &Other) const;
        FTransform GetRelativeTransform(const FTransform &Other) const;
        float GetMinimumAxisScale() const;
        vec3 GetSafeScaleReciprocal(const vec3 &InScale, float Tolerance = SMALL_NUMBER);

        static bool AnyHasNegativeScale(const vec3 &InScale3D, const vec3 &InOtherScale3D);

        mat4 ToMatrix() const;
        //static void Multiply(SceneObjectTransform *Output, SceneObjectTransform *A, SceneObjectTransform *B);
        friend std::ostream &operator<<(std::ostream &out, const FTransform &obj);
    };

    inline std::ostream &operator<<(std::ostream &out, const glm::vec2 &obj)
    {
        out << "glm::vec2: " << obj.x << " " << obj.y << " ";
        return out;
    }
    inline std::ostream &operator<<(std::ostream &out, const glm::vec3 &obj)
    {
        out << "glm::vec3: " << obj.x << " " << obj.y << " " << obj.z << " ";
        return out;
    }
    inline std::ostream &operator<<(std::ostream &out, const glm::vec4 &obj)
    {
        out << "glm::vec4: " << obj.x << " " << obj.y << " " << obj.z << " " << obj.z << " ";
        return out;
    }
    inline std::ostream &operator<<(std::ostream &out, const FRotator &obj)
    {
        out << "FRotator: Yaw=" << obj.Yaw << " Pitch=" << obj.Pitch << " Roll=" << obj.Roll << " ";
        return out;
    }
    inline std::ostream &operator<<(std::ostream &out, const quat &obj)
    {
        out << "quat: x=" << obj.x << " y=" << obj.y << " z=" << obj.z << " w=" << obj.w << " ";
        return out;
    }
}