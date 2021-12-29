//
// (c) 2020 Eduardo Doria.
//

#include "LuaBinding.h"

#include "Log.h"
#include "System.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "sol.hpp"

#include "Engine.h"
#include "Object.h"
//#include "GraphicObject.h"
//#include "Log.h"
#include "Scene.h"
//#include "Polygon.h"
//#include "Cube.h"
//#include "PlaneTerrain.h"
//#include "Model.h"
//#include "math/Ray.h"
//#include "math/Quaternion.h"
//#include "math/Plane.h"
//#include "Mesh.h"
//#include "Mesh2D.h"
//#include "Image.h"
//#include "ui/UIObject.h"
//#include "ui/UIImage.h"
//#include "Light.h"
//#include "PointLight.h"
//#include "SpotLight.h"
//#include "DirectionalLight.h"
//#include "Sound.h"
//#include "SkyBox.h"
//#include "Points.h"
//#include "Particles.h"
//#include "ui/Text.h"
//#include "Input.h"
//#include "Sprite.h"
//#include "util/Function.h"
//#include "util/FunctionSubscribe.h"
//#include "physics/Contact2D.h"
//#include "action/Action.h"
//#include "action/Ease.h"
//#include "action/TimeAction.h"
//#include "action/MoveAction.h"
//#include "action/RotateAction.h"
//#include "action/ScaleAction.h"
//#include "action/ColorAction.h"
//#include "action/AlphaAction.h"
//#include "action/SpriteAnimation.h"
//#include "action/ParticlesAnimation.h"
//#include "action/particleinit/ParticleInit.h"
//#include "action/particleinit/ParticleAccelerationInit.h"
//#include "action/particleinit/ParticleAlphaInit.h"
//#include "action/particleinit/ParticleColorInit.h"
//#include "action/particleinit/ParticleLifeInit.h"
//#include "action/particleinit/ParticlePositionInit.h"
//#include "action/particleinit/ParticleRotationInit.h"
//#include "action/particleinit/ParticleSizeInit.h"
//#include "action/particleinit/ParticleSpriteInit.h"
//#include "action/particleinit/ParticleVelocityInit.h"
//#include "action/particlemod/ParticleMod.h"
//#include "action/particlemod/ParticleAlphaMod.h"
//#include "action/particlemod/ParticleColorMod.h"
//#include "action/particlemod/ParticlePositionMod.h"
//#include "action/particlemod/ParticleRotationMod.h"
//#include "action/particlemod/ParticleSizeMod.h"
//#include "action/particlemod/ParticleSpriteMod.h"
//#include "action/particlemod/ParticleVelocityMod.h"

#include <map>
#include <locale>
#include <vector>
#include <memory>

using namespace Supernova;



lua_State *LuaBinding::luastate;


LuaBinding::LuaBinding() {

}

LuaBinding::~LuaBinding() {

}


void LuaBinding::createLuaState(){
    LuaBinding::luastate = luaL_newstate();
}

lua_State* LuaBinding::getLuaState(){
    return luastate;
}

void LuaBinding::luaCallback(int nargs, int nresults, int msgh){
    int status = lua_pcall(LuaBinding::getLuaState(), nargs, nresults, msgh);
    if (status != 0){
        Log::Error("Lua Error: %s\n", lua_tostring(LuaBinding::getLuaState(), -1));
    }
}

void LuaBinding::stackDump (lua_State *L) {
    int i = lua_gettop(L);
    Log::Debug(" ----------------  Stack Dump ----------------" );
    while(  i   ) {
        int t = lua_type(L, i);
        switch (t) {
            case LUA_TSTRING:
                Log::Debug("%d:`%s'", i, lua_tostring(L, i));
                break;
            case LUA_TBOOLEAN:
                Log::Debug("%d: %s",i,lua_toboolean(L, i) ? "true" : "false");
                break;
            case LUA_TNUMBER:
                Log::Debug("%d: %g",  i, lua_tonumber(L, i));
                break;
            default: Log::Debug("%d: %s", i, lua_typename(L, t)); break;
        }
        i--;
    }
    Log::Debug("--------------- Stack Dump Finished ---------------" );
}

int LuaBinding::setLuaSearcher(lua_CFunction f, bool cleanSearchers) {

    lua_State *L = LuaBinding::getLuaState();

    // Add the package loader to the package.loaders table.
    lua_getglobal(L, "package");
    if(lua_isnil(L, -1))
        return luaL_error(L, "package table does not exist.");

    lua_getfield(L, -1, "searchers");
    if(lua_isnil(L, -1))
        return luaL_error(L, "package.loaders table does not exist.");

    size_t numloaders = lua_rawlen(L, -1);

    if (cleanSearchers) {
        //remove preconfigured loaders
        for (int i = 0; i < numloaders; i++) {
            lua_pushnil(L);
            lua_rawseti(L, -2, i + 1);
        }
        //add new loader
        lua_pushcfunction(L, f);
        lua_rawseti(L, -2, 1);
    }else{
        lua_pushcfunction(L, f);
        lua_rawseti(L, -2, numloaders+1);
    }

    lua_pop(L, 1);

    return 0;
}

// Note it can be done in the same way with Sol2: https://github.com/ThePhD/sol2/issues/692
int LuaBinding::setLuaPath(const char* path) {
    lua_State *L = LuaBinding::getLuaState();

    lua_getglobal( L, "package" );
    if(lua_isnil(L, -1))
        return luaL_error(L, "package table does not exist.");

    lua_getfield( L, -1, "path" );
    if(lua_isnil(L, -1))
        return luaL_error(L, "package.path table does not exist.");

    std::string cur_path = lua_tostring( L, -1 );
    cur_path.append( ";" );
    cur_path.append( path );
    lua_pop( L, 1 );
    lua_pushstring( L, cur_path.c_str() );
    lua_setfield( L, -2, "path" );
    lua_pop( L, 1 );

    return 0;
}

int LuaBinding::moduleLoader(lua_State *L) {
    
    const char *filename = lua_tostring(L, 1);
    filename = luaL_gsub(L, filename, ".", std::to_string(System::instance().getDirSeparator()).c_str());
    
    std::string filepath;
    Data filedata;
    
    filepath = "lua://" + std::string("lua") + System::instance().getDirSeparator() + filename + ".lua";
    filedata.open(filepath.c_str());
    if (filedata.getMemPtr() != NULL) {
        
        luaL_loadbuffer(L, (const char *) filedata.getMemPtr(), filedata.length(),
                        filepath.c_str());
        
        return 1;
    }
    
    filepath = "lua://" + std::string("") + filename + ".lua";
    filedata.open(filepath.c_str());
    if (filedata.getMemPtr() != NULL) {
        
        luaL_loadbuffer(L, (const char *) filedata.getMemPtr(), filedata.length(),
                        filepath.c_str());
        
        return 1;
    }
    
    lua_pushstring(L, "\n\tno file in assets directory");
    
    return 1;
}

