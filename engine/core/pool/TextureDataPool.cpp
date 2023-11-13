//
// (c) 2023 Eduardo Doria.
//

#include "TextureDataPool.h"

#include "Engine.h"
#include "Log.h"

using namespace Supernova;

texturesdata_t& TextureDataPool::getMap(){
    //To prevent similar problem of static init fiasco but on deinitialization
    //https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use
    static texturesdata_t* map = new texturesdata_t();
    return *map;
};

std::shared_ptr<std::array<TextureData,6>> TextureDataPool::get(std::string id){
	auto& shared = getMap()[id];

	if (shared.use_count() > 0){
		return shared;
	}

	return std::make_shared<std::array<TextureData,6>>();
}

std::shared_ptr<std::array<TextureData,6>> TextureDataPool::get(std::string id, TextureType type, TextureData data[6], TextureFilter minFilter, TextureFilter magFilter, TextureWrap wrapU, TextureWrap wrapV){
	auto& shared = getMap()[id];

	if (shared.use_count() > 0){
		return shared;
	}

	int numFaces = 1;
	if (type == TextureType::TEXTURE_CUBE){
		numFaces = 6;
	}

	const auto resource =  std::make_shared<std::array<TextureData,6>>();

	void* data_array[6];
	size_t size_array[6];

	for (int f = 0; f < numFaces; f++){
		data_array[f] = data[f].getData();
		size_array[f] = (size_t)data[f].getSize();

        (*resource)[f] = data[f];
	}

	//resource->render.createTexture(id, data[0].getWidth(), data[0].getHeight(), data[0].getColorFormat(), type, numFaces, data_array, size_array, minFilter, magFilter, wrapU, wrapV);
	//Log::debug("Create texture %s", id.c_str());
	//shared = resource;

	return resource;
}

void TextureDataPool::remove(std::string id){
	if (getMap().count(id)){
		auto& shared = getMap()[id];
		if (shared.use_count() <= 1){
			//shared->render.destroyTexture();
			//Log::debug("Remove texture %s", id.c_str());
			getMap().erase(id);
		}
	}else{
		if (Engine::isViewLoaded()){
			Log::debug("Trying to destroy a non existent texture: %s", id.c_str());
		}
	}
}

void TextureDataPool::clear(){
	for (auto& it: getMap()) {
		//if (it.second)
			//it.second->render.destroyTexture();
	}
	//Log::debug("Remove all textures");
	getMap().clear();
}
