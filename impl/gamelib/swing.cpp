#include "swing.hpp"
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <math_helper.hpp>

Swing::Swing(std::shared_ptr<jt::Box2DWorldInterface> world)
{
    b2BodyDef suspensionBodyDef;
    suspensionBodyDef.type = b2BodyType::b2_staticBody;
    suspensionBodyDef.position = jt::Conversion::vec(GP::SwingSuspensionPosition());
    m_physicsObjectSuspension = std::make_shared<jt::Box2DObject>(world, &suspensionBodyDef);

    b2BodyDef bodyDef;
    bodyDef.position = jt::Conversion::vec(
        GP::SwingSuspensionPosition() + jt::Vector2f { 0.0f, GP::SwingLength() });
    bodyDef.type = b2BodyType::b2_dynamicBody;
    bodyDef.fixedRotation = false;
    bodyDef.angularDamping = 0.0f;
    bodyDef.linearDamping = GP::SwingDampingNormal();
    m_physicsObjectSwing = std::make_shared<jt::Box2DObject>(world, &bodyDef);

    b2DistanceJointDef jointDev;
    jointDev.bodyA = m_physicsObjectSwing->getB2Body();
    jointDev.bodyB = m_physicsObjectSuspension->getB2Body();
    jointDev.collideConnected = false;
    jointDev.length = GP::SwingLength();

    jointDev.dampingRatio = 0.8f;
    m_joint = std::make_shared<jt::Box2DJoint>(world, &jointDev);
}

void Swing::doCreate()
{
    m_shape = std::make_shared<jt::Shape>();
    m_shape->makeRect(jt::Vector2f { 16.0f, 16.0f }, textureManager());
    m_shape->setOrigin(jt::OriginMode::CENTER);
}

void Swing::doUpdate(float const elapsed)
{
    m_shape->setPosition(m_physicsObjectSwing->getPosition());
    m_shape->setRotation(m_physicsObjectSwing->getRotation());
    m_shape->update(elapsed);

    if (m_isInBreakMode) {
        m_shape->setColor(jt::Color { 150, 0, 0 });
        auto v = m_physicsObjectSwing->getVelocity();
        auto const isNearGround = m_physicsObjectSwing->getPosition().y
            > GP::SwingSuspensionPosition().y + GP::SwingLength() * 0.95f;
        if (isNearGround) {
            // break
            m_shape->setColor(jt::Color { 255, 0, 0 });
            v = v * GP::SwingGroundBrakingFactor();
            m_physicsObjectSwing->setVelocity(v);

            // switch back to non-breaking when velocity is slow enough
            if (jt::MathHelper::lengthSquared(v) < 0.05f) {
                m_physicsObjectSwing->setVelocity({ 0.0f, 0.0 });
                enableBreakMode(false);
            }
        }
    } else {
        m_shape->setColor(jt::colors::Gray);
    }
}

void Swing::doDraw() const { m_shape->draw(renderTarget()); }

void Swing::doKill() { }

void Swing::doDestroy() { }

void Swing::trigger(float strength)
{
    m_physicsObjectSwing->addForceToCenter(
        jt::Vector2f { GP::SwingForceScalingFactor() * strength, 0.0f });
}

void Swing::enableBreakMode(bool enable) { m_isInBreakMode = enable; }

float Swing::getHeight() const { return m_physicsObjectSwing->getPosition().y; }
