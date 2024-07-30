#include "Engine.h"
#include "API/OpenGL/GL_BackEnd.h"
#include "BackEnd/BackEnd.h"
#include "Core/AssetManager.h"
#include "Core/Game.h"
#include "Renderer/Renderer.h"

void Engine::Run() {

    BackEnd::Init(API::OPENGL);

    while (BackEnd::WindowIsOpen()) {

        BackEnd::BeginFrame();
        BackEnd::UpdateSubSystems();

        // Load
        if (!AssetManager::LoadingComplete()) {
            AssetManager::LoadNextItem();
            Renderer::RenderLoadingScreen();
        }
        // Render
        else {
            Game::Update();
            Renderer::RenderFrame();
        }
        BackEnd::EndFrame();
    }

    BackEnd::CleanUp();
}


