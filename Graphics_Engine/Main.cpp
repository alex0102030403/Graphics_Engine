#include <deque>
#include <iostream>
#include <memory>
#include <vector>

#include "gl.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include <SDL.h>

#include "Camera.h"
#include "Cube.h"
#include "Grid.h"
#include "Input.h"
#include "Light.h"
#include "Model.h"
#include "Quad.h"
#include "Screen.h"
#include "Shader.h"
#include "Utility.h"
#include "PhysicsWorld.h"
#include "ParticleSystem.h"
#include "Skybox.h"

auto isLit = false;
auto isAppRunning = true;

const auto SCREEN_WIDTH = 1920;
const auto SCREEN_HEIGHT = 1080;
const auto CONSOLE_WINDOW_HEIGHT = 250;
const auto PROPERTIES_WINDOW_WIDTH = 400;

std::deque<std::string> messages;
std::vector<std::unique_ptr<Cube>> objects;

Cube* currentSelectedObject = nullptr;
RigidBody* mainCubeBody = nullptr; // Reference to the main cube's RigidBody

// Helper function to convert quaternion to Euler angles (in radians)
glm::vec3 quaternionToEuler(const glm::quat& q) {
    return glm::eulerAngles(q);
}

void setupDemo(PhysicsWorld& world, Grid* grid) {
    // Ground (static) - Long and narrow plane
    RigidBody* ground = new RigidBody();
    ground->position = glm::vec3(0, -1, 0);
    ground->orientation = glm::quat(1, 0, 0, 0);
    ground->setMass(0); // Static
    ground->setInertiaTensor(glm::mat3(0));
    ground->collider = new BoxCollider(glm::vec3(5, 0.5, 500)); // 10 units wide, 1000 units long
    world.addBody(ground);

    auto groundCube = std::make_unique<Cube>("crate1.png", grid);
    groundCube->GetTransform().SetPosition(0, -1, 0);
    groundCube->GetTransform().SetScale(10, 1, 1000); // Match collider size visually
    groundCube->SetStatic(true);
    objects.push_back(std::move(groundCube));
    objects.back()->SetColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)); // Gray color for ground

    // Dynamic cubes - Randomly spawned along the plane
    for (int i = 0; i < 100; ++i) {
        RigidBody* cube = new RigidBody();
        float x = static_cast<float>(rand() % 8 - 4); // x from -4 to 4
        float y = 0.5f; // Just above the plane
        float z = static_cast<float>(rand() % 800 - 400); // z from -400 to 400
        cube->position = glm::vec3(x, y, z);
        cube->orientation = glm::quat(1, 0, 0, 0);
        cube->setMass(10.0f);
        float size = 1.0f;
        float I = (1.0f / 6.0f) * 10.0f * size * size;
        cube->setInertiaTensor(glm::mat3(I, 0, 0, 0, I, 0, 0, 0, I));
        cube->collider = new BoxCollider(glm::vec3(0.5f, 0.5f, 0.5f));
        cube->name = "DynamicCube" + std::to_string(i);
        world.addBody(cube);

        auto cubeObj = std::make_unique<Cube>("crate2.png", grid);
        cubeObj->GetTransform().SetPosition(cube->position.x, cube->position.y, cube->position.z);
        cubeObj->GetTransform().SetScale(size, size, size);
        cubeObj->SetStatic(false);
        objects.push_back(std::move(cubeObj));
        objects.back()->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red color for dynamic cubes
    }

    // Main cube - The player-controlled cube
    RigidBody* mainCube = new RigidBody();
    mainCube->position = glm::vec3(0, 0.5f, -400); // Start near the beginning
    mainCube->orientation = glm::quat(1, 0, 0, 0);
    mainCube->setMass(10.0f);
    float size = 1.0f;
    float I = (1.0f / 6.0f) * 10.0f * size * size;
    mainCube->setInertiaTensor(glm::mat3(I, 0, 0, 0, I, 0, 0, 0, I));
    mainCube->collider = new BoxCollider(glm::vec3(0.5f, 0.5f, 0.5f));
    mainCube->name = "MainCube";
    world.addBody(mainCube);
    mainCubeBody = mainCube; // Set global reference

    auto mainCubeObj = std::make_unique<Cube>("crate2.png", grid);
    mainCubeObj->GetTransform().SetPosition(mainCube->position.x, mainCube->position.y, mainCube->position.z);
    mainCubeObj->GetTransform().SetScale(size, size, size);
    mainCubeObj->SetStatic(false);
    objects.push_back(std::move(mainCubeObj));
    objects.back()->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // Green color for main cube
}

void RenderConsoleWindow() {
    ImGui::Begin("Output console", nullptr,
        ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);

    auto windowPos = ImVec2(0, SCREEN_HEIGHT - CONSOLE_WINDOW_HEIGHT - 25);
    auto windowSize = ImVec2(SCREEN_WIDTH - PROPERTIES_WINDOW_WIDTH, CONSOLE_WINDOW_HEIGHT);

    ImGui::SetWindowPos("Output console", windowPos);
    ImGui::SetWindowSize("Output console", windowSize);

    auto message = Utility::ReadMessage();
    if (!message.empty()) {
        messages.push_front(message);
    }

    for (const auto& message : messages) {
        ImGui::Text(message.c_str());
    }

    ImGui::End();
}

