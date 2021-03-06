#pragma once

#include <Rish/rlpch.h>
#include <Rish/Core/VFS.h>
#include <Rish/Core/Time.h>
#include <Rish/Core/FileSystem.h>
//
#include <Rish/Renderer/Buffer.h>
#include <Rish/Renderer/VertexArray.h>
#include <Rish/Renderer/Shader.h>
#include <Rish/Renderer/Texture2D.h>
#include <Rish/Renderer/SubTexture2D.h>
//
#include <Rish/Scene/SceneCamera.h>
//
#include <Rish/Utils/uuid.h>

#include <Rish/Utils/Serialize.h>

namespace rl {

class Entity;

/**
 * @defgroup components Components
 * @brief Components for entities
 * @ingroup scene
 * @{
 */

/**
 * @brief Tag
 */
struct TagComponent
{
	std::string tag{};
	UUID id{};

	TagComponent() = default;
	TagComponent(const std::string& t) : tag(t) {}

private:
	friend class cereal::access;
	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(cereal::make_nvp("Tag", tag), cereal::make_nvp("Id", id));
	}

};

/**
 * @brief Transform
 */
struct TransformComponent
{
    glm::vec3 translate {0.0f, 0.0f, 0.0f};  ///< Translation
	glm::vec3 scale {1.f, 1.f, 1.f};         ///< Scale in three axis
	float rotate = 0.f;                                 ///< Rotation in degrees

	TransformComponent() = default;
    TransformComponent(const glm::vec3 &pos) : translate(pos) {}
private:
	friend class cereal::access;
	template <class Archive>
	void serialize(Archive &ar)
	{
	    ar(
            CEREAL_NVP(translate),
            CEREAL_NVP(scale),
            CEREAL_NVP(rotate)
        );
	}
};

/**
 * @brief Render
 */
struct SpriteRenderComponent
{
	glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
	std::string texturePath = "assets/texture/1.png";
	bool useTexture = true;
	//
	bool useAsSubTexture = false;

	// Need initialize
    Ref<Texture2D> m_texture;
    // SubTexture
    Ref<SubTexture2D> m_subtexture;
    SubTexture2DSetting m_subSetting;

    float tiling = 1.f;

    // TODO: Check the meaning and rename it
	// states
	bool init = true;

    void loadFromSetting()
    {
        if(useTexture)
        {
            m_texture = Texture2D::LoadTextureVFS(texturePath);
        }
        //
        if(useAsSubTexture)
        {
            if (m_subSetting.type == SubTexture2DSetting::SubTextureSheet)
            {
                m_subtexture = SubTexture2D::CreateFromSheet(m_texture, m_subSetting.pos, m_subSetting.cellSize,
                                                             m_subSetting.spriteGridSize);
            }
            else if (m_subSetting.type == SubTexture2DSetting::SubTextureCoordinate)
            {
                m_subtexture = SubTexture2D::CreateFromCoordinate(m_texture, m_subSetting.leftUpper,
                                                                  m_subSetting.size);
            }
        }
    }

	void loadTexture(const std::string &path)
    {
	    texturePath = path;
        useTexture = true;
        //
        loadFromSetting();
    }

	void loadSubTexture(SubTexture2DSetting setting)
    {
        useAsSubTexture = true;
        m_subSetting = setting;
        //
        loadFromSetting();
    }

	SpriteRenderComponent() = default;
    explicit SpriteRenderComponent(const std::string &path) : texturePath(path)
    {
    }
private:
    friend class ComponentEditPanel;

    void initSubTexture()
    {
        if (m_subSetting.type == SubTexture2DSetting::SubTextureSheet)
        {
            m_subtexture = SubTexture2D::CreateFromSheet(m_texture, m_subSetting.pos, m_subSetting.cellSize,
                                                         m_subSetting.spriteGridSize);
        }
        else if (m_subSetting.type == SubTexture2DSetting::SubTextureCoordinate)
        {
            m_subtexture = SubTexture2D::CreateFromCoordinate(m_texture, m_subSetting.leftUpper,
                                                              m_subSetting.size);
        }
    }

