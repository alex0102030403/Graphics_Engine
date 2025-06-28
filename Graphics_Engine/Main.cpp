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
#include "Object.h"

auto isLit = false;
auto isAppRunning = true;

const auto SCREEN_WIDTH = 1920;
const auto SCREEN_HEIGHT = 1080;
const auto CONSOLE_WINDOW_HEIGHT = 300;
const auto PROPERTIES_WINDOW_WIDTH = 300;

std::deque<std::string> messages;
std::vector<std::unique_ptr<Cube>> objects;
std::vector<std::unique_ptr<Object>> models;
std::vector<std::unique_ptr<Object>> dynamicModels;

Cube* currentSelectedObject = nullptr;
RigidBody* mainCubeBody = nullptr; // Reference to the main cube's RigidBody

// Helper function to convert quaternion to Euler angles (in radians)
glm::vec3 quaternionToEuler(const glm::quat& q) {
    return glm::eulerAngles(q);
}

void setupDemo(PhysicsWorld& world, Grid* grid) {
    // Ground (static) - Large square plane with grass texture
    RigidBody* ground = new RigidBody();
    ground->position = glm::vec3(0, -1, 0);
    ground->orientation = glm::quat(1, 0, 0, 0);
    ground->setMass(0); // Static
    ground->setInertiaTensor(glm::mat3(0));
    ground->collider = new BoxCollider(glm::vec3(50, 0.5, 50)); // 100x100 units
    world.addBody(ground);

    auto groundCube = std::make_unique<Cube>("Grass.png", grid);
    groundCube->GetTransform().SetPosition(0, -1, 0);
    groundCube->GetTransform().SetScale(100, 1, 100); // Match collider size visually
    groundCube->SetStatic(true);
    objects.push_back(std::move(groundCube));
    objects.back()->SetColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)); // Base color, overridden by texture

    int cubeCounter = 0;
    for (int i = 0; i < 9; ++i) { // 9 layers
        int size = 9 - i;         // Size of the current layer (9 down to 1)
        for (int ix = 0; ix < size; ++ix) {
            for (int iz = 0; iz < size; ++iz) {
                RigidBody* cube = new RigidBody();
                float x = 20.0f + (ix - (size - 1) / 2.0f); // Center x at 20
                float y = 0.5f + i * 1.0f;                  // Stack vertically, 1 unit apart
                float z = 0.0f + (iz - (size - 1) / 2.0f);  // Center z at 0
                cube->position = glm::vec3(x, y, z);
                cube->orientation = glm::quat(1, 0, 0, 0);
                cube->setMass(10.0f);
                float cubeSize = 1.0f;
                float I = (1.0f / 6.0f) * 10.0f * cubeSize * cubeSize;
                cube->setInertiaTensor(glm::mat3(I, 0, 0, 0, I, 0, 0, 0, I));
                cube->collider = new BoxCollider(glm::vec3(0.5f, 0.5f, 0.5f));
                cube->collider->name = "box";
                cube->name = "DynamicCube" + std::to_string(cubeCounter++);
                world.addBody(cube);
                auto cubeObj = std::make_unique<Cube>("Crate_1.png", grid);
                cubeObj->GetTransform().SetPosition(cube->position.x, cube->position.y, cube->position.z);
                cubeObj->GetTransform().SetScale(cubeSize, cubeSize, cubeSize);
                cubeObj->SetStatic(false);
				cubeObj->IsTextured(true); // Enable texture for cubes
                objects.push_back(std::move(cubeObj));
                objects.back()->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // White color for dynamic cubes
            }
        }
    }

    // Dynamic cubes - Randomly spawned on the plane
    for (int i = 0; i < 4; ++i) {
        RigidBody* cube = new RigidBody();
        float x = static_cast<float>(rand() % 100 - 50); // x from -50 to 50
        float y = 0.5f; // Just above the plane
        float z = static_cast<float>(rand() % 100 - 50); // z from -50 to 50
        cube->position = glm::vec3(x, y, z);
        cube->orientation = glm::quat(1, 0, 0, 0);
        cube->setMass(10.0f);
        float size = 1.0f;
        float I = (1.0f / 6.0f) * 10.0f * size * size;
        cube->setInertiaTensor(glm::mat3(I, 0, 0, 0, I, 0, 0, 0, I));
        cube->collider = new BoxCollider(glm::vec3(0.5f, 0.5f, 0.5f));
        cube->collider->name = "box";
        cube->name = "DynamicCube" + std::to_string(i);
        world.addBody(cube);

        auto cubeObj = std::make_unique<Cube>("Crate_1.png", grid);
        cubeObj->GetTransform().SetPosition(cube->position.x, cube->position.y, cube->position.z);
        cubeObj->GetTransform().SetScale(size, size, size);
        cubeObj->SetStatic(false);
        objects.push_back(std::move(cubeObj));
        objects.back()->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red color for dynamic cubes
    }

    // Tree models - Static environment objects
    for (int i = 0; i < 10; ++i) {
        float x = static_cast<float>(rand() % 100 - 50); // x from -50 to 50
        float z = static_cast<float>(rand() % 100 - 50); // z from -50 to 50
        auto treeObj = std::make_unique<Model>("Models/Lowpoly_tree_sample.obj", grid);
		//treeObj->IsTextured(true);
        
        treeObj->GetTransform().SetPosition(x, 0, z); // At ground level
        //treeObj->GetTransform().SetScale(1, 1, 1); // Default scale, adjust as needed
        models.push_back(std::move(treeObj));
    }

    // Main cube - The player
    RigidBody* mainCube = new RigidBody();
    mainCube->position = glm::vec3(0, 0.5f, 0); // Start at center of plane
    mainCube->orientation = glm::quat(1, 0, 0, 0);
    mainCube->setMass(10.0f);
    float size = 1.0f;
    float I = (1.0f / 6.0f) * 10.0f * size * size;
    mainCube->setInertiaTensor(glm::mat3(I, 0, 0, 0, I, 0, 0, 0, I));
    mainCube->collider = new BoxCollider(glm::vec3(0.5f, 0.5f, 0.5f));
    mainCube->name = "MainCube";
    world.addBody(mainCube);
    mainCubeBody = mainCube; // Set global reference

    auto mainCubeObj = std::make_unique<Cube>("Crate_1.png", grid);
    mainCubeObj->GetTransform().SetPosition(mainCube->position.x, mainCube->position.y, mainCube->position.z);
    mainCubeObj->GetTransform().SetScale(0.1, 0.1, 0.1);
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

    Shader particleShader;
    particleShader.Create("Particle.vert", "Particle.frag", "Particle.geom");

    Shader defaultShader;
    defaultShader.Create("Shaders/Default.vert", "Shaders/Default.frag");

    Shader skyboxShader;
    skyboxShader.Create("skybox.vert", "skybox.frag");

    Shader lightShader;
    lightShader.Create("Shaders/Light.vert", "Shaders/Light.frag");

    std::vector<std::string> skyboxFaces = {
        "Textures/Skybox/left.tga",
        "Textures/Skybox/right.tga",
        "Textures/Skybox/top.tga",
        "Textures/Skybox/bottom.tga",
        "Textures/Skybox/front.tga",
        "Textures/Skybox/back.tga"
    };

    Skybox skybox(skyboxFaces, &skyboxShader);

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
    explosionSystem.Initialize(1000, "Textures/flame.png");

    ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/Arial.ttf", 16.0f);
    ImGui::GetIO().Fonts->Build();

    SDL_Rect mouseCollider = { 0 };
    SDL_Rect sceneCollider = { 0,
                               0,
                               SCREEN_WIDTH - PROPERTIES_WINDOW_WIDTH,
                               SCREEN_HEIGHT - CONSOLE_WINDOW_HEIGHT };

    float mouseSensitivity = 0.005f;

    Uint32 previousTime = SDL_GetTicks();

	bool isGameRunning = true; // Track cursor focus state

    while (isAppRunning) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;

        Screen::Instance()->ClearScreen();
        Input::Instance()->Update();

        if (isGameRunning) {
            SDL_SetRelativeMouseMode(SDL_TRUE);  // Lock and hide cursor
        }
        else {
            SDL_SetRelativeMouseMode(SDL_FALSE); // Restore cursor
        }
        
        if (Input::Instance()->GetKeyDown() == 'q') {
            isGameRunning = !isGameRunning; // Toggle cursor focus state
			if (isGameRunning) {
				SDL_SetRelativeMouseMode(SDL_TRUE);  // Lock and hide cursor
			}
			else {
				SDL_SetRelativeMouseMode(SDL_FALSE); // Restore cursor
			}
        }

        // Camera position attached to main cube (first-person view)
        glm::vec3 mainCubePos = mainCubeBody->position;
        glm::vec3 cameraPos = mainCubePos + glm::vec3(0, 1.5, 0); // Slightly above the cube
        camera.GetTransform().SetPosition(cameraPos.x, cameraPos.y, cameraPos.z);

        // Mouse control for camera rotation
        mouseCollider = { static_cast<int>(Input::Instance()->GetMousePosition().x),
                          static_cast<int>(Input::Instance()->GetMousePosition().y),
                          1,
                          1 };
        bool isMouseColliding = SDL_HasIntersection(&mouseCollider, &sceneCollider);
        if (isMouseColliding && isGameRunning) {
            auto mouseMotion = Input::Instance()->GetMouseMotion();
            camera.UpdateRotation(-mouseMotion.y * mouseSensitivity, -mouseMotion.x * mouseSensitivity);
        }
        

        // WASD movement controls
        float movementSpeed = 5.0f;
        glm::vec3 forward = camera.GetForwardVector();
        forward.y = 0; // Restrict to horizontal plane
        forward = glm::normalize(forward);
        glm::vec3 right = camera.GetRightVector();
        right.y = 0; // Restrict to horizontal plane
        right = glm::normalize(right);

        glm::vec3 velocity(0.0f);
        if (Input::Instance()->IsKeyDown('w')) {
            velocity += forward * movementSpeed;
        }
        if (Input::Instance()->IsKeyDown('s')) {
            velocity -= forward * movementSpeed;
        }
        if (Input::Instance()->IsKeyDown('a')) {
            velocity -= right * movementSpeed;
        }
        if (Input::Instance()->IsKeyDown('d')) {
            velocity += right * movementSpeed;
        }
        mainCubeBody->linearVelocity = velocity;

        if (isGameRunning) {
            world.step(deltaTime);
            explosionSystem.Update(deltaTime);
        }
        

		size_t numObjects = objects.size();

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

		// Synchronize Model transformations with RigidBody states
		for (size_t i = numObjects; i < world.bodies.size() && i < dynamicModels.size(); ++i) {
			RigidBody* body = world.bodies[i];
			if (body->inverseMass > 0) { // Dynamic objects only
				Model* model = dynamic_cast<Model*>(dynamicModels[i].get());
				model->GetTransform().SetPosition(body->position.x, body->position.y, body->position.z);
				glm::vec3 euler = quaternionToEuler(body->orientation);
				model->GetTransform().SetRotation(glm::degrees(euler.x), glm::degrees(euler.y), glm::degrees(euler.z));
			}
		}

        // Explosion trigger (left-click + 'F')
        if (Input::Instance()->IsLeftButtonClicked() && Input::Instance()->GetKeyDown() == 'f') {
            Utility::AddMessage("Shooting at: " +
                std::to_string(static_cast<int>(Input::Instance()->GetMousePosition().x)) + ", " +
                std::to_string(static_cast<int>(Input::Instance()->GetMousePosition().y)));
            glm::vec3 shootPos = camera.GetTransform().GetPosition() + camera.GetForwardVector() * 1.0f; // Spawn in front
            glm::vec3 shootDir = camera.GetForwardVector();
            float shootSpeed = 100.0f; // Medium speed

            RigidBody* bullet = new RigidBody();
            bullet->position = shootPos;
            bullet->orientation = glm::quat(1, 0, 0, 0);
            bullet->setMass(0.1f);
            float bulletSize = 0.1f;
            float I = (1.0f / 6.0f) * 1.0f * bulletSize * bulletSize;
            bullet->setInertiaTensor(glm::mat3(I, 0, 0, 0, I, 0, 0, 0, I));
            bullet->collider = new BoxCollider(glm::vec3(bulletSize / 2));
            bullet->collider->name = "fastbullet";
            bullet->linearVelocity = shootDir * shootSpeed;
            bullet->name = "Bullet";
            world.addBody(bullet);

            auto bulletCube = std::make_unique<Cube>("Crate_1.png", &grid);
            bulletCube->GetTransform().SetPosition(shootPos.x, shootPos.y, shootPos.z);
            bulletCube->GetTransform().SetScale(bulletSize, bulletSize, bulletSize);
            bulletCube->SetStatic(false);

            objects.push_back(std::move(bulletCube));
            objects.back()->SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow for bullets
        }

        // Shooting mechanic (left-click + 'G')
        if (Input::Instance()->IsLeftButtonClicked() && Input::Instance()->GetKeyDown() == 'g') {
			Utility::AddMessage("Shooting at: " +
				std::to_string(static_cast<int>(Input::Instance()->GetMousePosition().x)) + ", " +
				std::to_string(static_cast<int>(Input::Instance()->GetMousePosition().y)));
            glm::vec3 shootPos = camera.GetTransform().GetPosition() + camera.GetForwardVector() * 1.0f; // Spawn in front
            glm::vec3 shootDir = camera.GetForwardVector();
            float shootSpeed = 10.0f; // Medium speed

            RigidBody* bullet = new RigidBody();
            bullet->position = shootPos;
            bullet->orientation = glm::quat(1, 0, 0, 0);
            bullet->setMass(1.0f);
            float bulletSize = 0.2f;
            float I = (1.0f / 6.0f) * 1.0f * bulletSize * bulletSize;
            bullet->setInertiaTensor(glm::mat3(I, 0, 0, 0, I, 0, 0, 0, I));
            bullet->collider = new BoxCollider(glm::vec3(bulletSize / 2));
			bullet->collider->name = "bullet";
            bullet->linearVelocity = shootDir * shootSpeed;
            bullet->name = "Bullet";
            world.addBody(bullet);

            auto bulletCube = std::make_unique<Cube>("Crate_1.png", &grid);
            bulletCube->GetTransform().SetPosition(shootPos.x, shootPos.y, shootPos.z);
            bulletCube->GetTransform().SetScale(bulletSize, bulletSize, bulletSize);
            bulletCube->SetStatic(false);

            objects.push_back(std::move(bulletCube));
            objects.back()->SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow for bullets
        }

        // Object selection
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
                CONSOLE_WINDOW_HEIGHT + 30,
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
            }
            else {
                defaultShader.Use();
                camera.SendToShader(defaultShader);
                object->Render(defaultShader);
            }
        }

        for (auto& model : models) {
            if (isLit) {
                lightShader.Use();
                light.SendToShader(lightShader);
                camera.SendToShader(lightShader);
                model->Render(lightShader);
            }
            else {
                defaultShader.Use();
                camera.SendToShader(defaultShader);
                model->Render(defaultShader);
            }
        }

        // Check for bullet-box collisions
        for (const auto& contact : world.lastContacts) {
            std::string nameA = contact.bodyA->collider->name;
            std::string nameB = contact.bodyB->collider->name;
            if ((nameA == "bullet" && nameB == "box") || (nameA == "box" && nameB == "bullet")) {
                // Bullet hit box, apply explosion at bullet's position
                RigidBody* bulletBody = (nameA == "bullet") ? contact.bodyA : contact.bodyB;
                glm::vec3 explosionPos = bulletBody->position;
                world.applyExplosion(explosionPos, 5.0f, 10); // Radius 5, strength 100
				explosionSystem.SpawnExplosion(explosionPos, 100, 0.5f, 5.0f, 2.0f);
				Utility::AddMessage("Bullet hit a box at: " +
					std::to_string(static_cast<int>(explosionPos.x)) + ", " +
					std::to_string(static_cast<int>(explosionPos.y)) + ", " +
					std::to_string(static_cast<int>(explosionPos.z)));
                break; // Optional: Stop after first collision in frame
            }
        }

        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        skyboxShader.Use();
        camera.SendToShader(skyboxShader);
        skybox.DrawSkybox(camera.GetViewMatrix(), camera.GetProjectionMatrix());
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        particleShader.Use();
        camera.SendToShader(particleShader);
        explosionSystem.Render(particleShader, camera);

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