void RenderPropertiesWindow() {
    ImGui::Begin("Properties", nullptr,
        ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);

    auto windowPos = ImVec2(SCREEN_WIDTH - PROPERTIES_WINDOW_WIDTH, 0);
    auto windowSize = ImVec2(PROPERTIES_WINDOW_WIDTH, SCREEN_HEIGHT);

    ImGui::SetWindowPos("Properties", windowPos);
    ImGui::SetWindowSize("Properties", windowSize);

    if (currentSelectedObject) {
        ImGui::Text("Selected object: %s", typeid(*currentSelectedObject).name());

        auto position = currentSelectedObject->GetTransform().GetPosition();
        ImGui::SliderFloat3("Position", &position.x, -10.0f, 10.0f, "%.2f");
        currentSelectedObject->GetTransform().SetPosition(position.x, position.y, position.z);

        auto rotation = currentSelectedObject->GetTransform().GetRotation();
        ImGui::SliderFloat3("Rotation", &rotation.x, -360.0f, 360.0f, "%.2f");
        currentSelectedObject->GetTransform().SetRotation(rotation.x, rotation.y, rotation.z);

        auto scale = currentSelectedObject->GetTransform().GetScale();
        ImGui::SliderFloat3("Scale", &scale.x, 0.001f, 10.0f, "%.2f");
        currentSelectedObject->GetTransform().SetScale(scale.x, scale.y, scale.z);

        ImGui::Separator();
        ImGui::Button("Crate 1 texture");
        ImGui::Button("Crate 2 texture");
        ImGui::Separator();

        auto isTextured = currentSelectedObject->IsTextured();
        ImGui::Checkbox("Textured", &isTextured);
        currentSelectedObject->IsTextured(isTextured);

        ImGui::Separator();
        auto color = currentSelectedObject->GetColor();
        ImGui::ColorEdit4("Color", &color.r);
        currentSelectedObject->SetColor(color);
    }
    else {
        ImGui::Text("No object selected");
    }

    ImGui::Checkbox("Light the scene", &isLit);
    ImGui::Separator();
    ImGui::End();
}