	friend class cereal::access;
//	template<class Archive>
//	void serialize(Archive &ar)
//	{
//		ar(
//           cereal::make_nvp("Color", color),
//		   cereal::make_nvp("Texture", m_texture),
//		   cereal::make_nvp("UseTexture", useTexture),
//		   cereal::make_nvp("UseAsSubTexture", useAsSubTexture),
//		   cereal::make_nvp("setting", m_subSetting),
//		   cereal::make_nvp("tiling", tiling)
//		);
//	}

	template<class Archive>
	void load(Archive &ar)
    {
        ar(
            cereal::make_nvp("Color", color),
            cereal::make_nvp("Texture", m_texture),
            cereal::make_nvp("UseTexture", useTexture),
            cereal::make_nvp("UseAsSubTexture", useAsSubTexture),
            cereal::make_nvp("setting", m_subSetting),
            cereal::make_nvp("tiling", tiling)
        );

        init = false;
        texturePath = m_texture->getPath();
    }

    template<class Archive>
    void save(Archive &ar) const
    {
        ar(
            cereal::make_nvp("Color", color),
            cereal::make_nvp("Texture", m_texture),
            cereal::make_nvp("UseTexture", useTexture),
            cereal::make_nvp("UseAsSubTexture", useAsSubTexture),
            cereal::make_nvp("setting", m_subSetting),
            cereal::make_nvp("tiling", tiling)
        );
    }
};

/**
 * @brief Camera Component
 */
struct CameraComponent
{
    SceneCamera camera;
    bool primary = false;
    bool lockAspect = true;

    CameraComponent()
    {
        camera.setOrthographic(5.f, -1000.f, 1000.f);
    }

    CameraComponent(bool isPrimary, float aspect, float size, float near=-1.f, float far=1.f)
    {
        camera.setOrthographic(size, near, far);
        camera.setAspect(aspect);
        primary = isPrimary;
    }

    CameraComponent(const CameraComponent&) = default;
private:
    friend class cereal::access;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(cereal::make_nvp("camera", camera),
           cereal::make_nvp("lockAspect", lockAspect),
           cereal::make_nvp("primary", primary)
        );
    }
};

/**
 * @brief Group Component
 * @details This is a special component for grouping entities
 */
struct GroupComponent
{
    void addEntityUUID(const UUID &id);
    void delEntityUUID(const UUID &id);
    void clear();

    using difference_type   = ptrdiff_t;
    using size_type         = size_t;
    using value_type        = UUID;
    using pointer           = UUID*;
    using reference         = UUID&;
    using iterator          = std::set<UUID>::iterator;
    using const_iterator    = std::set<UUID>::const_iterator;

    iterator begin() { return childEntity.begin(); }
    iterator end()   { return childEntity.end();   }
    const_iterator begin() const { return childEntity.begin(); }
    const_iterator end() const   { return childEntity.end();   }
    bool isEmpty() { return childEntity.empty(); }
private:
    std::set<UUID> childEntity;
    //
    friend class cereal::access;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(cereal::make_nvp("children", childEntity));
    }
};

/**
 * @brief SubGroup Component
 */
struct SubGroupComponent
{

    void setGroup(const UUID &id)           { groupEntityID = id; }
    void setGroupPosition(glm::vec3 pos)    { m_groupPosition = pos; }
    void setRelativePosition(glm::vec3 pos) { m_relativePosition = pos; }
    void setGroupScale(glm::vec3 scale)     { m_groupScale = scale; }
    void setOriginScale(glm::vec3 scale)    { m_originScale = scale; }
    void setGroupRotate(float angle)        { m_groupRotate = angle; }
    void setOriginRotate(float angle)       { m_originRotate = angle; }
    void setPreRotate(float angle)          { m_preRotate = angle; }
    void setOffset(glm::vec3 offset)        { m_offset = offset; }

