#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <gdal.h>
#include <gdal_priv.h>
#include <memory>

#include <OpenGL/OpenGLDynamicRHI.h>
#include <Common/InputManager.h>
// #include <Common/SceneManager.h>
// #include <Common/ShaderManager.h>
#include <Common/AssetLoader.h>

// #include "Common/DrawPass/ForwardRenderPass.h"
// #include "Common/DrawPass/ShadowMappingPass.h"
// #include "Common/DrawPass/DebugHUDPass.h"
// #include "Common/DrawPass/SkyboxPass.h"
// #include "Common/DrawPass/OceanSurfacePass.h"
// #include "Common/DrawPass/SeabedSurfacePass.h"
// #include "Common/DrawPass/DeferredRenderPass.h"

#include "GLFWApplication.h"

namespace nilou {
    bool firstMouse = true;
    float lastX;
    float lastY;

    // Our state
    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    void framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        glViewport(0, 0, width, height);
        // g_pSceneManager->GetScene()->Observer->SetCameraAspectRatio((float)width / (float)height);
        GetAppication()->SetWindowWidth(width);
        GetAppication()->SetWindowHeight(height);
    }

    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
    {
        if (action == GLFW_PRESS)
            GetInputManager()->KeyPressed(button);
        else if (action == GLFW_RELEASE)
            GetInputManager()->KeyReleased(button);
    }

    void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
    {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;

        lastX = xpos;
        lastY = ypos;

        GetInputManager()->MouseMove(xoffset, yoffset);
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
        lastX = config.screenWidth / 2.0;
        lastY = config.screenHeight / 2.0;
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

        World = std::make_shared<UWorld>();
        Scene = std::make_shared<FScene>();
        World->Scene = Scene.get();
        Scene->World = World.get();

        while (!RenderingThread->IsRunnableInitialized()) { }
		GDALAllRegister();
        World->InitWorld();
        World->BeginPlay();
//         run_time_modules.push_back(GetAssetLoader());
//         run_time_modules.push_back(g_pSceneManager);
//         run_time_modules.push_back(FDynamicRHI::GetDynamicRHI());
//         run_time_modules.push_back(GetInputManager());
//         run_time_modules.push_back(g_pShaderManager);
//         _GM->GetError();
//         g_pShaderManager->Initialize();
//         _GM->GetError();
//         GetAssetLoader()->Initialize();
//         _GM->GetError();
//         g_pSceneManager->Initialize();
//         _GM->GetError();
//         FDynamicRHI::GetDynamicRHI()->Initialize();
//         _GM->GetError();
//         GetInputManager()->Initialize();

//         m_DrawPasses.push_back(new ShadowMappingPass);

//         m_DrawPasses.push_back(new ForwardRenderPass);

//         m_DrawPasses.push_back(new SkyboxPass);

//         m_DrawPasses.push_back(new OceanSurfacePass);

//         m_DrawPasses.push_back(new SeabedSurfacePass);

//         m_DrawPasses.push_back(new DeferredRenderPass);
// #ifdef NILOU_DEBUG
//         m_DrawPasses.push_back(new DebugHUDPass);
// #endif // NILOU_DEBUG
//         auto &frame = g_pSceneManager->frame;
//         for (auto &&pass : m_DrawPasses)
//         {
//             pass->Initialize(frame);
//             _GM->GetError();
//         }

        return true;
    }

    bool GLFWApplication::Initialize_RenderThread()
    {
        int result;
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RED_BITS, m_Config.redBits);
        glfwWindowHint(GLFW_GREEN_BITS, m_Config.greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, m_Config.blueBits);
        glfwWindowHint(GLFW_ALPHA_BITS, m_Config.alphaBits);
        glfwWindowHint(GLFW_DEPTH_BITS, m_Config.depthBits);


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
        InputActionMapping EnableCursor_mapping("EnableCursor");
        EnableCursor_mapping.AddGroup(InputKey::KEY_LEFT_CONTROL);
        GetInputManager()->BindAction(EnableCursor_mapping, InputEvent::IE_Pressed, this, &GLFWApplication::EnableCursor);
        return true;
    }

    void GLFWApplication::Tick(double DeltaTime)
    {
        deltaTime = DeltaTime;
        accumTime += deltaTime;

        // ImGui_ImplOpenGL3_NewFrame();
        // ImGui_ImplGlfw_NewFrame();
        // ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        //if (show_demo_window)
        //    ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            // static float f = 0.0f;
            // static int counter = 0;

            // ImGui::Begin("Ocean");                          // Create a window called "Hello, world!" and append into it.

            // ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            // ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

            // ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            // ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

            // if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            //    counter++;
            // ImGui::SameLine();
            // ImGui::Text("counter = %d", counter);

            // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        }
        World->Tick(DeltaTime);

        if (Scene)
        {
            UWorld *World = Scene->World;
            if (World)
                World->SendAllEndOfFrameUpdates();
        }

        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // ImGui::End();
        // ImGui::Render();

        // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    }

    void GLFWApplication::Tick_RenderThread()
    {
        glfwPollEvents();
        processInput();
        glfwSwapBuffers(window);
        m_bQuit = glfwWindowShouldClose(window);
    }

    void GLFWApplication::Finalize()
    {
        // for (auto pass : m_DrawPasses)
        // {
        //     delete pass;
        // }
        glfwTerminate();
    }

    void GLFWApplication::processInput()
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        #define DISPATCH_KEY_MESSAGE(key) \
            { \
                int state = glfwGetKey(window, key); \
                if (state == GLFW_PRESS) \
                    GetInputManager()->KeyPressed(key); \
                else if (state == GLFW_RELEASE) \
                    GetInputManager()->KeyReleased(key); \
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


        //if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        //    GetInputManager()->KeyWDown(1.0);
        //if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        //    GetInputManager()->KeySDown(1.0);
        //if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        //    GetInputManager()->KeyADown(1.0);
        //if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        //    GetInputManager()->KeyDDown(1.0);

        //if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        //    GetInputManager()->KeyUpDown(1.0);
        //if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        //    GetInputManager()->KeyDownDown(1.0);
        //if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        //    GetInputManager()->KeyRightDown(1.0);
        //if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        //    GetInputManager()->KeyLeftDown(1.0);
    }

    void GLFWApplication::EnableCursor()
    {
        CursorEnabled = !CursorEnabled;
        if (CursorEnabled)
            glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

}