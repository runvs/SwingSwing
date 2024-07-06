#include "state_game.hpp"
#include <box2dwrapper/box2d_world_impl.hpp>
#include <color/color.hpp>
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <hud/hud.hpp>
#include <screeneffects/vignette.hpp>
#include <shape.hpp>
#include <state_menu.hpp>

void StateGame::onCreate()
{
    m_world = std::make_shared<jt::Box2DWorldImpl>(jt::Vector2f { 0.0f, 200.0f });

    float const w = static_cast<float>(GP::GetWindowSize().x);
    float const h = static_cast<float>(GP::GetWindowSize().y);

    using jt::Shape;

    m_background = std::make_shared<Shape>();
    m_background->makeRect({ w, h }, textureManager());
    m_background->setColor(GP::PaletteBackground());
    m_background->setIgnoreCamMovement(true);
    m_background->update(0.0f);

    createPlayer();

    m_vignette = std::make_shared<jt::Vignette>(GP::GetScreenSize());
    add(m_vignette);
    m_hud = std::make_shared<Hud>();
    add(m_hud);

    // StateGame will call drawObjects itself.
    setAutoDraw(false);

    m_swingPowerBar = std::make_shared<jt::Bar>(16, 128, false, textureManager());
    m_expectedSwingTargetHeight = 35.0f;
}

void StateGame::onEnter() { }

void StateGame::createPlayer()
{
    m_swing = std::make_shared<Swing>(m_world);
    add(m_swing);
}

void StateGame::onUpdate(float const elapsed)
{
    if (m_running) {

        m_world->step(elapsed, GP::PhysicVelocityIterations(), GP::PhysicPositionIterations());
        // update game logic here

        if (getGame()->input().keyboard()->pressed(jt::KeyCode::LShift)
            && getGame()->input().keyboard()->pressed(jt::KeyCode::Escape)) {
            endGame();
        }

        if (!m_swing->isInSwing()) {
            if (getGame()->input().keyboard()->pressed(jt::KeyCode::Space)) {
                m_spacePressedTimer += elapsed;
            }
            if (getGame()->input().keyboard()->justReleased(jt::KeyCode::Space)) {
                triggerSwing();
            }
        }

        // Check Swing
        checkForSwingTargetHeight();

        m_swingPowerBar->setCurrentValue(convertTimeToPower());
        m_swingPowerBar->setMaxValue(1.0f);
        m_swingPowerBar->update(elapsed);
    }

    m_background->update(elapsed);
    m_vignette->update(elapsed);
}

void StateGame::checkForSwingTargetHeight()
{
    if (!m_swing->isInSwing()) {
        return;
    }
    if (m_swing->getBreakMode()) {
        return;
    }
    auto const targetHeight = m_expectedSwingTargetHeight;
    auto const hasReachedTarget = m_swing->getHeight() < targetHeight;
    if (hasReachedTarget) {
        m_swing->enableBreakMode(true);
        m_score++;
        m_hud->getObserverScoreP1()->notify(m_score);
    }
}

void StateGame::onDraw() const
{
    m_background->draw(renderTarget());
    drawObjects();
    m_vignette->draw();
    if (!m_swing->isInSwing()) {
        m_swingPowerBar->draw(renderTarget());
    }
    m_hud->draw();
}

void StateGame::endGame()
{
    if (m_hasEnded) {
        // trigger this function only once
        return;
    }
    m_hasEnded = true;
    m_running = false;

    getGame()->stateManager().switchToStoredState("menu");
}

std::string StateGame::getName() const { return "State Game"; }

void StateGame::triggerSwing()
{
    m_swing->trigger(convertTimeToPower());
    m_spacePressedTimer = 0.0f;
}

float StateGame::convertTimeToPower()
{
    constexpr float freq = 0.75f;
    float t = m_spacePressedTimer;
    return 2.0f * abs(t * freq - std::floor(t * freq + 0.5f));
}
