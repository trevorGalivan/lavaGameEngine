#include "engine.h"
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <iostream>
#include <random>
#include <thread>

ButtonEvent makeButtonEventFromGLFWkeypress(int key, int action, int mods)
{
    return { {keyboard, key}, action, bool( mods & GLFW_MOD_SHIFT), bool(mods & GLFW_MOD_CONTROL), bool(mods & GLFW_MOD_ALT) };
}

ButtonEvent makeButtonEventFromGLFWmousepress(int key, int action, int mods)
{
    return { {mouseButton, key}, action, bool(mods & GLFW_MOD_SHIFT), bool(mods & GLFW_MOD_CONTROL), bool(mods & GLFW_MOD_ALT) };
}

// Called by glfw api when a key is pressed. Adds input to a queue which is consumed on engine update
void keyActionCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    ButtonInputSystem& buttonInputSystem = ( engine )->buttonInputSystem;

    buttonInputSystem.pushButtonAction(makeButtonEventFromGLFWkeypress(key, action, mods));

    engine->keyMap[key].store(action);
}

void mouseButtonActionCallback(GLFWwindow* window, int key, int action, int mods)
{
    ButtonInputSystem& buttonInputSystem = (static_cast<Engine*>(glfwGetWindowUserPointer(window)))->buttonInputSystem;

    buttonInputSystem.pushButtonAction(makeButtonEventFromGLFWmousepress(key, action, mods));
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    GraphicsSystem& graphicsSystem = (static_cast<Engine*>(glfwGetWindowUserPointer(window)))->graphicsSystem;
    graphicsSystem.resolution = glm::uvec2(width, height);
    graphicsSystem.camera.aspectRatio = double(width) / double(height);
    graphicsSystem.onResizeFramebuffer();
    //glViewport(0, 0, width, height);
}

void updateForever(Engine* engine)
{
    double timeStamp=0.;
    double timeAcumulator = 0; // engine->physicsTimestep;
    double dt = 0;
    while (true) {
        timeAcumulator += dt;
        timeStamp = glfwGetTime();
        timeAcumulator -= engine->physicsTimestep;
        engine->update();
        while ((dt = glfwGetTime() - timeStamp) - timeAcumulator < engine->physicsTimestep); // spin until physics timestep has passed
    }
}

WindowWrapper::WindowWrapper(unsigned int winWidth, unsigned int winHeight)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_SAMPLES, 8);
    window = glfwCreateWindow(winWidth, winHeight, "Game engine test", NULL, NULL);

    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();

    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
    }

}


// Creates OpenGL context, registers callback functions, and calls each system's constructor
Engine::Engine(unsigned int winWidth, unsigned int winHeight) : windowWrapper(winWidth, winHeight)
{
    windowWrapper.window;

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, winWidth, winHeight);
    
    glfwSetWindowUserPointer(windowWrapper.window, this);
     

    glfwSetKeyCallback(windowWrapper.window, keyActionCallback);
    glfwSetMouseButtonCallback(windowWrapper.window, mouseButtonActionCallback);
    glfwSetFramebufferSizeCallback(windowWrapper.window, framebufferSizeCallback); // graphics system
    // load resources

    // TODO: initialize systems

    graphicsSystem.camera.position = glm::vec3(0., 0., 14.);
    graphicsSystem.camera.lookAt (glm::vec3(0));

    framebufferSizeCallback(windowWrapper.window, winWidth, winHeight);

    glfwSetTime(0.); 
}

Engine::~Engine()
{
    glfwTerminate();
}


// runs on secondary thread
void Engine::update()
{
    double newTime = glfwGetTime();
    double dt = newTime - lastUpdateTime;
    lastUpdateTime = newTime;



    glm::dvec2 newMousePos;
    glfwGetCursorPos(windowWrapper.window, &(newMousePos.x), &(newMousePos.y));
    newMousePos.x *= 2. / graphicsSystem.resolution.x;
    newMousePos.x -= 1.;
    newMousePos.y *= -2. / graphicsSystem.resolution.y;
    newMousePos.y += 1.;
    glm::dvec2 deltaMouse = newMousePos - lastMousePos;
    lastMousePos = newMousePos;

    // debug
#ifdef DEBUG_CAMERA
    graphicsSystem.camera.rotate(-1 * 2 * deltaMouse.x, 2* deltaMouse.y);

    if (checkButtonState( KEY_RIGHT )) {
        graphicsSystem.camera.walk(glm::vec3(10, 0, 0) * float(dt));
    }
    if (checkButtonState(KEY_LEFT)) {
        graphicsSystem.camera.walk(glm::vec3(-10, 0, 0) * float(dt));
    }
    if (checkButtonState(KEY_UP)) {
        graphicsSystem.camera.walk(glm::vec3(0, 0, 20) * float(dt));
    }
    if (checkButtonState(KEY_DOWN)) {
        graphicsSystem.camera.walk(glm::vec3(0, 0, -10) * float(dt));
    }
    if (checkButtonState(KEY_PAGE_UP)) {
        graphicsSystem.camera.walk(glm::vec3(0, 10, 0) * float(dt));
    }
    if (checkButtonState(KEY_PAGE_DOWN)) {
        graphicsSystem.camera.walk(glm::vec3(0, -10, 0) * float(dt));
    }
#endif
    if (dt > physicsTimestep) {
        dt = physicsTimestep;
    }

    // makes sure that the main thread isnt deleting entities
    std::lock_guard guard(entityDeletionMutex);
    buttonInputSystem.update(*this, dt);
	physicsSystem.update(*this, dt);
    fighterSystem.update(*this, dt);
    healthSystem.update(*this, dt);
}

