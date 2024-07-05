#ifndef SWINGSWING_SWING_HPP
#define SWINGSWING_SWING_HPP

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

private:
    std::shared_ptr<jt::Box2DObject> m_physicsObjectSuspension;
    std::shared_ptr<jt::Box2DObject> m_physicsObject;
    std::shared_ptr<jt::Shape> m_shape;
    std::shared_ptr<jt::Box2DJoint> m_joint;

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;
    void doKill() override;
    void doDestroy() override;
};

#endif // SWINGSWING_SWING_HPP
