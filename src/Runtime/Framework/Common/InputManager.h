#pragma once
#include "Interface/IRuntimeModule.h"
#include <vector>
#include <map>
#include <set>
#include <string>

#include <GLFW/glfw3.h>

namespace nilou {
	class BaseActor;
	enum InputEvent
	{
		IE_Pressed = 0,
		IE_Released = 1,
		IE_Repeat = 2,
		IE_DoubleClick = 3,
		IE_Axis = 4,
		IE_MAX = 5,
	};
	enum InputKey
	{
		KEY_MOUSEX,
		KEY_MOUSEY,
		KEY_KP_0 = GLFW_KEY_KP_0,
		KEY_KP_1 = GLFW_KEY_KP_1,
		KEY_KP_2 = GLFW_KEY_KP_2,
		KEY_KP_3 = GLFW_KEY_KP_3,
		KEY_KP_4 = GLFW_KEY_KP_4,
		KEY_KP_5 = GLFW_KEY_KP_5,
		KEY_KP_6 = GLFW_KEY_KP_6,
		KEY_KP_7 = GLFW_KEY_KP_7,
		KEY_KP_8 = GLFW_KEY_KP_8,
		KEY_KP_9 = GLFW_KEY_KP_9,
		KEY_W = GLFW_KEY_W,
		KEY_S = GLFW_KEY_S,
		KEY_A = GLFW_KEY_A,
		KEY_D = GLFW_KEY_D,
		KEY_E = GLFW_KEY_E,
		KEY_Q = GLFW_KEY_Q,
		KEY_O = GLFW_KEY_O,
		KEY_G = GLFW_KEY_G,
		KEY_B = GLFW_KEY_B,
		KEY_SPACE = GLFW_KEY_SPACE,
		KEY_UP = GLFW_KEY_UP,
		KEY_DOWN = GLFW_KEY_DOWN,
		KEY_RIGHT = GLFW_KEY_RIGHT,
		KEY_LEFT = GLFW_KEY_LEFT,
		KEY_MOUSE_LEFT = GLFW_MOUSE_BUTTON_LEFT,
		KEY_MOUSE_RIGHT = GLFW_MOUSE_BUTTON_RIGHT,
		KEY_MOUSE_MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE,
		KEY_PAGEUP = GLFW_KEY_PAGE_UP,
		KEY_PAGEDOWN = GLFW_KEY_PAGE_DOWN,
		KEY_LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL,
		KEY_LEFT_ALT = GLFW_KEY_LEFT_ALT
	};
	struct KeyState
	{
		bool bRepeating;

		bool bDown;

		float Value;


		KeyState()
			: Value(0.f)
			, bDown(false)
			, bRepeating(false)
		{
		}
	};

	struct InputAxisMapping
	{
		std::string MappingName;
		std::map<InputKey, float> Groups;
		InputAxisMapping(const std::string &name) : MappingName(name) {}
		void AddGroup(InputKey key, float scale)
		{
			Groups[key] = scale;
		}
	};

	struct InputActionMapping
	{
		std::string MappingName;
		std::set<InputKey> Groups;
		InputActionMapping(const std::string &name) : MappingName(name) {}
		void AddGroup(InputKey key)
		{
			Groups.insert(key);
		}
	};
	// ����ʵ��ģ��Ķ�̬
	Interface IAxisDelegate
	{
	public:
		virtual ~IAxisDelegate() { }
		virtual void execute(float) = 0;
	};
	template<class UserClass>
	class AxisDelegate : implements IAxisDelegate
	{
	private:
		UserClass *m_obj;
		void (UserClass:: *m_func)(float);
		float m_scale;
	public:
		AxisDelegate(UserClass *obj, void (UserClass:: *func)(float), float scale) 
			: m_obj(obj), m_func(func), m_scale(scale)
		{
		}
		virtual void execute(float AxisValue)
		{
			(m_obj->*m_func)(AxisValue * m_scale);
		}
	};

	Interface IActionDelegate
	{
	public:
		virtual ~IActionDelegate() { }
		virtual void execute() = 0;
	};
	template<class UserClass>
	class ActionDelegate : implements IActionDelegate
	{
	private:
		UserClass *m_obj;
		void (UserClass:: *m_func)();
	public:
		ActionDelegate(UserClass *obj, void (UserClass:: *func)())
			: m_obj(obj), m_func(func)
		{
		}
		virtual void execute()
		{
			(m_obj->*m_func)();
		}
	};

	template<class Lambda>
	class LambdaActionDelegate : implements IActionDelegate
	{
	private:
		Lambda const &m_func;
	public:
		LambdaActionDelegate(Lambda const &func)
			: m_func(func)
		{
		}
		virtual void execute()
		{
			(m_func)();
		}
	};

    class InputManager : implements IRuntimeModule
    {
	private:
		std::map<InputKey, std::vector<IAxisDelegate *>>		m_AxisDelegateMap;
		std::map<InputKey, std::map<InputEvent, std::vector<IActionDelegate *>>>	m_ActionDelegateMap;
		std::map<InputKey, KeyState>			m_KeyStates;
    public:
		InputManager();

        int StartupModule() override;
        void ShutdownModule() override;
        // void Tick(double DeltaTime) override;

		void KeyPressed(int key);
		void KeyReleased(int key);
		void MouseMove(float xoffset, float yoffset);

		template<class UserClass>
		void BindAxis(const InputAxisMapping &mapping, UserClass *obj, void (UserClass:: *func)(float))
		{
			for (auto &group : mapping.Groups)
			{
				AxisDelegate<UserClass> *axis_delegate = new AxisDelegate<UserClass>(obj, func, group.second);
				m_AxisDelegateMap[group.first].push_back(axis_delegate);
			}
		}

		template<class UserClass>
		void BindAction(const InputActionMapping &mapping, InputEvent e, UserClass *obj, void (UserClass:: *func)())
		{
			for (auto &key : mapping.Groups)
			{
				ActionDelegate<UserClass> *action_delegate = new ActionDelegate<UserClass>(obj, func);
				m_ActionDelegateMap[key][e].push_back(action_delegate);
			}
		}

		template<class Lambda>
		void BindAction(const InputActionMapping &mapping, InputEvent e, Lambda const &func/*void (*func)()*/)
		{
			for (auto &key : mapping.Groups)
			{
				LambdaActionDelegate<Lambda> *action_delegate = new LambdaActionDelegate<Lambda>(func);
				m_ActionDelegateMap[key][e].push_back(action_delegate);
			}
		}

		const KeyState &GetKeyState(InputKey key);
    private:
		bool checkHasActionKeyEventBinding(InputKey key, InputEvent e);
		bool checkHasAxisKeyBinding(InputKey key);
    };

    extern InputManager *g_pInputManager;
}