// must run on main thread
void Engine::draw()
{
    // locks physics thread while entities are deleted
    if (!destructionQueue.empty()) {
        std::lock_guard guard(entityDeletionMutex);
        while (!destructionQueue.empty()) {
            destroyEntity(destructionQueue.front());
            destructionQueue.pop();
        }
    }


    double newTime = glfwGetTime();
    double dt = newTime - lastDrawTime;
    lastDrawTime = newTime;
    
    graphicsSystem.draw(*this, dt);
}

void Engine::doLoopForever()
{
    glfwSetTime(0.);
    lastUpdateTime = 0.;
    lastDrawTime = 0.;
    std::thread updateThread(updateForever, this); // launches physics / game logic thread
    while (true) {
        //update();
        draw();
        glfwSwapBuffers(windowWrapper.window);
        updateJoySticks();
        glfwPollEvents();
    }

}

void Engine::updateJoySticks()
{
    GLFWgamepadstate state;
    if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1) && glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
        for (int i = 0; i < LAST_GAMEPAD; ++i) {
            int buttonState = state.buttons[i];
            if (gamepad1Map[i] != buttonState) {
                gamepad1Map[i] = buttonState;
                buttonInputSystem.pushButtonAction(ButtonEvent{ Button(gamepad1, i), buttonState, false, false, false });
            }
        }
    }
}

double Engine::getTime()
{
    return glfwGetTime();
}

bool Engine::checkButtonState(Button button) {
    if (button.inputType == keyboard) {
        if (button.key > LAST_KEY) return false;
        return keyMap[button.key].load();
    }
    else if (button.inputType == gamepad1) {
        if (button.key > LAST_GAMEPAD) return false;
        return gamepad1Map[button.key].load();
    }
    return false;
}

Entity& Engine::makeEntity(unsigned int catagories) {
    Entity newentity;
    newentity.catagory = catagories;
    handle<Entity> ID = entities.add(std::move(newentity));
    return *entities.find(ID);
}

void Engine::destroyEntity(handle<Entity> ID) {
    Entity* entityptr = getEntity(ID);

    if (entityptr) {
        auto& entity = *entityptr;
        if (entity.constraintHandle != INVALID_HANDLE) {
            destroyComponent(entity.constraintHandle);
            entity.constraintHandle = handle<ConstraintComponent>(INVALID_HANDLE);
        }
        if (entity.physicsHandle != INVALID_HANDLE) {
            destroyComponent(entity.physicsHandle);
            entity.physicsHandle = handle<PhysicsComponent>(INVALID_HANDLE);
        }
        if (entity.spriteHandle != INVALID_HANDLE) {
            destroyComponent(entity.spriteHandle);
            entity.spriteHandle = handle<SpriteComponent>(INVALID_HANDLE);
        }
        if (entity.lightHandle != INVALID_HANDLE) {
            destroyComponent(entity.lightHandle);
            entity.lightHandle = handle<LightComponent>(INVALID_HANDLE);
        }
        if (entity.modelHandle != INVALID_HANDLE) {
            destroyComponent(entity.modelHandle);
            entity.modelHandle = handle<ModelComponent>(INVALID_HANDLE);
        }
        if (entity.healthHandle != INVALID_HANDLE) {
            destroyComponent(entity.healthHandle);
            entity.healthHandle = handle<HealthComponent>(INVALID_HANDLE);
        }
        if (entity.damageHandle != INVALID_HANDLE) {
            destroyComponent(entity.damageHandle);
            entity.damageHandle = handle<DamageComponent>(INVALID_HANDLE);
        }
        if (entity.fighterHandle != INVALID_HANDLE) {
            destroyComponent(entity.fighterHandle);
            entity.fighterHandle = handle<FighterComponent>(INVALID_HANDLE);
        }

        entities.remove(ID);
    }
    
}
