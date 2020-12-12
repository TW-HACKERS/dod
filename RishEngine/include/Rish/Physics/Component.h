#pragma once

#include <Rish/rlpch.h>

#include <Rish/Physics/vector_math.h>
#include <Rish/Utils/uuid.h>

namespace rl {

/**
 * @brief RigidBody2D
 */
struct RigidBody2DComponent
{
    enum class BodyType
    {
        Static = 0,
        Dynamic
    };
    BodyType BodyTypeState;

    /// 角速度
    float angularVelocity = 0.0f;
    /// 角度
    float angle = 0.0f;
    /// 速度
    Vec2 velocity = Vec2(0.0f, 0.0f);
    /// 持續受力
    bool keepingForce = false;
    /// 顯示受力接觸點
    bool showAttachPoint = false;
    /// 受力接觸點
    Vec2 attachPoint = Vec2(0.0f, 0.0f);
    /// 質量，質量倒數
    float mass = 10.0f;
    /// 轉動慣量
    float I = 0.0f;
    /// 受的力
    Vec2 force = Vec2(0.0f, 0.0f);
    /// 力矩
    float torque = 0.0f;
    /// 摩擦力
    float friction = 0.2f;
    /// 限制旋轉
    bool RestrictRotation = false;
    /// 限制重力
    bool RestrictGravity = false;
    /// 引擎內部限制重力
    bool engineRestrictGravity = false;


    RigidBody2DComponent() = default;
    RigidBody2DComponent(float mass) : mass(mass)
    {
    }
private:
    friend class cereal::access;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(
            CEREAL_NVP(BodyTypeState),
            CEREAL_NVP(angularVelocity),
            CEREAL_NVP(angle),
            CEREAL_NVP(velocity),
            CEREAL_NVP(friction),
            CEREAL_NVP(mass),
            CEREAL_NVP(I),
            CEREAL_NVP(force),
            CEREAL_NVP(torque),
            CEREAL_NVP(keepingForce),
            CEREAL_NVP(attachPoint),
            CEREAL_NVP(showAttachPoint),
            CEREAL_NVP(RestrictRotation),
            CEREAL_NVP(RestrictGravity)
            // TODO: add serializable parameter
        );
    }
};

struct Collider2DComponent
{
    enum class Type
    {
        Box,
        Circle,
        Polygon
    } type = Type::Box;
    float x = 0.0f;
    float y = 0.0f;
    float w = 1.0f;
    float h = 1.0f;
    float radius = 1.0f;
    int pointSize = 3;
    Vec2 pt[MaxVertexCount];
    std::vector<UUID> whoCollide;
    bool FollowTransform = true;
    bool isCollision = false;

    Collider2DComponent();
private:
    friend class cereal::access;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(

            CEREAL_NVP(x),
            CEREAL_NVP(y),
            CEREAL_NVP(w),
            CEREAL_NVP(h),
            CEREAL_NVP(radius),
            CEREAL_NVP(pt),
            CEREAL_NVP(type),
            CEREAL_NVP(pointSize),
            CEREAL_NVP(FollowTransform)
            // TODO: add serializable parameter
        );
    }
};


struct Joint2DComponent
{
    UUID rigidBody1;
    UUID rigidBody2;
    Vec2 anchor = Vec2(0.0f, 0.0f);

    // Fixed Value
    Vec2 localAnchor1, localAnchor2;
    Vec2 r1, r2;
    Vec2 P = Vec2(0.0f, 0.0f);
    Vec2 bias = Vec2(0.0f, 0.0f);
    float biasFactor = 0.2f;
    float softness = 0.0f;

    Joint2DComponent() = default;
private:
    friend class cereal::access;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(
            CEREAL_NVP(rigidBody1),
            CEREAL_NVP(rigidBody2),
            CEREAL_NVP(anchor),
            CEREAL_NVP(localAnchor1),
            CEREAL_NVP(localAnchor2),
            CEREAL_NVP(r1),
            CEREAL_NVP(r2),
            CEREAL_NVP(P),
            CEREAL_NVP(bias),
            CEREAL_NVP(biasFactor),
            CEREAL_NVP(softness)
        );
    }
};

}
