#include "state_game.hpp"
#include "random/random.hpp"
#include "strutils.hpp"
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

    using jt::Shape;

    m_background = std::make_shared<jt::Sprite>("assets/background.aseprite", textureManager());
    m_background->setIgnoreCamMovement(true);
    m_background->update(0.0f);

    m_swingBack = std::make_shared<jt::Sprite>("assets/schaukel_hinten.aseprite", textureManager());
    m_swingBack->update(0.0f);

    m_swingFront = std::make_shared<jt::Sprite>("assets/schaukel_vorne.aseprite", textureManager());
    m_swingFront->update(0.0f);

    m_clouds = std::make_shared<jt::MarioCloudsHorizontal>(
        5, jt::Vector2f { GP::GetScreenSize().x, 100.0f }, jt::Vector2f { 40.0f, 10.0f });
    m_clouds->setGameInstance(getGame());
    m_clouds->create();

    createGrass();

    createPlayer();

    m_vignette = std::make_shared<jt::Vignette>(GP::GetScreenSize());
    add(m_vignette);

    m_hud = std::make_shared<Hud>();
    add(m_hud);

    m_swingPowerBar = std::make_shared<jt::Bar>(16, 128, false, textureManager());

    createNewTarget();

    m_targetLineLower = std::make_shared<jt::Line>(jt::Vector2f { GP::GetScreenSize().x, 0.0f });
    m_targetLineUpper = std::make_shared<jt::Line>(jt::Vector2f { GP::GetScreenSize().x, 0.0f });
    m_targetLineLower->setColor(jt::Color { 115, 149, 197, 200 });
    m_targetLineUpper->setColor(jt::Color { 115, 149, 197, 200 });
    auto spawnParticle = [this]() {
        auto shape = std::make_shared<jt::Shape>();
        shape->makeRect(jt::Vector2f { 1.0f, 1.0f }, textureManager());
        return shape;
    };

    m_targetLineParticlesLower = ParticleSystemType::createPS(
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

    m_targetLineParticlesUpper = ParticleSystemType::createPS(
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

    m_swingParticles = ParticleSystemType::createPS(
        spawnParticle, [this](auto& particle, jt::Vector2f const& pos) {
            auto const startPos
                = pos + jt::Random::getRandomPointIn(jt::Rectf { -8.0f, -8.0f, 16.0f, 16.0f });
            particle->setPosition(startPos);

            auto const endPos = startPos + jt::Random::getRandomPointInCircle(48);

            float const totalTime = 1.0f;

            add(jt::TweenPosition::create(particle, totalTime, startPos, endPos));

            std::shared_ptr<jt::Tween> twa = jt::TweenAlpha::create(
                particle, totalTime * 0.75f, jt::Random::getInt(150, 200), 0);
            twa->setStartDelay(totalTime * 0.25f);
            add(twa);
        });
    add(m_swingParticles);

    // StateGame will call drawObjects itself.
    setAutoDraw(false);
}

void StateGame::createGrass()
{
    for (auto i = 0u; i != 80u; ++i) {
        auto g = std::make_shared<jt::Animation>();
        std::string name = "gras";
        if (jt::Random::getChance(0.3f)) {
            name = "flower";
        }

        if (jt::Random::getChance()) {
            name += "1";
        } else {
            name += "2";
        }
        g->loadFromAseprite("assets/" + name + ".aseprite", textureManager());
        g->play("idle");
        if (strutil::contains(name, "gras")) {
            auto const v = jt::Random::getInt(200, 230);
            g->setColor(jt::Color(v, v, v));
        }
        g->setAnimationSpeedFactor(jt::Random::getFloat(0.7f, 1.2f));
        g->setPosition(jt::Random::getRandomPointIn(jt::Rectf { 0.0f, 203.0f, 320.0f, 37.0f }));
        m_grass.emplace_back(std::move(g));
    }
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
        for (auto& g : m_grass) {
            g->update(elapsed);
        }

        m_clouds->update(elapsed);
    }

    m_background->update(elapsed);
    m_vignette->update(elapsed);
    m_targetLineParticlesLower->update(elapsed);
    m_targetLineParticlesUpper->update(elapsed);
    m_swingParticles->update(elapsed);
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

        if (m_swingTargetTimeSinceReachedLower < 0.5f) {
            auto const hasReachedUpperTarget = m_swing->getHeight() < m_swingTargetHeightUpper;
            if (hasReachedUpperTarget) {
                // fail
                m_swing->enableBreakMode(true);
                m_swingTargetTimeSinceReachedLower = 0.0f;

                auto failSound = getGame()->audio().addTemporarySound("event:/fail");
                failSound->play();
            }
        } else {
            // succeed
            m_score++;
            m_hud->getObserverScoreP1()->notify(m_score);

            m_swingParticles->fire(100, m_swing->getPosition());
            m_swingTargetTimeSinceReachedLower = 0.0f;

            m_swing->enableBreakMode(true);

            createNewTarget();
            auto successSound = getGame()->audio().addTemporarySound("event:/success-applause");
            successSound->play();
        }
    }
}

void StateGame::onDraw() const
{
    m_background->draw(renderTarget());
    m_clouds->draw();
    for (auto& g : m_grass) {
        g->draw(renderTarget());
    }
    m_swingBack->draw(renderTarget());

    drawObjects();

    m_swingFront->draw(renderTarget());

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

    auto releaseSwingSound = getGame()->audio().addTemporarySound("event:/swing-squeaks-up");
    releaseSwingSound->play();

    auto happyShoutSound = getGame()->audio().addTemporarySound("event:/happy-shouts");
    happyShoutSound->play();
}

float StateGame::convertTimeToPower()
{
    constexpr float freq = 0.75f;
    float t = m_spacePressedTimer;
    return 2.0f * abs(t * freq - std::floor(t * freq + 0.5f));
}

void StateGame::createNewTarget()
{
    float newVal = jt::Random::getFloat(getCurrentWidth(), 150);
    int numberOfTries = 0;
    while (abs(newVal - m_swingTargetHeightLower) < 10) {
        numberOfTries += 1;
        newVal = jt::Random::getFloat(getCurrentWidth(), 150);
        if (numberOfTries > 10)
            break;
    }
    m_swingTargetHeightLower = newVal;
    m_swingTargetHeightUpper = m_swingTargetHeightLower - getCurrentWidth();
}

float StateGame::getCurrentWidth()
{
    float v = 64.0f - 3.0f * m_score;

    return std::clamp(v, 4.0f, v);
}
