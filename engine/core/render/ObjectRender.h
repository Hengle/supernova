#ifndef OBJECTRENDER_H
#define OBJECTRENDER_H

#define S_VERTEXATTRIBUTE_VERTICES 1
#define S_VERTEXATTRIBUTE_NORMALS 3
#define S_VERTEXATTRIBUTE_POINTSIZES 10
#define S_VERTEXATTRIBUTE_POINTCOLORS 11
#define S_VERTEXATTRIBUTE_TEXTURERECTS 12

#define S_PROPERTY_MVPMATRIX 1
#define S_PROPERTY_MODELMATRIX 2
#define S_PROPERTY_NORMALMATRIX 3
#define S_PROPERTY_CAMERAPOS 4

#define S_PROPERTYDATA_FLOAT1 1
#define S_PROPERTYDATA_FLOAT2 2
#define S_PROPERTYDATA_FLOAT3 3
#define S_PROPERTYDATA_FLOAT4 4
#define S_PROPERTYDATA_INT1 5
#define S_PROPERTYDATA_INT2 6
#define S_PROPERTYDATA_INT3 7
#define S_PROPERTYDATA_INT4 8
#define S_PROPERTYDATA_MATRIX2 9
#define S_PROPERTYDATA_MATRIX3 10
#define S_PROPERTYDATA_MATRIX4 11

#include <unordered_map>
#include "Program.h"
#include "SceneRender.h"
#include "Texture.h"

namespace Supernova {
    class ObjectRender {
        
    private:
        
        void checkLighting();
        void checkFog();
    
    protected:
        
        struct attributeData{
            unsigned int elements;
            unsigned long size;
            void* data;
        };
        
        struct propertyData{
            int datatype;
            unsigned long size;
            void* data;
        };
        
        std::unordered_map<int, attributeData> vertexAttributes;
        attributeData indexAttribute;
        std::unordered_map<int, propertyData> properties;
        
        bool haslight;
        bool hasfog;
        
        SceneRender* sceneRender;
        Texture* texture;
        Program* program;
        
        unsigned int minBufferSize;
        
        ObjectRender();

    public:
        static ObjectRender* newInstance();
        
        virtual ~ObjectRender();
        
        void setTexture(Texture* texture);
        void setSceneRender(SceneRender* sceneRender);
        void setMinBufferSize(unsigned int minBufferSize);

        void addVertexAttribute(int type, unsigned int elements, unsigned long size, void* data);
        void addIndex(unsigned long size, void* data);
        void addProperty(int type, int datatype, unsigned long size, void* data);

        virtual bool load();
        virtual bool draw();
        virtual void destroy();
    };
}


#endif //OBJECTRENDER_H
