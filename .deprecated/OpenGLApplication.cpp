#include "OpenGLApplication.h"
#include <OpenGL/OpenGLDynamicRHI.h>
#include <glad/glad_wgl.h>
#include <tchar.h>

namespace und {
	//extern GfxConfiguration config;
	extern FDynamicRHI *GDynamicRHI;
	//OpenGLApplication  g_App(config);
	//IApplication *g_pApp = &g_App;
}
static LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, uiMsg, wParam, lParam);
	}

	return 0;
}
int und::OpenGLApplication::Initialize()
{
	int result;
	auto colorBits = m_Config.redBits + m_Config.greenBits + m_Config.blueBits;

	// create a temporary window for OpenGL context loading
	DWORD Style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	WNDCLASSEX WndClassEx;
	memset(&WndClassEx, 0, sizeof(WNDCLASSEX));

	HINSTANCE hInstance = GetModuleHandle(NULL);

	WndClassEx.cbSize = sizeof(WNDCLASSEX);
	WndClassEx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClassEx.lpfnWndProc = WndProc;
	WndClassEx.hInstance = hInstance;
	WndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClassEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClassEx.lpszClassName = _T("InitWindow");

	RegisterClassEx(&WndClassEx);
	HWND TemphWnd = CreateWindowEx(WS_EX_APPWINDOW, WndClassEx.lpszClassName, _T("InitWindow"), Style, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = colorBits;
	pfd.cAlphaBits = m_Config.alphaBits;
	pfd.cDepthBits = m_Config.depthBits;
	pfd.cStencilBits = m_Config.stencilBits;
	pfd.iLayerType = PFD_MAIN_PLANE;

	HDC TemphDC = GetDC(TemphWnd);
	// Set a temporary default pixel format.
	int nPixelFormat = ChoosePixelFormat(TemphDC, &pfd);
	if (nPixelFormat == 0) return -1;

	result = SetPixelFormat(TemphDC, nPixelFormat, &pfd);
	if (result != 1)
	{
		return result;
	}

	// Create a temporary rendering context.
	m_RenderContext = wglCreateContext(TemphDC);
	if (!m_RenderContext)
	{
		printf("wglCreateContext failed!\n");
		return -1;
	}

	// Set the temporary rendering context as the current rendering context for this window.
	result = wglMakeCurrent(TemphDC, m_RenderContext);
	if (result != 1)
	{
		return result;
	}

	if (!gladLoadWGL(TemphDC)) {
		printf("WGL initialize failed!\n");
		result = -1;
	}
	else {
		result = 0;
		printf("WGL initialize finished!\n");
	}

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(m_RenderContext);
	ReleaseDC(TemphWnd, TemphDC);
	DestroyWindow(TemphWnd);

	result = WindowsApplication::Initialize();
	if (result) {
		printf("Windows Application initialize failed!");
		return result;
	}

	m_hDC = GetDC(m_hWnd);

	// Set pixel format.
	nPixelFormat = ChoosePixelFormat(m_hDC, &pfd);
	if (nPixelFormat == 0) return -1;

	result = SetPixelFormat(m_hDC, nPixelFormat, &pfd);
	if (result != 1)
	{
		return result;
	}

	// Create rendering context.
	m_RenderContext = wglCreateContext(m_hDC);
	if (!m_RenderContext)
	{
		printf("wglCreateContext failed!\n");
		return -1;
	}

	// Set the rendering context as the current rendering context for this window.
	result = wglMakeCurrent(m_hDC, m_RenderContext);
	if (result != 1)
	{
		return result;
	}

	result = 0;

	return result;
	//int result;
	//result = und::WindowsApplication::Initialize();

	//if (result != 0)
	//	exit(result);

 //   PIXELFORMATDESCRIPTOR pfd;
 //   memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
 //   pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
 //   pfd.nVersion = 1;
 //   pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
 //   pfd.iPixelType = PFD_TYPE_RGBA;
 //   pfd.cColorBits = m_Config.redBits + m_Config.greenBits + m_Config.blueBits + m_Config.alphaBits;
 //   pfd.cDepthBits = m_Config.depthBits;
 //   pfd.iLayerType = PFD_MAIN_PLANE;

 //   HWND hWnd = reinterpret_cast<WindowsApplication *>(g_pApp)->GetMainWindow();
 //   HDC  hDC = GetDC(hWnd);
 //   int nPixelFormat = ChoosePixelFormat(hDC, &pfd);
 //   if (nPixelFormat == 0) return -1;

 //   result = SetPixelFormat(hDC, nPixelFormat, &pfd);
 //   if (result != 1)
 //   {
 //       return -1;
 //   }

 //   m_RenderContext = wglCreateContext(hDC);
 //   if (!m_RenderContext)
 //   {
 //       return -1;
 //   }

 //   result = wglMakeCurrent(hDC, m_RenderContext);
 //   if (result != 1)
 //   {
 //       return -1;
 //   }

 //   if (!gladLoadWGL(hDC)) {
 //       printf("WGL initialize failed!\n");
 //       result = -1;
 //   }
 //   else {
 //       result = 0;
 //       printf("WGL initialize finished!\n");
 //   }

    //const char *vertexShaderSource = "#version 330 core\n"
    //    "layout (location = 0) in vec3 aPos;\n"
    //    "void main()\n"
    //    "{\n"
    //    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    //    "}\0";
    //const char *fragmentShaderSource = "#version 330 core\n"
    //    "layout (location = 0) out vec4 FragColor;\n"
    //    "void main()\n"
    //    "{\n"
    //    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    //    "}\n\0";
    //unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    //glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    //glCompileShader(vertexShader);
    //// check for shader compile errors
    //int success;
    //char infoLog[512];
    //glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    //if (!success)
    //{
    //    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    //    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    //}
    //// fragment shader
    //unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    //glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    //glCompileShader(fragmentShader);
    //// check for shader compile errors
    //glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    //if (!success)
    //{
    //    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    //    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    //}
    //// link shaders
    //shaderProgram = glCreateProgram();
    //glAttachShader(shaderProgram, vertexShader);
    //glAttachShader(shaderProgram, fragmentShader);
    //glLinkProgram(shaderProgram);
    //// check for linking errors
    //glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    //if (!success) {
    //    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    //    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    //}
    //glDeleteShader(vertexShader);
    //glDeleteShader(fragmentShader);

    //// set up vertex data (and buffer(s)) and configure vertex attributes
    //// ------------------------------------------------------------------
    //float vertices[] = {
    //     0.5f,  0.5f, 0.0f,  // top right
    //     0.5f, -0.5f, 0.0f,  // bottom right
    //    -0.5f, -0.5f, 0.0f,  // bottom left
    //    -0.5f,  0.5f, 0.0f   // top left 
    //};
    //unsigned int indices[] = {  // note that we start from 0!
    //    0, 1, 3,  // first Triangle
    //    1, 2, 3   // second Triangle
    //};
    //unsigned int VBO, EBO;
    //glGenVertexArrays(1, &VAO);
    //glGenBuffers(1, &VBO);
    //glGenBuffers(1, &EBO);
    //// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    //glBindVertexArray(VAO);

    //glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    //glEnableVertexAttribArray(0);

    //// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    //glBindBuffer(GL_ARRAY_BUFFER, 0);

    //// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    ////glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    //// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    //glBindVertexArray(0);

    return result;
}

void und::OpenGLApplication::Finalize()
{
    if (m_RenderContext) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(m_RenderContext);
        m_RenderContext = 0;
    }

    WindowsApplication::Finalize();
}

void und::OpenGLApplication::Tick(double DeltaTime)
{
    //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);

    // draw our first triangle
    //glUseProgram(shaderProgram);
    //glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    ////glDrawArrays(GL_TRIANGLES, 0, 6);
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    WindowsApplication::Tick(DeltaTime);
	GDynamicRHI->Clear();
	GDynamicRHI->Draw();

	// Present the back buffer to the screen since rendering is complete.
	SwapBuffers(m_hDC);
}
