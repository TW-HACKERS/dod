#include <Rish/Physics/PhysicsWorld.h>

bool rl::PhysicsWorld::accumulateImpulses = true;
bool rl::PhysicsWorld::warmStarting = true;
bool rl::PhysicsWorld::positionCorrection = true;
float rl::PhysicsWorld::width;
float rl::PhysicsWorld::height;
// world coordinate
Vec2 rl::PhysicsWorld::m_center = Vec2(0, 0);

rl::PhysicsWorld::PhysicsWorld(Vec2 gravity_)
{
    gravity = gravity_;
}

rl::PhysicsWorld::PhysicsWorld(Vec2 gravity_, float width_, float height_)
{
    gravity = gravity_;
    width = width_;
    height = height_;
}

void rl::PhysicsWorld::setGravity(Vec2 gravity_)
{
    gravity = gravity_;
}

void rl::PhysicsWorld::Clear()
{
    //release space
    for (auto obj : bodies)
    {
        obj.reset();
    }
    bodies.clear();
    joints.clear();
    arbList.clear();
}

void rl::PhysicsWorld::Add(const Ref<RigidBody2D> &body)
{
    bodies.push_back(body);
}

void rl::PhysicsWorld::AddJoints(const Ref<Joint> &joint)
{
    joints.push_back(joint);
}

void rl::PhysicsWorld::Step(float delta_t)
{
    float inv_dt = delta_t > 0.0f ? 1.0f / delta_t : 0.0f;

    // BoardPhase detection
    BoardPhase();

    // Compute Forces
    for(int i = 0; i < bodies.size(); i++)
    {
        if (bodies.at(i)->invMass == 0.0f)
        {
            continue;
        }
        else
        {
            bodies.at(i)->ComputeForce(delta_t, gravity);
        }
    }

    // Prepare to Calculate
    for (auto &arb : arbList)
    {
        arb.PreStep(inv_dt, gravity);
    }

    for(auto &jit : joints)
    {
        jit->PreStep(inv_dt);
    }

    // Apply impulse
    for (int i = 0; i < iterations; ++i)
    {
        for(int j = 0; j < arbList.size( ); ++j)
        {
            arbList[j].ApplyImpulse();
        }

        for(int k = 0; k < joints.size(); ++k)
        {
            joints[k]->ApplyImpulse();
        }
    }

    // Integrate Velocities
    for(int i = 0; i < bodies.size(); i++)
    {
        bodies.at(i)->IntegrateVelocities(delta_t);
    }

    // Correct positions
    for(int i = 0; i < arbList.size( ); ++i)
    {
        arbList[i].PositionalCorrection();
    }
}

void rl::PhysicsWorld::BoardPhase()
{
    for(int i = 0; i < bodies.size(); i++)
    {
        for(int j = i+1; j < bodies.size(); j++)
        {
            // Inverse Mass
            if(bodies[i]->invMass == 0.0f && bodies[j]->invMass == 0.0f)
                continue;

            // Add in Arbiter
            if(!bodies[i]->isCollider || !bodies[j]->isCollider)
            {
                continue;
            }

            Arbiter newArb(bodies[i], bodies[j]);
            newArb.Solve();

            auto iter = find(arbList.begin(), arbList.end(), newArb);
            if (newArb.contactCounter > 0)
            {
                if (iter == arbList.end())
                {
                    arbList.emplace_back(newArb);
                }
                else
                {
                    iter->Update();
                }
            }
            else
            {
                if (iter != arbList.end())
                {
                    arbList.erase(iter);
                }
            }
        }
    }
}

Vec2 rl::PhysicsWorld::ChangeToPhysicsWorld(const Vec2& ps)
{
    float w = float(width);
    float h = float(height);
    float u = ps.x / w;
    float v = (h - ps.y) / h;

    float ratio = w / h;
    Vec2 extents(ratio * 25.0f, 25.0f);

    Vec2 lower = m_center - extents;
    Vec2 upper = m_center + extents;

    Vec2 pw;
    pw.x = (1.0f - u) * lower.x + u * upper.x;
    pw.y = (1.0f - v) * lower.y + v * upper.y;
    return pw;
};

Vec2 rl::PhysicsWorld::ConvertWorldToScreen(const Vec2& pw)
{
    float w = float(width);
    float h = float(height);
    float ratio = w / h;

    Vec2 extents(ratio * 25.0f, 25.0f);

    Vec2 lower = m_center - extents;
    Vec2 upper = m_center + extents;

    float u = (pw.x - lower.x) / (upper.x - lower.x);
    float v = (pw.y - lower.y) / (upper.y - lower.y);

    Vec2 ps;
    ps.x = u * w;
    ps.y = (1.0f - v) * h;
    return ps;
};

