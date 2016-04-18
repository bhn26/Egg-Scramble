//
//  Chicken.cpp
//  egg scramble
//
//  Created by Phoebe on 4/11/16.
//  Copyright © 2016 sunny side up. All rights reserved.
//

#include "ChickenAnim.h"
#include "../Scene.h"
#include "../PointLight.h"

ChickenAnim::ChickenAnim() {
    m_pEffect = NULL;
    m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
    m_directionalLight.AmbientIntensity = 0.55f;
    m_directionalLight.DiffuseIntensity = 0.9f;
    m_directionalLight.Direction = Vector3f(1.0f, 0.0, 0.0);
    
    m_persProjInfo.FOV = 60.0f;
    m_persProjInfo.Height = WINDOW_HEIGHT;
    m_persProjInfo.Width = WINDOW_WIDTH;
    m_persProjInfo.zNear = 1.0f;
    m_persProjInfo.zFar = 100.0f;
    
    m_position = Vector3f(0.0f, 0.0f, 6.0f);
}

ChickenAnim::~ChickenAnim() {
    SAFE_DELETE(m_pEffect);
}

void ChickenAnim::Draw(Camera* camera) {
    m_pEffect = new SkinningTechnique();
    
    if (!m_pEffect->Init()) {
        printf("Error initializing the lighting technique\n");
    }
    
    m_pEffect->Enable();
    
    m_pEffect->SetColorTextureUnit(0); // color texture unit index = 0
    m_pEffect->SetDirectionalLight(m_directionalLight);
    m_pEffect->SetMatSpecularIntensity(0.0f);
    m_pEffect->SetMatSpecularPower(0);
    
    if (!m_mesh.LoadMesh("assets/chickens/animations/chicken_dance.fbx")) {
        printf("Mesh load failed\n");
    }

    m_mesh.Render();
}

void ChickenAnim::Update() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    m_pEffect->Enable();
    
    vector<Matrix4f> Transforms;
    
    float RunningTime = GetRunningTime();
    
    m_mesh.BoneTransform(RunningTime, Transforms);
    
    for (uint i = 0 ; i < Transforms.size() ; i++) {
        m_pEffect->SetBoneTransform(i, Transforms[i]);
    }
    
    glm::vec3 pos = Scene::camera->Position();
    m_pEffect->SetEyeWorldPos(Vector3f(pos.x, pos.y, pos.z));
    
    Pipeline p;
    p.SetCamera(Scene::camera->Position(), Scene::camera->Position(), Scene::camera->Up());
    p.SetPerspectiveProj(m_persProjInfo);
    p.Scale(0.1f, 0.1f, 0.1f);
    
    Vector3f Pos(m_position);
    p.WorldPos(Pos);
    p.Rotate(270.0f, 180.0f, 0.0f);
    m_pEffect->SetWVP(p.GetWVPTrans());
    m_pEffect->SetWorldMatrix(p.GetWorldTrans());
    
    m_mesh.Render();
    
    //RenderFPS();
    
    //glutSwapBuffers();
}

