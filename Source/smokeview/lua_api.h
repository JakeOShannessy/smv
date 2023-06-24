#include "lua.h"

int lua_render(lua_State  *L);
lua_State *initLua();
int RunLuaBranch(lua_State *L, int argc, char **argv);

int lua_initsmvproginfo(lua_State *L);
int lua_initsmvdata(lua_State *L);
void addLuaPaths(lua_State *L);

int load_script(const char *filename);
int loadLuaScript(const char *filename);
int runLuaScript();
int loadSSFScript(const char *filename);
int runSSFScript();

int runScriptString(const char *string);
int lua_get_sliceinfo(lua_State *L);
int lua_get_csvinfo(lua_State *L);
int lua_initsmvdata(lua_State *L);
