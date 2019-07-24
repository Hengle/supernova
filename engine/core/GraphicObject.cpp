#include "GraphicObject.h"
#include "Scene.h"
#include "Log.h"

//
// (c) 2018 Eduardo Doria.
//

using namespace Supernova;

GraphicObject::GraphicObject(): Object(){
    visible = true;
    transparent = false;
    distanceToCamera = -1;

    minBufferSize = 0;

    render = NULL;
    shadowRender = NULL;

    scissor = Rect(0, 0, 0, 0);

    body = NULL;
}

GraphicObject::~GraphicObject(){
}

bool GraphicObject::instanciateRender(){
    if (render == NULL) {
        render = ObjectRender::newInstance();
        if (render == NULL)
            return false;
    }

    return true;
}

bool GraphicObject::instanciateShadowRender(){
    if (shadowRender == NULL) {
        shadowRender = ObjectRender::newInstance();
        if (shadowRender == NULL)
            return false;
    }

    return true;
}

void GraphicObject::updateBuffer(std::string name){
    if (name == defaultBuffer) {
        if (render)
            render->setVertexSize(buffers[name]->getCount());
        if (shadowRender)
            shadowRender->setVertexSize(buffers[name]->getCount());
    }
    if (render)
        render->updateBuffer(name, buffers[name]->getSize(), buffers[name]->getData());
    if (shadowRender)
        shadowRender->updateBuffer(name, buffers[name]->getSize(), buffers[name]->getData());
}

Matrix4 GraphicObject::getNormalMatrix(){
    return normalMatrix;
}

unsigned int GraphicObject::getMinBufferSize(){
    return minBufferSize;
}

void GraphicObject::setVisible(bool visible){
    this->visible = visible;
}

bool GraphicObject::isVisible(){
    return visible;
}

void GraphicObject::updateDistanceToCamera(){
    distanceToCamera = (this->cameraPosition - this->getWorldPosition()).length();
}

void GraphicObject::setSceneTransparency(bool transparency){
    if (scene) {
        if (scene->getUserDefinedTransparency() != S_OPTION_NO)
            scene->useTransparency = transparency;
    }
}

void GraphicObject::updateVPMatrix(Matrix4* viewMatrix, Matrix4* projectionMatrix, Matrix4* viewProjectionMatrix, Vector3* cameraPosition){
    Object::updateVPMatrix(viewMatrix, projectionMatrix, viewProjectionMatrix, cameraPosition);

    updateDistanceToCamera();
}

void GraphicObject::updateModelMatrix(){
    Object::updateModelMatrix();
    
    this->normalMatrix.identity();

    updateDistanceToCamera();
}

bool GraphicObject::draw(){

    bool drawReturn = false;

    if (scene && scene->isDrawingShadow()){
        shadowDraw();
    }else{
        if (transparent && scene && scene->useDepth && distanceToCamera >= 0){
            scene->transparentQueue.insert(std::make_pair(distanceToCamera, this));
        }else{
            if (visible)
                renderDraw();
        }

        if (transparent){
            setSceneTransparency(true);
        }
    }

    if (!scissor.isZero() && scene){

        SceneRender* sceneRender = scene->getSceneRender();
        bool on = sceneRender->isEnabledScissor();
        Rect rect = sceneRender->getActiveScissor();

        if (on)
            scissor.fitOnRect(rect);

        sceneRender->enableScissor(scissor);

        drawReturn = Object::draw();

        if (!on)
            sceneRender->disableScissor();

    }else{

        drawReturn = Object::draw();
    }

    return drawReturn;
}