    const UUID& getGroupEntityID() const        { return groupEntityID;}
    const glm::vec3 getOffset() const           { return m_offset; }
    const glm::vec3 getRelativePosition() const { return m_relativePosition; }
    const glm::vec3 getGroupPosition() const    { return m_groupPosition; }
    const glm::vec3 getGroupScale() const       { return m_groupScale; }
    const glm::vec3 getOriginScale() const      { return m_originScale; }
    const float getGroupRotate() const          { return m_groupRotate; }
    const float getOriginRotate() const         { return m_originRotate; }
    const float getPreRotate() const            { return m_preRotate; }

    glm::vec3 calculateCurrentPosition();
    glm::vec3 calculateCurrentScale();
    float calculateCurrentRotate();

private:
    UUID groupEntityID{};

    glm::vec3 m_relativePosition;
    glm::vec3 m_originScale;
    float m_originRotate;

    glm::vec3 m_groupPosition;
    glm::vec3 m_groupScale;
    float m_groupRotate;
    float m_preRotate;

    glm::vec3 m_offset;

    //
    friend class cereal::access;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(cereal::make_nvp("groupID", groupEntityID));
        ar(cereal::make_nvp("relativePosition", m_relativePosition));
        ar(cereal::make_nvp("originScale", m_originScale));
        ar(cereal::make_nvp("originRotate", m_originRotate));
        ar(cereal::make_nvp("groupPosition", m_groupPosition));
        ar(cereal::make_nvp("groupScale", m_groupScale));
        ar(cereal::make_nvp("groupRotate", m_groupRotate));
        ar(cereal::make_nvp("preRotate", m_preRotate));
        ar(cereal::make_nvp("offset", m_offset));
    }
};


/**
 * @}
 */

// TODO move to other file?
struct EmitData
{
    glm::vec2 offset = {0.f, 0.f};
    float disX = 0.f;
    float disY = 0.f;


    glm::vec2 angleRange = {0.f, 360.f};

    float startSpeed = 0.f;
    float endSpeed = 0.8f;
    float startSize = 0.0f;
    float endSize = 0.5f;
    float rotateSpeed = 0.f;

    uint32_t emitNumber = 3;
    uint32_t emitVariance = 2;
    uint32_t maxParticleLife = 80;

    float life = -1;
    float sleepTime = -1;

    glm::vec4 startColor = {1.0, 0.3921568691730499f, 0.0, 1};
    glm::vec4 endColor = {0.8235294222831726f, 0.8235294222831726f, 0.8235294222831726f, 0};

    glm::vec2 rotSpeedRand = {-1.f, 1.f};
    glm::vec2 startSpeedRand = { 0.f, 1.f};
    glm::vec2 endSpeedRand = {0.f, 1.f};
    glm::vec2 emitVarianceRand = {0.f, 1.f};
    glm::vec2 startSizeRand = {0.f, 1.f};
    glm::vec2 endSizeRand = {1.f, 1.f};
    glm::vec2 lifeRand = {0.5f, 1.f};

    std::string texturePath = "assets/texture/1.png";

    bool vortexSensitive = false;

    int vortexType = 0;
    glm::vec2 vortexPos = {0.f, -50.f};
    glm::vec2 vortexAngleRange = {250.f, 280.f};

    glm::vec2 vortexSpeed = {0.f, 0.f};
    float vortexStartSpeed = 200.f;
    float vortexEndSpeed = 200.f;

    float vortexStartSize = 0.1f;
    float vortexEndSize = 0.1f;

    uint32_t vortexEmitNumber = 50;
    uint32_t vortexMaxParticleLife = 80;

    glm::vec2 vortexTurbulence = {100.f, 50.f};

    float vortexSleepTime = 1;

