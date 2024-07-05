#include "swing.hpp"
#include <game_interface.hpp>
#include <game_properties.hpp>

void Swing::doCreate()
{
    m_shape = std::make_shared<jt::Shape>();
    m_shape->makeRect(jt::Vector2f { 16.0f, 16.0f }, textureManager());
    m_shape->setOrigin(jt::OriginMode::CENTER);
}

void Swing::doUpdate(float const elapsed)
{
    m_shape->setPosition(m_physicsObject->getPosition());
    m_shape->setRotation(m_physicsObject->getRotation());
    m_shape->update(elapsed);
}

void Swing::doDraw() const { m_shape->draw(renderTarget()); }

void Swing::doKill() { }

void Swing::doDestroy() { }

Swing::Swing(std::shared_ptr<jt::Box2DWorldInterface> world)
{
    b2BodyDef suspensionBodyDef;
    suspensionBodyDef.type = b2BodyType::b2_staticBody;
    suspensionBodyDef.position
        = b2Vec2 { GP::GetScreenSize().x / 2.0f, GP::GetScreenSize().y / 2.0f - 32 };
    m_physicsObjectSuspension = std::make_shared<jt::Box2DObject>(world, &suspensionBodyDef);

    b2BodyDef bodyDef;
    bodyDef.position = b2Vec2 { GP::GetScreenSize().x / 2.0f, GP::GetScreenSize().y / 2.0f };
    bodyDef.type = b2BodyType::b2_dynamicBody;
    bodyDef.fixedRotation = false;
    bodyDef.angularDamping = 0.0f;
    bodyDef.linearDamping = 0.2f;
    m_physicsObject = std::make_shared<jt::Box2DObject>(world, &bodyDef);

    b2DistanceJointDef jointDev;
    jointDev.bodyA = m_physicsObject->getB2Body();
    jointDev.bodyB = m_physicsObjectSuspension->getB2Body();
    jointDev.collideConnected = false;
    jointDev.length = 96;

    jointDev.dampingRatio = 0.8f;
    m_joint = std::make_shared<jt::Box2DJoint>(world, &jointDev);
}

void Swing::trigger(float strength)
{
    std::cout << strength << std::endl;
    m_physicsObject->addForceToCenter(jt::Vector2f { 40000.0f * strength, 0.0f });
}
