// Copyright (c) 2022. Johan Lind

#include "oMyPlayer.h"
#include "hexHelperFunctions.h"
#include "jleCore.h"

#include "plog/Log.h"

#include <glm/glm.hpp>


void oMyPlayer::SetupDefaultObject() {
    oCharacter::SetupDefaultObject();
}

void oMyPlayer::Start() {
    SetHexagonPlacementTeleport(mHexagonQ, mHexagonR);
}

void oMyPlayer::Update(float dt) {

    oCharacter::Update(dt);

    LookAtMouse();

    Movement(dt);

    if (jle::jleCore::core->input->keyboard->GetKeyDown('T')) {
        SetHexagonPlacementTeleport(5, 3);
    }

    if (jle::jleCore::core->input->keyboard->GetKeyDown('G')) {
        SetHexagonPlacementTeleport(2, 6);
    }

}

void oMyPlayer::ToJson(nlohmann::json &j_out) {
    oCharacter::ToJson(j_out);
}

void oMyPlayer::FromJson(const nlohmann::json &j_in) {
    oCharacter::FromJson(j_in);
}

void oMyPlayer::LookAtMouse() {
    auto x = hexHelperFunctions::GetPixelatedMouseXWorldSpace();
    auto y = hexHelperFunctions::GetPixelatedMouseYWorldSpace();

    constexpr int smallXAdjustment = -2;

    glm::vec2 target = {x, y};
    glm::vec2 origin = {mTransform->GetWorldX() + smallXAdjustment, mTransform->GetWorldY()};

    glm::vec2 vector2 = target - origin;
    glm::vec2 vector1{0, 1};

    const double angleRad = atan2(vector2.y, vector2.x) - atan2(vector1.y, vector1.x);
    const double angleDeg = glm::degrees(angleRad);

    if (angleDeg > -240.0 && angleDeg < -180) {
        SetCharacterDirection(oCharacterDirection::northwest);
    }

    if (angleDeg > -180 && angleDeg < -120) {
        SetCharacterDirection(oCharacterDirection::northeast);
    }

    if (angleDeg > -120 && angleDeg < -60) {
        SetCharacterDirection(oCharacterDirection::east);
    }

    if (angleDeg > -60 && angleDeg < 0) {
        SetCharacterDirection(oCharacterDirection::southeast);
    }

    if (angleDeg > 0 && angleDeg < 60) {
        SetCharacterDirection(oCharacterDirection::southwest);
    }

    if (angleDeg > 60 && angleDeg < 240) {
        SetCharacterDirection(oCharacterDirection::west);
    }
}

void oMyPlayer::Movement(float dt) {

    static bool canMove = true;
    static float lastMovement = 0.f;
    constexpr float defaultMoveTime = 0.15f;
    static float currentMoveTime = defaultMoveTime;

    if (!canMove) {
        if (jle::jleCore::core->status->GetCurrentFrameTime() > lastMovement + currentMoveTime) {
            canMove = true;
        } else {
            return;
        }
    }
    bool w = jle::jleCore::core->input->keyboard->GetKeyDown('W');
    bool a = jle::jleCore::core->input->keyboard->GetKeyDown('A');
    bool s = jle::jleCore::core->input->keyboard->GetKeyDown('S');
    bool d = jle::jleCore::core->input->keyboard->GetKeyDown('D');

    if ((w && a && s && d) || (w && s) || (a && d)) {
        return;
    }

    static bool verticalSide = true;

    if (w && d) {
        if (mHexagonR % 2 == 0) {
            SetHexagonPlacementInterp(mHexagonQ, mHexagonR - 1);
        } else {
            SetHexagonPlacementInterp(mHexagonQ + 1, mHexagonR - 1);
        }
        canMove = false;
        lastMovement = jle::jleCore::core->status->GetCurrentFrameTime();
        currentMoveTime = defaultMoveTime;
        return;
    } else if (w && a) {
        if (mHexagonR % 2 == 0) {
            SetHexagonPlacementInterp(mHexagonQ - 1, mHexagonR - 1);
        } else {
            SetHexagonPlacementInterp(mHexagonQ, mHexagonR - 1);
        }
        canMove = false;
        lastMovement = jle::jleCore::core->status->GetCurrentFrameTime();
        currentMoveTime = defaultMoveTime;
        return;
    } else if (a && s) {
        if (mHexagonR % 2 == 0) {
            SetHexagonPlacementInterp(mHexagonQ - 1, mHexagonR + 1);
        } else {
            SetHexagonPlacementInterp(mHexagonQ, mHexagonR + 1);
        }
        canMove = false;
        lastMovement = jle::jleCore::core->status->GetCurrentFrameTime();
        currentMoveTime = defaultMoveTime;
        return;
    } else if (s && d) {
        if (mHexagonR % 2 == 0) {
            SetHexagonPlacementInterp(mHexagonQ, mHexagonR + 1);
        } else {
            SetHexagonPlacementInterp(mHexagonQ + 1, mHexagonR + 1);
        }
        canMove = false;
        lastMovement = jle::jleCore::core->status->GetCurrentFrameTime();
        currentMoveTime = defaultMoveTime;
        return;
    } else if (a) {
        SetHexagonPlacementInterp(mHexagonQ - 1, mHexagonR);
        canMove = false;
        lastMovement = jle::jleCore::core->status->GetCurrentFrameTime();
        currentMoveTime = defaultMoveTime;
        return;
    } else if (d) {
        SetHexagonPlacementInterp(mHexagonQ + 1, mHexagonR);
        canMove = false;
        lastMovement = jle::jleCore::core->status->GetCurrentFrameTime();
        currentMoveTime = defaultMoveTime;
        return;
    } else if (w) {
        SetHexagonPlacementInterp(mHexagonQ, mHexagonR - 2);
        canMove = false;
        lastMovement = jle::jleCore::core->status->GetCurrentFrameTime();
        currentMoveTime = defaultMoveTime;
        return;
    } else if (s) {
        SetHexagonPlacementInterp(mHexagonQ, mHexagonR + 2);
        canMove = false;
        lastMovement = jle::jleCore::core->status->GetCurrentFrameTime();
        currentMoveTime = defaultMoveTime;
        return;
    }

}