    glm::vec2 vortexStartSpeedRand = { 0.f, 1.f};
    glm::vec2 vortexEndSpeedRand = {0.f, 1.f};
    glm::vec2 vortexLifeRand = {0.5f, 1.f};
    glm::vec2 vortexStartSizeRand = {0.f, 1.f};
    glm::vec2 vortexEndSizeRand = {1.f, 1.f};
    glm::vec2 vortexTurbulenceRand = {0.f, 1.f};

    std::string dataPath = "";

    template<typename Archrive>
    void serialize(Archrive &ar)
    {
        ar(cereal::make_nvp("dataPath", dataPath));
        ar(cereal::make_nvp("offset", offset));
        ar(cereal::make_nvp("angleRange", angleRange));
        ar(cereal::make_nvp("startSpeed", startSpeed));
        ar(cereal::make_nvp("endSpeed", endSpeed));
        ar(cereal::make_nvp("startSize", startSize));
        ar(cereal::make_nvp("endSize", endSize));
        ar(cereal::make_nvp("rotateSpeed", rotateSpeed));
        ar(cereal::make_nvp("emitNumber", emitNumber));
        ar(cereal::make_nvp("emitVariance", emitVariance));
        ar(cereal::make_nvp("maxParticleLife", maxParticleLife));
        ar(cereal::make_nvp("life", life));
        ar(cereal::make_nvp("sleepTime", sleepTime));
        ar(cereal::make_nvp("startColor", startColor));
        ar(cereal::make_nvp("endColor", endColor));
        ar(cereal::make_nvp("rotSpeedRand", rotSpeedRand));
        ar(cereal::make_nvp("startSpeedRand", startSpeedRand));
        ar(cereal::make_nvp("endSpeedRand", endSpeedRand));
        ar(cereal::make_nvp("emitVarianceRand", emitVarianceRand));
        ar(cereal::make_nvp("startSizeRand", startSizeRand));
        ar(cereal::make_nvp("endSizeRand", endSizeRand));
        ar(cereal::make_nvp("disX", disX));
        ar(cereal::make_nvp("disY", disY));
        ar(cereal::make_nvp("texturePath", texturePath));
        ar(cereal::make_nvp("vortexSensitive", vortexSensitive));
        ar(cereal::make_nvp("vortexType", vortexType));
        ar(cereal::make_nvp("vortexOffset", vortexPos));
        ar(cereal::make_nvp("vortexAngleRange", vortexAngleRange));
        ar(cereal::make_nvp("vortexStartSpeed", vortexStartSpeed));
        ar(cereal::make_nvp("vortexEndSpeed", vortexEndSpeed));
        ar(cereal::make_nvp("vortexStartSize", vortexStartSize));
        ar(cereal::make_nvp("vortexEndSize", vortexEndSize));
        ar(cereal::make_nvp("vortexEmitNumber", vortexEmitNumber));
        ar(cereal::make_nvp("vortexMaxParticleLife", vortexMaxParticleLife));
        ar(cereal::make_nvp("vortexTurbulence", vortexTurbulence));
        ar(cereal::make_nvp("vortexSleepTime", vortexSleepTime));
        ar(cereal::make_nvp("vortexStartSpeedRand", vortexStartSpeedRand));
        ar(cereal::make_nvp("vortexEndSpeedRand", vortexEndSpeedRand));
        ar(cereal::make_nvp("vortexLifeRand", vortexLifeRand));
        ar(cereal::make_nvp("vortexStartSizeRand", vortexStartSizeRand));
        ar(cereal::make_nvp("vortexEndSizeRand", vortexEndSizeRand));
        ar(cereal::make_nvp("vortexTurbulence", vortexTurbulence));
    }
};

// TODO move to other file?
struct Particle
{
    uint32_t startLife;

    glm::vec2 pos;

    glm::vec2 startVel;

    glm::vec2 endVel;

    glm::vec2 currentVel;

    float startSize, currentSize, endSize;

    float angle;

    float startRotSpeed;

    float currentRotSpeed;

