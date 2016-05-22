//
//  Puck.cpp
//  bRenderer_ios
//
//  Created by Björn Hasselmann on 03.05.16.
//
//

#include "Puck.hpp"

Puck::Puck(Field *field, Player *one, Player *two, GLfloat dimensionX, GLfloat dimensionY, GLfloat scale) : ObjectModel("hockeypuck"), trans_z(0.5), current(0.0, 0.0), last(current), initVelocity(0.03f) {
    this->field = field;
    this->dimension.x = dimensionX * scale;
    this->dimension.y = dimensionY * scale;
    this->radius = dimensionX / 2 * scale;
    this->scale = vmml::Vector3f(scale);
    if(one->stick->left){
        this->left = one;
        this->right = two;
    } else {
        this->left = two;
        this->right = one;
    }
    this->velocity = initVelocity;
}

void Puck::drawModel(Renderer &bRenderer, const std::string &cameraName = ObjectModel::CAMERA_NAME) {
//    this->makeMovement();
    vmml::Matrix4f modelMatrixHockeypuck = this->field->fieldMatrix
                        * vmml::create_translation(vmml::Vector3f(this->current.x, trans_z, this->current.y))
                        * vmml::create_scaling(scale);
    ShaderPtr shaderHockeypuck = bRenderer.getObjects()->getShader(MODEL_NAME);
    
    this->ObjectModel::drawModel(bRenderer, MODEL_NAME, cameraName, modelMatrixHockeypuck, std::vector<std::string>({ }));
    
    vmml::Matrix4f viewMatrixHockeypuck = bRenderer.getObjects()->getCamera("camera")->getViewMatrix();
    if (shaderHockeypuck.get())
    {
        shaderHockeypuck->setUniform("ProjectionMatrix", vmml::Matrix4f::IDENTITY);
        shaderHockeypuck->setUniform("ViewMatrix", viewMatrixHockeypuck);
        shaderHockeypuck->setUniform("ModelMatrix", modelMatrixHockeypuck);
    
        vmml::Matrix3f normalMatrixHockeypuck;
        vmml::compute_inverse(vmml::transpose(vmml::Matrix3f(modelMatrixHockeypuck)),normalMatrixHockeypuck);
        shaderHockeypuck->setUniform("NormalMatrix", normalMatrixHockeypuck);

        vmml::Vector4f eyePos = vmml::Vector4f(0.0f, 0.0f, 10.0f, 1.0f);
        shaderHockeypuck->setUniform("EyePos", eyePos);
    
        shaderHockeypuck->setUniform("LightPos", vmml::Vector4f(0.f, 1.f, .5f, 1.f));
        shaderHockeypuck->setUniform("Ia", vmml::Vector3f(1.f));
        shaderHockeypuck->setUniform("Id", vmml::Vector3f(1.f));
        shaderHockeypuck->setUniform("Is", vmml::Vector3f(1.f));
        
        std::vector<std::string> cubeMapFileNames;
        cubeMapFileNames.push_back("skyboxSide5.png");
        cubeMapFileNames.push_back("skyboxSide2.png");
        cubeMapFileNames.push_back("skyboxSide4.png");
        cubeMapFileNames.push_back("skyboxSide1.png");
        cubeMapFileNames.push_back("skyboxSide3.png"); //not visible
        cubeMapFileNames.push_back("skyboxSide6.png");
        
        CubeMapPtr cubeMap = bRenderer.getObjects()->loadCubeMap(MODEL_NAME, cubeMapFileNames);
        shaderHockeypuck->setUniform("skybox", cubeMap);
    }
    else
    {
        bRenderer::log("No shader available.");
    }
}

/*
 * Returns true, if there was a score.
 */
bool Puck::checkPlayerCollision(Player *player, Point &next, GLfloat xBound, bool &collision) {
    float stickTransY = player->stick->translation.y;
    float stickDimY = player->stick->dimension.y/2;
    float diff = next.x + (xBound > 0 ? 1 : -1) * this->radius - xBound;
    if(next.y <= stickTransY + stickDimY
        && next.y >= stickTransY - stickDimY
        && !next.playerCollision ) {
        // collision with stick
        next.addVelocity(-diff, this->current);
        next.playerCollision = true;
        collision = true;
        this->velocity += 0.02;
        std::cout << "stick collision!" << "\n";
    } else {
        // score!
        next.playerCollision = true;
        if( (diff > 0 && next.x > xBound + 0.5f) || (diff < 0 && next.x < xBound - 0.5f)){
            std::cout << "SCORE!" << "\n";
            std::cout << "reset puck to origin" << "\n";
            sleep(2);
            this->current = Point(0.0,0.0);
            this->velocity = this->initVelocity;
            return true;
        }
    }
    return false;
}

void Puck::makeMovement(){
    
    std::cout << "makeMovement start" << "\n";
    
    Point next(0.0, 0.0);
    if(this->current.isImpact){
        std::cout << "is impact!" << "\n";
        next = this->bounce(this->last, this->current, this->current.playerCollision);
    } else {
        next = Point(this->current);
        next.addVelocity(this->velocity, this->last); // starts always the same from 0.0
    }
    bool collision = false;
    
    // y axis
    GLfloat yBound = this->field->dimension.y/2;
    // upper bound
    if(next.y + radius >= yBound) {
        float diff = next.y + this->radius - yBound;
        next.addVelocity(-diff, this->current);
        collision = true;
        std::cout << "collision at the upper bound!" << "\n";
    }
    // lower bound
    else if (next.y - radius <= -yBound) {
        float diff = next.y - this->radius + yBound;
        next.addVelocity(diff, this->current);
        collision = true;
        std::cout << "collision at the lower bound!" << "\n";
    }
    
    //x axis
    GLfloat xBound = this->right->stick->translation.x;
    if(next.x + radius >= xBound) {
        std::cout << "possible player collision on the right.." << "\n";
        if(this->checkPlayerCollision(this->right, next, xBound, collision)) {
            this->left->scorePoint();
            return;
        }
    } else if (next.x - radius <= -xBound) {
        std::cout << "possible player collision on the left.." << "\n";
        if(this->checkPlayerCollision(this->left, next, -xBound, collision)) {
            this->right->scorePoint();
            return;
        }
    } else {
        next.playerCollision = false;
    }
    
    next.isImpact = collision;
    this->last = Point(this->current);
    this->current = next;
    
}

GLfloat computeDependentCoord(GLfloat last, GLfloat current){
    GLfloat coord = current + (current - last);
    coord += (rand() % 200 + (-100)) / 10000.0f; // add a little "English" TODO: won't work
    return coord;
}

Puck::Point Puck::bounce(Point &last, Point &impact, bool playerCollision){
    GLfloat x, y;
    if(playerCollision) {
        y = computeDependentCoord(last.y, impact.y);
        x = last.x;
    } else {
        y = last.y;
        x = computeDependentCoord(last.x, impact.x);
    }
    Point next(x, y);
    next.playerCollision = playerCollision;
    return next;
}
