#define GLFW_INCLUDE_VULKAN
#include <glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>

// #include "DynamicRHI.h"
#include "Common/ContentManager.h"
#include "Common/Asset/AssetLoader.h"

#include "GLFWApplication.h"

namespace nilou {

    double lastX;
    double lastY;
    double MousePositionX;
    double MousePositionY;
    bool firstMouse = true;
    std::unordered_map<InputKey, int> KeyStates;
    bool ScreenResized = false;


    std::unordered_map<InputKey, int> CreateInputKey_GLFW_Map()
    {
        std::unordered_map<InputKey, int> Map;
		Map[KEY_KP_0] = GLFW_KEY_KP_0;
		Map[KEY_KP_1] = GLFW_KEY_KP_1;
		Map[KEY_KP_2] = GLFW_KEY_KP_2;
		Map[KEY_KP_3] = GLFW_KEY_KP_3;
		Map[KEY_KP_4] = GLFW_KEY_KP_4;
		Map[KEY_KP_5] = GLFW_KEY_KP_5;
		Map[KEY_KP_6] = GLFW_KEY_KP_6;
		Map[KEY_KP_7] = GLFW_KEY_KP_7;
		Map[KEY_KP_8] = GLFW_KEY_KP_8;
		Map[KEY_KP_9] = GLFW_KEY_KP_9;
		Map[KEY_W] = GLFW_KEY_W;
		Map[KEY_S] = GLFW_KEY_S;
		Map[KEY_A] = GLFW_KEY_A;
		Map[KEY_D] = GLFW_KEY_D;
		Map[KEY_E] = GLFW_KEY_E;
		Map[KEY_Q] = GLFW_KEY_Q;
		Map[KEY_O] = GLFW_KEY_O;
		Map[KEY_G] = GLFW_KEY_G;
		Map[KEY_B] = GLFW_KEY_B;
		Map[KEY_SPACE] = GLFW_KEY_SPACE;
		Map[KEY_UP] = GLFW_KEY_UP;
		Map[KEY_DOWN] = GLFW_KEY_DOWN;
		Map[KEY_RIGHT] = GLFW_KEY_RIGHT;
		Map[KEY_LEFT] = GLFW_KEY_LEFT;
		Map[KEY_MOUSE_LEFT] = GLFW_MOUSE_BUTTON_LEFT;
		Map[KEY_MOUSE_RIGHT] = GLFW_MOUSE_BUTTON_RIGHT;
		Map[KEY_MOUSE_MIDDLE] = GLFW_MOUSE_BUTTON_MIDDLE;
		Map[KEY_PAGEUP] = GLFW_KEY_PAGE_UP;
		Map[KEY_PAGEDOWN] = GLFW_KEY_PAGE_DOWN;
		Map[KEY_LEFT_CONTROL] = GLFW_KEY_LEFT_CONTROL;
		Map[KEY_LEFT_ALT] = GLFW_KEY_LEFT_ALT;
        return Map;
    }
    std::unordered_map<InputKey, int> InputKey_GLFW_Map = CreateInputKey_GLFW_Map();

