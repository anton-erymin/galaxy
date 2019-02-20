#pragma once

#include <cassert>
#include <memory>
#include <algorithm>
#include <unordered_map>

#include <GL/freeglut.h>

#include "Math.h"

class ImageLoader;
class Universe;
class BruteforceSolver;
class BarnesHutSolver;
class Solver;

class Orbit
{
public:
    void Transform()
    {
        glTranslatef(0.0f, 0.0f, -distance);
        glRotatef(phi, 1.0f, 0.0f, 0.0f);
        glRotatef(theta, 0.0f, 1.0f, 0.0f);
        glTranslatef(-center.m_x, -center.m_y, -center.m_z);

        float modelview[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
        right.m_x = modelview[0];
        right.m_y = modelview[4];
        right.m_z = modelview[8];
        up.m_x = modelview[1];
        up.m_y = modelview[5];
        up.m_z = modelview[9];
        forward.m_x = modelview[2];
        forward.m_y = modelview[6];
        forward.m_z = modelview[10];
    }

    void Rotate(float x, float y)
    {
        theta += x;
        phi += y;
    }

    void MoveForward(float dist)
    {
        distance = std::max(0.01f, distance + dist);
    }

    void Pan(float x, float y)
    {
        center += right * x;
        center += up * y;
    }

    float GetDistance() const { return distance; }

private:
    lpVec3 center;
    float distance = 30.0f;
    float phi = 0.0f;
    float theta = 0.0f;
    lpVec3 right;
    lpVec3 up;
    lpVec3 forward;
};

class Application
{
public:
    Application();
    ~Application();

    int Run(int argc, char** argv);

    void OnDraw();
    void OnResize(int width, int height);
    void OnIdle();
    void OnKeyboard(unsigned char key, int x, int y);
    void OnKeyboardUp(unsigned char key, int x, int y); 
    void OnMousePressed(int button, int state, int x, int y);
    void OnMouseMove(int x, int y);
    void OnMouseWheel(int button, int dir, int x, int y);

    ImageLoader& GetImageLoader() { return *imageLoader; }
    Universe& GetUniverse() { return *universe; }

    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }

    static Application& GetInstance() { assert(instance);  return *instance; }

private:
    uint32_t width;
    uint32_t height;

    std::unique_ptr<ImageLoader> imageLoader;
    std::unique_ptr<Universe> universe;
    std::unique_ptr<BruteforceSolver> solverBruteforce;
    std::unique_ptr<BarnesHutSolver> solverBarneshut;

    Solver* solver = nullptr;

    struct InputState
    {
        uint32_t buttons = 0;
        float2 prevPos;
        bool brightnessUp = false;
        bool brightnessDown = false;
    };

    InputState inputState;

    Orbit orbit;

    float cSecsInTimeUnit = 0;
    float cMillionYearsInTimeUnit = 0;

    float deltaTime = 0.0f;
    float simulationTime = 0.0f;

    float universeSize;

    bool started = false;
    bool saveToFiles;
    int mode;
    int num1, num2;

    uint64_t lastTime, newTime;
    float accTime, frameTime;

    struct RenderParameters
    {
        bool renderTree = false;

        enum class ParticleMode
        {
            Point,
            Billboard
        } particleMode = ParticleMode::Billboard;

        float brightness = 1.0f;

    } renderParams;

    std::unordered_map<char, bool*> inputMappings;

    static Application* instance;
};