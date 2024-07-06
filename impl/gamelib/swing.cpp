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

    m_childAnimation = std::make_shared<jt::Animation>();
    m_childAnimation->loadFromAseprite("assets/kind.aseprite", textureManager());
    m_childAnimation->play("middle");
    m_childAnimation->setLooping("right", false);
    m_childAnimation->setLooping("left", false);
    m_childAnimation->setLooping("reverse-right", false);
    m_childAnimation->setLooping("reverse-left", false);
    m_childAnimation->setOrigin(jt::OriginMode::CENTER);
}

void Swing::doUpdate(float const elapsed)
{
    m_shape->setPosition(m_physicsObjectSwing->getPosition());
    m_shape->update(elapsed);

    if (m_isInBreakMode) {
        m_shape->setColor(jt::Color { 150, 0, 0 });
    } else {
        m_shape->setColor(jt::colors::Gray);
    }
    if (m_isInSwing) {
        m_timeInSwingMode += elapsed;

        bool const isLeft = getPosition().x < GP::GetScreenSize().x / 2.0f;
        bool const isRight = !isLeft;

        if (m_childAnimation->getCurrentAnimationName() == "right") {
            if (isRight) {
                if (m_timeInSwingMode > 0.1f) {
                    if (m_wasGoingUpLastFrame) {
                        if (m_physicsObjectSwing->getVelocity().y > 0) {
                            m_wasGoingUpLastFrame = false;
                            m_childAnimation->play("left");
                        }
                    }
                }
            }
        }
        if (m_childAnimation->getCurrentAnimationName() == "left") {
            if (isLeft) {
                if (m_timeInSwingMode > 0.1f) {
                    if (m_wasGoingUpLastFrame) {
                        if (m_physicsObjectSwing->getVelocity().y > 0) {
                            m_wasGoingUpLastFrame = false;
                            m_childAnimation->play("right");
                        }
                    }
                }
            }
        }
        if (m_wasRightLastFrame != isRight) {
            // crossed center
            m_wasGoingUpLastFrame = true;
        }
        m_wasRightLastFrame = isRight;
    }
    if (m_timeInSwingMode >= 3.5f) {
        enableBreakMode(true);
        m_timeInSwingMode = 0.0f;
    }

    auto v = m_physicsObjectSwing->getVelocity();
    auto const isNearGround = m_physicsObjectSwing->getPosition().y
        > GP::SwingSuspensionPosition().y + GP::SwingLength() * 0.95f;

    if (isNearGround) {
        if (m_isInBreakMode) {
            m_shape->setColor(jt::Color { 255, 0, 0 });
            v = v * GP::SwingGroundBrakingFactor();

            if (m_childAnimation->getCurrentAnimationName() == "right") {
                m_childAnimation->play("reverse-right");
            }
            if (m_childAnimation->getCurrentAnimationName() == "left") {
                m_childAnimation->play("reverse-left");
            }

            m_physicsObjectSwing->setVelocity(v);
        }

        if (m_isInSwing) {
            // switch back to non-breaking when velocity is slow enough

            if (jt::MathHelper::lengthSquared(v) < 0.025f) {
                enableBreakMode(false);
                if (m_timeInSwingMode >= 0.3f) {
                    m_isInSwing = false;
                    m_childAnimation->play("middle");
                }
            }
        }
    }

    m_childAnimation->setPosition(m_physicsObjectSwing->getPosition());
    m_childAnimation->update(elapsed);
}

void Swing::doDraw() const
{
    m_shape->draw(renderTarget());
    m_childAnimation->draw(renderTarget());
}

void Swing::doKill() { }

void Swing::doDestroy() { }

void Swing::trigger(float strength)
{
    m_physicsObjectSwing->addForceToCenter(
        jt::Vector2f { GP::SwingForceScalingFactor() * strength, 0.0f });
    m_isInSwing = true;
    m_timeInSwingMode = 0.0f;
    m_childAnimation->play("right");
    m_wasGoingUpLastFrame = true;
}

void Swing::enableBreakMode(bool enable) { m_isInBreakMode = enable; }

float Swing::getHeight() const { return m_physicsObjectSwing->getPosition().y; }

bool Swing::isInSwing() const { return m_isInSwing; }

bool Swing::getBreakMode() const { return m_isInBreakMode; }

jt::Vector2f Swing::getPosition() const { return m_physicsObjectSwing->getPosition(); }
