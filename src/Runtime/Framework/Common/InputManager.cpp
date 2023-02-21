#include "InputManager.h"
#include "Templates/ObjectMacros.h"

#include <iostream>

//#include "GraphicsManager.h"
// #include "SceneManager.h"
// #include "Common/BaseActor.h"

using namespace std;

namespace nilou {

    InputManager *GetInputManager()
    {  
        static InputManager *GInputManager = new InputManager;
        return GInputManager;
    }

    void nilou::InputManager::KeyPressed(InputKey input_key)
    {
        KeyState former_state = m_KeyStates[input_key];
        m_KeyStates[input_key].Value = 1.0;
        m_KeyStates[input_key].bDown = true;
        if (former_state.bDown == false)
        {
            m_KeyStates[input_key].bRepeating = false;
            if (checkHasActionKeyEventBinding(input_key, IE_Pressed))
                m_ActionDelegateMap[input_key][IE_Pressed].Broadcast();
                // for (auto &&dlg : m_ActionDelegateMap[input_key][IE_Pressed])
                //     dlg->execute();
        }
        else if (former_state.bDown == true)
        {
            m_KeyStates[input_key].bRepeating = true;
            if (checkHasActionKeyEventBinding(input_key, IE_Repeat))
                m_ActionDelegateMap[input_key][IE_Repeat].Broadcast();
                // for (auto &&dlg : m_ActionDelegateMap[input_key][IE_Repeat])
                //     dlg->execute();
        }
        if (checkHasAxisKeyBinding(input_key))
            m_AxisDelegateMap[input_key].Broadcast(1.0);
            // for (auto &&dlg : m_AxisDelegateMap[input_key])
            //     dlg->execute(1.0);
    }

    void InputManager::KeyReleased(InputKey input_key)
    {
        KeyState former_state = m_KeyStates[input_key];
        m_KeyStates[input_key].Value = 0.0;
        m_KeyStates[input_key].bDown = false;
        m_KeyStates[input_key].bRepeating = false;
        if (former_state.bDown == true)
        {
            if (checkHasActionKeyEventBinding(input_key, IE_Released))
                m_ActionDelegateMap[input_key][IE_Released].Broadcast();
                // for (auto &&dlg : m_ActionDelegateMap[input_key][IE_Released])
                //     dlg->execute();
        }
    }

    void InputManager::MouseMove(float xoffset, float yoffset)
    {
        //m_KeyStates[AXIS_MOUSEX].Value = xoffset;
        ////m_KeyStates[AXIS_MOUSEX].AccumulateValue += xoffset;
        //m_KeyStates[AXIS_MOUSEX].bDown = true;
        if (checkHasAxisKeyBinding(AXIS_MOUSEX))
            m_AxisDelegateMap[AXIS_MOUSEX].Broadcast(xoffset);
            // for (auto &&dlg : m_AxisDelegateMap[AXIS_MOUSEX])
            //     dlg->execute(xoffset);
        if (checkHasAxisKeyBinding(AXIS_MOUSEY))
            m_AxisDelegateMap[AXIS_MOUSEY].Broadcast(yoffset);
            // for (auto &&dlg : m_AxisDelegateMap[AXIS_MOUSEY])
            //     dlg->execute(yoffset);

        //m_KeyStates[AXIS_MOUSEY].Value = yoffset;
        ////m_KeyStates[AXIS_MOUSEY].AccumulateValue += yoffset;
        //m_KeyStates[AXIS_MOUSEY].bDown = true;
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
}
