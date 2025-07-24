﻿#pragma once

#include <Haketon.h>

struct ParticleProps
{
    FVec2 Position;
    FVec2 Velocity, VelocityVariation;
    glm::vec4 ColorBegin, ColorEnd;
    float SizeBegin, SizeEnd, SizeVariation;
    float LifeTime = 1.0f;
};

class ParticleSystem
{
public:
    ParticleSystem(uint32_t maxParticles = 1000);

    void OnUpdate(Haketon::Timestep ts);
    void OnRender(Haketon::OrthographicCamera& camera);

    void Emit(const ParticleProps& particleProps);
private:
    struct Particle
    {
        FVec2 Position;
        FVec2 Velocity;
        glm::vec4 ColorBegin, ColorEnd;
        float Rotation = 0.0f;
        float SizeBegin, SizeEnd;

        float LifeTime = 1.0f;
        float LifeRemaining = 0.0f;

        bool Active = false;
    };
    std::vector<Particle> m_ParticlePool;
    uint32_t m_PoolIndex = 999;
};