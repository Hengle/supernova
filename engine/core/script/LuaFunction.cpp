#include "LuaFunction.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "LuaIntf/LuaIntf.h"

#include "script/LuaBinding.h"
#include "Log.h"
#include "Object.h"
//#include "physics/Contact2D.h"

//
// (c) 2018 Eduardo Doria.
//

using namespace Supernova;

LuaFunction::LuaFunction(){
    this->function = 0;
}

LuaFunction::LuaFunction(const LuaFunction& t){
    this->function = t.function;
}

LuaFunction& LuaFunction::operator = (const LuaFunction& t){
    this->function = t.function;

    return *this;
}

bool LuaFunction::operator == ( const LuaFunction& t ) const{
    return ( function == t.function );
}

LuaFunction::operator bool() const
{
    if (function > 0)
        return true;

    return false;
}

int LuaFunction::set(lua_State *L){
    if (lua_type(L, lua_gettop(L)) == LUA_TFUNCTION){
        // The function to store is on the top of stack L
        function = luaL_ref(L, LUA_REGISTRYINDEX);
    }else{
        return luaL_error(L, "This is not a valid function");
    }
    return 0;
}

void LuaFunction::remove(){
    function = 0;
}

template<>
void LuaFunction::call(){
    if (function != 0){
        lua_rawgeti(LuaBinding::getLuaState(), LUA_REGISTRYINDEX, function);
        LuaBinding::luaCallback(0, 0, 0);
    }
};

template<>
void LuaFunction::call(int p1){
    if (function != 0){
        lua_rawgeti(LuaBinding::getLuaState(), LUA_REGISTRYINDEX, function);
        lua_pushnumber(LuaBinding::getLuaState(), p1);
        LuaBinding::luaCallback(1, 0, 0);
    }
};

template<>
void LuaFunction::call(int p1, int p2){
    if (function != 0){
        lua_rawgeti(LuaBinding::getLuaState(), LUA_REGISTRYINDEX, function);
        lua_pushnumber(LuaBinding::getLuaState(), p1);
        lua_pushnumber(LuaBinding::getLuaState(), p2);
        LuaBinding::luaCallback(2, 0, 0);
    }
};

template<>
void LuaFunction::call(float p1){
    if (function != 0){
        lua_rawgeti(LuaBinding::getLuaState(), LUA_REGISTRYINDEX, function);
        lua_pushnumber(LuaBinding::getLuaState(), p1);
        LuaBinding::luaCallback(1, 0, 0);
    }
};

template<>
void LuaFunction::call(float p1, float p2){
    if (function != 0){
        lua_rawgeti(LuaBinding::getLuaState(), LUA_REGISTRYINDEX, function);
        lua_pushnumber(LuaBinding::getLuaState(), p1);
        lua_pushnumber(LuaBinding::getLuaState(), p2);
        LuaBinding::luaCallback(2, 0, 0);
    }
};

template<>
void LuaFunction::call(int p1, float p2, float p3){
    if (function != 0){
        lua_rawgeti(LuaBinding::getLuaState(), LUA_REGISTRYINDEX, function);
        lua_pushnumber(LuaBinding::getLuaState(), p1);
        lua_pushnumber(LuaBinding::getLuaState(), p2);
        lua_pushnumber(LuaBinding::getLuaState(), p3);
        LuaBinding::luaCallback(3, 0, 0);
    }
};

template<>
void LuaFunction::call(int p1, bool p2, int p3){
    if (function != 0){
        lua_rawgeti(LuaBinding::getLuaState(), LUA_REGISTRYINDEX, function);
        lua_pushnumber(LuaBinding::getLuaState(), p1);
        lua_pushboolean(LuaBinding::getLuaState(), p2);
        lua_pushnumber(LuaBinding::getLuaState(), p3);
        LuaBinding::luaCallback(3, 0, 0);
    }
};

template<>
void LuaFunction::call(Object* p1){
    if (function != 0){
        lua_rawgeti(LuaBinding::getLuaState(), LUA_REGISTRYINDEX, function);
        LuaIntf::Lua::push(LuaBinding::getLuaState(), p1);
        LuaBinding::luaCallback(1, 0, 0);
    }
};

template<>
void LuaFunction::call(Object* p1, float p2){
    if (function != 0){
        lua_rawgeti(LuaBinding::getLuaState(), LUA_REGISTRYINDEX, function);
        LuaIntf::Lua::push(LuaBinding::getLuaState(), p1);
        lua_pushnumber(LuaBinding::getLuaState(), p2);
        LuaBinding::luaCallback(2, 0, 0);
    }
};

template<>
void LuaFunction::call(std::string p1){
    if (function != 0){
        lua_rawgeti(LuaBinding::getLuaState(), LUA_REGISTRYINDEX, function);
        LuaIntf::Lua::push(LuaBinding::getLuaState(), p1);
        LuaBinding::luaCallback(1, 0, 0);
    }
};

template<>
void LuaFunction::call(wchar_t p1){
    if (function != 0){
        lua_rawgeti(LuaBinding::getLuaState(), LUA_REGISTRYINDEX, function);
        lua_pushnumber(LuaBinding::getLuaState(), p1);
        LuaBinding::luaCallback(1, 0, 0);
    }
};

/*
template<>
void LuaFunction::call(Contact2D* p1){
    if (function != 0){
        lua_rawgeti(LuaBinding::getLuaState(), LUA_REGISTRYINDEX, function);
        LuaIntf::Lua::push(LuaBinding::getLuaState(), p1);
        LuaBinding::luaCallback(1, 0, 0);
    }
};
*/
template<>
float LuaFunction::call(float p1){
    if (function != 0){
        lua_rawgeti(LuaBinding::getLuaState(), LUA_REGISTRYINDEX, function);
        lua_pushnumber(LuaBinding::getLuaState(), p1);
        LuaBinding::luaCallback(1, 1, 0);

        if (!lua_isnumber(LuaBinding::getLuaState(), -1))
            Log::Error("Lua Error: function in Action must return a number\n");
        float value = lua_tonumber(LuaBinding::getLuaState(), -1);
        lua_pop(LuaBinding::getLuaState(), 1);

        return value;
    }
    return 0;
}