bool GraphicObject::load(){
    Object::load();

    setSceneTransparency(transparent);

    instanciateRender();

    for (auto const& buf : buffers){
        if (buf.first == defaultBuffer) {
            render->setVertexSize(buf.second->getCount());
        }
        render->addBuffer(buf.first, buf.second->getSize(), buf.second->getData(), buf.second->getBufferType(), true);
        if (buf.second->isRenderAttributes()) {
            for (auto const &x : buf.second->getAttributes()) {
                render->addVertexAttribute(x.first, buf.first, x.second.getElements(), x.second.getDataType(), x.second.getStride(), x.second.getOffset());
            }
        }
    }

    render->addProperty(S_PROPERTY_MODELMATRIX, S_PROPERTYDATA_MATRIX4, 1, &modelMatrix);
    render->addProperty(S_PROPERTY_NORMALMATRIX, S_PROPERTYDATA_MATRIX4, 1, &normalMatrix);
    render->addProperty(S_PROPERTY_MVPMATRIX, S_PROPERTYDATA_MATRIX4, 1, &modelViewProjectionMatrix);
    render->addProperty(S_PROPERTY_CAMERAPOS, S_PROPERTYDATA_FLOAT3, 1, &cameraPosition); //TODO: put cameraPosition on Scene

    if (scene){

        render->setNumLights((int)scene->getLights()->size());
        render->setNumShadows2D(scene->getLightData()->numShadows2D);
        render->setNumShadowsCube(scene->getLightData()->numShadowsCube);

        render->setSceneRender(scene->getSceneRender());

        scene->addLightProperties(render);
        scene->addFogProperties(render);

        render->addTextureVector(S_TEXTURESAMPLER_SHADOWMAP2D, scene->getLightData()->shadowsMap2D);
        render->addProperty(S_PROPERTY_NUMSHADOWS2D, S_PROPERTYDATA_INT1, 1, &scene->getLightData()->numShadows2D);
        render->addProperty(S_PROPERTY_DEPTHVPMATRIX, S_PROPERTYDATA_MATRIX4, scene->getLightData()->numShadows2D, &scene->getLightData()->shadowsVPMatrix.front());
        render->addProperty(S_PROPERTY_SHADOWBIAS2D, S_PROPERTYDATA_FLOAT1, scene->getLightData()->numShadows2D, &scene->getLightData()->shadowsBias2D.front());
        render->addProperty(S_PROPERTY_SHADOWCAMERA_NEARFAR2D, S_PROPERTYDATA_FLOAT2, scene->getLightData()->numShadows2D, &scene->getLightData()->shadowsCameraNearFar2D.front());
        render->addProperty(S_PROPERTY_NUMCASCADES2D, S_PROPERTYDATA_INT1, scene->getLightData()->numShadows2D, &scene->getLightData()->shadowNumCascades2D.front());

        render->addTextureVector(S_TEXTURESAMPLER_SHADOWMAPCUBE, scene->getLightData()->shadowsMapCube);
        render->addProperty(S_PROPERTY_SHADOWBIASCUBE, S_PROPERTYDATA_FLOAT1, scene->getLightData()->numShadowsCube, &scene->getLightData()->shadowsBiasCube.front());
        render->addProperty(S_PROPERTY_SHADOWCAMERA_NEARFARCUBE, S_PROPERTYDATA_FLOAT2, scene->getLightData()->numShadowsCube, &scene->getLightData()->shadowsCameraNearFarCube.front());
    }

    return render->load();
}

bool GraphicObject::textureLoad(){
    
    return true;
}

bool GraphicObject::shadowLoad(){

    instanciateShadowRender();

    for (auto const& buf : buffers) {
        if (buf.first == defaultBuffer) {
            shadowRender->setVertexSize(buf.second->getCount());
        }
        shadowRender->addBuffer(buf.first, buf.second->getSize(), buf.second->getData(), buf.second->getBufferType(), true);
        if (buf.second->isRenderAttributes()) {
            for (auto const &x : buf.second->getAttributes()) {
                shadowRender->addVertexAttribute(x.first, buf.first, x.second.getElements(), x.second.getDataType(), x.second.getStride(), x.second.getOffset());
            }
        }
    }

    shadowRender->addProperty(S_PROPERTY_MVPMATRIX, S_PROPERTYDATA_MATRIX4, 1, &modelViewProjectionMatrix);
    shadowRender->addProperty(S_PROPERTY_MODELMATRIX, S_PROPERTYDATA_MATRIX4, 1, &modelMatrix);

    if (scene){
        shadowRender->addProperty(S_PROPERTY_SHADOWLIGHT_POS, S_PROPERTYDATA_FLOAT3, 1, &scene->drawShadowLightPos);
        shadowRender->addProperty(S_PROPERTY_SHADOWCAMERA_NEARFAR, S_PROPERTYDATA_FLOAT2, 1, &scene->drawShadowCameraNearFar);
        shadowRender->addProperty(S_PROPERTY_ISPOINTSHADOW, S_PROPERTYDATA_INT1, 1, &scene->drawIsPointShadow);
    }
    
    return shadowRender->load();
}

bool GraphicObject::shadowDraw(){

    return true;
}

bool GraphicObject::renderDraw(){

    return true;
}

void GraphicObject::destroy(){
    if (render)
        render->destroy();

    Object::destroy();
}
