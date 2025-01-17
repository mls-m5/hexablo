// Copyright (c) 2023. Johan Lind

#include "cMesh.h"
#include "jleCore.h"
#include "jleResource.h"
#include "jleRendering.h"

JLE_EXTERN_TEMPLATE_CEREAL_CPP(cMesh)


cMesh::cMesh(jleObject *owner, jleScene *scene) : jleComponent(owner, scene) {}

void
cMesh::start()
{}

void
cMesh::update(float dt)
{
    if (_meshRef) {
        std::shared_ptr<jleMesh> mesh = _meshRef.get();
        std::shared_ptr<jleMaterial> material = _materialRef.get();
        gCore->rendering().rendering3d().sendMesh(
            mesh, material, getTransform().getWorldMatrix(), _attachedToObject->instanceID(), true);
    }
}

void
cMesh::editorUpdate(float dt)
{
    update(dt);
}

std::shared_ptr<jleMesh>
cMesh::getMesh()
{
    return _meshRef.get();
}
