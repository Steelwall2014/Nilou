#pragma once

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "MarkedClasses.generated.h"

namespace nilou {

    class FInheritanceNode
    {
    public:
        EUClasses ClassEnum;
        std::set<FInheritanceNode *> ParentClasses;
        std::set<FInheritanceNode *> DerivedClasses;

    };
    
    class FInheritanceGraph
    {
    public:
        static FInheritanceGraph *GetInheritanceGraph();

        /** It is implemented in InheritanceGraph.generated.cpp*/
        FInheritanceGraph();
        ~FInheritanceGraph()
        {
            for (auto &kv : Nodes)
                delete kv.second;
        }

        std::unordered_map<EUClasses, FInheritanceNode *> Nodes;

        bool IsDerived(EUClasses DerivedClassEnum, EUClasses BaseClassEnum);

        void AddEdge(EUClasses ParentClassEnum, EUClasses DerivedClassEnum);

        void AddNode(EUClasses ClassEnum);
    };

}