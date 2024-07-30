#include "Game.h"
#include "Input.h"
#include "../BackEnd/BackEnd.h"
#include "../Core/Audio.hpp"
#include "../Core/Pathfinding.h"
#include "../Renderer/Renderer.h"

namespace Game {

    bool g_isLoaded = false;
    double g_deltaTimeAccumulator = 0.0;
    double g_fixedDeltaTime = 1.0 / 60.0;
    bool g_showDebugText = true;

    bool g_firstFrame = true;
    double g_lastFrame = 0;
    double g_thisFrame = 0;

    void EvaluateDebugKeyPresses();

    void Create() {
        Pathfinding::Init();
        g_isLoaded = true;
        g_firstFrame = true;
    }

    bool IsLoaded() {
        return g_isLoaded;
    }

    void Update() {

        if (!g_isLoaded) {
            Create();
        }

        if (g_firstFrame) {
            g_thisFrame = glfwGetTime();
            g_firstFrame = false;
        }

        // Delta time
        g_lastFrame = g_thisFrame;
        g_thisFrame = glfwGetTime();
        double deltaTime = g_thisFrame - g_lastFrame;
        g_deltaTimeAccumulator += deltaTime;

        Pathfinding::Update(deltaTime);

        EvaluateDebugKeyPresses();
    }

    void EvaluateDebugKeyPresses() {

        if (Input::KeyPressed(HELL_KEY_B)) {
            Renderer::NextDebugLineRenderMode();
            Audio::PlayAudio(AUDIO_SELECT, 1.00f);
        }
        if (Input::KeyPressed(HELL_KEY_GRAVE_ACCENT)) {
            g_showDebugText = !g_showDebugText;
            Audio::PlayAudio(AUDIO_SELECT, 1.00f);
        }
    }

    const bool DebugTextIsEnabled() {
        return g_showDebugText;
    }
}
