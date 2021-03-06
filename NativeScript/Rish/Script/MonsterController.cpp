#include <Rish/Script/MonsterController.h>


namespace rl
{

void MonsterController::onCreate()
{

}

void MonsterController::onDestroy()
{

}

void MonsterController::onUpdate(Time dt)
{
    auto &rigid = GetComponent<RigidBody2DComponent>();
    auto &trans = GetComponent<TransformComponent>();;
    auto &boxc = GetComponent<BoxCollider2DComponent>();
    auto &render = GetComponent<SpriteRenderComponent>();


    setSprite(render, monsterState);
    switch (monsterState) {
        case MonsterState::Left:
            rigid.velocity.x = -1.0f;
            break;
        case MonsterState::Right:
            rigid.velocity.x = 1.0f;
            break;
    }

    // if state Change, rigid value must will be calculated by physics
    if(stateChange == true)
    {
        stateChange = false;
        return;
    }

    // TODO: should be optimize
    if (boxc.whoCollide.size() >= 2)
    {
        for(auto uuid : boxc.whoCollide)
        {
            auto collideEntity = GetScene().getEntityByUUID(uuid);

            if(!collideEntity.m_scene)
                break;

            if(collideEntity.hasComponent<NativeScriptComponent>())
            {
                auto collideEntity = GetScene().getEntityByUUID(uuid);
                if (collideEntity.getComponent<NativeScriptComponent>().scriptName == "rl::BoxEventController")
                {
                    if (monsterState == MonsterState::Left)
                    {
                        monsterState = MonsterState::Right;
                    }
                    else if (monsterState == MonsterState::Right)
                    {
                        monsterState = MonsterState::Left;
                    }
                    stateChange = true;
                }
            }

        }
    }
}

void MonsterController::onImGuiRender()
{
    ImGui::Text("State: %d", static_cast<int>(monsterState));
}

void MonsterController::setSprite(SpriteRenderComponent &render, MonsterState &monsterState)
{
    // TODO: Simplify, Flip features
    render.loadTexture("assets\\texture\\mario\\role.png");

    SubTexture2DSetting setting;
    setting.type = SubTexture2DSetting::SubTextureCoordinate;

    switch (monsterState)
    {
        case MonsterState::Left:
        {
            setting.leftUpper = glm::vec2(181, 198.5);
            setting.size      = glm::vec2(18, 30);
            break;
        }
        case MonsterState::Right:
        {
            setting.leftUpper = glm::vec2(294, 198.5);
            setting.size      = glm::vec2(18, 30);
            break;
        }
    }
    //
    render.loadSubTexture(setting);
}

};
