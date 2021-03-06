#include <Rish/Effect/Particle/ParticleSystem.h>

#include <Rish/Scene/Scene.h>
#include <Rish/Scene/Component.h>
#include <Rish/Renderer/RendererCommand.h>
#include <Rish/Renderer/Renderer2D.h>
#include <Rish/Math/Random.h>

namespace rl {

Ref<Scene> ParticleSystem::s_Scene;

void ParticleSystem::RegisterScene(const Ref <Scene> &scene)
{
    s_Scene = scene;
}

void ParticleSystem::onUpdate(float dt)
{
    auto &registry = s_Scene->m_registry;
    auto state = s_Scene->getSceneState();

    if(state != Scene::SceneState::Play)
        return;
    //
    auto view = registry.view<TransformComponent, ParticleComponent>();
    for(auto entity : view)
    {
        auto &emitter   = registry.get<ParticleComponent>(entity);
        auto &transform = registry.get<TransformComponent>(entity);

        if (emitter.active)
        {
            // cal the num that should respawn this frame
            emitter.emissionRate = (int)(emitter.emitNumber + emitter.emitVariance * randomFloat(emitter.emitVarianceRand.x, emitter.emitVarianceRand.y));

            for(int i = 0 ; i < emitter.emissionRate ; i++)
            {
                // find the unused Particle
                int unusedParticle = firstUnusedParticle(registry, entity);
                // 激活他
                respawnParticle(emitter, transform, unusedParticle);
            }

            if(emitter.vortexSensitive && (emitter.vortexType == DYNAMIC_VORTEX))
            {
                emitter.vortexEmissionRate = emitter.vortexEmitNumber;

                if(emitter.vortexActive)
                {
                    for(int i = 0 ; i < emitter.vortexEmissionRate ; i++)
                    {
                        int unusedVortex = firstUnusedVortex(registry, entity);
                        if(unusedVortex == -1)
                            break;
                        respawnVortex(emitter, transform, unusedVortex);
                    }
                }
            }
        }

        if(emitter.vortexSleepTime > 0.f)
        {
            if(emitter.vortexSleepTimer.getElapsedTime() <= emitter.vortexSleepTime)
            {
                emitter.vortexActive = false;
            }
            else
            {
                emitter.vortexActive = true;
                emitter.vortexSleepTimer.restart();
            }
        }

        // Update particles' states by sleep time and life time
        if(emitter.sleepTime > 0.f)
        {
            if(emitter.sleepTimer.getElapsedTime() <= emitter.sleepTime)
            {
                emitter.active = false;
            }
            else
            {
                emitter.active = true;
                emitter.sleepTimer.restart();
            }
        }

        if(emitter.life > 0.f)
        {
            // exceeded
            if(emitter.lifeTimer.getElapsedTime() >= emitter.life)
            {
                emitter.active = false;
            }
        }

        // Update Dynamic vortex
        if(emitter.vortexType == DYNAMIC_VORTEX && emitter.vortexSensitive)
        {
            for(auto &vortex: emitter.dynamic_vortexes)
            {
                if(vortex.life > 0.f)
                {
                    vortex.currentSize  = interpolateBetweenRange(vortex.startSize, vortex.timeStep, vortex.endSize);
                    vortex.currentVel.x = interpolateBetweenRange(vortex.startVel.x, vortex.timeStep, vortex.endVel.x);
                    vortex.currentVel.y = interpolateBetweenRange(vortex.startVel.y, vortex.timeStep, vortex.endVel.y);

                    vortex.pos.x += vortex.currentVel.x * dt;
                    vortex.pos.y += vortex.currentVel.y * dt;

                    vortex.timeStep += (1.0f/(float)vortex.startLife);
                    if(vortex.timeStep >= 1.f)
                        vortex.timeStep = 0.f;
                    vortex.life--;
                }
            }
        }

        // Update particles
        for(int i = 0 ; i < emitter.poolSize ; i++)
        {
            auto &particle = emitter.particles[i];
            if(particle.life > 0.f) {
                // Update
                // Interpolate values
                // 利用差質法算出目前的資訊
                particle.currentSize  = interpolateBetweenRange(particle.startSize, particle.timeStep, particle.endSize);
                particle.currentVel.x = interpolateBetweenRange(particle.startVel.x, particle.timeStep, particle.endVel.x);
                particle.currentVel.y = interpolateBetweenRange(particle.startVel.y, particle.timeStep, particle.endVel.y);
                particle.currentColor = RGBAInterpolation(particle.startColor, particle.timeStep, particle.endColor);

                // 計算受vortex影響的結果

                // multi vortex
                float factor_ = 0;
                float vortexEffectX = 0;
                float vortexEffectY = 0;

                if(emitter.vortexSensitive)
                {
                    if(emitter.vortexType == STATIC_VORTEX)
                    {
                        for(auto vortex : emitter.static_vortexes)
                        {
                            float vortexScaleSize = vortex.currentSize * 0.001;

                            if(vortex.currentSize == 0.f || vortex.turbulence == glm::vec2(0, 0))
                                continue;

                            float dx = particle.pos.x - vortex.pos.x;
                            float dy = particle.pos.y - vortex.pos.y;
                            float vx = dy * vortex.turbulence.x;
                            float vy = dx * vortex.turbulence.y;
                            factor_ = 1.0f/ (1.0f + (dx*dx + dy*dy)/(vortexScaleSize));

                            vortexEffectX += (vx - particle.currentVel.x) * factor_;
                            vortexEffectY += (vy - particle.currentVel.y) * factor_;

                        }
                    }

                    if(emitter.vortexType == DYNAMIC_VORTEX)
                    {
                        for(auto vortex : emitter.dynamic_vortexes)
                        {
                            float vortexScaleSize = vortex.currentSize * 0.1;

                            if(vortex.currentSize == 0.f || vortex.turbulence == glm::vec2(0, 0))
                                continue;

                            if(vortex.life > 0.f)
                            {
                                float dx = particle.pos.x - vortex.pos.x;
                                float dy = particle.pos.y - vortex.pos.y;
                                float vx = -dy * vortex.turbulence.x;
                                float vy = dx * vortex.turbulence.y;
                                factor_ = 1.0f/ (1.0f + (dx*dx + dy*dy)/(vortexScaleSize));
                                float lifeFactor = vortex.life/emitter.vortexMaxParticleLife;
                                factor_ *= (1-lifeFactor)*lifeFactor*4;

                                vortexEffectX += (vx - particle.currentVel.x) * factor_;
                                vortexEffectY += (vy - particle.currentVel.y) * factor_;
                            }
                        }
                    }
                }

                particle.pos.x += vortexEffectX + particle.currentVel.x * dt;
                particle.pos.y += vortexEffectY + particle.currentVel.y * dt;
                particle.life--;
                // 計算差值的time
                particle.timeStep+= (1.0f/(float)particle.startLife);
                if(particle.timeStep >= 1.f)
                    particle.timeStep = 0.f;
                //
                // 計算rotate速度跟角度
                particle.currentRotSpeed += particle.startRotSpeed * 0.1;
                particle.angle += dt * particle.currentRotSpeed*10;
                particle.angle = fmod(particle.angle, 360.f);
            }
        }
    }
}

void ParticleSystem::onRender()
{
    auto &registry = s_Scene->m_registry;
    //
    auto view = registry.view<TransformComponent, ParticleComponent>();
    for (auto &entity: view)
    {
        auto &transform = registry.get<TransformComponent>(entity);
        auto &emitter   = registry.get<ParticleComponent>(entity);
        auto &tag       = registry.get<TagComponent>(entity);

        // Draw all living particles
        for(auto &particle: emitter.particles)
        {
            if(particle.life > 0)
                Renderer2D::DrawRotatedQuad(particle.pos,
                    {particle.currentSize, particle.currentSize}, emitter.texture,
                    particle.currentColor, particle.angle);
        }
    }
}

void ParticleSystem::onEditorRender(bool isSimulate)
{
    auto &registry = s_Scene->m_registry;
    auto state = s_Scene->getSceneState();
    //
    auto view = registry.view<TransformComponent, ParticleComponent>();
    for (auto &entity: view)
    {
        auto &transform = registry.get<TransformComponent>(entity);
        auto &emitter   = registry.get<ParticleComponent>(entity);
        auto &tag       = registry.get<TagComponent>(entity);

        if(isSimulate)
        {
            // Draw all living particles
            for(auto &particle: emitter.particles)
            {
                if(particle.life > 0)
                    Renderer2D::DrawRotatedQuad(particle.pos,
                        {particle.currentSize, particle.currentSize}, emitter.texture,
                        particle.currentColor, particle.angle);
            }
        }
        else
        {
            auto size = (emitter.startSize + emitter.endSize)/2.f * 0.01f;
            Renderer2D::DrawQuad(transform.translate + glm::vec3(emitter.offset, 0),
                 {size, size},
                 emitter.texture);
        }

        if(emitter.vortexSensitive)
        {
            if(emitter.vortexType == STATIC_VORTEX)
            {
                for(auto vortex : emitter.static_vortexes)
                {
                    if(vortex.draw)
                    {
                        Renderer2D::DrawFgLine(vortex.pos, vortex.pos+glm::vec2{(vortex.turbulence.x < 0 ? -1 : 1)*vortex.currentSize, 0.f}, glm::vec4{0.f, 0.f, 1.f, 1.f});
                        Renderer2D::DrawFgLine(vortex.pos, vortex.pos+glm::vec2{0.f, (vortex.turbulence.y < 0 ? -1 : 1)*vortex.currentSize}, glm::vec4{0.f, 1.f, 0.f, 1.f});
                    }
                }
            }

            if(emitter.vortexType == DYNAMIC_VORTEX && emitter.drawVortex)
            {
                for(auto vortex : emitter.dynamic_vortexes)
                {
                    if(vortex.life > 0.f)
                    {
                        Renderer2D::DrawFgLine(vortex.pos, vortex.pos+glm::vec2{(vortex.turbulence.x < 0 ? -1 : 1)*vortex.currentSize*100, 0.f}, glm::vec4{0.f, 0.f, 1.f, 1.f});
                        Renderer2D::DrawFgLine(vortex.pos, vortex.pos+glm::vec2{0.f, (vortex.turbulence.y < 0 ? -1 : 1)*vortex.currentSize*100}, glm::vec4{0.f, 1.f, 0.f, 1.f});
                    }
                }
            }
        }
    }
}

float ParticleSystem::randomFloat(float min, float max)
{
    return Math::RandomFloat(min, max);
}

uint32_t ParticleSystem::firstUnusedParticle(entt::registry &registry, entt::entity& entity)
{
    auto &emitter = registry.get<ParticleComponent>(entity);

    for(int i = emitter.lastUnusedParticle ; i < emitter.poolSize ; i++)
    {
        if(emitter.particles[i].life <= 0)
        {
            emitter.lastUnusedParticle = i;
            return i;
        }
    }

    for(int i = 0 ; i < emitter.lastUnusedVortex ; i++)
    {
        if(emitter.particles[i].life <= 0)
        {
            emitter.lastUnusedParticle = i;
            return i;
        }
    }

    emitter.lastUnusedParticle = 0;
    return emitter.lastUnusedParticle;
}

void ParticleSystem::respawnParticle(ParticleComponent &emitter, TransformComponent &transform, int unusedParticle)
{
    auto &particle = emitter.particles[unusedParticle];

    float startSpeed         = emitter.startSpeed * randomFloat(emitter.startSpeedRand.x, emitter.startSpeedRand.y);
    float endSpeed           = emitter.endSpeed   * randomFloat(emitter.endSpeedRand.x, emitter.endSpeedRand.y);
    float randAngle          = randomFloat(emitter.angleRange.x, emitter.angleRange.y);
    float randStartSize      = emitter.startSize  * randomFloat(emitter.startSizeRand.x, emitter.startSizeRand.y);
    float randEndSize        = emitter.endSize    * randomFloat(emitter.endSizeRand.x, emitter.endSizeRand.y);
    float randRadius         = randomFloat(randStartSize, randEndSize/2);
    float randLife           = emitter.maxParticleLife * randomFloat(emitter.lifeRand.x, emitter.lifeRand.y);
    float randRotateSpeed    = emitter.rotateSpeed * randomFloat(emitter.rotSpeedRand.x, emitter.rotSpeedRand.y) * 0.1;
    float randRotAngle       = randomFloat(0.f, emitter.rotateSpeed == 0? 0.f : 360.f);
    glm::vec3 distance       = {randomFloat(emitter.disX, -emitter.disX), randomFloat(emitter.disY, -emitter.disY), 0.f};

    particle.pos             = transform.translate + distance + glm::vec3(emitter.offset, 0);
    particle.startVel        = {startSpeed * cos(glm::radians(randAngle)) * 0.1, startSpeed * sin(glm::radians(randAngle)) * 0.1};
    particle.endVel          = {endSpeed   * cos(glm::radians(randAngle)) * 0.1, endSpeed   * sin(glm::radians(randAngle)) * 0.1};
    particle.angle           = randRotAngle;
    particle.startRotSpeed   = particle.currentRotSpeed = randRotateSpeed;
    particle.life            = particle.startLife       = randLife;
    particle.currentSize     = particle.startSize       = randRadius * 0.01;
    particle.endSize         = randEndSize * 0.01;
    particle.timeStep        = 0.f;

    particle.startColor      = emitter.startColor;
    particle.endColor        = emitter.endColor;

    particle.vortexSensitive = emitter.vortexSensitive;
}

uint32_t ParticleSystem::firstUnusedVortex(entt::registry &registry, entt::entity& entity)
{
    auto &emitter = registry.get<ParticleComponent>(entity);
    for(int i = emitter.lastUnusedVortex ; i < emitter.vortexPoolSize ; i++)
    {
        if(emitter.dynamic_vortexes[i].life <= 0)

        {
            emitter.lastUnusedVortex = i;
            return i;
        }
    }

    for(int i = 0 ; i < emitter.lastUnusedVortex ; i++)
    {
        if(emitter.dynamic_vortexes[i].life <= 0)
        {
            emitter.lastUnusedVortex = i;
            return i;
        }
    }

    emitter.lastUnusedVortex = 0;
    return -1;
}

void ParticleSystem::respawnVortex(ParticleComponent &emitter, TransformComponent &transform, int unusedVortex)
{
    auto &vortex = emitter.dynamic_vortexes[unusedVortex];

    float randomAngle    = randomFloat(emitter.vortexAngleRange.x, emitter.vortexAngleRange.y);
    float startSpeed     = emitter.vortexStartSpeed * randomFloat(emitter.vortexStartSpeedRand.x, emitter.vortexStartSpeedRand.y);
    float endSpeed       = emitter.vortexEndSpeed * randomFloat(emitter.vortexEndSpeedRand.x, emitter.vortexEndSpeedRand.y);
    float startSize      = emitter.vortexStartSize * randomFloat(emitter.vortexStartSizeRand.x, emitter.vortexStartSizeRand.y);
    float endSize        = emitter.vortexEndSize * randomFloat(emitter.vortexEndSizeRand.x, emitter.vortexEndSizeRand.y);
    float randLife       = emitter.vortexMaxParticleLife * randomFloat(emitter.vortexLifeRand.x, emitter.vortexLifeRand.y);
    glm::vec2 randTurbulence = emitter.vortexTurbulence * randomFloat(emitter.vortexTurbulenceRand.x, emitter.vortexTurbulenceRand.y);

    glm::vec3 distance = {randomFloat(emitter.vortexDisX, -emitter.vortexDisX), randomFloat(emitter.vortexDisY, -emitter.vortexDisY), 0.f};

    vortex.pos        = transform.translate + distance + glm::vec3(emitter.vortexPos, 0.f);
    vortex.startVel   = {startSpeed * cos(glm::radians(randomAngle))*10, startSpeed * sin(glm::radians(randomAngle))*10};
    vortex.endVel     = {endSpeed   * cos(glm::radians(randomAngle))*10, endSpeed   * sin(glm::radians(randomAngle))*10};
    vortex.turbulence = randTurbulence;

    vortex.life = vortex.startLife = randLife;
    vortex.startSize = vortex.currentSize = startSize * 0.01;
    vortex.endSize = endSize * 0.01;
    vortex.timeStep = 0.f;
}

float ParticleSystem::interpolateBetweenRange(float min, float timestep, float max)
{
    return min + (max - min) * timestep;
}

glm::vec4 ParticleSystem::RGBAInterpolation(glm::vec4 startColor, float timestep, glm::vec4 endColor)
{
    glm::vec4 resultColor;

    resultColor = startColor + (endColor - startColor) * timestep;

    return resultColor;
}

}
