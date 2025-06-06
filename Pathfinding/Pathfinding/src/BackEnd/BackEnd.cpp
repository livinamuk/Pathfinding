#include "BackEnd.h"
#include <iostream>
#include <string>
#include "../API/OpenGL/GL_backEnd.h"
#include "../API/OpenGL/GL_renderer.h"
#include "../Core/AssetManager.h"
#include "../Core/Audio.hpp"
#include "../Core/Input.h"

namespace BackEnd {

    API g_api = API::UNDEFINED;
    GLFWwindow* g_window = NULL;
    WindowedMode g_windowedMode = WindowedMode::WINDOWED;
    GLFWmonitor* g_monitor;
    const GLFWvidmode* g_mode;
    bool g_forceCloseWindow = false;
    bool g_windowHasFocus = true;
    int g_windowedWidth = 0;
    int g_windowedHeight = 0;
    int g_fullscreenWidth = 0;
    int g_fullscreenHeight = 0;
    int g_currentWindowWidth = 0;
    int g_currentWindowHeight = 0;
    int g_presentTargetWidth = 0;
    int g_presentTargetHeight = 0;

    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void window_focus_callback(GLFWwindow* window, int focused);


    ////////////////////
    //                //
    //      Core      //

    void Init(API api, WindowedMode windowedMode) {

        g_api = api;

        if (GetAPI() == API::OPENGL) {
            // Nothing required
        }

        glfwInit();
        glfwSetErrorCallback([](int error, const char* description) { std::cout << "GLFW Error (" << std::to_string(error) << "): " << description << "\n";});

        if (GetAPI() == API::OPENGL) {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
        }
        else if (GetAPI() == API::VULKAN) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        }
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);

        // Resolution and window size
        g_monitor = glfwGetPrimaryMonitor();
        g_mode = glfwGetVideoMode(g_monitor);

        glfwWindowHint(GLFW_RED_BITS, g_mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, g_mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, g_mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, g_mode->refreshRate); 
        
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

        g_fullscreenWidth = g_mode->width;
        g_fullscreenHeight = g_mode->height;
        g_windowedWidth = g_fullscreenWidth * 0.75f;
        g_windowedHeight = g_fullscreenHeight * 0.75f;

        // Create window
        g_windowedMode = windowedMode;
        if (g_windowedMode == WindowedMode::WINDOWED) {
            g_currentWindowWidth = g_windowedWidth;
            g_currentWindowHeight = g_windowedHeight;
            g_window = glfwCreateWindow(g_windowedWidth, g_windowedHeight, "Unloved", NULL, NULL);
            glfwSetWindowPos(g_window, 0, 0);
        }
        else if (windowedMode == WindowedMode::FULLSCREEN) {
            g_currentWindowWidth = g_fullscreenWidth;
            g_currentWindowHeight = g_fullscreenHeight;
            g_window = glfwCreateWindow(g_fullscreenWidth, g_fullscreenHeight, "Unloved", g_monitor, NULL);
        }

        if (g_window == NULL) {
            std::cout << "Failed to create GLFW window\n";
            glfwTerminate();
            return;
        }

        glfwSetFramebufferSizeCallback(g_window, framebuffer_size_callback);
        glfwSetWindowFocusCallback(g_window, window_focus_callback);

        AssetManager::FindAssetPaths();

        if (GetAPI() == API::OPENGL) {
            glfwMakeContextCurrent(g_window);
            OpenGLBackEnd::InitMinimum();
            OpenGLRenderer::InitMinimum();
            glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
        }

        AssetManager::LoadFont();

        // Init sub-systems
        Input::Init();
        Audio::Init();
        glfwShowWindow(BackEnd::GetWindowPointer());
    }

    void BeginFrame() {
        glfwPollEvents();
    }

    void EndFrame() {
        if (!g_window) {
            std::cout << "BackEnd::EndFrame(): g_window was nullptr\n";
        }

        // OpenGL
        if (GetAPI() == API::OPENGL) {
            glfwSwapBuffers(g_window);
        }
        // Vulkan
        else if (GetAPI() == API::VULKAN){

        }
    }

    void UpdateSubSystems() {
        Input::Update();
        Audio::Update();
    }

    void CleanUp() {
        if (g_window) {
            glfwMakeContextCurrent(nullptr);
            glfwDestroyWindow(g_window);
            g_window = nullptr;
        }
        glfwTerminate();
    }

    ///////////////////
    //               //
    //      API      //

    void SetAPI(API api) {
        g_api = api;
    }

    const API& GetAPI() {
        return g_api;
    }

    // Window
    GLFWwindow* GetWindowPointer() {
        return g_window;
    }

    void SetWindowPointer(GLFWwindow* window) {
        g_window = window;
    }

    void SetWindowedMode(const WindowedMode& windowedMode) {
        if (windowedMode == WindowedMode::WINDOWED) {
            g_currentWindowWidth = g_windowedWidth;
            g_currentWindowHeight = g_windowedHeight;
            glfwSetWindowMonitor(g_window, nullptr, 0, 0, g_windowedWidth, g_windowedHeight, g_mode->refreshRate);
            glfwSetWindowPos(g_window, 0, 0);
        }
        else if (windowedMode == WindowedMode::FULLSCREEN) {
            g_currentWindowWidth = g_fullscreenWidth;
            g_currentWindowHeight = g_fullscreenHeight;
            glfwSetWindowMonitor(g_window, nullptr, 0, 0, g_fullscreenWidth - 1, g_fullscreenHeight - 1, g_mode->refreshRate);
        }
        g_windowedMode = windowedMode;
    }

    void ToggleFullscreen() {
        if (g_windowedMode == WindowedMode::WINDOWED) {
            SetWindowedMode(WindowedMode::FULLSCREEN);
        }
        else {
            SetWindowedMode(WindowedMode::WINDOWED);
        }
    }

    void ForceCloseWindow() {
        g_forceCloseWindow = true;
    }

    bool WindowHasFocus() {
        return g_windowHasFocus;
    }

    bool WindowHasNotBeenForceClosed() {
        return !g_forceCloseWindow;
    }

    int GetWindowedWidth() {
        return g_windowedWidth;
    }

    int GetWindowedHeight() {
        return g_windowedHeight;
    }

    int GetFullScreenWidth() {
        return g_fullscreenWidth;
    }

    int GetFullScreenHeight() {
        return g_fullscreenHeight;
    }

    int GetCurrentWindowWidth() {
        return g_currentWindowWidth;
    }

    int GetCurrentWindowHeight() {
        return g_currentWindowHeight;
    }

    bool WindowIsOpen() {
        return !(glfwWindowShouldClose(g_window) || g_forceCloseWindow);
    }

    bool WindowIsMinimized() {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(g_window, &width, &height);
        return (width == 0 || height == 0);
    }

    const WindowedMode& GetWindowMode() {
        return g_windowedMode;
    }

    //////////////////////////////
    //                          //
    //      Render Targets      //

    void SetPresentTargetSize(int width, int height) {
        g_presentTargetWidth = width;
        g_presentTargetHeight = height;
        if (GetAPI() == API::OPENGL) {
            //OpenGLBackEnd::SetPresentTargetSize(width, height);
        }
        else {
            //VulkanBackEnd::SetPresentTargetSize(width, height);
        }
    }

    int GetPresentTargetWidth() {
        return g_presentTargetWidth;
    }

    int GetPresentTargetHeight() {
        return g_presentTargetHeight;
    }


    /////////////////////////
    //                     //
    //      Callbacks      //

    void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
        if (GetAPI() == API::OPENGL) {

        }
    }

    void window_focus_callback(GLFWwindow* /*window*/, int focused) {
        if (focused) {
            BackEnd::g_windowHasFocus = true;
        }
        else {
            BackEnd::g_windowHasFocus = false;
        }
    }
}