    std::unordered_map<int, InputKey> CreateGLFW_InputKey_Map()
    {
        std::unordered_map<int, InputKey> Map;
		Map[GLFW_KEY_KP_0] = InputKey::KEY_KP_0;
		Map[GLFW_KEY_KP_1] = InputKey::KEY_KP_1;
		Map[GLFW_KEY_KP_2] = InputKey::KEY_KP_2;
		Map[GLFW_KEY_KP_3] = InputKey::KEY_KP_3;
		Map[GLFW_KEY_KP_4] = InputKey::KEY_KP_4;
		Map[GLFW_KEY_KP_5] = InputKey::KEY_KP_5;
		Map[GLFW_KEY_KP_6] = InputKey::KEY_KP_6;
		Map[GLFW_KEY_KP_7] = InputKey::KEY_KP_7;
		Map[GLFW_KEY_KP_8] = InputKey::KEY_KP_8;
		Map[GLFW_KEY_KP_9] = InputKey::KEY_KP_9;
		Map[GLFW_KEY_W] = InputKey::KEY_W;
		Map[GLFW_KEY_S] = InputKey::KEY_S;
		Map[GLFW_KEY_A] = InputKey::KEY_A;
		Map[GLFW_KEY_D] = InputKey::KEY_D;
		Map[GLFW_KEY_E] = InputKey::KEY_E;
		Map[GLFW_KEY_Q] = InputKey::KEY_Q;
		Map[GLFW_KEY_O] = InputKey::KEY_O;
		Map[GLFW_KEY_G] = InputKey::KEY_G;
		Map[GLFW_KEY_B] = InputKey::KEY_B;
		Map[GLFW_KEY_SPACE] = InputKey::KEY_SPACE;
		Map[GLFW_KEY_UP] = InputKey::KEY_UP;
		Map[GLFW_KEY_DOWN] = InputKey::KEY_DOWN;
		Map[GLFW_KEY_RIGHT] = InputKey::KEY_RIGHT;
		Map[GLFW_KEY_LEFT] = InputKey::KEY_LEFT;
		Map[GLFW_MOUSE_BUTTON_LEFT] = InputKey::KEY_MOUSE_LEFT;
		Map[GLFW_MOUSE_BUTTON_RIGHT] = InputKey::KEY_MOUSE_RIGHT;
		Map[GLFW_MOUSE_BUTTON_MIDDLE] = InputKey::KEY_MOUSE_MIDDLE;
		Map[GLFW_KEY_PAGE_UP] = InputKey::KEY_PAGEUP;
		Map[GLFW_KEY_PAGE_DOWN] = InputKey::KEY_PAGEDOWN;
		Map[GLFW_KEY_LEFT_CONTROL] = InputKey::KEY_LEFT_CONTROL;
		Map[GLFW_KEY_LEFT_ALT] = InputKey::KEY_LEFT_ALT;
        return Map;
    }
    std::unordered_map<int, InputKey> GLFW_InputKey_Map = CreateGLFW_InputKey_Map();

