#include "swing.hpp"
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
    m_shape->update(elapsed);
}

void Swing::doDraw() const { m_shape->draw(renderTarget()); }

void Swing::doKill() { }

void Swing::doDestroy() { }

Swing::Swing(std::shared_ptr<jt::Box2DWorldInterface> world)
{
    b2BodyDef bodyDef;
    bodyDef.position = b2Vec2 { GP::GetScreenSize().x / 2.0f, GP::GetScreenSize().y / 2.0f };
    bodyDef.type = b2BodyType::b2_dynamicBody;
    m_physicsObject = std::make_shared<jt::Box2DObject>(world, &bodyDef);
}