int main(int argc, char* argv[]) {
    if (!Screen::Instance()->Initialize()) {
        return 0;
    }

    if (!Shader::Initialize()) {
        return 0;
    }

    Shader defaultShader;
    defaultShader.Create("Shaders/Default.vert", "Shaders/Default.frag");

    Shader skyboxShader;
    skyboxShader.Create("Shaders/Skybox.vert", "Shaders/Skybox.frag");

    Shader lightShader;
    lightShader.Create("Shaders/Light.vert", "Shaders/Light.frag");

    Shader particleShader;
    particleShader.Create("Shaders/Particle.vert", "Shaders/Particle.frag", "Shaders/Particle.geom");

    

    std::vector<std::string> skyboxFaces = {
        "Crate_1.png",
         "Crate_1.png",
          "Crate_1.png",
          "Crate_1.png",
           "Crate_1.png",
            "Crate_1.png",
    };

    Skybox skybox(skyboxFaces);

    Grid grid;

    PhysicsWorld world;
    setupDemo(world, &grid);

    Camera camera;
    camera.Set3DView();
    camera.SetSpeed(3.0f);
    camera.SetViewport(0,
        CONSOLE_WINDOW_HEIGHT,
        SCREEN_WIDTH - PROPERTIES_WINDOW_WIDTH,
        SCREEN_HEIGHT - CONSOLE_WINDOW_HEIGHT);

    Light light;
    light.SetSpeed(0.5f);

    ParticleSystem explosionSystem;
    explosionSystem.Initialize(1000, "Textures/Crate_1.png");

    ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/Arial.ttf", 16.0f);
    ImGui::GetIO().Fonts->Build();

    SDL_Rect mouseCollider = { 0 };
    SDL_Rect sceneCollider = { 0,
                               0,
                               SCREEN_WIDTH - PROPERTIES_WINDOW_WIDTH,
                               SCREEN_HEIGHT - CONSOLE_WINDOW_HEIGHT };

    float mouseSensitivity = 0.005f;

    Uint32 previousTime = SDL_GetTicks();

    while (isAppRunning) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;

        Screen::Instance()->ClearScreen();
        Input::Instance()->Update();

        // Control main cube movement
        static float speed = 5.0f;
        speed += 0.01f * deltaTime; // Gradually increase speed
        mainCubeBody->linearVelocity.z = speed;

        if (Input::Instance()->IsKeyDown('d')) {
            mainCubeBody->linearVelocity.x = -2.0f; // Move left
        }
        else if (Input::Instance()->IsKeyDown('a')) {
            mainCubeBody->linearVelocity.x = 2.0f; // Move right
        }
        else {
            mainCubeBody->linearVelocity.x = 0.0f; // No lateral movement
        }

        world.step(deltaTime);

        explosionSystem.Update(deltaTime);

        // Synchronize Cube transformations with RigidBody states
        for (size_t i = 0; i < world.bodies.size() && i < objects.size(); ++i) {
            RigidBody* body = world.bodies[i];
            Cube* cube = objects[i].get();
            if (body->inverseMass > 0) { // Dynamic objects only
                cube->GetTransform().SetPosition(body->position.x, body->position.y, body->position.z);
                glm::vec3 euler = quaternionToEuler(body->orientation);
                cube->GetTransform().SetRotation(glm::degrees(euler.x), glm::degrees(euler.y), glm::degrees(euler.z));
            }
        }

        // Camera follows main cube
        glm::vec3 mainCubePos = mainCubeBody->position;
        glm::vec3 cameraPos = mainCubePos + glm::vec3(0, 5, -10); // Behind and above
        camera.GetTransform().SetPosition(cameraPos.x, cameraPos.y, cameraPos.z);
        camera.LookAt(mainCubePos);

        mouseCollider = { static_cast<int>(Input::Instance()->GetMousePosition().x),
                          static_cast<int>(Input::Instance()->GetMousePosition().y),
                          1,
                          1 };

        bool isMouseColliding = SDL_HasIntersection(&mouseCollider, &sceneCollider);

        if (isMouseColliding && Input::Instance()->IsRightButtonClicked()) {
            auto mouseMotion = Input::Instance()->GetMouseMotion();
            camera.UpdateRotation(-mouseMotion.y * mouseSensitivity, -mouseMotion.x * mouseSensitivity);
            // WASD controls removed
        }

        if (isMouseColliding && Input::Instance()->IsLeftButtonClicked() && Input::Instance()->GetKeyDown() == 'f') {
            messages.push_front("Explosion triggered at: " +
                std::to_string(static_cast<int>(Input::Instance()->GetMousePosition().x)) + ", " +
                std::to_string(static_cast<int>(Input::Instance()->GetMousePosition().y)));

            auto ray = camera.GetPickingRay(
                Input::Instance()->GetMousePosition().x,
                Input::Instance()->GetMousePosition().y,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                0,
                CONSOLE_WINDOW_HEIGHT,
                SCREEN_WIDTH - PROPERTIES_WINDOW_WIDTH,
                SCREEN_HEIGHT - CONSOLE_WINDOW_HEIGHT
            );

            float minT = std::numeric_limits<float>::max();
            glm::vec3 intersectionPoint;
            bool hit = false;

            for (auto& object : objects) {
                float t;
                if (object->IntersectsRay(ray, t) && t < minT) {
                    minT = t;
                    intersectionPoint = ray.origin + t * ray.direction;
                    hit = true;
                }
            }

            if (hit) {
                world.applyExplosion(intersectionPoint, 5.0f, 20.0f);
                explosionSystem.SpawnExplosion(intersectionPoint, 100, 0.5f, 2.0f, 1.0f);
                currentSelectedObject = nullptr;
            }
        }

        if (isMouseColliding && Input::Instance()->IsLeftButtonPressed()) {
            messages.push_front("Picking ray casted at: " +
                std::to_string(static_cast<int>(Input::Instance()->GetMousePosition().x)) + ", " +
                std::to_string(static_cast<int>(Input::Instance()->GetMousePosition().y)));

            auto ray = camera.GetPickingRay(
                Input::Instance()->GetMousePosition().x,
                Input::Instance()->GetMousePosition().y,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                0,
                CONSOLE_WINDOW_HEIGHT,
                SCREEN_WIDTH - PROPERTIES_WINDOW_WIDTH,
                SCREEN_HEIGHT - CONSOLE_WINDOW_HEIGHT
            );

            float minT = std::numeric_limits<float>::max();
            Cube* selectedObject = nullptr;
            for (auto& object : objects) {
                float t;
                if (object->IntersectsRay(ray, t) && t < minT) {
                    minT = t;
                    selectedObject = object.get();
                }
            }
            currentSelectedObject = selectedObject;
        }

        isAppRunning = !Input::Instance()->IsXClicked();

        defaultShader.Use();
        camera.SendToShader(defaultShader);

        glDepthMask(GL_FALSE);
        skybox.Render(skyboxShader, camera);
        glDepthMask(GL_TRUE);

        grid.Render(defaultShader);
        light.Render(defaultShader);

        for (auto& object : objects) {
            if (isLit) {
                lightShader.Use();
                light.SendToShader(lightShader);
                camera.SendToShader(lightShader);
                object->Render(lightShader);
            }
            else {
                defaultShader.Use();
                camera.SendToShader(defaultShader);
                object->Render(defaultShader);
            }
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        explosionSystem.Render(particleShader, camera);
        glDisable(GL_BLEND);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        RenderConsoleWindow();
        RenderPropertiesWindow();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        Screen::Instance()->Present();
    }

    skyboxShader.Destroy();
    lightShader.Destroy();
    defaultShader.Destroy();
    particleShader.Destroy();

    Shader::Shutdown();
    Screen::Instance()->Shutdown();

    return 0;
}