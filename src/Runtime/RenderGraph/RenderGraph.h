#pragma once
#include <vector>

#include "ShaderType.h"

namespace nilou {

class FRDGResourceNode
{
};

class FRDGPassNode
{
public:
    std::vector<FRDGResourceNode*> InResourceNodes;
    std::vector<FRDGResourceNode*> OutResourceNodes;
    std::vector<FRDGResourceNode*> InOutResourceNodes;

};

class FRenderGraph
{
protected:

private:

    friend class FRDGBuilder;

    std::vector<FRDGPassNode*> Passes;

    std::vector<FRDGResourceNode*> Resources;

};

}