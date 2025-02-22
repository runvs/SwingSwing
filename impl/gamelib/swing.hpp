#ifndef SWINGSWING_SWING_HPP
#define SWINGSWING_SWING_HPP

#include "animation.hpp"
#include "box2dwrapper/box2d_joint.hpp"
#include "box2dwrapper/box2d_object.hpp"
#include "shape.hpp"
#include "sprite.hpp"
#include <box2dwrapper/box2d_world_interface.hpp>
#include <game_object.hpp>

class Swing : public jt::GameObject {
public:
    explicit Swing(std::shared_ptr<jt::Box2DWorldInterface> world);

    void trigger(float strength);

    void enableBreakMode(bool enable);

    float getHeight() const;

    bool isInSwing() const;
    bool getBreakMode() const;
    jt::Vector2f getPosition() const;

private:
    std::shared_ptr<jt::Box2DObject> m_physicsObjectSuspension;
    std::shared_ptr<jt::Box2DObject> m_physicsObjectSwing;
    std::shared_ptr<jt::Animation> m_childAnimation;
    std::shared_ptr<jt::Box2DJoint> m_joint;

    bool m_isInBreakMode { false };
    bool m_isInSwing { false };
    float m_timeInSwingMode { 0.0f };

    bool m_wasGoingUpLastFrame { true };
    bool m_wasRightLastFrame { false };

    bool m_wasOnTop { false };

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;
    void doKill() override;
    void doDestroy() override;
    void checkIfOvershootSoundShouldBePlayed();
    void breakAfterSwingingForTooLong();
};

#endif // SWINGSWING_SWING_HPP