    glm::vec4 startColor, currentColor, endColor;

    bool vortexSensitive;

    float timeStep;

    float life;

    glm::vec2 turbulence;

    bool draw = false;

    // ID for static_vortex
    int staticID;

    Particle() = default;

    Particle(int ID) : staticID(ID){}

    bool operator==(const Particle &rhs) const
    {
        return staticID == rhs.staticID;
    }
};

struct ParticleComponent
{
    /**************** Particle Attribute *****************/

    #define MAX_PARTICLE_POOLSIZE 100000

    // Emitter active
    bool active = true;

    // pos offset
    glm::vec2 offset = {0.f, 0.f};
    // Determine particle pos range
    // disX = 100 -> emitter pos center +- 100 will generate particle
    float disX = 0.f;

    float disY = 0.f;

    /// Particle restrain in the range of angle
    glm::vec2 angleRange {80.f, 110.f};

    /// The color when a particle generate
    glm::vec4 startColor = {1.f, 0.3921568691730499f, 0.f, 1.f};

    /// The color when a particle die
    glm::vec4 endColor = {0.8235294222831726f, 0.8235294222831726f, 0.8235294222831726f, 0.f};

    /// The speed when a particle generate
    float startSpeed = 0.f;

    /// The speed when a particle die
    float endSpeed = 200.8f;

    /// The size when a particle generate
    float startSize = 0.f;

    /// The size when a particle die
    float endSize = 80.0f;

    /// The speed that the particle texture rotate
    float rotateSpeed = 0.f;

    /// Particle generate per frame
    /// Cal by emitNumber and emitVariance
    int emissionRate = 0;

    /// Least generate per frame
    uint32_t emitNumber = 3;

    /// Offset of random
    uint32_t emitVariance = 2;

    /// Particle life
    uint32_t maxParticleLife = 80;

    /// Use to cal pool size
    uint32_t maxParticlesPerFrame = 0;

    /// Particle Pool Size
    int poolSize = 0;

    /// Emitter life time
    float life = -1;

    /// For ImGui stuff. If ImGui change "life", check if life != life_store, then restart timer and active emitter
    float life_store = -1;

    // /Particle generate per sleep time
    float sleepTime = -1;

    // Timer that control emitter life and sleep time
    // Clock???
    Clock lifeTimer;
    Clock sleepTimer;

    // Random control
    // XXRand -> XX random range
    glm::vec2 startSpeedRand = {0.f, 1.f};
    glm::vec2 endSpeedRand = {0.f, 1.f};
    glm::vec2 startSizeRand = {0.f, 1.f};
    glm::vec2 endSizeRand = {1.f, 1.f};
    glm::vec2 rotSpeedRand = {-1.f, 1.f};
    glm::vec2 emitVarianceRand = {0.f, 1.f};
    glm::vec2 lifeRand = {0.5f, 1.f};

    // When respawn particle, need to find first unusedParticle
    int lastUnusedParticle = 0;

    // Particle texture
    Ref<Texture2D> texture;

    // Particle pool
    std::vector<Particle> particles;

    /**************** Vortex Attribute *****************/

    // Vortex pool
    std::vector<Particle> static_vortexes;
    std::vector<Particle> dynamic_vortexes;

    #define STATIC_VORTEX 0
    #define DYNAMIC_VORTEX 1

    // Vortex on/off
    bool vortexSensitive = false;

    // Vortex draw
    bool drawVortex = false;

    // Vortex type
    int vortexType = STATIC_VORTEX;

    // Vortex is Active
    bool vortexActive = true;

    // Vortex distance between emitter center
    float vortexDisX = 0.f;
    float vortexDisY = 0.f;

    // Vortex pos
    glm::vec2 vortexPos {0.f, 0.f};

    // Vortex emit angle
    glm::vec2 vortexAngleRange = {0.f, 0.f};

    // Vortex Start Speed
    float vortexStartSpeed = 0.f;

    // Vortex End Speed
    float vortexEndSpeed = 0.f;

