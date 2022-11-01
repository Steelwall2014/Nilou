#include "SceneObjectGeometry.h"

void und::SceneObjectGeometry::AddMesh(std::shared_ptr<SceneObjectMesh> &mesh)
{
    m_Mesh.push_back(std::move(mesh));
}

std::vector<std::shared_ptr<und::SceneObjectMesh>> &und::SceneObjectGeometry::GetMeshes()
{
    return m_Mesh;
}

void und::SceneObjectGeometry::dump(std::ostream &out)
{
    auto count = m_Mesh.size();
    for (decltype(count) i = 0; i < count; i++) {
        out << "Mesh: ";
        m_Mesh[i]->dump(out);
        out << std::endl;
    }
}