//The same msghandler of lua.c
int LuaBinding::handleLuaError(lua_State* L) {
    const char *msg = lua_tostring(L, 1);
    if (msg == NULL) {  /* is error object not a string? */
    if (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
        return 1;  /* that is the message */
    else
        msg = lua_pushfstring(L, "(error object is a %s value)",
                                luaL_typename(L, 1));
    }
    luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
    return 1;  /* return the traceback */
}

void LuaBinding::bind(){

    lua_State *L = LuaBinding::getLuaState();
    luaL_openlibs(L);

    registerClasses(L);

    std::string luadir = std::string("lua") + System::instance().getDirSeparator();

    setLuaPath(std::string("lua://" + luadir + "?.lua").c_str());
    setLuaSearcher(moduleLoader, true);

    std::string luafile = std::string("lua://") + "main.lua";
    std::string luafile_subdir = std::string("lua://") + luadir + "main.lua";

    Data filedata;

    //First try open on root assets dir
    if (filedata.open(luafile.c_str()) != FileErrors::NO_ERROR){
        //Second try to open on lua dir
        filedata.open(luafile_subdir.c_str());
    }

    lua_pushcfunction(L, handleLuaError);
    int msgh = lua_gettop(L);

    //int luaL_dofile (lua_State *L, const char *filename);
    if (luaL_loadbuffer(L,(const char*)filedata.getMemPtr(),filedata.length(), luafile.c_str()) == 0){
        #ifndef NO_LUA_INIT
        if(lua_pcall(L, 0, LUA_MULTRET, msgh) != 0){
            Log::Error("Lua Error: %s\n", lua_tostring(L,-1));
            lua_close(L);
        }
        #endif
    }else{
        Log::Error("Lua Error: %s\n", lua_tostring(L,-1));
        lua_close(L);
    }

}

void LuaBinding::registerClasses(lua_State *L){

    //sol::state lua;
    sol::state_view lua(L);

    // luaL_openlibs() opened all libraries already: base, string, io, os, package, table, debug
    //lua.open_libraries(sol::lib::base);

    lua.new_enum("Scaling",
                "FITWIDTH", Scaling::FITWIDTH,
                "FITHEIGHT", Scaling::FITHEIGHT,
                "LETTERBOX", Scaling::LETTERBOX,
                "CROP", Scaling::CROP,
                "STRETCH", Scaling::STRETCH)
                ;

    lua.new_usertype<Engine>("Engine",
            sol::default_constructor,
	        "setScene", &Engine::setScene,
            "getCanvasWidth", &Engine::getCanvasWidth,
            "setScalingMode", &Engine::setScalingMode,
            "onViewLoaded", sol::property([] () { return &Engine::onViewLoaded; }, [] (sol::function func) { Engine::onViewLoaded.add("luaFunction", func);}),
            "onUpdate", sol::property([] () { return &Engine::onUpdate; }, [] (sol::function func) { Engine::onUpdate.add("luaFunction", func);})
            );

    // sol::meta_function::call and other metafunctions are automatically generated: https://sol2.readthedocs.io/en/latest/api/usertype.html
    lua.new_usertype<FunctionSubscribe<void()>>("FunctionSubscribe_V",
            sol::default_constructor,
            "call", &FunctionSubscribe<void()>::call,
            "add", (bool (FunctionSubscribe<void()>::*)(const std::string&, sol::function))&FunctionSubscribe<void()>::add
            );

    lua.new_usertype<Scene>("Scene",
	     sol::default_constructor
         );

/*
    LuaIntf::LuaBinding(L).beginClass<Engine>("Engine")
            .addConstructor(LUA_ARGS())
            .addStaticFunction("setScene", &Engine::setScene)
            .addStaticFunction("getCanvasWidth", &Engine::getCanvasWidth)
            .addStaticFunction("getCanvasHeight", &Engine::getCanvasHeight)
            .addStaticFunction("setCanvasSize", &Engine::setCanvasSize)
            .addStaticFunction("setCallMouseInTouchEvent", &Engine::setCallMouseInTouchEvent)
            .addStaticFunction("setCallTouchInMouseEvent", &Engine::setCallTouchInMouseEvent)
            .addStaticFunction("setScalingMode", &Engine::setScalingMode)
            .addStaticFunction("setDefaultNearestScaleTexture", &Engine::setDefaultNearestScaleTexture)
            .addStaticFunction("setDefaultResampleToPOTTexture", &Engine::setDefaultResampleToPOTTexture)
            .addStaticFunction("setUpdateTime", &Engine::setUpdateTime)
            .addStaticFunction("getFramerate", &Engine::getFramerate)
            .addStaticFunction("getDeltatime", &Engine::getDeltatime)
            .addConstant("SCALING_FITWIDTH", Scaling::FITWIDTH)
            .addConstant("SCALING_FITHEIGHT", Scaling::FITHEIGHT)
            .addConstant("SCALING_LETTERBOX", Scaling::LETTERBOX)
            .addConstant("SCALING_CROP", Scaling::CROP)
            .addConstant("SCALING_STRETCH", Scaling::STRETCH)
            .addStaticProperty("onCanvasLoaded", [] () { return &Engine::onCanvasLoaded; }, [] (lua_State* L) { Engine::onCanvasLoaded.add("luaFunction", L); })
            .addStaticProperty("onCanvasChanged", [] () { return &Engine::onCanvasChanged; }, [] (lua_State* L) { Engine::onCanvasChanged.add("luaFunction", L); })
            .addStaticProperty("onDraw", [] () { return &Engine::onDraw; }, [] (lua_State* L) { Engine::onDraw.add("luaFunction", L); })
            .addStaticProperty("onUpdate", [] () { return &Engine::onUpdate; }, [] (lua_State* L) { Engine::onUpdate.add("luaFunction", L); })
            .addStaticProperty("onTouchStart", [] () { return &Engine::onTouchStart; }, [] (lua_State* L) { Engine::onTouchStart.add("luaFunction", L); })
            .addStaticProperty("onTouchEnd", [] () { return &Engine::onTouchEnd; }, [] (lua_State* L) { Engine::onTouchEnd.add("luaFunction", L); })
            .addStaticProperty("onTouchDrag", [] () { return &Engine::onTouchDrag; }, [] (lua_State* L) { Engine::onTouchDrag.add("luaFunction", L); })
            .addStaticProperty("onMouseDown", [] () { return &Engine::onMouseDown; }, [] (lua_State* L) { Engine::onMouseDown.add("luaFunction", L); })
            .addStaticProperty("onMouseUp", [] () { return &Engine::onMouseUp; }, [] (lua_State* L) { Engine::onMouseUp.add("luaFunction", L); })
            .addStaticProperty("onMouseDrag", [] () { return &Engine::onMouseDrag; }, [] (lua_State* L) { Engine::onMouseDrag.add("luaFunction", L); })
            .addStaticProperty("onMouseMove", [] () { return &Engine::onMouseMove; }, [] (lua_State* L) { Engine::onMouseMove.add("luaFunction", L); })
            .addStaticProperty("onKeyDown", [] () { return &Engine::onKeyDown; }, [] (lua_State* L) { Engine::onKeyDown.add("luaFunction", L); })
            .addStaticProperty("onKeyUp", [] () { return &Engine::onKeyUp; }, [] (lua_State* L) { Engine::onKeyUp.add("luaFunction", L); })
            .addStaticProperty("onTextInput", [] () { return &Engine::onTextInput; }, [] (lua_State* L) { Engine::onTextInput.add("luaFunction", L); })
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<Function<float(float)>>("Function_F_F")
            .addFunction("__call", &Function<float(float)>::call)
            .addFunction("call", &Function<float(float)>::call)
            .addFunction("set", (int (Function<float(float)>::*)(lua_State*))&Function<float(float)>::set)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<FunctionSubscribe<void()>>("FunctionSubscribe_V")
            .addFunction("__call", &FunctionSubscribe<void()>::call)
            .addFunction("call", &FunctionSubscribe<void()>::call)
            .addFunction("add", (bool (FunctionSubscribe<void()>::*)(const std::string&, lua_State*))&FunctionSubscribe<void()>::add)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<FunctionSubscribe<void(int)>>("FunctionSubscribe_V_I")
            .addFunction("__call", &FunctionSubscribe<void(int)>::call)
            .addFunction("call", &FunctionSubscribe<void(int)>::call)
            .addFunction("add", (bool (FunctionSubscribe<void(int)>::*)(const std::string&, lua_State*))&FunctionSubscribe<void(int)>::add)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<FunctionSubscribe<void(int,int)>>("FunctionSubscribe_V_II")
            .addFunction("__call", &FunctionSubscribe<void(int,int)>::call)
            .addFunction("call", &FunctionSubscribe<void(int,int)>::call)
            .addFunction("add", (bool (FunctionSubscribe<void(int,int)>::*)(const std::string&, lua_State*))&FunctionSubscribe<void(int,int)>::add)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<FunctionSubscribe<void(float)>>("FunctionSubscribe_V_F")
            .addFunction("__call", &FunctionSubscribe<void(float)>::call)
            .addFunction("call", &FunctionSubscribe<void(float)>::call)
            .addFunction("add", (bool (FunctionSubscribe<void(float)>::*)(const std::string&, lua_State*))&FunctionSubscribe<void(float)>::add)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<FunctionSubscribe<void(float,float)>>("FunctionSubscribe_V_FF")
            .addFunction("__call", &FunctionSubscribe<void(float,float)>::call)
            .addFunction("call", &FunctionSubscribe<void(float,float)>::call)
            .addFunction("add", (bool (FunctionSubscribe<void(float,float)>::*)(const std::string&, lua_State*))&FunctionSubscribe<void(float,float)>::add)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<FunctionSubscribe<void(int,float,float)>>("FunctionSubscribe_V_IFF")
            .addFunction("__call", &FunctionSubscribe<void(int,float,float)>::call)
            .addFunction("call", &FunctionSubscribe<void(int,float,float)>::call)
            .addFunction("add", (bool (FunctionSubscribe<void(int,float,float)>::*)(const std::string&, lua_State*))&FunctionSubscribe<void(int,float,float)>::add)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<FunctionSubscribe<void(Object*)>>("FunctionSubscribe_V_Obj")
            .addFunction("__call", &FunctionSubscribe<void(Object*)>::call)
            .addFunction("call", &FunctionSubscribe<void(Object*)>::call)
            .addFunction("add", (bool (FunctionSubscribe<void(Object*)>::*)(const std::string&, lua_State*))&FunctionSubscribe<void(Object*)>::add)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<FunctionSubscribe<void(std::string)>>("FunctionSubscribe_V_S")
            .addFunction("__call", &FunctionSubscribe<void(std::string)>::call)
            .addFunction("call", &FunctionSubscribe<void(std::string)>::call)
            .addFunction("add", (bool (FunctionSubscribe<void(std::string)>::*)(const std::string&, lua_State*))&FunctionSubscribe<void(std::string)>::add)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<FunctionSubscribe<void(Contact2D*)>>("FunctionSubscribe_V_Contact2D")
            .addFunction("__call", &FunctionSubscribe<void(Contact2D*)>::call)
            .addFunction("call", &FunctionSubscribe<void(Contact2D*)>::call)
            .addFunction("add", (bool (FunctionSubscribe<void(Contact2D*)>::*)(const std::string&, lua_State*))&FunctionSubscribe<void(Contact2D*)>::add)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<Input>("Input")
            .addConstructor(LUA_ARGS())
            .addStaticFunction("isKeyPressed", &Input::isKeyPressed)
            .addStaticFunction("isMousePressed", &Input::isMousePressed)
            .addStaticFunction("isTouch", &Input::isTouch)
            .addStaticFunction("getMousePosition", &Input::getMousePosition)
            .addStaticFunction("getTouchPosition", &Input::getTouchPosition)

            .addConstant("KEY_SPACE", S_KEY_SPACE)
            .addConstant("KEY_APOSTROPHE", S_KEY_APOSTROPHE)
            .addConstant("KEY_COMMA", S_KEY_COMMA)
            .addConstant("KEY_MINUS", S_KEY_MINUS)
            .addConstant("KEY_PERIOD", S_KEY_PERIOD)
            .addConstant("KEY_SLASH", S_KEY_SLASH)
            .addConstant("KEY_0", S_KEY_0)
            .addConstant("KEY_1", S_KEY_1)
            .addConstant("KEY_2", S_KEY_2)
            .addConstant("KEY_3", S_KEY_3)
            .addConstant("KEY_4", S_KEY_4)
            .addConstant("KEY_5", S_KEY_5)
            .addConstant("KEY_6", S_KEY_6)
            .addConstant("KEY_7", S_KEY_7)
            .addConstant("KEY_8", S_KEY_8)
            .addConstant("KEY_9", S_KEY_9)
            .addConstant("KEY_SEMICOLON", S_KEY_SEMICOLON)
            .addConstant("KEY_EQUAL", S_KEY_EQUAL)
            .addConstant("KEY_A", S_KEY_A)
            .addConstant("KEY_B", S_KEY_B)
            .addConstant("KEY_C", S_KEY_C)
            .addConstant("KEY_D", S_KEY_D)
            .addConstant("KEY_E", S_KEY_E)
            .addConstant("KEY_F", S_KEY_F)
            .addConstant("KEY_G", S_KEY_G)
            .addConstant("KEY_H", S_KEY_H)
            .addConstant("KEY_I", S_KEY_I)
            .addConstant("KEY_J", S_KEY_J)
            .addConstant("KEY_K", S_KEY_K)
            .addConstant("KEY_L", S_KEY_L)
            .addConstant("KEY_M", S_KEY_M)
            .addConstant("KEY_N", S_KEY_N)
            .addConstant("KEY_O", S_KEY_O)
            .addConstant("KEY_P", S_KEY_P)
            .addConstant("KEY_Q", S_KEY_Q)
            .addConstant("KEY_R", S_KEY_R)
            .addConstant("KEY_S", S_KEY_S)
            .addConstant("KEY_T", S_KEY_T)
            .addConstant("KEY_U", S_KEY_U)
            .addConstant("KEY_V", S_KEY_V)
            .addConstant("KEY_W", S_KEY_W)
            .addConstant("KEY_X", S_KEY_X)
            .addConstant("KEY_Y", S_KEY_Y)
            .addConstant("KEY_Z", S_KEY_Z)
            .addConstant("KEY_LEFT_BRACKET", S_KEY_LEFT_BRACKET)
            .addConstant("KEY_BACKSLASH", S_KEY_BACKSLASH)
            .addConstant("KEY_RIGHT_BRACKET", S_KEY_RIGHT_BRACKET)
            .addConstant("KEY_GRAVE_ACCENT", S_KEY_GRAVE_ACCENT)
            .addConstant("KEY_ESCAPE", S_KEY_ESCAPE)
            .addConstant("KEY_ENTER", S_KEY_ENTER)
            .addConstant("KEY_TAB", S_KEY_TAB)
            .addConstant("KEY_BACKSPACE", S_KEY_BACKSPACE)
            .addConstant("KEY_INSERT", S_KEY_INSERT)
            .addConstant("KEY_DELETE", S_KEY_DELETE)
            .addConstant("KEY_RIGHT", S_KEY_RIGHT)
            .addConstant("KEY_LEFT", S_KEY_LEFT)
            .addConstant("KEY_DOWN", S_KEY_DOWN)
            .addConstant("KEY_UP", S_KEY_UP)
            .addConstant("KEY_PAGE_UP", S_KEY_PAGE_UP)
            .addConstant("KEY_PAGE_DOWN", S_KEY_PAGE_DOWN)
            .addConstant("KEY_HOME", S_KEY_HOME)
            .addConstant("KEY_END", S_KEY_END)
            .addConstant("KEY_CAPS_LOCK", S_KEY_CAPS_LOCK)
            .addConstant("KEY_SCROLL_LOCK", S_KEY_SCROLL_LOCK)
            .addConstant("KEY_NUM_LOCK", S_KEY_NUM_LOCK)
            .addConstant("KEY_PRINT_SCREEN", S_KEY_PRINT_SCREEN)
            .addConstant("KEY_PAUSE", S_KEY_PAUSE)
            .addConstant("KEY_F1", S_KEY_F1)
            .addConstant("KEY_F2", S_KEY_F2)
            .addConstant("KEY_F3", S_KEY_F3)
            .addConstant("KEY_F4", S_KEY_F4)
            .addConstant("KEY_F5", S_KEY_F5)
            .addConstant("KEY_F6", S_KEY_F6)
            .addConstant("KEY_F7", S_KEY_F7)
            .addConstant("KEY_F8", S_KEY_F8)
            .addConstant("KEY_F9", S_KEY_F9)
            .addConstant("KEY_F10", S_KEY_F10)
            .addConstant("KEY_F11", S_KEY_F11)
            .addConstant("KEY_F12", S_KEY_F12)
            .addConstant("KEY_KP_0", S_KEY_KP_0)
            .addConstant("KEY_KP_1", S_KEY_KP_1)
            .addConstant("KEY_KP_2", S_KEY_KP_2)
            .addConstant("KEY_KP_3", S_KEY_KP_3)
            .addConstant("KEY_KP_4", S_KEY_KP_4)
            .addConstant("KEY_KP_5", S_KEY_KP_5)
            .addConstant("KEY_KP_6", S_KEY_KP_6)
            .addConstant("KEY_KP_7", S_KEY_KP_7)
            .addConstant("KEY_KP_8", S_KEY_KP_8)
            .addConstant("KEY_KP_9", S_KEY_KP_9)
            .addConstant("KEY_KP_DECIMAL", S_KEY_KP_DECIMAL)
            .addConstant("KEY_KP_DIVIDE", S_KEY_KP_DIVIDE)
            .addConstant("KEY_KP_MULTIPLY", S_KEY_KP_MULTIPLY)
            .addConstant("KEY_KP_SUBTRACT", S_KEY_KP_SUBTRACT)
            .addConstant("KEY_KP_ADD", S_KEY_KP_ADD)
            .addConstant("KEY_KP_ENTER", S_KEY_KP_ENTER)
            .addConstant("KEY_KP_EQUAL", S_KEY_KP_EQUAL)
            .addConstant("KEY_LEFT_SHIFT", S_KEY_LEFT_SHIFT)
            .addConstant("KEY_LEFT_CONTROL", S_KEY_LEFT_CONTROL)
            .addConstant("KEY_LEFT_ALT", S_KEY_LEFT_ALT)
            .addConstant("KEY_LEFT_SUPER", S_KEY_LEFT_SUPER)
            .addConstant("KEY_RIGHT_SHIFT", S_KEY_RIGHT_SHIFT)
            .addConstant("KEY_RIGHT_CONTROL", S_KEY_RIGHT_CONTROL)
            .addConstant("KEY_RIGHT_ALT", S_KEY_RIGHT_ALT)
            .addConstant("KEY_RIGHT_SUPER", S_KEY_RIGHT_SUPER)
            .addConstant("KEY_MENU", S_KEY_MENU)

            .addConstant("MOUSE_BUTTON_1", S_MOUSE_BUTTON_1)
            .addConstant("MOUSE_BUTTON_2", S_MOUSE_BUTTON_2)
            .addConstant("MOUSE_BUTTON_3", S_MOUSE_BUTTON_3)
            .addConstant("MOUSE_BUTTON_4", S_MOUSE_BUTTON_4)
            .addConstant("MOUSE_BUTTON_5", S_MOUSE_BUTTON_5)
            .addConstant("MOUSE_BUTTON_6", S_MOUSE_BUTTON_6)
            .addConstant("MOUSE_BUTTON_7", S_MOUSE_BUTTON_7)
            .addConstant("MOUSE_BUTTON_8", S_MOUSE_BUTTON_8)
            .addConstant("MOUSE_BUTTON_LAST", S_MOUSE_BUTTON_LAST)
            .addConstant("MOUSE_BUTTON_LEFT", S_MOUSE_BUTTON_LEFT)
            .addConstant("MOUSE_BUTTON_RIGHT", S_MOUSE_BUTTON_RIGHT)
            .addConstant("MOUSE_BUTTON_MIDDLE", S_MOUSE_BUTTON_MIDDLE)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<Object>("Object")
            .addFunction("addObject", &Object::addObject)
            .addFunction("removeObject", &Object::removeObject)
            .addFunction("addAction", &Object::addAction)
            .addFunction("removeAction", &Object::removeAction)
            .addFunction("setPosition", (void (Object::*)(const float, const float, const float))&Object::setPosition)
            .addFunction("setPosition2D", (void (Object::*)(const float, const float))&Object::setPosition)
            .addFunction("getPosition", &Object::getPosition)
            .addProperty("position", &Object::getPosition, (void (Object::*)(Vector3))&Object::setPosition)
            .addFunction("setRotation", (void (Object::*)(const float, const float, const float))&Object::setRotation)
            .addFunction("getRotation", &Object::getRotation)
            .addProperty("rotation", &Object::getRotation, (void (Object::*)(Quaternion))&Object::setRotation)
            .addFunction("setScale", (void (Object::*)(const float))&Object::setScale)
            .addFunction("getScale", &Object::getScale)
            .addProperty("scale", &Object::getScale, (void (Object::*)(Vector3))&Object::setScale)
            .addFunction("setCenter", (void (Object::*)(const float, const float, const float))&Object::setCenter)
            .addFunction("getCenter", &Object::getCenter)
            .addFunction("find", &Object::find)
            .addFunction("moveTo", &Object::moveTo)
            .addFunction("moveToFirst", &Object::moveToFirst)
            .addFunction("moveToLast", &Object::moveToLast)
            .addFunction("moveUp", &Object::moveUp)
            .addFunction("moveDown", &Object::moveDown)
            .addProperty("center", &Object::getCenter, (void (Object::*)(Vector3))&Object::setCenter)
            .addFunction("destroy", &Object::destroy)
            .endClass()

            .beginExtendClass<Scene, Object>("Scene")
            .addConstructor(LUA_ARGS())
            .addFunction("setCamera", &Scene::setCamera)
            .addFunction("setAmbientLight", (void (Scene::*)(const float))&Scene::setAmbientLight)
            .addProperty("ambientLight", &Scene::getAmbientLight, (void (Scene::*)(Vector3))&Scene::setAmbientLight)
            .endClass()

            .beginExtendClass<Camera, Object>("Camera")
            .addConstructor(LUA_ARGS())
            .addFunction("setView", (void (Camera::*)(const float, const float, const float))&Camera::setView)
            .addFunction("getView", &Camera::getView)
            .addProperty("view", &Camera::getView, (void (Camera::*)(Vector3))&Camera::setView)
            .addFunction("setUp", (void (Camera::*)(const float, const float, const float))&Camera::setUp)
            .addFunction("getUp", &Camera::getUp)
            .addProperty("up", &Camera::getUp, (void (Camera::*)(Vector3))&Camera::setUp)
            .addFunction("setType", &Camera::setType)
            .addFunction("setPerspective", &Camera::setPerspective)
            .addFunction("setOrtho", &Camera::setOrtho)
            .addFunction("pointsToRay", &Camera::pointsToRay)
            .addFunction("rotateView", &Camera::rotateView)
            .addFunction("rotatePosition", &Camera::rotatePosition)
            .addFunction("elevateView", &Camera::elevateView)
            .addFunction("elevatePosition", &Camera::elevatePosition)
            .addFunction("moveForward", &Camera::moveForward)
            .addFunction("walkForward", &Camera::walkForward)
            .addFunction("slide", &Camera::slide)
            .addConstant("CAMERA_2D", S_CAMERA_2D)
            .addConstant("CAMERA_ORTHO", S_CAMERA_ORTHO)
            .addConstant("CAMERA_PERSPECTIVE", S_CAMERA_PERSPECTIVE)
            .endClass()

            .beginExtendClass<Light, Object>("Light")
            .addConstructor(LUA_ARGS())
            .addFunction("getColor", &Light::getColor)
            .addFunction("getTarget", &Light::getTarget)
            .addFunction("getDirection", &Light::getDirection)
            .addFunction("getPower", &Light::getPower)
            .addFunction("getSpotAngle", &Light::getSpotAngle)
            .addFunction("setPower", &Light::setPower)
            .addFunction("setShadow", &Light::setShadow)
            .endClass()

            .beginExtendClass<PointLight, Light>("PointLight")
            .addConstructor(LUA_ARGS())
            .endClass()

            .beginExtendClass<SpotLight, Light>("SpotLight")
            .addConstructor(LUA_ARGS())
            .addFunction("setTarget", (void (SpotLight::*)(const float, const float, const float))&SpotLight::setTarget)
            .addProperty("target", &SpotLight::getTarget, (void (SpotLight::*)(Vector3))&SpotLight::setTarget)
            .addFunction("setSpotAngle", &SpotLight::setSpotAngle)
            .endClass()

            .beginExtendClass<DirectionalLight, Light>("DirectionalLight")
            .addConstructor(LUA_ARGS())
            .addFunction("setDirection", (void (DirectionalLight::*)(const float, const float, const float))&DirectionalLight::setDirection)
            .addProperty("target", &DirectionalLight::getDirection, (void (DirectionalLight::*)(Vector3))&DirectionalLight::setDirection)
            .endClass()

            .beginExtendClass<GraphicObject, Object>("GraphicObject")
            .addFunction("setColor", (void (GraphicObject::*)(float, float, float, float))&GraphicObject::setColor)
            .addFunction("setColorVector", (void (GraphicObject::*)(Vector4))&GraphicObject::setColor)
            .addFunction("setTexture", (void (GraphicObject::*)(std::string))&GraphicObject::setTexture)
            .endClass()

            .beginExtendClass<Mesh, GraphicObject>("Mesh")
            .addConstructor(LUA_ARGS())
            .endClass()

            .beginExtendClass<Points, GraphicObject>("Points")
            .addConstructor(LUA_ARGS())
            .addFunction("setSizeAttenuation", &Points::setSizeAttenuation)
            .addFunction("setPointScaleFactor", &Points::setPointScaleFactor)
            .addFunction("setMinPointSize", &Points::setMinPointSize)
            .addFunction("setMaxPointSize", &Points::setMaxPointSize)
            .endClass()

            .beginExtendClass<Particles, Points>("Particles")
            .addConstructor(LUA_ARGS())
            .addFunction("setRate", (void (Particles::*)(int))&Particles::setRate)
            .addFunction("setRateMinMax", (void (Particles::*)(int, int))&Particles::setRate)
            .addFunction("setMaxParticles", &Particles::setMaxParticles)
            .endClass()

            .beginExtendClass<SkyBox, Mesh>("SkyBox")
            .addConstructor(LUA_ARGS())
            .addFunction("setTextureFront", &SkyBox::setTextureFront)
            .addFunction("setTextureBack", &SkyBox::setTextureBack)
            .addFunction("setTextureLeft", &SkyBox::setTextureLeft)
            .addFunction("setTextureRight", &SkyBox::setTextureRight)
            .addFunction("setTextureUp", &SkyBox::setTextureUp)
            .addFunction("setTextureDown", &SkyBox::setTextureDown)
            .endClass()

            .beginExtendClass<Mesh2D, Mesh>("Mesh2D")
            .addFunction("setSize", &Mesh2D::setSize)
            .addFunction("setBillboard", &Mesh2D::setBillboard)
            .addFunction("setFixedSizeBillboard", &Mesh2D::setFixedSizeBillboard)
            .addFunction("setBillboardScaleFactor", &Mesh2D::setBillboardScaleFactor)
            .endClass()

            .beginExtendClass<Text, Mesh2D>("Text")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<const char *>))
            .addFunction("getAscent", &Text::getAscent)
            .addFunction("getDescent", &Text::getDescent)
            .addFunction("getLineGap", &Text::getLineGap)
            .addFunction("getLineHeight", &Text::getLineHeight)
            .addFunction("setWidth", &Text::setWidth)
            .addFunction("setHeight", &Text::setHeight)
            .addFunction("setText", &Text::setText)
            .addFunction("setFont", &Text::setFont)
            .addFunction("setFontSize", &Text::setFontSize)
            .addFunction("setMultiline", &Text::setMultiline)
            .endClass()

            .beginExtendClass<UIImage, Mesh2D>("UIImage")
            .addConstructor(LUA_ARGS())
            .endClass()

            .beginExtendClass<Image, Mesh2D>("Image")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<const char *>))
            .addFunction("setTextureRect", (void (Image::*)(float, float, float, float))&Image::setTextureRect)
            .addFunction("setSize", &Image::setSize)
            .addFunction("setInvertTexture", &Image::setInvertTexture)
            .endClass()

            .beginExtendClass<Sprite, Image>("Sprite")
            .addConstructor(LUA_ARGS())
            .addFunction("addFrame", (void (Sprite::*)(float, float, float, float))&Sprite::addFrame)
            .addFunction("addFrameString", (void (Sprite::*)(std::string, float, float, float, float))&Sprite::addFrame)
            .addFunction("removeFrame", (void (Sprite::*)(int))&Sprite::removeFrame)
            .addFunction("removeFrameString", (void (Sprite::*)(std::string))&Sprite::removeFrame)
            .addFunction("setFrame", (void (Sprite::*)(int))&Sprite::setFrame)
            .addFunction("setFrameString", (void (Sprite::*)(std::string))&Sprite::setFrame)
            .addFunction("findFramesByString", &Sprite::findFramesByString)
            .addFunction("isAnimation", &Sprite::isAnimation)
            .addFunction("runAnimation", (void (Sprite::*)(std::vector<int>, std::vector<int>, bool))&Sprite::runAnimation)
            .addFunction("stopAnimation", &Sprite::stopAnimation)
            .endClass()

            .beginExtendClass<Polygon, Mesh2D>("Polygon")
            .addConstructor(LUA_ARGS())
            .addFunction("addVertex", (void (Polygon::*)(float, float))&Polygon::addVertex)
                    //.addFunction("addVertex", LUA_FN(void, Polygon::addVertex, float))
            .endClass()

            .beginExtendClass<Cube, Mesh>("Cube")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .endClass()

            .beginExtendClass<PlaneTerrain, Mesh>("PlaneTerrain")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .endClass()

            .beginExtendClass<Model, Mesh>("Model")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<const char *>))
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<Vector2>("Vector2")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .addVariable("x", &Vector2::x)
            .addVariable("y", &Vector2::y)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<Vector3>("Vector3")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .addVariable("x", &Vector3::x)
            .addVariable("y", &Vector3::y)
            .addVariable("z", &Vector3::z)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<Vector4>("Vector4")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .addVariable("x", &Vector4::x)
            .addVariable("y", &Vector4::y)
            .addVariable("z", &Vector4::z)
            .addVariable("w", &Vector4::w)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<Quaternion>("Quaternion")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .addVariable("w", &Quaternion::w)
            .addVariable("x", &Quaternion::x)
            .addVariable("y", &Quaternion::y)
            .addVariable("z", &Quaternion::z)
            .addFunction("fromAxes", (void (Quaternion::*)(const Vector3&, const Vector3&, const Vector3&))&Quaternion::fromAxes)
            .addFunction("fromAngle", (void (Quaternion::*)(const float))&Quaternion::fromAngle)
            .addFunction("fromAngleAxis", (void (Quaternion::*)(const float, const Vector3&))&Quaternion::fromAngleAxis)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<Plane>("Plane")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<Vector3>, LuaIntf::_opt<Vector3>))
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<Ray>("Ray")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<Vector3>, LuaIntf::_opt<Vector3>))
            .addFunction("intersectionPointPlane", (Vector3 (Ray::*)(Plane))&Ray::intersectionPoint)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<Sound>("Sound")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<const char *>))
            .addFunction("load", &Sound::load)
            .addFunction("play", &Sound::play)
            .addFunction("stop", &Sound::stop)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<Action>("Action")
            .addConstructor(LUA_ARGS())
            .addFunction("run", &Action::run)
            .addFunction("pause", &Action::pause)
            .addFunction("stop", &Action::stop)
            .addFunction("isRunning", &Action::isRunning)
            .addFunction("getObject", &Action::getObject)
            .addProperty("onStart", [] (Action* action) { return &action->onStart; }, [] (Action* action, lua_State* L) { action->onStart.add("luaFunction", L); })
            .addProperty("onRun", [] (Action* action) { return &action->onRun; }, [] (Action* action, lua_State* L) { action->onRun.add("luaFunction", L); })
            .addProperty("onPause", [] (Action* action) { return &action->onPause; }, [] (Action* action, lua_State* L) { action->onPause.add("luaFunction", L); })
            .addProperty("onStop", [] (Action* action) { return &action->onStop; }, [] (Action* action, lua_State* L) { action->onStop.add("luaFunction", L); })
            .addProperty("onFinish", [] (Action* action) { return &action->onFinish; }, [] (Action* action, lua_State* L) { action->onFinish.add("luaFunction", L); })
            .addProperty("onUpdate", [] (Action* action) { return &action->onUpdate; }, [] (Action* action, lua_State* L) { action->onUpdate.add("luaFunction", L); })
            .addConstant("LINEAR", S_LINEAR)
            .addConstant("EASE_QUAD_IN", S_EASE_QUAD_IN)
            .addConstant("EASE_QUAD_OUT", S_EASE_QUAD_OUT)
            .addConstant("EASE_QUAD_IN_OUT", S_EASE_QUAD_IN_OUT)
            .addConstant("EASE_CUBIC_IN", S_EASE_CUBIC_IN)
            .addConstant("EASE_CUBIC_OUT", S_EASE_CUBIC_OUT)
            .addConstant("EASE_CUBIC_IN_OUT", S_EASE_CUBIC_IN_OUT)
            .addConstant("EASE_QUART_IN", S_EASE_QUART_IN)
            .addConstant("EASE_QUART_OUT", S_EASE_QUART_OUT)
            .addConstant("EASE_QUART_IN_OUT", S_EASE_QUART_IN_OUT)
            .addConstant("EASE_QUINT_IN", S_EASE_QUINT_IN)
            .addConstant("EASE_QUINT_OUT", S_EASE_QUINT_OUT)
            .addConstant("EASE_QUINT_IN_OUT", S_EASE_QUINT_IN_OUT)
            .addConstant("EASE_SINE_IN", S_EASE_SINE_IN)
            .addConstant("EASE_SINE_OUT", S_EASE_SINE_OUT)
            .addConstant("EASE_SINE_IN_OUT", S_EASE_SINE_IN_OUT)
            .addConstant("EASE_EXPO_IN", S_EASE_EXPO_IN)
            .addConstant("EASE_EXPO_OUT", S_EASE_EXPO_OUT)
            .addConstant("EASE_EXPO_IN_OUT", S_EASE_EXPO_IN_OUT)
            .addConstant("EASE_CIRC_IN", S_EASE_CIRC_IN)
            .addConstant("EASE_CIRC_OUT", S_EASE_CIRC_OUT)
            .addConstant("EASE_CIRC_IN_OUT", S_EASE_CIRC_IN_OUT)
            .addConstant("EASE_ELASTIC_IN", S_EASE_ELASTIC_IN)
            .addConstant("EASE_ELASTIC_OUT", S_EASE_ELASTIC_OUT)
            .addConstant("EASE_ELASTIC_IN_OUT", S_EASE_ELASTIC_IN_OUT)
            .addConstant("EASE_BACK_IN", S_EASE_BACK_IN)
            .addConstant("EASE_BACK_OUT", S_EASE_BACK_OUT)
            .addConstant("EASE_BACK_IN_OUT", S_EASE_BACK_IN_OUT)
            .addConstant("EASE_BOUNCE_IN", S_EASE_BOUNCE_IN)
            .addConstant("EASE_BOUNCE_OUT", S_EASE_BOUNCE_OUT)
            .addConstant("EASE_BOUNCE_IN_OUT", S_EASE_BOUNCE_IN_OUT)
            .endClass()

            .beginExtendClass<TimeAction, Action>("TimeAction")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<bool>))
            .addFunction("setFunction", (int (TimeAction::*)(lua_State*))&TimeAction::setFunction)
            .addFunction("setFunctionType", &TimeAction::setFunctionType)
            .addFunction("getDuration", &TimeAction::getDuration)
            .addFunction("setDuration", &TimeAction::setDuration)
            .addFunction("isLoop", &TimeAction::isLoop)
            .addFunction("setLoop", &TimeAction::setLoop)
            .addFunction("getTime", &TimeAction::getTime)
            .addFunction("getValue", &TimeAction::getValue)
            .endClass()

            .beginExtendClass<MoveAction, TimeAction>("MoveAction")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<Vector3>, LuaIntf::_opt<Vector3>, LuaIntf::_opt<float>, LuaIntf::_opt<bool>))
            .endClass()

            .beginExtendClass<RotateAction, TimeAction>("RotateAction")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<Quaternion>, LuaIntf::_opt<Quaternion>, LuaIntf::_opt<float>, LuaIntf::_opt<bool>))
            .endClass()

            .beginExtendClass<ScaleAction, TimeAction>("ScaleAction")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<Vector3>, LuaIntf::_opt<Vector3>, LuaIntf::_opt<float>, LuaIntf::_opt<bool>))
            .endClass()

            .beginExtendClass<ColorAction, TimeAction>("ColorAction")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<bool>))
            .endClass()

            .beginExtendClass<AlphaAction, TimeAction>("AlphaAction")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<bool>))
            .endClass()

            .beginExtendClass<SpriteAnimation, Action>("SpriteAnimation")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<std::vector<int>>, LuaIntf::_opt<std::vector<int>>, LuaIntf::_opt<bool>))
            .endClass()

            .beginExtendClass<ParticlesAnimation, Action>("ParticlesAnimation")
            .addConstructor(LUA_ARGS())
            .addFunction("addInit", &ParticlesAnimation::addInit)
            .addFunction("addMod", &ParticlesAnimation::addMod)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<ParticleInit>("ParticleInit")
            .endClass()

            .beginExtendClass<ParticleAccelerationInit, ParticleInit>("ParticleAccelerationInit")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<Vector3>, LuaIntf::_opt<Vector3>))
            .addFunction("setAcceleration", (void (ParticleAccelerationInit::*)(Vector3))&ParticleAccelerationInit::setAcceleration)
            .addFunction("setAccelerationMinMax", (void (ParticleAccelerationInit::*)(Vector3, Vector3))&ParticleAccelerationInit::setAcceleration)
            .addFunction("getMinAcceleration", &ParticleAccelerationInit::getMinAcceleration)
            .addFunction("getMaxAcceleration", &ParticleAccelerationInit::getMaxAcceleration)
            .endClass()

            .beginExtendClass<ParticleAlphaInit, ParticleInit>("ParticleAlphaInit")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .addFunction("setAlpha", (void (ParticleAlphaInit::*)(float))&ParticleAlphaInit::setAlpha)
            .addFunction("setAlphaMinMax", (void (ParticleAlphaInit::*)(float, float))&ParticleAlphaInit::setAlpha)
            .addFunction("getMinAlpha", &ParticleAlphaInit::getMinAlpha)
            .addFunction("getMaxAlpha", &ParticleAlphaInit::getMaxAlpha)
            .endClass()

            .beginExtendClass<ParticleColorInit, ParticleInit>("ParticleColorInit")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .addFunction("setColor", (void (ParticleColorInit::*)(float, float, float))&ParticleColorInit::setColor)
            .addFunction("setColorMinMax", (void (ParticleColorInit::*)(float, float, float, float, float, float))&ParticleColorInit::setColor)
            .addFunction("getMinColor", &ParticleColorInit::getMinColor)
            .addFunction("getMaxColor", &ParticleColorInit::getMaxColor)
            .endClass()

            .beginExtendClass<ParticleLifeInit, ParticleInit>("ParticleLifeInit")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .addFunction("setLife", (void (ParticleLifeInit::*)(float))&ParticleLifeInit::setLife)
            .addFunction("setLifeMinMax", (void (ParticleLifeInit::*)(float, float))&ParticleLifeInit::setLife)
            .addFunction("getMinLife", &ParticleLifeInit::getMinLife)
            .addFunction("getMaxLife", &ParticleLifeInit::getMaxLife)
            .endClass()

            .beginExtendClass<ParticlePositionInit, ParticleInit>("ParticlePositionInit")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<Vector3>, LuaIntf::_opt<Vector3>))
            .addFunction("setPosition", (void (ParticlePositionInit::*)(Vector3))&ParticlePositionInit::setPosition)
            .addFunction("setPositionMinMax", (void (ParticlePositionInit::*)(Vector3, Vector3))&ParticlePositionInit::setPosition)
            .addFunction("getMinPosition", &ParticlePositionInit::getMinPosition)
            .addFunction("getMaxPosition", &ParticlePositionInit::getMaxPosition)
            .endClass()

            .beginExtendClass<ParticleRotationInit, ParticleInit>("ParticleRotationInit")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .addFunction("setRotation", (void (ParticleRotationInit::*)(float))&ParticleRotationInit::setRotation)
            .addFunction("setRotationMinMax", (void (ParticleRotationInit::*)(float, float))&ParticleRotationInit::setRotation)
            .addFunction("getMinRotation", &ParticleRotationInit::getMinRotation)
            .addFunction("getMaxRotation", &ParticleRotationInit::getMaxRotation)
            .endClass()

            .beginExtendClass<ParticleSizeInit, ParticleInit>("ParticleSizeInit")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .addFunction("setSize", (void (ParticleSizeInit::*)(float))&ParticleSizeInit::setSize)
            .addFunction("setSizeMinMax", (void (ParticleSizeInit::*)(float, float))&ParticleSizeInit::setSize)
            .addFunction("getMinSize", &ParticleSizeInit::getMinSize)
            .addFunction("getMaxSize", &ParticleSizeInit::getMaxSize)
            .endClass()

            .beginExtendClass<ParticleSpriteInit, ParticleInit>("ParticleSpriteInit")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<std::vector<int>>))
            .addFunction("setFrames", (void (ParticleSpriteInit::*)(std::vector<int>))&ParticleSpriteInit::setFrames)
            .addFunction("setFramesMinMax", (void (ParticleSpriteInit::*)(int, int))&ParticleSpriteInit::setFrames)
            .addFunction("getFrames", &ParticleSpriteInit::getFrames)
            .endClass()

            .beginExtendClass<ParticleVelocityInit, ParticleInit>("ParticleVelocityInit")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<Vector3>, LuaIntf::_opt<Vector3>))
            .addFunction("setVelocity", (void (ParticleVelocityInit::*)(Vector3))&ParticleVelocityInit::setVelocity)
            .addFunction("setVelocityMinMax", (void (ParticleVelocityInit::*)(Vector3, Vector3))&ParticleVelocityInit::setVelocity)
            .addFunction("getMinVelocity", &ParticleVelocityInit::getMinVelocity)
            .addFunction("getMaxVelocity", &ParticleVelocityInit::getMaxVelocity)
            .endClass();

    LuaIntf::LuaBinding(L).beginClass<ParticleMod>("ParticleMod")
            .endClass()

            .beginExtendClass<ParticleAlphaMod, ParticleMod>("ParticleAlphaMod")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .addFunction("setAlpha", (void (ParticleAlphaMod::*)(float, float))&ParticleAlphaMod::setAlpha)
            .addFunction("getFromAlpha", &ParticleAlphaMod::getFromAlpha)
            .addFunction("getToAlpha", &ParticleAlphaMod::getToAlpha)
            .endClass()

            .beginExtendClass<ParticleColorMod, ParticleMod>("ParticleColorMod")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .addFunction("setColor", (void (ParticleColorMod::*)(float, float, float, float, float, float))&ParticleColorMod::setColor)
            .addFunction("getFromColor", &ParticleColorMod::getFromColor)
            .addFunction("getToColor", &ParticleColorMod::getToColor)
            .endClass()

            .beginExtendClass<ParticlePositionMod, ParticleMod>("ParticlePositionMod")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<Vector3>, LuaIntf::_opt<Vector3>))
            .addFunction("setPosition", (void (ParticlePositionMod::*)(Vector3, Vector3))&ParticlePositionMod::setPosition)
            .addFunction("getFromPosition", &ParticlePositionMod::getFromPosition)
            .addFunction("getToPosition", &ParticlePositionMod::getToPosition)
            .endClass()

            .beginExtendClass<ParticleRotationMod, ParticleMod>("ParticleRotationMod")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .addFunction("setRotation", (void (ParticleRotationMod::*)(float, float))&ParticleRotationMod::setRotation)
            .addFunction("getFromRotation", &ParticleRotationMod::getFromRotation)
            .addFunction("getToRotation", &ParticleRotationMod::getToRotation)
            .endClass()

            .beginExtendClass<ParticleSizeMod, ParticleMod>("ParticleSizeMod")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<float>))
            .addFunction("setSize", (void (ParticleSizeMod::*)(float, float))&ParticleSizeMod::setSize)
            .addFunction("getFromSize", &ParticleSizeMod::getFromSize)
            .addFunction("getToSize", &ParticleSizeMod::getToSize)
            .endClass()

            .beginExtendClass<ParticleSpriteMod, ParticleMod>("ParticleSpriteMod")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<std::vector<int>>))
            .addFunction("setSize", (void (ParticleSpriteMod::*)(std::vector<int>))&ParticleSpriteMod::setFrames)
            .addFunction("getFrames", &ParticleSpriteMod::getFrames)
            .endClass()

            .beginExtendClass<ParticleVelocityMod, ParticleMod>("ParticleVelocityMod")
            .addConstructor(LUA_ARGS(LuaIntf::_opt<float>, LuaIntf::_opt<float>, LuaIntf::_opt<Vector3>, LuaIntf::_opt<Vector3>))
            .addFunction("setVelocity", (void (ParticleVelocityMod::*)(Vector3, Vector3))&ParticleVelocityMod::setVelocity)
            .addFunction("getFromVelocity", &ParticleVelocityMod::getFromVelocity)
            .addFunction("getToVelocity", &ParticleVelocityMod::getToVelocity)
            .endClass();
*/
}
