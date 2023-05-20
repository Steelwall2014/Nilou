#include "Common/Transform.h"

using namespace std;
using namespace nilou;

int main()
{
    FTransform transA;
    transA.SetRotation(glm::angleAxis(glm::radians(90.f), glm::normalize(vec3(0, 1, 0))));

    FTransform transB;
    transB.SetRotation(glm::angleAxis(glm::radians(90.f), glm::normalize(vec3(0, 0, 1))));

    FTransform tranAB;
    tranAB = transA * transB;

    auto res = transA.TransformVectorNoScale(vec3(1, 0, 0));
    return 0;
}