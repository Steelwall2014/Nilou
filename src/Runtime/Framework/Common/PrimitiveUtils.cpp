#include "PrimitiveUtils.h"

namespace nilou {


    // vec3 unreal_vec3_xor(const vec3& this_, const vec3& V)
    // {
    //     return vec3
    //         (
    //         this_.y * V.z - this_.z * V.y,
    //         this_.z * V.x - this_.x * V.z,
    //         this_.x * V.y - this_.y * V.x
    //         );
    // }

    vec3 GenerateYAxis(const vec4& XAxis, const vec4& ZAxis)
    {
        vec3 x = vec3(XAxis);
        vec3 z = vec3(ZAxis);
        return glm::cross(x, z) * ZAxis.w;
    }

    float GetBasisDeterminantSign( const vec3& XAxis, const vec3& YAxis, const vec3& ZAxis )
    {
        mat4 Basis(
            vec4(XAxis,0),
            vec4(YAxis,0),
            vec4(ZAxis,0),
            vec4(0,0,0,1)
            );
        return (glm::determinant(Basis) < 0) ? -1.0f : +1.0f;
    }

    vec3 CalcConeVert(float Angle1, float Angle2, float AzimuthAngle)
    {
        float ang1 = glm::clamp<float>(Angle1, 0.01f, (float)PI - 0.01f);
        float ang2 = glm::clamp<float>(Angle2, 0.01f, (float)PI - 0.01f);

        float sinX_2 = glm::sin(0.5f * ang1);
        float sinY_2 = glm::sin(0.5f * ang2);

        float sinSqX_2 = sinX_2 * sinX_2;
        float sinSqY_2 = sinY_2 * sinY_2;

        float tanX_2 = glm::tan(0.5f * ang1);
        float tanY_2 = glm::tan(0.5f * ang2);


        float phi = std::atan2(glm::sin(AzimuthAngle)*sinY_2, glm::cos(AzimuthAngle)*sinX_2);
        float sinPhi = glm::sin(phi);
        float cosPhi = glm::cos(phi);
        float sinSqPhi = sinPhi*sinPhi;
        float cosSqPhi = cosPhi*cosPhi;

        float rSq, r, Sqr, alpha, beta;

        rSq = sinSqX_2*sinSqY_2 / (sinSqX_2*sinSqPhi + sinSqY_2*cosSqPhi);
        r = glm::sqrt(rSq);
        Sqr = glm::sqrt(1 - rSq);
        alpha = r*cosPhi;
        beta = r*sinPhi;

        vec3 ConeVert;

        ConeVert.x = (1 - 2 * rSq);
        ConeVert.y = 2 * Sqr*alpha;
        ConeVert.z = 2 * Sqr*beta;

        return ConeVert;
    }

    void BuildConeVerts(float Angle1, float Angle2, float Scale, float XOffset, uint32 NumSides, vec4 Color, std::vector<FDynamicMeshVertex>& OutVerts, std::vector<uint32>& OutIndices)
    {
        std::vector<vec3> ConeVerts;
        ConeVerts.resize(NumSides);

        for (uint32 i = 0; i < NumSides; i++)
        {
            float Fraction = (float)i / (float)(NumSides);
            float Azi = 2.f*PI*Fraction;
            ConeVerts[i] = (CalcConeVert(Angle1, Angle2, Azi) * Scale) + vec3(XOffset,0,0);
        }

        for (uint32 i = 0; i < NumSides; i++)
        {
            // Normal of the current face 
            vec3 TriTangentZ = glm::cross(ConeVerts[(i + 1) % NumSides], ConeVerts[i]); // aka triangle normal
            vec3 TriTangentY = ConeVerts[i];
            vec3 TriTangentX = glm::cross(TriTangentZ, TriTangentY);


            FDynamicMeshVertex V0, V1, V2;

            V0.Color = Color;
            V1.Color = Color;
            V2.Color = Color;

            V0.Position = vec3(0) + vec3(XOffset,0,0);
            V0.TextureCoordinate[0].x = 0.0f;
            V0.TextureCoordinate[0].y = (float)i / NumSides;
            V0.SetTangents(TriTangentX, TriTangentY, vec3(-1, 0, 0));
            int32 I0 = OutVerts.size();
            OutVerts.push_back(V0);

            V1.Position = ConeVerts[i];
            V1.TextureCoordinate[0].x = 1.0f;
            V1.TextureCoordinate[0].y = (float)i / NumSides;
            vec3 TriTangentZPrev = glm::cross(ConeVerts[i], ConeVerts[i == 0 ? NumSides - 1 : i - 1]); // Normal of the previous face connected to this face
            V1.SetTangents(TriTangentX, TriTangentY, glm::normalize(TriTangentZPrev + TriTangentZ));
            int32 I1 = OutVerts.size();
            OutVerts.push_back(V1);

            V2.Position = ConeVerts[(i + 1) % NumSides];
            V2.TextureCoordinate[0].x = 1.0f;
            V2.TextureCoordinate[0].y = (float)((i + 1) % NumSides) / NumSides;
            vec3 TriTangentZNext = glm::cross(ConeVerts[(i + 2) % NumSides], ConeVerts[(i + 1) % NumSides]); // Normal of the next face connected to this face
            V2.SetTangents(TriTangentX, TriTangentY, glm::normalize(TriTangentZNext + TriTangentZ));
            int32 I2 = OutVerts.size();
            OutVerts.push_back(V2);

            // Flip winding for negative scale
            if(Scale >= 0.f)
            {
                OutIndices.push_back(I0);
                OutIndices.push_back(I1);
                OutIndices.push_back(I2);
            }
            else
            {
                OutIndices.push_back(I0);
                OutIndices.push_back(I2);
                OutIndices.push_back(I1);
            }
        }
    }