    // Vortex Start Size
    float vortexStartSize = 20.f;

    // Vortex End SIze
    float vortexEndSize = 20.f;

    // When respawn vortex, find first unused vortex
    int lastUnusedVortex = 0;

    // vortex emit per frame
    int vortexEmissionRate = 0;

    // Least Vortex generate per frame
    uint32_t vortexEmitNumber = 1;

    // Vortex life
    uint32_t vortexMaxParticleLife = 100;

    // Vortex Pool size
    int vortexPoolSize = 0;

    // Random control
    glm::vec2 vortexStartSpeedRand = { 0.f, 1.f};
    glm::vec2 vortexEndSpeedRand = {0.f, 1.f};
    glm::vec2 vortexLifeRand = {0.5f, 1.f};
    glm::vec2 vortexStartSizeRand = {0.f, 1.f};
    glm::vec2 vortexEndSizeRand = {1.f, 1.f};
    glm::vec2 vortexTurbulenceRand = {0.f, 1.f};

    // The vel that effect particle;
    glm::vec2 vortexTurbulence = {0.f, 0.f};

    float vortexSleepTime = 0;
    Clock vortexSleepTimer;

    std::string texturePath = "assets/texture/texture.png";

    std::string dataPath = "";

    // TODO Move To ParticleSystem?
    void loadEmitData()
    {
        RL_CORE_TRACE("Emitter Load Data");
        EmitData data;

        if(!dataPath.empty())
        {
            std::string context = FileSystem::ReadTextFile(dataPath);
            std::string exceptionMsg;
            std::stringstream oos(context);

            try
            {
                cereal::JSONInputArchive ar(oos);
                ar(data);
            }
            catch (cereal::RapidJSONException &e)
            {
                RL_CORE_ERROR("Failed to load EmitData {}", e.what());
                exceptionMsg = e.what();
            }
            catch (cereal::Exception &e)
            {
                RL_CORE_ERROR("Failed to load EmitData {}", e.what());
                exceptionMsg = e.what();
            }
        }

        angleRange           = data.angleRange;
        startSpeed           = data.startSpeed;
        endSpeed             = data.endSpeed;
        startSize            = data.startSize;
        endSize              = data.endSize;
        rotateSpeed          = data.rotateSpeed;
        emitNumber           = data.emitNumber;
        emitVariance         = data.emitVariance;
        maxParticleLife      = data.maxParticleLife;
        maxParticlesPerFrame = data.emitNumber + data.emitVariance;
        poolSize             = maxParticlesPerFrame * (maxParticleLife + 1) > MAX_PARTICLE_POOLSIZE ? MAX_PARTICLE_POOLSIZE : maxParticlesPerFrame * (maxParticleLife + 1);
        particles.clear();
        particles.resize(poolSize);
//        particles.clear();
        startColor           = data.startColor;
        endColor             = data.endColor;
        active               = true;
        life_store           = life = data.life;
        sleepTime            = data.sleepTime;
        rotSpeedRand         = data.rotSpeedRand;
        startSpeedRand       = data.startSpeedRand;
        endSpeedRand         = data.endSpeedRand;
        emitVarianceRand     = data.emitVarianceRand;
        startSizeRand        = data.startSizeRand;
        endSizeRand          = data.endSizeRand;
        lifeRand             = data.lifeRand;
        disX                 = data.disX;
        disY                 = data.disY;

        vortexSensitive      = data.vortexSensitive;
        vortexType           = data.vortexType;
        static_vortexes.resize(1);
        if(vortexSensitive)
        {
            for(auto vortex : static_vortexes)
            {
                vortex.turbulence = data.vortexTurbulence;
                vortex.currentSize = data.vortexStartSize;
            }
        }

        vortexPos             = data.vortexPos; // offset
        vortexAngleRange      = data.vortexAngleRange;
        vortexStartSpeed      = data.vortexStartSpeed;
        vortexEndSpeed        = data.vortexEndSpeed;
        vortexStartSize       = data.vortexStartSize;
        vortexEndSize         = data.vortexEndSize;
        vortexEmitNumber      = data.vortexEmitNumber;
        vortexEmissionRate    = data.vortexEmitNumber;
        vortexMaxParticleLife = data.vortexMaxParticleLife;

        vortexPoolSize       = data.vortexEmitNumber*3;
        dynamic_vortexes.resize(vortexPoolSize);

        vortexTurbulence     = data.vortexTurbulence;
        vortexSleepTime      = data.vortexSleepTime;
        vortexStartSpeedRand = data.vortexStartSpeedRand;
        vortexEndSpeedRand   = data.vortexEndSpeedRand;
        vortexStartSizeRand  = data.vortexStartSizeRand;
        vortexEndSizeRand    = data.vortexEndSizeRand;
        vortexLifeRand       = data.vortexLifeRand;

        texturePath          = data.texturePath;

        texture = Texture2D::LoadTextureVFS(texturePath);
    }

