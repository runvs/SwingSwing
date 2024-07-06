#include "state_game.hpp"
#include "random/random.hpp"
#include "tweens/tween_alpha.hpp"
#include "tweens/tween_position.hpp"
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

    m_swingPowerBar = std::make_shared<jt::Bar>(16, 128, false, textureManager());
    m_swingTargetHeightLower = 60.0f;
    m_swingTargetHeightUpper = 16.0f;

    m_targetLineLower = std::make_shared<jt::Line>(jt::Vector2f { GP::GetScreenSize().x, 0.0f });
    m_targetLineUpper = std::make_shared<jt::Line>(jt::Vector2f { GP::GetScreenSize().x, 0.0f });

    auto spawnParticle = [this]() {
        auto shape = std::make_shared<jt::Shape>();
        shape->makeRect(jt::Vector2f { 1.0f, 1.0f }, textureManager());
        return shape;
    };

    m_targetLineParticlesLower = jt::ParticleSystem<jt::Shape, 100>::createPS(
        spawnParticle, [this](auto& particle, jt::Vector2f const& /*pos*/) {
            auto const startPos = jt::Vector2f { 0.0f, m_swingTargetHeightLower }
                + jt::Random::getRandomPointIn(jt::Vector2f { GP::GetScreenSize().x, 1.0f });
            particle->setPosition(startPos);

            float const totalTime = 1.5f;

            jt::Vector2f endPos
                = startPos + jt::Vector2f { 0.0f, -16.0f * jt::Random::getFloat(0.4f, 1.0f) };
            endPos.y = std::clamp(endPos.y, m_swingTargetHeightUpper, m_swingTargetHeightLower);
            add(jt::TweenPosition::create(particle, totalTime, startPos, endPos));

            std::shared_ptr<jt::Tween> twa = jt::TweenAlpha::create(
                particle, totalTime * 0.75f, jt::Random::getInt(150, 200), 0);
            twa->setStartDelay(totalTime * 0.25f);
            add(twa);
        });
    add(m_targetLineParticlesLower);

    m_targetLineParticlesUpper = jt::ParticleSystem<jt::Shape, 100>::createPS(
        spawnParticle, [this](auto& particle, jt::Vector2f const& /*pos*/) {
            auto const startPos = jt::Vector2f { 0.0f, m_swingTargetHeightUpper }
                + jt::Random::getRandomPointIn(jt::Vector2f { GP::GetScreenSize().x, 1.0f });
            particle->setPosition(startPos);

            float const totalTime = 1.5f;
            jt::Vector2f endPos
                = startPos + jt::Vector2f { 0.0f, 16.0f * jt::Random::getFloat(0.4f, 1.0f) };
            endPos.y = std::clamp(endPos.y, m_swingTargetHeightUpper, m_swingTargetHeightLower);
            add(jt::TweenPosition::create(particle, totalTime, startPos, endPos));

            std::shared_ptr<jt::Tween> twa = jt::TweenAlpha::create(
                particle, totalTime * 0.75f, jt::Random::getInt(150, 200), 0);
            twa->setStartDelay(totalTime * 0.25f);
            add(twa);
        });
    add(m_targetLineParticlesUpper);

    // StateGame will call drawObjects itself.
    setAutoDraw(false);
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

        updateTargetLine(elapsed);

        m_targetLineParticlesTimer += elapsed;
        constexpr auto timerMax = 0.1f;
        while (m_targetLineParticlesTimer > timerMax) {
            m_targetLineParticlesLower->fire(1);
            m_targetLineParticlesUpper->fire(1);
            m_targetLineParticlesTimer -= timerMax;
        }

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
        checkForSwingTargetHeight(elapsed);

        m_swingPowerBar->setCurrentValue(convertTimeToPower());
        m_swingPowerBar->setMaxValue(1.0f);
        m_swingPowerBar->update(elapsed);
    }

    m_background->update(elapsed);
    m_vignette->update(elapsed);
    m_targetLineParticlesLower->update(elapsed);
    m_targetLineParticlesUpper->update(elapsed);
}

void StateGame::updateTargetLine(float const elapsed) const
{
    m_targetLineLower->setPosition(jt::Vector2f { 0.0f, m_swingTargetHeightLower });
    m_targetLineLower->update(elapsed);

    m_targetLineUpper->setPosition(jt::Vector2f { 0.0f, m_swingTargetHeightUpper });
    m_targetLineUpper->update(elapsed);
}

void StateGame::checkForSwingTargetHeight(float elapsed)
{
    if (!m_swing->isInSwing()) {
        return;
    }
    if (m_swing->getBreakMode()) {
        return;
    }
    auto const hasReachedLowerTarget = m_swing->getHeight() < m_swingTargetHeightLower;
    if (hasReachedLowerTarget) {
        m_swingTargetTimeSinceReachedLower += elapsed;

        if (m_swingTargetTimeSinceReachedLower < 0.75f) {
            auto const hasReachedUpperTarget = m_swing->getHeight() < m_swingTargetHeightUpper;
            if (hasReachedUpperTarget) {
                m_swing->enableBreakMode(true);
                m_swingTargetTimeSinceReachedLower = 0.0f;
            }
        } else {
            m_score++;
            m_hud->getObserverScoreP1()->notify(m_score);
            m_swingTargetTimeSinceReachedLower = 0.0f;
        }
    }
}

void StateGame::onDraw() const
{
    m_background->draw(renderTarget());
    drawObjects();
    m_targetLineParticlesLower->draw();

    m_targetLineLower->draw(renderTarget());
    m_targetLineUpper->draw(renderTarget());
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
