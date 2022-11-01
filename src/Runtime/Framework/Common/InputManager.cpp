#include "InputManager.h"
#include "Modules/ModuleManager.h"
#include "Templates/ObjectMacros.h"

#include <iostream>

//#include "GraphicsManager.h"
// #include "SceneManager.h"
// #include "Common/BaseActor.h"

using namespace std;

namespace nilou {
    InputManager *g_pInputManager = new InputManager;
    int InputManager::StartupModule() { return 0; }

    void InputManager::ShutdownModule() {}

    // void InputManager::Tick(double DeltaTime) 
    // {

    // }

    void nilou::InputManager::KeyPressed(int key)
    {
        InputKey input_key = (InputKey)key;
        KeyState former_state = m_KeyStates[input_key];
        m_KeyStates[input_key].Value = 1.0;
        m_KeyStates[input_key].bDown = true;
        if (former_state.bDown == false)
        {
            m_KeyStates[input_key].bRepeating = false;
            if (checkHasActionKeyEventBinding(input_key, IE_Pressed))
                for (auto &&dlg : m_ActionDelegateMap[input_key][IE_Pressed])
                    dlg->execute();
        }
        else if (former_state.bDown == true)
        {
            m_KeyStates[input_key].bRepeating = true;
            if (checkHasActionKeyEventBinding(input_key, IE_Repeat))
                for (auto &&dlg : m_ActionDelegateMap[input_key][IE_Repeat])
                    dlg->execute();
        }
        if (checkHasAxisKeyBinding(input_key))
            for (auto &&dlg : m_AxisDelegateMap[input_key])
                dlg->execute(1.0);
    }

    void InputManager::KeyReleased(int key)
    {
        InputKey input_key = (InputKey)key;
        KeyState former_state = m_KeyStates[input_key];
        m_KeyStates[input_key].Value = 0.0;
        m_KeyStates[input_key].bDown = false;
        m_KeyStates[input_key].bRepeating = false;
        if (former_state.bDown == true)
        {
            if (checkHasActionKeyEventBinding(input_key, IE_Released))
                for (auto &&dlg : m_ActionDelegateMap[input_key][IE_Released])
                    dlg->execute();
        }
    }

    void InputManager::MouseMove(float xoffset, float yoffset)
    {
        //m_KeyStates[KEY_MOUSEX].Value = xoffset;
        ////m_KeyStates[KEY_MOUSEX].AccumulateValue += xoffset;
        //m_KeyStates[KEY_MOUSEX].bDown = true;
        if (checkHasAxisKeyBinding(KEY_MOUSEX))
            for (auto &&dlg : m_AxisDelegateMap[KEY_MOUSEX])
                dlg->execute(xoffset);
        if (checkHasAxisKeyBinding(KEY_MOUSEY))
            for (auto &&dlg : m_AxisDelegateMap[KEY_MOUSEY])
                dlg->execute(yoffset);

        //m_KeyStates[KEY_MOUSEY].Value = yoffset;
        ////m_KeyStates[KEY_MOUSEY].AccumulateValue += yoffset;
        //m_KeyStates[KEY_MOUSEY].bDown = true;
    }

    const KeyState &InputManager::GetKeyState(InputKey key)
    {
        return m_KeyStates[key];
    }

    bool InputManager::checkHasActionKeyEventBinding(InputKey key, InputEvent e)
    {
        if (m_ActionDelegateMap.find(key) != m_ActionDelegateMap.end() &&
            m_ActionDelegateMap[key].find(e) != m_ActionDelegateMap[key].end())
            return true;
        return false;
    }

    bool InputManager::checkHasAxisKeyBinding(InputKey key)
    {
        if (m_AxisDelegateMap.find(key) != m_AxisDelegateMap.end())
            return true;
        return false;
    }
    
    IMPLEMENT_MODULE(InputManager);
    //void InputManager::BindAxis(const std::string &key, BaseActor *obj, void(BaseActor:: *func)(float))


    //void InputManager::UpArrowKeyDown() {
    //    //g_pGameLogic->OnUpKeyDown();
    //    if (!m_bUpKeyPressed) {
    //        //g_pGameLogic->OnUpKey();
    //        m_bUpKeyPressed = true;
    //    }
    //}

    //void InputManager::UpArrowKeyUp() {
    //    //g_pGameLogic->OnUpKeyUp();
    //    m_bUpKeyPressed = false;
    //}

    //void InputManager::DownArrowKeyDown() {
    //    //g_pGameLogic->OnDownKeyDown();
    //    if (!m_bDownKeyPressed) {
    //        //g_pGameLogic->OnDownKey();
    //        m_bDownKeyPressed = true;
    //    }
    //}

    //void InputManager::DownArrowKeyUp() {
    //    //g_pGameLogic->OnDownKeyUp();
    //    m_bDownKeyPressed = false;
    //}

    //void InputManager::LeftArrowKeyDown() {
    //    //g_pGameLogic->OnLeftKeyDown();
    //    if (!m_bLeftKeyPressed) {
    //        //g_pGameLogic->OnLeftKey();
    //        m_bLeftKeyPressed = true;
    //    }
    //}

    //void InputManager::LeftArrowKeyUp() {
    //    //g_pGameLogic->OnLeftKeyUp();
    //    m_bLeftKeyPressed = false;
    //}

    //void InputManager::RightArrowKeyDown() {
    //    //g_pGameLogic->OnRightKeyDown();
    //    if (!m_bRightKeyPressed) {
    //        //g_pGameLogic->OnRightKey();
    //        m_bRightKeyPressed = true;
    //    }
    //}

    //void InputManager::RightArrowKeyUp() {
    //    //g_pGameLogic->OnRightKeyUp();
    //    m_bRightKeyPressed = false;
    //}

    //void InputManager::LeftMouseButtonDown() {}

    //void InputManager::LeftMouseButtonUp() {}

    //void InputManager::LeftMouseDrag(int deltaX, int deltaY) {
    //    //g_pGameLogic->OnAnalogStick(0, (float)deltaX, (float)deltaY);
    //}

    //void InputManager::RightMouseButtonDown() {}

    //void InputManager::RightMouseButtonUp() {}

    //void InputManager::RightMouseDrag(int deltaX, int deltaY) {
    //    //g_pGameLogic->OnAnalogStick(1, (float)deltaX, (float)deltaY);
    //}
}
