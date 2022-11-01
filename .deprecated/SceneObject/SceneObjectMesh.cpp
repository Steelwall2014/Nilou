#include "SceneObjectMesh.h"

#define INDENT std::string(indent, ' ') << "|"
// 只是为了少打几个字和好修改

und::SceneObjectMesh::SceneObjectMesh(tinygltf::Mesh &mesh)
	: BaseSceneObject(SceneObjectType::kSceneObjectTypeMesh)
{

}

void und::SceneObjectMesh::AddPrimitive(const und::SceneObjectMesh::Primitive &primitive)
{
	m_Primitives.push_back(primitive);
}

const std::vector<und::SceneObjectMesh::Primitive> &und::SceneObjectMesh::GetPrimitives() const
{
	return m_Primitives;
}

void und::SceneObjectMesh::dump(std::ostream &out)
{
	und::BaseSceneObject::dump(out);
	for (auto &prim : this->m_Primitives)
	{
		out << prim;
	}
}

void und::SceneObjectMesh::Primitive::AddAttribute(und::SceneObjectMesh::AttriPointer &attr)
{
	Attributes.push_back(attr);
}
namespace und {
	extern thread_local int32_t indent;
	std::ostream &operator<<(std::ostream &out, const SceneObjectMesh::AttriPointer &obj)
	{
		std::string type_str = "";
		switch (obj.type)
		{
		case 5120:
			type_str = "BYTE"; break;
		case 5121:
			type_str = "UNSIGNED_BYTE"; break;
		case 5122:
			type_str = "SHORT"; break;
		case 5123:
			type_str = "UNSIGNED_SHORT"; break;
		case 5125:
			type_str = "UNSIGNED_INT"; break;
		case 5126:
			type_str = "FLOAT"; break;
		default:
			break;
		}

		out << INDENT << "--------------AttriPointer---------------" << std::endl;
		out << INDENT << "name: " << obj.name << std::endl;
		out << INDENT << "size: " << obj.size << std::endl;
		out << INDENT << "type: " << obj.type << "("  << type_str << ")" << std::endl;
		out << INDENT << "normalized: " << obj.normalized << std::endl;
		out << INDENT << "stride: " << obj.stride << std::endl;
		out << INDENT << "offset: " << obj.offset << std::endl;
		out << INDENT << "pVertexAttriArray: " << obj.pVertexAttriArray << std::endl;
		out << INDENT << "VertexAttriArraySize: " << obj.VertexAttriArraySize << std::endl;
		out << INDENT << "^-------------AttriPointer--------------^" << std::endl;
		return out;
	}
	std::ostream &operator<<(std::ostream &out, const SceneObjectMesh::Primitive &obj)
	{
		out << INDENT << "---------------Primitive----------------" << std::endl;
		out << INDENT << "pIndexArray: " << obj.pIndexArray << std::endl;
		out << INDENT << "IndexArraySize: " << obj.IndexArraySize << std::endl;
		for (auto &attr : obj.Attributes)
		{
			out << attr;
		}
		out << INDENT << "^--------------Primitive---------------^" << std::endl;
		return out;
	}
	//std::ostream &operator<<(std::ostream &out, const SceneObjectMesh &obj)
	//{
	//	for (auto &prim : obj.m_Primitives)
	//	{
	//		out << prim;
	//	}
	//	return out;
	//}
}

