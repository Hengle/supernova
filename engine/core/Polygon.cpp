#include "Polygon.h"

#include "Log.h"
#include "render/ObjectRender.h"

using namespace Supernova;

Polygon::Polygon(): Mesh2D() {
	primitiveType = S_PRIMITIVE_TRIANGLES;

	buffer.setName("vertices");
	buffer.addAttribute(S_VERTEXATTRIBUTE_VERTICES, 3);
	buffer.addAttribute(S_VERTEXATTRIBUTE_TEXTURECOORDS, 2);
	buffer.addAttribute(S_VERTEXATTRIBUTE_NORMALS, 3);
}

Polygon::~Polygon() {
}

void Polygon::setSize(int width, int height){
    Log::Error("Can't set size of Polygon");
}

void Polygon::setInvertTexture(bool invertTexture){
    Mesh2D::setInvertTexture(invertTexture);
    if (loaded) {
        generateTexcoords();
        updateTexcoords();
    }
}

void Polygon::clear(){
    buffer.clearBuffer();
}

void Polygon::addVertex(Vector3 vertex){

    buffer.addValue(S_VERTEXATTRIBUTE_VERTICES, vertex);
    buffer.addValue(S_VERTEXATTRIBUTE_NORMALS, Vector3(0.0f, 0.0f, 1.0f));

    if (buffer.getCount() > 3){
        primitiveType = S_PRIMITIVE_TRIANGLES_STRIP;
    }
}

void Polygon::addVertex(float x, float y){
   addVertex(Vector3(x, y, 0));
}

void Polygon::generateTexcoords(){

    float min_X = std::numeric_limits<float>::max();
    float max_X = std::numeric_limits<float>::min();
    float min_Y = std::numeric_limits<float>::max();
    float max_Y = std::numeric_limits<float>::min();

    AttributeData* attVertex = buffer.getAttribute(S_VERTEXATTRIBUTE_VERTICES);

    for ( unsigned int i = 0; i < buffer.getCount(); i++){
        min_X = fmin(min_X, buffer.getValue(attVertex, i, 0));
        min_Y = fmin(min_Y, buffer.getValue(attVertex, i, 1));
        max_X = fmax(max_X, buffer.getValue(attVertex, i, 0));
        max_Y = fmax(max_Y, buffer.getValue(attVertex, i, 1));
    }

    double k_X = 1/(max_X - min_X);
    double k_Y = 1/(max_Y - min_Y);

    float u = 0;
    float v = 0;

    for ( unsigned int i = 0; i < buffer.getCount(); i++){
        u = (buffer.getValue(attVertex, i, 0) - min_X) * k_X;
        v = (buffer.getValue(attVertex, i, 1) - min_Y) * k_Y;
        if (invertTexture) {
            buffer.addValue(S_VERTEXATTRIBUTE_TEXTURECOORDS, Vector2(u, 1.0 - v));
        }else{
            buffer.addValue(S_VERTEXATTRIBUTE_TEXTURECOORDS, Vector2(u, v));
        }
    }

    width = (int)(max_X - min_X);
    height = (int)(max_Y - min_Y);
}

bool Polygon::load(){

    setInvertTexture(isIn3DScene());
    generateTexcoords();

    return Mesh::load();

}
