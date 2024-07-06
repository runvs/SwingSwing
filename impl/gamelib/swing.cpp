#include "swing.hpp"
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <math_helper.hpp>

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

    // TODO check for target position
    auto const hasReachedTarget = m_physicsObject->getPosition().y < 35;
    if (hasReachedTarget)
        enableBreakMode(true);

    if (m_isInBreakMode) {
        auto v = m_physicsObject->getVelocity();
        auto const isNearGround = m_physicsObject->getPosition().y > 170;
        if (isNearGround) {
            // break;
            v = v * 0.99f;
            m_physicsObject->setVelocity(v);

            // switch back to non-breaking when velocity is slow enough
            if (jt::MathHelper::lengthSquared(v) < 0.01f) {
                m_physicsObject->setVelocity({ 0.0f, 0.0 });
                enableBreakMode(false);
            }
        }
    }
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
    bodyDef.linearDamping = GP::SwingDampingNormal();
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
    m_physicsObject->addForceToCenter(jt::Vector2f { 40000.0f * strength, 0.0f });
}

void Swing::enableBreakMode(bool enable)
{
    m_isInBreakMode = enable;
    //    if (m_isInBreakMode) {
    //        m_physicsObject->getB2Body()->SetLinearDamping(GP::SwingDampingNormal());
    //    } else {
    //        m_physicsObject->getB2Body()->SetLinearDamping(GP::SwingDampingWhenBreaking());
    //    }
}
