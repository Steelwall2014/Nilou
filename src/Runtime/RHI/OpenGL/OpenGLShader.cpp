#include <iostream>
#include <memory>

#include "OpenGLShader.h"
#include "RHIResources.h"


nilou::OpenGLLinkedProgram::OpenGLLinkedProgram(OpenGLComputeShader *InComputeShader)
    : ComputeShader(InComputeShader)
{
    // OpenGLComputeShader *GLComputeShader = static_cast<OpenGLComputeShader*>(InComputeShader);
    Resource = glCreateProgram();
    glAttachShader(Resource, ComputeShader->Resource);
    glLinkProgram(Resource);
}

nilou::OpenGLLinkedProgram::OpenGLLinkedProgram(OpenGLVertexShader *InVertexShader, OpenGLPixelShader *InPixelShader)
    : VertexShader(InVertexShader)
    , PixelShader(InPixelShader)
{
    // OpenGLVertexShader *GLVertexShader = static_cast<OpenGLVertexShader*>(InVertexShader);
    // OpenGLPixelShader *GLPixelShader = static_cast<OpenGLPixelShader*>(InPixelShader);
    Resource = glCreateProgram();
    glAttachShader(Resource, VertexShader->Resource);
    glAttachShader(Resource, PixelShader->Resource);
    glLinkProgram(Resource);
}

nilou::OpenGLLinkedProgram::~OpenGLLinkedProgram()
{
    glDeleteProgram(Resource);
}