    // TODO Remove me?
    void init()
    {
        RL_CORE_TRACE("Particle Component Init");
        RL_CORE_TRACE("texture path {}", texturePath);
        texture = Texture2D::LoadTextureVFS(texturePath);
        maxParticlesPerFrame = emitNumber + emitVariance;
        poolSize             = maxParticlesPerFrame * (maxParticleLife + 1);
        particles.resize(poolSize);
        vortexPoolSize = vortexEmitNumber*3;
        dynamic_vortexes.resize(vortexPoolSize);
        static_vortexes.resize(1);
    }

    ParticleComponent() = default;
    ParticleComponent(std::string texturePath) : texturePath(texturePath) {}
    ParticleComponent(std::shared_ptr<Texture2D> texture) : texture(texture){}

    template<typename Archive>
    void serialize(Archive &ar)
    {
        ar(cereal::make_nvp("dataPath", dataPath));
        ar(cereal::make_nvp("offset", offset));
        ar(cereal::make_nvp("angleRange", angleRange));
        ar(cereal::make_nvp("startSpeed", startSpeed));
        ar(cereal::make_nvp("endSpeed", endSpeed));
        ar(cereal::make_nvp("startSize", startSize));
        ar(cereal::make_nvp("endSize", endSize));
        ar(cereal::make_nvp("rotateSpeed", rotateSpeed));
        ar(cereal::make_nvp("emitNumber", emitNumber));
        ar(cereal::make_nvp("emitVariance", emitVariance));
        ar(cereal::make_nvp("maxParticleLife", maxParticleLife));
        ar(cereal::make_nvp("life", life));
        ar(cereal::make_nvp("sleepTime", sleepTime));
        ar(cereal::make_nvp("startColor", startColor));
        ar(cereal::make_nvp("endColor", endColor));
        ar(cereal::make_nvp("rotSpeedRand", rotSpeedRand));
        ar(cereal::make_nvp("startSpeedRand", startSpeedRand));
        ar(cereal::make_nvp("endSpeedRand", endSpeedRand));
        ar(cereal::make_nvp("emitVarianceRand", emitVarianceRand));
        ar(cereal::make_nvp("startSizeRand", startSizeRand));
        ar(cereal::make_nvp("endSizeRand", endSizeRand));
        ar(cereal::make_nvp("disX", disX));
        ar(cereal::make_nvp("disY", disY));
        ar(cereal::make_nvp("texturePath", texturePath));
        ar(cereal::make_nvp("vortexSensitive", vortexSensitive));
        ar(cereal::make_nvp("vortexType", vortexType));
        ar(cereal::make_nvp("vortexOffset", vortexPos));
        ar(cereal::make_nvp("vortexAngleRange", vortexAngleRange));
        ar(cereal::make_nvp("vortexStartSpeed", vortexStartSpeed));
        ar(cereal::make_nvp("vortexEndSpeed", vortexEndSpeed));
        ar(cereal::make_nvp("vortexStartSize", vortexStartSize));
        ar(cereal::make_nvp("vortexEndSize", vortexEndSize));
        ar(cereal::make_nvp("vortexEmitNumber", vortexEmitNumber));
        ar(cereal::make_nvp("vortexMaxParticleLife", vortexMaxParticleLife));
        ar(cereal::make_nvp("vortexTurbulence", vortexTurbulence));
        ar(cereal::make_nvp("vortexSleepTime", vortexSleepTime));
        ar(cereal::make_nvp("vortexStartSpeedRand", vortexStartSpeedRand));
        ar(cereal::make_nvp("vortexEndSpeedRand", vortexEndSpeedRand));
        ar(cereal::make_nvp("vortexLifeRand", vortexLifeRand));
        ar(cereal::make_nvp("vortexStartSizeRand", vortexStartSizeRand));
        ar(cereal::make_nvp("vortexEndSizeRand", vortexEndSizeRand));
        ar(cereal::make_nvp("vortexTurbulence", vortexTurbulence));
    }
};

