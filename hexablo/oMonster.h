// Copyright (c) 2022. Johan Lind

#pragma once

#include "oCharacter.h"

class oMonster : public oCharacter {
    JLE_REGISTER_OBJECT_TYPE(oMonster)
public:
    void SetupDefaultObject() override;

    void Start() override;

    void Update(float dt) override;

    void ToJson(nlohmann::json& j_out) override;

    void FromJson(const nlohmann::json& j_in) override;

private:
    void LookAtPlayer();
};