    // Our state
    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    void framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        GetAppication()->SetWindowWidth(width);
        GetAppication()->SetWindowHeight(height);
        ScreenResized = true;
    }

    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
    {
        InputKey key = GLFW_InputKey_Map[button];
        if (action == GLFW_PRESS)
            KeyStates[key] = GLFW_PRESS;
        else if (action == GLFW_RELEASE)
            KeyStates[key] = GLFW_RELEASE;
    }

    void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
    {
        MousePositionX = xposIn;
        MousePositionY = yposIn;
        if (firstMouse)
        {
            lastX = MousePositionX;
            lastY = MousePositionY;
            firstMouse = false;
        }
        //GetInputManager()->MouseYMove(yoffset);
    }

    // glfw: whenever the mouse scroll wheel scrolls, this callback is called
    // ----------------------------------------------------------------------
    void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
        //GetInputManager()->MouseScroll(static_cast<float>(yoffset));
    }

    GLFWApplication::GLFWApplication(GfxConfiguration &config)
        : BaseApplication(config)
    {
        // lastX = config.screenWidth / 2.0;
        // lastY = config.screenHeight / 2.0;
    }
    bool GLFWApplication::Initialize()
    {
        BaseApplication::Initialize();
 
        // Setup Dear ImGui context
        // IMGUI_CHECKVERSION();
        // ImGui::CreateContext();
        // ImGuiIO &io = ImGui::GetIO(); (void)io;
        // ImGui::StyleColorsDark();

        // ImGui_ImplGlfw_InitForOpenGL(window, true);
        // ImGui_ImplOpenGL3_Init("#version 130");

        return true;
    }

    void GetChannelsBit(EPixelFormat Format, uint8& redBits, uint8& greenBits, uint8& blueBits, uint8& alphaBits)
    {
        switch (Format) 
        {
        case EPixelFormat::PF_R8G8B8A8: 
        case EPixelFormat::PF_R8G8B8A8_sRGB:
        case EPixelFormat::PF_B8G8R8A8:
        case EPixelFormat::PF_B8G8R8A8_sRGB:
            redBits = 8; 
            greenBits = 8, 
            blueBits = 8, 
            alphaBits = 8; 
            break;
        default:
            assert(false);  // Not supported swap chain format
            break;
        }
    }

    void GetDepthBit(EPixelFormat Format, uint8& depthBits)
    {
        switch (Format) 
        {
        case EPixelFormat::PF_D24S8:
            depthBits = 24;
            break;
        case EPixelFormat::PF_D32F:
        case EPixelFormat::PF_D32FS8:
            depthBits = 32;
            break;
        default:
            assert(false);  // Not supported swap chain format
            break;
        }
    }

    bool GLFWApplication::Initialize_RenderThread()
    {
        int result;
        glfwInit();
        auto RHI_API = FDynamicRHI::Get()->GetCurrentGraphicsAPI();
        if (RHI_API == EGraphicsAPI::Vulkan) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }
        else if (RHI_API == EGraphicsAPI::OpenGL) {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }
        uint8 redBits, greenBits, blueBits, alphaBits, depthBits;
        GetChannelsBit(m_Config.SwapChainFormat, redBits, greenBits, blueBits, alphaBits);
        GetDepthBit(m_Config.DepthFormat, depthBits);
        glfwWindowHint(GLFW_RED_BITS, redBits);
        glfwWindowHint(GLFW_GREEN_BITS, greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, blueBits);
        glfwWindowHint(GLFW_ALPHA_BITS, alphaBits);
        glfwWindowHint(GLFW_DEPTH_BITS, depthBits);


        window = glfwCreateWindow(m_Config.screenWidth, m_Config.screenHeight, "Nilou", NULL, NULL);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetWindowPos(window, 100, 100);
        // tell GLFW to capture our mouse
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // Setup Dear ImGui context
        // IMGUI_CHECKVERSION();
        // ImGui::CreateContext();
        // ImGuiIO &io = ImGui::GetIO(); (void)io;
        // ImGui::StyleColorsDark();

        // ImGui_ImplGlfw_InitForOpenGL(window, true);
        // ImGui_ImplOpenGL3_Init("#version 460");

        InputActionMapping EnableCursor_mapping("EnableCursor");
        EnableCursor_mapping.AddGroup(InputKey::KEY_LEFT_CONTROL);
        GetInputManager()->BindAction(EnableCursor_mapping, InputEvent::IE_Pressed, this, &GLFWApplication::EnableCursor);
        return true;
    }

    void GLFWApplication::Tick(double DeltaTime)
    {
        deltaTime = DeltaTime;
        accumTime += deltaTime;

        DispatchScreenResizeMessage();
        DispatchMouseMoveMessage();
        DispatchKeyMessage();

        BaseApplication::Tick(DeltaTime);
    }

    void GLFWApplication::DispatchScreenResizeMessage()
    {
        if (ScreenResized)
        {
            GetScreenResizeDelegate().Broadcast(m_Config.screenWidth, m_Config.screenHeight);
            ScreenResized = false;
        }
    }

    void GLFWApplication::DispatchMouseMoveMessage()
    {
        if (!firstMouse)
        {
            float xoffset = MousePositionX - lastX;
            float yoffset = lastY - MousePositionY;

            lastX = MousePositionX;
            lastY = MousePositionY;

            GetInputManager()->MouseMove(xoffset, yoffset);
        }
    }

    void GLFWApplication::DispatchKeyMessage()
    {
        #define DISPATCH_KEY_MESSAGE(key) \
            { \
                int state = KeyStates[GLFW_InputKey_Map[key]]; \
                if (state == GLFW_PRESS) \
                    GetInputManager()->KeyPressed(GLFW_InputKey_Map[key]); \
                else if (state == GLFW_RELEASE) \
                    GetInputManager()->KeyReleased(GLFW_InputKey_Map[key]); \
            }

        DISPATCH_KEY_MESSAGE(GLFW_KEY_KP_0)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_KP_1)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_KP_2)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_KP_3)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_KP_4)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_KP_5)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_KP_6)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_KP_7)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_KP_8)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_KP_9)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_W)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_S)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_A)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_D)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_E)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_Q)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_O)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_G)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_B)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_SPACE)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_UP)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_DOWN)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_RIGHT)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_LEFT)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_PAGE_UP)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_PAGE_DOWN)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_LEFT_CONTROL)
        DISPATCH_KEY_MESSAGE(GLFW_KEY_LEFT_ALT)
        DISPATCH_KEY_MESSAGE(GLFW_MOUSE_BUTTON_LEFT)
        DISPATCH_KEY_MESSAGE(GLFW_MOUSE_BUTTON_RIGHT)
        DISPATCH_KEY_MESSAGE(GLFW_MOUSE_BUTTON_MIDDLE)
    }

    void GLFWApplication::Tick_RenderThread()
    {
        glfwPollEvents();
        ProcessInput_RenderThread();

        // ImGui_ImplOpenGL3_NewFrame();
        // ImGui_ImplGlfw_NewFrame();
        // ImGui::NewFrame();
        // ImGui::Begin("Nilou");
        // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);


        // ImGui::End();
        // ImGui::Render();

        // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        m_bQuit = glfwWindowShouldClose(window);

        BaseApplication::Tick_RenderThread();
    }

    void GLFWApplication::Finalize_RenderThread()
    {
        BaseApplication::Finalize_RenderThread();
        glfwTerminate();
    }

    void GLFWApplication::ProcessInput_RenderThread()
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        #define DISPATCH_KEY_STATE(key) \
            { \
                int state = glfwGetKey(window, key); \
                if (state == GLFW_PRESS) \
                    KeyStates[GLFW_InputKey_Map[key]] = GLFW_PRESS; \
                else if (state == GLFW_RELEASE) \
                    KeyStates[GLFW_InputKey_Map[key]] = GLFW_RELEASE; \
            }

        DISPATCH_KEY_STATE(GLFW_KEY_KP_0)
        DISPATCH_KEY_STATE(GLFW_KEY_KP_1)
        DISPATCH_KEY_STATE(GLFW_KEY_KP_2)
        DISPATCH_KEY_STATE(GLFW_KEY_KP_3)
        DISPATCH_KEY_STATE(GLFW_KEY_KP_4)
        DISPATCH_KEY_STATE(GLFW_KEY_KP_5)
        DISPATCH_KEY_STATE(GLFW_KEY_KP_6)
        DISPATCH_KEY_STATE(GLFW_KEY_KP_7)
        DISPATCH_KEY_STATE(GLFW_KEY_KP_8)
        DISPATCH_KEY_STATE(GLFW_KEY_KP_9)
        DISPATCH_KEY_STATE(GLFW_KEY_W)
        DISPATCH_KEY_STATE(GLFW_KEY_S)
        DISPATCH_KEY_STATE(GLFW_KEY_A)
        DISPATCH_KEY_STATE(GLFW_KEY_D)
        DISPATCH_KEY_STATE(GLFW_KEY_E)
        DISPATCH_KEY_STATE(GLFW_KEY_Q)
        DISPATCH_KEY_STATE(GLFW_KEY_O)
        DISPATCH_KEY_STATE(GLFW_KEY_G)
        DISPATCH_KEY_STATE(GLFW_KEY_B)
        DISPATCH_KEY_STATE(GLFW_KEY_SPACE)
        DISPATCH_KEY_STATE(GLFW_KEY_UP)
        DISPATCH_KEY_STATE(GLFW_KEY_DOWN)
        DISPATCH_KEY_STATE(GLFW_KEY_RIGHT)
        DISPATCH_KEY_STATE(GLFW_KEY_LEFT)
        DISPATCH_KEY_STATE(GLFW_KEY_PAGE_UP)
        DISPATCH_KEY_STATE(GLFW_KEY_PAGE_DOWN)
        DISPATCH_KEY_STATE(GLFW_KEY_LEFT_CONTROL)
        DISPATCH_KEY_STATE(GLFW_KEY_LEFT_ALT)
    }

    void GLFWApplication::EnableCursor()
    {
        ENQUEUE_RENDER_COMMAND(GLFWApplication_EnableCursor)(
            [this](RenderGraph& Graph) 
            {
                CursorEnabled = !CursorEnabled;
                if (CursorEnabled)
                    glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                else
                    glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            });
    }

}