struct AmbientLightComponent
{
    glm::vec4 colorMask = {1, 1, 1, 0.5};

private:
    friend class cereal::access;
    template<typename Achrive>
    void serialize(Achrive &ar)
    {
        ar(CEREAL_NVP(colorMask));
    }
};

struct LightComponent
{
//    glm::vec3 lightPos; // Use Transform's Pos;
    glm::vec3 viewPortPos;
    bool customViewPos = false;
    glm::vec2 viewPortSize = {10, 10};
    glm::vec4 color = {1, 1, 1, 1};
    float radius = 100;
    float strength = 100;
    float shadowScale = 10;
    glm::vec4 shadowColor{0, 0, 0, 1};
    bool penetrateRadius = false;

    static std::set<UUID> ENTITY_NO_RAY_CAST;

private:
    friend class cereal::access;

    template<typename Achrive>
    void load(Achrive &ar)
    {
        ar( CEREAL_NVP(viewPortPos),
            CEREAL_NVP(customViewPos),
            CEREAL_NVP(viewPortSize),
            CEREAL_NVP(color),
            CEREAL_NVP(radius),
            CEREAL_NVP(strength),
            CEREAL_NVP(shadowScale),
            CEREAL_NVP(shadowColor),
            CEREAL_NVP(ENTITY_NO_RAY_CAST),
            CEREAL_NVP(penetrateRadius)
        );

        if (ENTITY_NO_RAY_CAST.find(UUID("Trash")) != ENTITY_NO_RAY_CAST.end())
        {
            ENTITY_NO_RAY_CAST.erase(UUID("Trash"));
        }
    }
    template<typename Achrive>
    void save(Achrive &ar) const
    {
        ENTITY_NO_RAY_CAST.insert(UUID("Trash"));
        ar( CEREAL_NVP(viewPortPos),
            CEREAL_NVP(customViewPos),
            CEREAL_NVP(viewPortSize),
            CEREAL_NVP(color),
            CEREAL_NVP(radius),
            CEREAL_NVP(strength),
            CEREAL_NVP(shadowScale),
            CEREAL_NVP(shadowColor),
            CEREAL_NVP(ENTITY_NO_RAY_CAST),
            CEREAL_NVP(penetrateRadius)
        );
    }


//    template<typename Achrive>
//    void serialize(Achrive &ar)
//    {
//        ar( CEREAL_NVP(viewPortPos),
//            CEREAL_NVP(customViewPos),
//            CEREAL_NVP(viewPortSize),
//            CEREAL_NVP(color),
//            CEREAL_NVP(radius),
//            CEREAL_NVP(strength),
//            CEREAL_NVP(shadowScale),
//            CEREAL_NVP(shadowColor),
//            CEREAL_NVP(ENTITY_NO_RAY_CAST)
//        );
//    }
};

} // end of rl
