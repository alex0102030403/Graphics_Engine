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

auto isLit = false;
auto isAppRunning = true;

const auto SCREEN_WIDTH = 1920;
const auto SCREEN_HEIGHT = 1080;
const auto CONSOLE_WINDOW_HEIGHT = 250;
const auto PROPERTIES_WINDOW_WIDTH = 400;

std::deque<std::string> messages;
std::vector<std::unique_ptr<Cube>> objects;

Cube* currentSelectedObject = nullptr;

// Helper function to convert quaternion to Euler angles (in radians)
glm::vec3 quaternionToEuler(const glm::quat& q) {
    return glm::eulerAngles(q); // Returns in radians; assumes GLM convention (XYZ order)
}

void setupDemo(PhysicsWorld& world, Grid* grid) {
    // Ground (static)
    RigidBody* ground = new RigidBody();
    ground->position = glm::vec3(0, -1, 0);
    ground->orientation = glm::quat(1, 0, 0, 0);
    ground->orientation = glm::normalize(ground->orientation);
    ground->setMass(0); // Static
    ground->setInertiaTensor(glm::mat3(0));
    ground->collider = new BoxCollider(glm::vec3(50, 1, 50));
    world.addBody(ground);

    auto groundCube = std::make_unique<Cube>("crate1.png", grid);
    groundCube->GetTransform().SetPosition(0, -1, 0);
    groundCube->GetTransform().SetScale(50, 1, 50); // Full extents: 100*2, 1*2, 100*2
    groundCube->SetStatic(true);
    objects.push_back(std::move(groundCube));
	objects.back()->SetColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)); // Gray color for ground

    
    RigidBody* wall = new RigidBody();
    wall->position = glm::vec3(0, -1, 0);
    wall->orientation = glm::quat(1, 0, 0, 0);
    wall->setMass(0);
    wall->setInertiaTensor(glm::mat3(0));
    wall->collider = new BoxCollider(glm::vec3(10, 10, 1));
    world.addBody(wall);

    auto wallCube = std::make_unique<Cube>("crate1.png", grid);
    wallCube->GetTransform().SetPosition(0, -1, 0);
    wallCube->GetTransform().SetScale(10, 10, 1); // Full extents: 100*2, 1*2, 100*2
    wallCube->SetStatic(true);
    objects.push_back(std::move(wallCube));
    objects.back()->SetColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)); // Gray color for ground


  

    // Dynamic cubes
    for (int i = 0; i < 40; ++i) {
        RigidBody* cube = new RigidBody();
		// Set random position for each cube
		// Randomize position along X-axis, Y is fixed at 10, Z is 0
		// This will create a line of cubes along the X-axis
		// Randomize position along X-axis, Y is fixed at 10, Z is 0
		// Randomize position along X-axis, Y is fixed at 10, Z is 0

		cube->position = glm::vec3(static_cast<float>(rand() % 15 - 10), static_cast<float>(rand() % 15
            - 5), static_cast<float>(rand() % 20 - 10));
        //apply random rotation
		cube->orientation = glm::quat(glm::vec3(glm::radians(static_cast<float>(rand() % 360)), 0, 0)); // Random Y rotation
		// Randomize orientation slightly for variety   
        cube->orientation = glm::quat(1, 0, 0, 0);
        cube->setMass(10.0f);
        float halfSize = 0.5f; // Half extents for a 1x1x1 cube
        float size = 1.0f;     // Full size for rendering
        float I = (1.0f / 6.0f) * 10.0f * size * size; // Inertia for a cube
        cube->setInertiaTensor(glm::mat3(I, 0, 0, 0, I, 0, 0, 0, I));
        cube->collider = new BoxCollider(glm::vec3(halfSize, halfSize, halfSize));
        world.addBody(cube);

        auto cubeObj = std::make_unique<Cube>("crate2.png", grid);
        cubeObj->GetTransform().SetPosition(cube->position.x, cube->position.y, cube->position.z);
        cubeObj->GetTransform().SetScale(size, size, size);
        cubeObj->SetStatic(false);
        objects.push_back(std::move(cubeObj));
		objects.back()->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red color for dynamic cubes
    }
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
    } else {
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

    Shader lightShader;
    lightShader.Create("Shaders/Light.vert", "Shaders/Light.frag");

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
        float deltaTime = (currentTime - previousTime) / 1000.0f; // Convert to seconds
        previousTime = currentTime;

        Screen::Instance()->ClearScreen();
        Input::Instance()->Update();

        world.step(deltaTime);

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

        mouseCollider = { static_cast<int>(Input::Instance()->GetMousePosition().x),
                          static_cast<int>(Input::Instance()->GetMousePosition().y),
                          1,
                          1 };

        bool isMouseColliding = SDL_HasIntersection(&mouseCollider, &sceneCollider);

        if (isMouseColliding && Input::Instance()->IsRightButtonClicked()) {
            auto mouseMotion = Input::Instance()->GetMouseMotion();
            camera.UpdateRotation(-mouseMotion.y * mouseSensitivity, -mouseMotion.x * mouseSensitivity);

            if (Input::Instance()->IsKeyDown('w')) {
                camera.MoveForward(deltaTime);
            }
            if (Input::Instance()->IsKeyDown('s')) {
                camera.MoveBackward(deltaTime);
            }
            if (Input::Instance()->IsKeyDown('a')) {
                camera.MoveLeft(deltaTime);
            }
            if (Input::Instance()->IsKeyDown('d')) {
                camera.MoveRight(deltaTime);
            }
        }

        if (isMouseColliding && Input::Instance()->IsLeftButtonClicked()) {
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
                // Trigger explosion at intersection point
                world.applyExplosion(intersectionPoint, 5.0f, 20.0f); // radius = 5, strength = 20
                currentSelectedObject = nullptr; // Deselect object after explosion
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

        grid.Render(defaultShader);
        light.Render(defaultShader);

       

        for (auto& object : objects) {
            if (isLit) {
                lightShader.Use();
                light.SendToShader(lightShader);
                camera.SendToShader(lightShader);
                object->Render(lightShader);
            } else {
                defaultShader.Use();
                camera.SendToShader(defaultShader);
                object->Render(defaultShader);
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        RenderConsoleWindow();
        RenderPropertiesWindow();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        Screen::Instance()->Present();
    }

    lightShader.Destroy();
    defaultShader.Destroy();

    Shader::Shutdown();
    Screen::Instance()->Shutdown();

    return 0;
}