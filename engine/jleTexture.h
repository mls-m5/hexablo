// Copyright (c) 2022. Johan Lind

#pragma once

#include "no_copy_no_move.h"

#include <climits>
#include <string>

class jleTexture {
    NO_COPY_NO_MOVE(jleTexture)

public:
    jleTexture() = default;

    ~jleTexture();

    // Returns true if this Texture is the globally active texture
    bool IsActive();

    // Set this Texture to be the globally active texture
    void SetToActiveTexture(int texture_slot = 0);

    unsigned int GetTextureID();

    int32_t GetWidth();

    int32_t GetHeight();

private:
    friend class jleTextureCreator_OpenGL;

    int32_t width, height, nrChannels;

    unsigned int texture_id = UINT_MAX; // OpenGL Texture ID
};