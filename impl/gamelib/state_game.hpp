#ifndef GAME_STATE_GAME_HPP
#define GAME_STATE_GAME_HPP

#include "bar.hpp"
#include "line.hpp"
#include "particle_system.hpp"
#include "swing.hpp"
#include <box2dwrapper/box2d_world_interface.hpp>
#include <game_state.hpp>
#include <memory>
#include <vector>

// fwd decls
namespace jt {
class Shape;
class Sprite;
class Vignette;
} // namespace jt

class Hud;

class StateGame : public jt::GameState {
public:
    std::string getName() const override;

private:
    std::shared_ptr<jt::Shape> m_background;
    std::shared_ptr<jt::Vignette> m_vignette;
    std::shared_ptr<Hud> m_hud;
    std::shared_ptr<jt::Box2DWorldInterface> m_world { nullptr };

    std::shared_ptr<jt::Bar> m_swingPowerBar { nullptr };
    std::shared_ptr<Swing> m_swing;

    std::shared_ptr<jt::Line> m_targetLineLower { nullptr };
    std::shared_ptr<jt::Line> m_targetLineUpper { nullptr };
    using ParticleSystemType = jt::ParticleSystem<jt::Shape, 100>;
    std::shared_ptr<ParticleSystemType> m_targetLineParticlesLower;
    std::shared_ptr<ParticleSystemType> m_targetLineParticlesUpper;
    std::shared_ptr<ParticleSystemType> m_swingParticles;
    float m_swingTargetHeightLower { 0.0f };
    float m_swingTargetHeightUpper { 0.0f };

    float m_swingTargetTimeSinceReachedLower { 0.0f };

    float m_targetLineParticlesTimer { 0.0f };

    bool m_running { true };
    bool m_hasEnded { false };

    int m_score { 0 };
    float m_spacePressedTimer { 0.0f };

    void onCreate() override;
    void onEnter() override;
    void onUpdate(float const elapsed) override;
    void onDraw() const override;

    void endGame();
    void createPlayer();

    void triggerSwing();

    float convertTimeToPower();
    void checkForSwingTargetHeight(float elapsed);
    void updateTargetLine(float const elapsed) const;

    void createNewTarget();
    float getCurrentWidth();
};

#endif
