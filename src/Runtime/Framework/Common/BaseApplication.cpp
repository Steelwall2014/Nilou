#include "Common/BaseApplication.h"

bool nilou::BaseApplication::m_bQuit = false;

nilou::BaseApplication::BaseApplication(GfxConfiguration &cfg) :
    m_Config(cfg)
{
}

int nilou::BaseApplication::Initialize()
{
    m_bQuit = false;

    return 0;
}


void nilou::BaseApplication::Finalize()
{
}


void nilou::BaseApplication::Tick(double DeltaTime)
{
}

bool nilou::BaseApplication::IsQuit()
{
    return m_bQuit;
}

nilou::GfxConfiguration &nilou::BaseApplication::GetConfiguration()
{
    return m_Config;
}

void nilou::BaseApplication::SetWindowWidth(int width)
{
    m_Config.screenWidth = width;
}

void nilou::BaseApplication::SetWindowHeight(int height)
{
    m_Config.screenHeight = height;
}

float nilou::BaseApplication::GetTimeSinceStart()
{
    return accumTime;
}

bool nilou::BaseApplication::IsCursorEnabled()
{
    return CursorEnabled;
}