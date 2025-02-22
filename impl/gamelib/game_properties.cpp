﻿#include "game_properties.hpp"
#include <color/palette_builder.hpp>

namespace {

jt::Palette createPalette()
{
    jt::PaletteBuilder builder;
    return builder
        .addColorsFromGPL(R"(GIMP Palette
#Palette Name: Dreamscape8
#Description: A palette made of low saturation colours to give your art a dreamlike quality.
#Colors: 8
201	204	161	c9cca1
202	160	90	caa05a
174	106	71	ae6a47
139	64	73	8b4049
84	51	68	543344
81	82	98	515262
99	120	125	63787d
142	160	145	8ea091
)")
        .create();
}

} // namespace

jt::Palette GP::getPalette()
{
    static auto const p = createPalette();
    return p;
}

int GP::PhysicVelocityIterations() { return 20; }

int GP::PhysicPositionIterations() { return 20; }

float GP::SwingDampingNormal() { return 0.2f; }

jt::Vector2f GP::SwingSuspensionPosition()
{
    return GetScreenSize() * 0.5 + jt::Vector2f { 8.0f, -32.0f };
}

float GP::SwingLength() { return 128.0f; }

float GP::SwingForceScalingFactor() { return 80000.0f; }

float GP::SwingGroundBrakingFactor() { return 0.99f; }
