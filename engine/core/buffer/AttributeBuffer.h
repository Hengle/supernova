//
// (c) 2018 Eduardo Doria.
//

#ifndef ATTRIBUTEBUFFER_H
#define ATTRIBUTEBUFFER_H

#include <string>
#include <vector>
#include <map>

#include "buffer/Buffer.h"
#include "render/ProgramRender.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"

namespace Supernova{

    class AttributeBuffer: public Buffer{

    private:

        std::vector<unsigned char> buffer;
        unsigned int vertexSize;

    public:
        AttributeBuffer();
        virtual ~AttributeBuffer();

        virtual bool resize(size_t pos);
        virtual void clearAll();
        virtual void clear();

        void addAttribute(int attribute, int elements);

    };

}


#endif //ATTRIBUTEBUFFER_H