    void BuildCylinderVerts(const vec3 &Base, const vec3 &XAxis, const vec3 &YAxis, const vec3 &ZAxis, float Radius, float HalfHeight, uint32 Sides, vec4 Color, std::vector<FDynamicMeshVertex> &OutVerts, std::vector<uint32> &OutIndices)
    {
        const float	AngleDelta = 2.0f * PI / Sides;
        vec3 LastVertex = Base + XAxis * Radius;

        vec2 TC = vec2(0.0f, 0.0f);
        float TCStep = 1.0f / Sides;

        vec3 TopOffset = HalfHeight * ZAxis;

        int32 BaseVertIndex = OutVerts.size();

        //Compute vertices for base circle.
        for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
        {
            const vec3 Vertex = Base + (XAxis * glm::cos(AngleDelta * (SideIndex + 1)) + YAxis * glm::sin(AngleDelta * (SideIndex + 1))) * Radius;
            vec3 Normal = Vertex - Base;
            Normal = glm::normalize(Normal);

            FDynamicMeshVertex MeshVertex;

            MeshVertex.Position = vec3(Vertex - TopOffset);
            MeshVertex.TextureCoordinate[0] = vec2(TC);

            MeshVertex.SetTangents(
                -ZAxis,
                glm::cross(-ZAxis, Normal),
                Normal
            );

            MeshVertex.Color = Color;

            OutVerts.push_back(MeshVertex); //Add bottom vertex

            LastVertex = Vertex;
            TC.x += TCStep;
        }

        LastVertex = Base + XAxis * Radius;
        TC = vec2(0.0f, 1.0f);

        //Compute vertices for the top circle
        for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
        {
            const vec3 Vertex = Base + (XAxis * glm::cos(AngleDelta * (SideIndex + 1)) + YAxis * glm::sin(AngleDelta * (SideIndex + 1))) * Radius;
            vec3 Normal = Vertex - Base;
            Normal = glm::normalize(Normal);

            FDynamicMeshVertex MeshVertex;

            MeshVertex.Position = vec3(Vertex + TopOffset);
            MeshVertex.TextureCoordinate[0] = vec2(TC);

            MeshVertex.SetTangents(
                -ZAxis,
                glm::cross(-ZAxis, Normal),
                Normal
            );

            MeshVertex.Color = Color;

            OutVerts.push_back(MeshVertex); //Add top vertex

            LastVertex = Vertex;
            TC.x += TCStep;
        }

        //Add top/bottom triangles, in the style of a fan.
        //Note if we wanted nice rendering of the caps then we need to duplicate the vertices and modify
        //texture/tangent coordinates.
        for (uint32 SideIndex = 1; SideIndex < Sides; SideIndex++)
        {
            int32 V0 = BaseVertIndex;
            int32 V1 = BaseVertIndex + SideIndex;
            int32 V2 = BaseVertIndex + ((SideIndex + 1) % Sides);

            //bottom
            OutIndices.push_back(V0);
            OutIndices.push_back(V1);
            OutIndices.push_back(V2);

            // top
            OutIndices.push_back(Sides + V2);
            OutIndices.push_back(Sides + V1);
            OutIndices.push_back(Sides + V0);
        }

        //Add sides.

        for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
        {
            int32 V0 = BaseVertIndex + SideIndex;
            int32 V1 = BaseVertIndex + ((SideIndex + 1) % Sides);
            int32 V2 = V0 + Sides;
            int32 V3 = V1 + Sides;

            OutIndices.push_back(V0);
            OutIndices.push_back(V2);
            OutIndices.push_back(V1);

            OutIndices.push_back(V2);
            OutIndices.push_back(V3);
            OutIndices.push_back(V1);
        }

    }
}