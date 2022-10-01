// Copyright (c) 2022. Johan Lind

#pragma once

#include "jleInputInterface.h"
#include <plog/Log.h>

class jleMouseInputInterface : public jleInputInterface {
public:
    ~jleMouseInputInterface() override = default;

    // Gets the X mouse position on the Window
    virtual int GetMouseX() = 0;

    // Gets the Y mouse position on the Window
    virtual int GetMouseY() = 0;

    // Gets the X mouse delta for 1 frame
    virtual float GetMouseXDelta() = 0;

    // Gets the Y mouse delta for 1 frame
    virtual float GetMouseYDelta() = 0;

    // Returns a float != 0 if scroll detected, value depend on scroll
    virtual float GetScrollX() = 0;

    // Returns a float != 0 if scroll detected, value depend on scroll
    virtual float GetScrollY() = 0;

    // Returns the X coordinate on the main pixelated framebuffer
    virtual int GetPixelatedMouseX() = 0;

    // Returns the Y coordinate on the main pixelated framebuffer
    virtual int GetPixelatedMouseY() = 0;

    virtual bool GetMouseClick(int button) = 0;
};