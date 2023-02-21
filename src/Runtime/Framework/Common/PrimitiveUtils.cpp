#include "PrimitiveUtils.h"
#include "Transform.h"

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

    void GetOrientedHalfSphereMesh(const vec3& Center, const FRotator& Orientation, const vec3& Radii, int32 NumSides, int32 NumRings, float StartAngle, float EndAngle, std::vector<FDynamicMeshVertex>& OutVerts, std::vector<uint32>& OutIndices)
    {
        // Use a mesh builder to draw the sphere.
        //FDynamicMeshBuilder MeshBuilder(Collector.GetFeatureLevel());
        {
            // The first/last arc are on top of each other.
            int32 NumVerts = (NumSides + 1) * (NumRings + 1);
            FDynamicMeshVertex* Verts = new FDynamicMeshVertex[NumVerts];

            // Calculate verts for one arc
            FDynamicMeshVertex* ArcVerts = new FDynamicMeshVertex[NumRings + 1];

            for (int32 i = 0; i<NumRings + 1; i++)
            {
                FDynamicMeshVertex* ArcVert = &ArcVerts[i];

                float angle = StartAngle + ((float)i / NumRings) * (EndAngle-StartAngle);

                // Note- unit sphere, so position always has mag of one. We can just use it for normal!			
                ArcVert->Position.x = 0.0f;
                ArcVert->Position.y = glm::sin(angle);
                ArcVert->Position.z = glm::cos(angle);

                ArcVert->SetTangents(
                    vec3(1, 0, 0),
                    vec3(0.0f, -ArcVert->Position.z, ArcVert->Position.y),
                    ArcVert->Position
                    );

                ArcVert->TextureCoordinate[0].x = 0.0f;
                ArcVert->TextureCoordinate[0].y = ((float)i / NumRings);
            }

            // Then rotate this arc NumSides+1 times.
            for (int32 s = 0; s<NumSides + 1; s++)
            {
                FRotator ArcRotator(0.0, 360.0 * s / NumSides, 0.0);
                dmat4 ArcRot = mat4_cast(ArcRotator.ToQuat());
                float XTexCoord = ((float)s / NumSides);

                for (int32 v = 0; v<NumRings + 1; v++)
                {
                    int32 VIx = (NumRings + 1)*s + v;

                    Verts[VIx].Position = ArcRot * vec4(ArcVerts[v].Position, 1);

                    Verts[VIx].SetTangents(
                        ArcRot * vec4(vec3(ArcVerts[v].Tangent), 0),
                        ArcRot * vec4(ArcVerts[v].GetTangentY(), 0),
                        ArcRot * vec4(vec3(ArcVerts[v].Normal), 0)
                        );

                    Verts[VIx].TextureCoordinate[0].x = XTexCoord;
                    Verts[VIx].TextureCoordinate[0].y = ArcVerts[v].TextureCoordinate[0].y;
                }
            }

            // Add all of the vertices we generated to the mesh builder.
            for (int32 VertIdx = 0; VertIdx < NumVerts; VertIdx++)
            {
                OutVerts.push_back(Verts[VertIdx]);
            }

            // Add all of the triangles we generated to the mesh builder.
            for (int32 s = 0; s<NumSides; s++)
            {
                int32 a0start = (s + 0) * (NumRings + 1);
                int32 a1start = (s + 1) * (NumRings + 1);

                for (int32 r = 0; r<NumRings; r++)
                {
                    OutIndices.push_back(a0start + r + 0);
                    OutIndices.push_back(a1start + r + 0);
                    OutIndices.push_back(a0start + r + 1);
                    OutIndices.push_back(a1start + r + 0);
                    OutIndices.push_back(a1start + r + 1);
                    OutIndices.push_back(a0start + r + 1);
                }
            }

            // Free our local copy of verts and arc verts
            delete Verts;
            delete ArcVerts;

            FTransform LocalToWorldTransform(Radii, Orientation.ToQuat(), Center);
            mat4 LocalToWorld = LocalToWorldTransform.ToMatrix();
            mat3 mat3_LocalToWorld = mat3(LocalToWorld);
            mat3 invtrans_LocalToWorld = mat3(glm::transpose(glm::inverse(LocalToWorld)));
            for (int32 VertIdx = 0; VertIdx < NumVerts; VertIdx++)
            {
                OutVerts[VertIdx].Position = LocalToWorld * vec4(OutVerts[VertIdx].Position, 1);
                vec3 Normal = glm::normalize(invtrans_LocalToWorld * vec3(OutVerts[VertIdx].Normal));
                vec3 Tangent = glm::normalize(mat3_LocalToWorld * vec3(OutVerts[VertIdx].Tangent));
                vec3 TangentY = glm::cross(Normal, Tangent);
                OutVerts[VertIdx].SetTangents(Tangent, TangentY, Normal);
            }   
        }
     
        // MeshBuilder.GetMesh(FScaleMatrix(Radii) * FRotationMatrix(Orientation) * FTranslationMatrix(Center), MaterialRenderProxy, DepthPriority, bDisableBackfaceCulling, false, bUseSelectionOutline, ViewIndex, Collector, HitProxy);
    }

    void GetHalfSphereMesh(const vec3& Center, const FRotator& Orientation, const vec3& Radii, int32 NumSides, int32 NumRings, std::vector<FDynamicMeshVertex>& OutVerts, std::vector<uint32>& OutIndices)
    {
        GetOrientedHalfSphereMesh(Center, Orientation, Radii, NumSides, NumRings, 0, PI / 2, OutVerts, OutIndices);
    }

    void GetSphereMesh(const vec3& Center, const FRotator& Orientation, const vec3& Radii, int32 NumSides, int32 NumRings, std::vector<FDynamicMeshVertex>& OutVerts, std::vector<uint32>& OutIndices)
    {
        GetOrientedHalfSphereMesh(Center, Orientation, Radii, NumSides, NumRings, 0, PI, OutVerts, OutIndices);
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
            V1.Position = ConeVerts[i];
            V2.Position = ConeVerts[(i + 1) % NumSides];
            // vec3 V3_Position = ConeVerts[(i + 2) % NumSides];
            vec3 V0V1 = V1.Position-V0.Position;
            vec3 V0V2 = V2.Position-V0.Position;
            // vec3 V0V3 = V3_Position-V0.Position;
            vec3 V1V2 = V2.Position-V1.Position;
            // vec3 V2V3 = V3_Position-V2.Position;

            // UE圆锥体生成的法线在圆锥顶是错的
            vec3 normal = glm::cross(V0V1, V0V2);
            V0.TextureCoordinate[0].x = 0.0f;
            V0.TextureCoordinate[0].y = (float)i / NumSides;
            V0.SetTangents(V1V2, glm::cross(normal, V1V2), normal);
            int32 I0 = OutVerts.size();
            OutVerts.push_back(V0);

            V1.TextureCoordinate[0].x = 1.0f;
            V1.TextureCoordinate[0].y = (float)i / NumSides;
            vec3 TriTangentZPrev = glm::cross(ConeVerts[i], ConeVerts[i == 0 ? NumSides - 1 : i - 1]); // Normal of the previous face connected to this face
            V1.SetTangents(TriTangentX, TriTangentY, TriTangentZPrev + TriTangentZ);
            int32 I1 = OutVerts.size();
            OutVerts.push_back(V1);

            V2.TextureCoordinate[0].x = 1.0f;
            V2.TextureCoordinate[0].y = (float)((i + 1) % NumSides) / NumSides;
            vec3 TriTangentZNext = glm::cross(ConeVerts[(i + 2) % NumSides], ConeVerts[(i + 1) % NumSides]); // Normal of the next face connected to this face
            V2.SetTangents(TriTangentX, TriTangentY, TriTangentZNext + TriTangentZ);
            int32 I2 = OutVerts.size();
            OutVerts.push_back(V2);

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

    void BuildCuboidVerts(float width, float length, float height, std::vector<FDynamicMeshVertex>& OutVerts, std::vector<uint32>& OutIndices)
    {
        /**                              z
         *       _______________         |   x
         *      /|    5        /|        |  /
         *     /_________2____/ |        | /
         *     |3|____________|1|        |/________ y
         *     | /     0      | /
         *     |/______4______|/
         * 
         *     0-4 are side faces.
         *     4 is bottom, 5 is top.
         */


        vec3 halfAxis = vec3(width/2, length/2, height/2);
        FDynamicMeshVertex vertex0, vertex1, vertex2, vertex3;
        vertex0.TextureCoordinate[0] = vec2(0, 1);
        vertex1.TextureCoordinate[0] = vec2(1, 1);
        vertex2.TextureCoordinate[0] = vec2(1, 0);
        vertex3.TextureCoordinate[0] = vec2(0, 0);
        vec3 normal;
        vec3 tangentx;
        vec3 tangenty;

        for (int face_index = 0; face_index < 6; face_index++)
        {
            OutIndices.push_back(face_index*4 + 0);
            OutIndices.push_back(face_index*4 + 1);
            OutIndices.push_back(face_index*4 + 2);
            OutIndices.push_back(face_index*4 + 0);
            OutIndices.push_back(face_index*4 + 2);
            OutIndices.push_back(face_index*4 + 3);
        }
        
        {   // face 0
            normal = vec3(-1, 0, 0);
            tangentx = vec3(0, 1, 0);
            tangenty = vec3(0, 0, -1);
            vertex0.Position = halfAxis * vec3(-1, -1, -1);
            vertex0.SetTangents(tangentx, tangenty, normal);
            vertex1.Position = halfAxis * vec3(-1, 1, -1);
            vertex1.SetTangents(tangentx, tangenty, normal);
            vertex2.Position = halfAxis * vec3(-1, 1, 1);
            vertex2.SetTangents(tangentx, tangenty, normal);
            vertex3.Position = halfAxis * vec3(-1, -1, 1);
            vertex3.SetTangents(tangentx, tangenty, normal);
            OutVerts.push_back(vertex0);
            OutVerts.push_back(vertex1);
            OutVerts.push_back(vertex2);
            OutVerts.push_back(vertex3);
        }
        {   // face 1
            normal = vec3(0, 1, 0);
            tangentx = vec3(1, 0, 0);
            tangenty = vec3(0, 0, -1);
            vertex0.Position = halfAxis * vec3(-1, 1, -1);
            vertex0.SetTangents(tangentx, tangenty, normal);
            vertex1.Position = halfAxis * vec3(1, 1, -1);
            vertex1.SetTangents(tangentx, tangenty, normal);
            vertex2.Position = halfAxis * vec3(1, 1, 1);
            vertex2.SetTangents(tangentx, tangenty, normal);
            vertex3.Position = halfAxis * vec3(-1, 1, 1);
            vertex3.SetTangents(tangentx, tangenty, normal);
            OutVerts.push_back(vertex0);
            OutVerts.push_back(vertex1);
            OutVerts.push_back(vertex2);
            OutVerts.push_back(vertex3);
        }

        {   // face 2
            normal = vec3(1, 0, 0);
            tangentx = vec3(0, -1, 0);
            tangenty = vec3(0, 0, -1);
            vertex0.Position = halfAxis * vec3(1, 1, -1);
            vertex0.SetTangents(tangentx, tangenty, normal);
            vertex1.Position = halfAxis * vec3(1, -1, -1);
            vertex1.SetTangents(tangentx, tangenty, normal);
            vertex2.Position = halfAxis * vec3(1, -1, 1);
            vertex2.SetTangents(tangentx, tangenty, normal);
            vertex3.Position = halfAxis * vec3(1, 1, 1);
            vertex3.SetTangents(tangentx, tangenty, normal);
            OutVerts.push_back(vertex0);
            OutVerts.push_back(vertex1);
            OutVerts.push_back(vertex2);
            OutVerts.push_back(vertex3);
        }

        {   // face 3
            normal = vec3(0, -1, 0);
            tangentx = vec3(-1, 0, 0);
            tangenty = vec3(0, 0, -1);
            vertex0.Position = halfAxis * vec3(1, -1, -1);
            vertex0.SetTangents(tangentx, tangenty, normal);
            vertex1.Position = halfAxis * vec3(-1, -1, -1);
            vertex1.SetTangents(tangentx, tangenty, normal);
            vertex2.Position = halfAxis * vec3(-1, -1, 1);
            vertex2.SetTangents(tangentx, tangenty, normal);
            vertex3.Position = halfAxis * vec3(1, -1, 1);
            vertex3.SetTangents(tangentx, tangenty, normal);
            OutVerts.push_back(vertex0);
            OutVerts.push_back(vertex1);
            OutVerts.push_back(vertex2);
            OutVerts.push_back(vertex3);
        }

        {   // face 4
            normal = vec3(0, 0, -1);
            tangentx = vec3(-1, 0, 0);
            tangenty = vec3(0, 1, 0);
            vertex0.Position = halfAxis * vec3(1, 1, -1);
            vertex0.SetTangents(tangentx, tangenty, normal);
            vertex1.Position = halfAxis * vec3(-1, 1, -1);
            vertex1.SetTangents(tangentx, tangenty, normal);
            vertex2.Position = halfAxis * vec3(-1, -1, -1);
            vertex2.SetTangents(tangentx, tangenty, normal);
            vertex3.Position = halfAxis * vec3(1, -1, -1);
            vertex3.SetTangents(tangentx, tangenty, normal);
            OutVerts.push_back(vertex0);
            OutVerts.push_back(vertex1);
            OutVerts.push_back(vertex2);
            OutVerts.push_back(vertex3);
        }

        {   // face 5
            normal = vec3(0, 0, 1);
            tangentx = vec3(1, 0, 0);
            tangenty = vec3(0, 1, 0);
            vertex0.Position = halfAxis * vec3(-1, 1, 1);
            vertex0.SetTangents(tangentx, tangenty, normal);
            vertex1.Position = halfAxis * vec3(1, 1, 1);
            vertex1.SetTangents(tangentx, tangenty, normal);
            vertex2.Position = halfAxis * vec3(1, -1, 1);
            vertex2.SetTangents(tangentx, tangenty, normal);
            vertex3.Position = halfAxis * vec3(-1, -1, 1);
            vertex3.SetTangents(tangentx, tangenty, normal);
            OutVerts.push_back(vertex0);
            OutVerts.push_back(vertex1);
            OutVerts.push_back(vertex2);
            OutVerts.push_back(vertex3);
        }
    }
}