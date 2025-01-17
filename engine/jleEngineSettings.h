// Copyright (c) 2023. Johan Lind

#ifndef JLE_ENGINESETTINGS
#define JLE_ENGINESETTINGS

#include "jleSerializedResource.h"
#include "jleWindowSettings.h"
#include "jleTypeReflectionUtils.h"


class jleEngineSettings : public jleSerializedResource, public std::enable_shared_from_this<jleEngineSettings>
{
public:

    JLE_REGISTER_RESOURCE_TYPE(jleEngineSettings, es)

    WindowSettings windowSettings;

    SAVE_SHARED_THIS_SERIALIZED_JSON(jleSerializedResource)

    ~jleEngineSettings() override = default;

    template <class Archive>
    void
    serialize(Archive &ar)
    {
        ar(CEREAL_NVP(windowSettings));
    }
};

CEREAL_REGISTER_TYPE(jleEngineSettings)
CEREAL_REGISTER_POLYMORPHIC_RELATION(jleSerializedResource, jleEngineSettings)


#endif // JLE_ENGINESETTINGS
