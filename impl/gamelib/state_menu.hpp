﻿#ifndef GAME_STATE_MENU_HPP
#define GAME_STATE_MENU_HPP

#include "screeneffects/mario_clouds_horizontal.hpp"
#include <game_state.hpp>

namespace jt {

class Text;
class Shape;
class Animation;
class Sprite;
class Vignette;

} // namespace jt

class StateMenu : public jt::GameState {
public:
    std::string getName() const override;

    void setScore(int s) { m_score = s; };

    void createGrass();

private:
    std::shared_ptr<jt::Animation> m_background;
    std::vector<std::shared_ptr<jt::Animation>> m_grass;
    std::shared_ptr<jt::MarioCloudsHorizontal> m_clouds;

    std::shared_ptr<jt::Text> m_textTitle;
    std::shared_ptr<jt::Text> m_textStart;
    std::shared_ptr<jt::Text> m_textExplanation;
    std::shared_ptr<jt::Text> m_textCredits;
    std::shared_ptr<jt::Text> m_textVersion;

    std::shared_ptr<jt::Shape> m_overlay;
    std::shared_ptr<jt::Vignette> m_vignette;

    int m_score { 0 };

    bool m_started { false };

    void onCreate() override;
    void onEnter() override;
    void createVignette();
    void createShapes();

    void createMenuText();
    void createTextCredits();
    void createTextStart();
    void createTextTitle();

    void createTweens();
    void createInstructionTweenColor1();
    void createInstructionTweenColor2();
    void createTweenTitleAlpha();
    void createTweenOverlayAlpha();
    void createTweenCreditsPosition();
    void createTweenExplanation();

    void onUpdate(float const elapsed) override;
    void updateDrawables(float const& elapsed);
    void checkForTransitionToStateGame();
    void startTransitionToStateGame();

    void onDraw() const override;
    void createTextExplanation();
};

#endif
