#include "InheritanceGraph.h"
#include <queue>

namespace nilou {

    bool FInheritanceGraph::IsDerived(EUClasses DerivedClassEnum, EUClasses BaseClassEnum)
    {
        if (DerivedClassEnum == BaseClassEnum)
            return true;
        std::queue<FInheritanceNode *> q;
        q.push(Nodes[DerivedClassEnum]);
        while (!q.empty())
        {
            FInheritanceNode *temp_class = q.front(); q.pop();
            for (FInheritanceNode *parent_class_node : temp_class->ParentClasses)
            {
                if (parent_class_node->ClassEnum == BaseClassEnum)
                    return true;
                q.push(parent_class_node);
            }
        }
        return false;
    }

    void FInheritanceGraph::AddEdge(EUClasses ParentClassEnum, EUClasses DerivedClassEnum)
    {
        AddNode(ParentClassEnum);
        AddNode(DerivedClassEnum);

        Nodes[ParentClassEnum]->DerivedClasses.insert(Nodes[DerivedClassEnum]);
        Nodes[DerivedClassEnum]->ParentClasses.insert(Nodes[ParentClassEnum]);
    }

    void FInheritanceGraph::AddNode(EUClasses ClassEnum)
    {
        if (Nodes.find(ClassEnum) == Nodes.end())
        {
            Nodes[ClassEnum] = new FInheritanceNode(ClassEnum);
        }
    }

}