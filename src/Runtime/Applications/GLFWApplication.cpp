#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <OpenGL/OpenGLDynamicRHI.h>
#include <Common/InputManager.h>
// #include <Common/SceneManager.h>
// #include <Common/ShaderManager.h>
#include <Common/AssetLoader.h>
#include <memory>

// #include "Common/DrawPass/ForwardRenderPass.h"
// #include "Common/DrawPass/ShadowMappingPass.h"
// #include "Common/DrawPass/DebugHUDPass.h"
// #include "Common/DrawPass/SkyboxPass.h"
// #include "Common/DrawPass/OceanSurfacePass.h"
// #include "Common/DrawPass/SeabedSurfacePass.h"
// #include "Common/DrawPass/DeferredRenderPass.h"

#include "GLFWApplication.h"
#include "Interface/IRuntimeModule.h"
#include "Modules/ModuleManager.h"
#include "Common/Renderer/Renderer.h"

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
        g_pApp->SetWindowWidth(width);
        g_pApp->SetWindowHeight(height);
    }

    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
    {
        if (action == GLFW_PRESS)
            g_pInputManager->KeyPressed(button);
        else if (action == GLFW_RELEASE)
            g_pInputManager->KeyReleased(button);
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

        g_pInputManager->MouseMove(xoffset, yoffset);
        //g_pInputManager->MouseYMove(yoffset);
    }

    // glfw: whenever the mouse scroll wheel scrolls, this callback is called
    // ----------------------------------------------------------------------
    void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
        //g_pInputManager->MouseScroll(static_cast<float>(yoffset));
    }

    GLFWApplication::GLFWApplication(GfxConfiguration &config)
        : BaseApplication(config)
    {
        lastX = config.screenWidth / 2.0;
        lastY = config.screenHeight / 2.0;
    }
    int GLFWApplication::Initialize()
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
            return -1;
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetWindowPos(window, 100, 100);
        // tell GLFW to capture our mouse
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
        }

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
        for (auto [Name, Module] : GetModuleManager()->Modules)
        {
            Module->StartupModule();
        }

        World->BeginPlay();
//         run_time_modules.push_back(g_pAssetLoader);
//         run_time_modules.push_back(g_pSceneManager);
//         run_time_modules.push_back(GDynamicRHI);
//         run_time_modules.push_back(g_pInputManager);
//         run_time_modules.push_back(g_pShaderManager);
//         _GM->GLDEBUG();
//         g_pShaderManager->Initialize();
//         _GM->GLDEBUG();
//         g_pAssetLoader->Initialize();
//         _GM->GLDEBUG();
//         g_pSceneManager->Initialize();
//         _GM->GLDEBUG();
//         GDynamicRHI->Initialize();
//         _GM->GLDEBUG();
//         g_pInputManager->Initialize();

//         m_DrawPasses.push_back(new ShadowMappingPass);

//         m_DrawPasses.push_back(new ForwardRenderPass);

//         m_DrawPasses.push_back(new SkyboxPass);

//         m_DrawPasses.push_back(new OceanSurfacePass);

//         m_DrawPasses.push_back(new SeabedSurfacePass);

//         m_DrawPasses.push_back(new DeferredRenderPass);
// #ifdef _DEBUG
//         m_DrawPasses.push_back(new DebugHUDPass);
// #endif // _DEBUG
//         auto &frame = g_pSceneManager->frame;
//         for (auto &&pass : m_DrawPasses)
//         {
//             pass->Initialize(frame);
//             _GM->GLDEBUG();
//         }

        InputActionMapping EnableCursor_mapping("EnableCursor");
        EnableCursor_mapping.AddGroup(InputKey::KEY_LEFT_ALT);
        g_pInputManager->BindAction(EnableCursor_mapping, InputEvent::IE_Pressed, this, &GLFWApplication::EnableCursor);
        return 0;
    }
    void GLFWApplication::Tick(double DeltaTime)
    {
        //float currentFrame = static_cast<float>(glfwGetTime());
        //deltaTime = currentFrame - lastFrame;
        //lastFrame = currentFrame;
        deltaTime = DeltaTime;
        accumTime += deltaTime;
        glfwPollEvents();
        processInput();

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

        FRendererModule *RenderModule = static_cast<FRendererModule*>(GetModuleManager()->GetModule("FRendererModule"));
        RenderModule->Draw(Scene.get());
        // for (auto &module : run_time_modules) {
        //     module->Tick(DeltaTime);
        // }
        // auto &frame = g_pSceneManager->frame;
        // for (auto pass : m_DrawPasses)
        // {
        //     pass->Draw(frame);
        //     _GM->GLDEBUG();
        // }

        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // ImGui::End();
        // ImGui::Render();

        // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


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

        auto process_key = [this](int key) {
            int state = glfwGetKey(window, key);
            if (state == GLFW_PRESS)
                g_pInputManager->KeyPressed(key);
            else if (state == GLFW_RELEASE)
                g_pInputManager->KeyReleased(key);
        };

        process_key(GLFW_KEY_KP_0);
        process_key(GLFW_KEY_KP_1);
        process_key(GLFW_KEY_KP_2);
        process_key(GLFW_KEY_KP_3);
        process_key(GLFW_KEY_KP_4);
        process_key(GLFW_KEY_KP_5);
        process_key(GLFW_KEY_KP_6);
        process_key(GLFW_KEY_KP_7);
        process_key(GLFW_KEY_KP_8);
        process_key(GLFW_KEY_KP_9);
        process_key(GLFW_KEY_W);
        process_key(GLFW_KEY_S);
        process_key(GLFW_KEY_A);
        process_key(GLFW_KEY_D);
        process_key(GLFW_KEY_E);
        process_key(GLFW_KEY_Q);
        process_key(GLFW_KEY_O);
        process_key(GLFW_KEY_G);
        process_key(GLFW_KEY_B);
        process_key(GLFW_KEY_SPACE);
        process_key(GLFW_KEY_UP);
        process_key(GLFW_KEY_DOWN);
        process_key(GLFW_KEY_RIGHT);
        process_key(GLFW_KEY_LEFT);
        process_key(GLFW_KEY_PAGE_UP);
        process_key(GLFW_KEY_PAGE_DOWN);
        process_key(GLFW_KEY_LEFT_CONTROL);
        process_key(GLFW_KEY_LEFT_ALT);


        //if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        //    g_pInputManager->KeyWDown(1.0);
        //if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        //    g_pInputManager->KeySDown(1.0);
        //if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        //    g_pInputManager->KeyADown(1.0);
        //if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        //    g_pInputManager->KeyDDown(1.0);

        //if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        //    g_pInputManager->KeyUpDown(1.0);
        //if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        //    g_pInputManager->KeyDownDown(1.0);
        //if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        //    g_pInputManager->KeyRightDown(1.0);
        //if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        //    g_pInputManager->KeyLeftDown(1.0);
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