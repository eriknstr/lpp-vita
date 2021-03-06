/*----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#------  This File is Part Of : ----------------------------------------------------------------------------------------#
#------- _  -------------------  ______   _   --------------------------------------------------------------------------#
#------ | | ------------------- (_____ \ | |  --------------------------------------------------------------------------#
#------ | | ---  _   _   ____    _____) )| |  ____  _   _   ____   ____   ----------------------------------------------#
#------ | | --- | | | | / _  |  |  ____/ | | / _  || | | | / _  ) / ___)  ----------------------------------------------#
#------ | |_____| |_| |( ( | |  | |      | |( ( | || |_| |( (/ / | |  --------------------------------------------------#
#------ |_______)\____| \_||_|  |_|      |_| \_||_| \__  | \____)|_|  --------------------------------------------------#
#------------------------------------------------- (____/  -------------------------------------------------------------#
#------------------------   ______   _   -------------------------------------------------------------------------------#
#------------------------  (_____ \ | |  -------------------------------------------------------------------------------#
#------------------------   _____) )| | _   _   ___   ------------------------------------------------------------------#
#------------------------  |  ____/ | || | | | /___)  ------------------------------------------------------------------#
#------------------------  | |      | || |_| ||___ |  ------------------------------------------------------------------#
#------------------------  |_|      |_| \____|(___/   ------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Licensed under the GPL License --------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Copyright (c) Nanni <lpp.nanni@gmail.com> ---------------------------------------------------------------------------#
#- Copyright (c) Rinnegatamante <rinnegatamante@gmail.com> -------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Credits : -----------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- All the devs involved in Rejuvenate and vita-toolchain --------------------------------------------------------------#
#- xerpi for drawing libs and for FTP server code ----------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
extern "C"{
	#include <vitasdk.h>
}
#include "include/Archives.h"
#include "include/luaplayer.h"
#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)
#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

int FREAD = SCE_O_RDONLY;
int FWRITE = SCE_O_WRONLY;
int FCREATE = SCE_O_CREAT | SCE_O_WRONLY;
int FRDWR = SCE_O_RDWR;
uint32_t SET = SEEK_SET;
uint32_t CUR = SEEK_CUR;
uint32_t END = SEEK_END;

static int lua_dofile(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	#endif
	char* file = (char*)luaL_checkstring(L,1);
	unsigned char* buffer;
	SceUID script = sceIoOpen(file, SCE_O_RDONLY, 0777);
	if (script < 0) return luaL_error(L, "error opening file.");
	else{
		SceOff size = sceIoLseek(script, 0, SEEK_END);
		sceIoLseek(script, 0, SEEK_SET);
		buffer = (unsigned char*)malloc(size + 1);
		sceIoRead(script, buffer, size);
		buffer[size] = 0;
		sceIoClose(script);
	}
	lua_settop(L, 1);
	if (luaL_loadbuffer(L, (const char*)buffer, strlen((const char*)buffer), NULL) != LUA_OK)	return lua_error(L);
	lua_KFunction dofilecont = (lua_KFunction)(lua_gettop(L) - 1);
	lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
	return (int)dofilecont;
}

static int lua_launch(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	#endif
	char* file = (char*)luaL_checkstring(L,1);
	unsigned char* buffer;
	SceUID bin = sceIoOpen(file, SCE_O_RDONLY, 0777);
	#ifndef SKIP_ERROR_HANDLING
	if (bin < 0) return luaL_error(L, "error opening file.");
	#endif
	else sceIoClose(bin);
	sceAppMgrLoadExec(file, NULL, NULL);
	return 0;
}

static int lua_openfile(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 2) return luaL_error(L, "wrong number of arguments");
	#endif
	const char *file_tbo = luaL_checkstring(L, 1);
	int type = luaL_checkinteger(L, 2);
	SceUID fileHandle = sceIoOpen(file_tbo, type, 0777);
	#ifndef SKIP_ERROR_HANDLING
    if (fileHandle < 0) return luaL_error(L, "cannot open requested file.");
	#endif
	lua_pushinteger(L,fileHandle);
	return 1;
}

static int lua_readfile(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 2) return luaL_error(L, "wrong number of arguments");
	#endif
	SceUID file = luaL_checkinteger(L, 1);
	uint32_t size = luaL_checkinteger(L, 2);
	uint8_t* buffer = (uint8_t*)malloc(size);
	int len = sceIoRead(file,buffer, size);
	buffer[len] = 0;
	lua_pushlstring(L,(const char*)buffer,len);
	free(buffer);
	return 1;
}

static int lua_writefile(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 3) return luaL_error(L, "wrong number of arguments");
	#endif
	SceUID fileHandle = luaL_checkinteger(L, 1);
	const char *text = luaL_checkstring(L, 2);
	int size = luaL_checknumber(L, 3);
	sceIoWrite(fileHandle, text, size);
	return 0;
}

static int lua_closefile(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	SceUID fileHandle = luaL_checkinteger(L, 1);
	sceIoClose(fileHandle);
	return 0;
}

static int lua_seekfile(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 3) return luaL_error(L, "wrong number of arguments");
	#endif
	SceUID fileHandle = luaL_checkinteger(L, 1);
	int pos = luaL_checkinteger(L, 2);
	uint32_t type = luaL_checkinteger(L, 3);
	sceIoLseek(fileHandle, pos, type);	
	return 0;
}

static int lua_sizefile(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	SceUID fileHandle = luaL_checkinteger(L, 1);
	uint32_t cur_off = sceIoLseek(fileHandle, 0, SEEK_CUR);
	uint32_t size = sceIoLseek(fileHandle, 0, SEEK_END);
	sceIoLseek(fileHandle, cur_off, SEEK_SET);
	lua_pushinteger(L, size);
	return 1;
}

static int lua_checkexist(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	const char *file_tbo = luaL_checkstring(L, 1);
	SceUID fileHandle = sceIoOpen(file_tbo, SCE_O_RDONLY, 0777);
	if (fileHandle < 0) lua_pushboolean(L, false);
	else{
		sceIoClose(fileHandle);
		lua_pushboolean(L,true);
	}
	return 1;
}

static int lua_rename(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 2) return luaL_error(L, "wrong number of arguments");
	#endif
	const char *old_file = luaL_checkstring(L, 1);
	const char *new_file = luaL_checkstring(L, 2);
	sceIoRename(old_file, new_file);
	return 0;
}

static int lua_removef(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	const char *old_file = luaL_checkstring(L, 1);
	sceIoRemove(old_file);
	return 0;
}

static int lua_removef2(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	const char *old_file = luaL_checkstring(L, 1);
	sceIoRmdir(old_file);
	return 0;
}

static int lua_newdir(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	const char *newdir = luaL_checkstring(L, 1);
	sceIoMkdir(newdir, 0777);
	return 0;
}

static int lua_exit(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	char stringbuffer[256];
	strcpy(stringbuffer,"lpp_shutdown");
	luaL_dostring(L, "collectgarbage()");
	return luaL_error(L, stringbuffer); //Fake LUA error
}

static int lua_wait(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	int microsecs = luaL_checkinteger(L, 1);
	sceKernelDelayThread(microsecs);
	return 0;
}

static int lua_screenshot(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 1 && argc != 2 && argc != 3) return luaL_error(L, "wrong number of arguments");
	#endif
	const char *filename = luaL_checkstring(L, 1);
	bool isJPG = (argc > 1) ? lua_toboolean(L, 2) : false;
	int ratio = (argc == 3) ? luaL_checkinteger(L, 3) : 127;
	SceDisplayFrameBuf param;
	param.size = sizeof(SceDisplayFrameBuf);
	sceDisplayWaitVblankStart();
	sceDisplayGetFrameBuf(&param, SCE_DISPLAY_SETBUF_NEXTFRAME);
	int fd = sceIoOpen(filename, SCE_O_CREAT|SCE_O_WRONLY|SCE_O_TRUNC, 0777);
	if (!isJPG){
		uint8_t* bmp_content = (uint8_t*)malloc(((param.pitch*param.height)<<2)+0x36);
		memset(bmp_content, 0, 0x36);
		*(uint16_t*)&bmp_content[0x0] = 0x4D42;
		*(uint32_t*)&bmp_content[0x2] = ((param.pitch*param.height)<<2)+0x36;
		*(uint32_t*)&bmp_content[0xA] = 0x36;
		*(uint32_t*)&bmp_content[0xE] = 0x28;
		*(uint32_t*)&bmp_content[0x12] = param.pitch;
		*(uint32_t*)&bmp_content[0x16] = param.height;
		*(uint32_t*)&bmp_content[0x1A] = 0x00200001;
		*(uint32_t*)&bmp_content[0x22] = ((param.pitch*param.height)<<2);
		int x, y;
		uint32_t* buffer = (uint32_t*)bmp_content;
		uint32_t* framebuf = (uint32_t*)param.base;
		for (y = 0; y<param.height; y++){
			for (x = 0; x<param.pitch; x++){
				buffer[x+y*param.pitch+0x36] = framebuf[x+(param.height-y)*param.pitch];
				uint8_t* clr = (uint8_t*)&buffer[x+y*param.pitch+0x36];
				uint8_t r = clr[1];
				clr[1] = clr[3];
				clr[3] = r;
			}
		}
		sceIoWrite(fd, bmp_content, ((param.pitch*param.height)<<2)+0x36);
		free(bmp_content);
	}else{
		uint32_t in_size = ALIGN((param.width * param.height)<<1, 256);
		uint32_t out_size = ALIGN(param.width * param.height, 256);
		uint32_t buf_size = ALIGN(in_size + out_size, 0x40000);
		SceUID memblock = sceKernelAllocMemBlock("encoderBuffer", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, buf_size, NULL);
		void* buf_addr = NULL;
		sceKernelGetMemBlockBase(memblock, &buf_addr);
		SceJpegEncoderContext context = malloc(sceJpegEncoderGetContextSize());
		sceJpegEncoderInit(context, param.width, param.height, PIXELFORMAT_YCBCR420 | PIXELFORMAT_CSC_ARGB_YCBCR, buf_addr + in_size, out_size);
		sceJpegEncoderSetValidRegion(context, param.width, param.height);
		sceJpegEncoderSetCompressionRatio(context, ratio);
		sceJpegEncoderSetOutputAddr(context, buf_addr + in_size, out_size);
		sceJpegEncoderCsc(context, buf_addr, param.base, param.pitch, PIXELFORMAT_ARGB8888);
		int filesize = sceJpegEncoderEncode(context, buf_addr);
		sceIoWrite(fd, buf_addr + in_size, filesize);
		sceJpegEncoderEnd(context);
		free(context);
		sceKernelFreeMemBlock(memblock);
	}
	sceIoClose(fd);
	return 0;
}


SceIoDirent g_dir;

static int lua_dir(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0 && argc != 1) return luaL_error(L, "System.listDirectory([path]) takes zero or one argument");
	#endif
	const char *path = "";
	if (argc == 0) path = "";
	else path = luaL_checkstring(L, 1);
	int fd = sceIoDopen(path);
	if (fd < 0) {
		lua_pushnil(L);  /* return nil */
		return 1;
	}
	lua_newtable(L);
	int i = 1;
	while (sceIoDread(fd, &g_dir) > 0) {
		lua_pushnumber(L, i++);  /* push key for file entry */
		lua_newtable(L);
		lua_pushstring(L, "name");
		lua_pushstring(L, g_dir.d_name);
		lua_settable(L, -3);
		lua_pushstring(L, "size");
		lua_pushnumber(L, g_dir.d_stat.st_size);
		lua_settable(L, -3);
		lua_pushstring(L, "directory");
		lua_pushboolean(L, SCE_S_ISDIR(g_dir.d_stat.st_mode));
		lua_settable(L, -3);
		lua_settable(L, -3);
	}
	sceIoDclose(fd);
	return 1;  /* table is already on top */
}

static int lua_charging(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	lua_pushboolean(L, scePowerIsBatteryCharging());
	return 1;
}

static int lua_percent(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	lua_pushinteger(L, scePowerGetBatteryLifePercent());
	return 1;
}

static int lua_lifetime(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	lua_pushinteger(L, scePowerGetBatteryLifeTime());
	return 1;
}

static int lua_nopower(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	sceKernelPowerTick(0);
	return 0;
}

static int lua_setcpu(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	int freq = luaL_checkinteger(L, 1);
	scePowerSetArmClockFrequency(freq);
	return 0;
}

static int lua_setbus(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	int freq = luaL_checkinteger(L, 1);
	scePowerSetBusClockFrequency(freq);
	return 0;
}

static int lua_setgpu(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	int freq = luaL_checkinteger(L, 1);
	scePowerSetGpuClockFrequency(freq);
	return 0;
}

static int lua_setgpu2(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	int freq = luaL_checkinteger(L, 1);
	scePowerSetGpuXbarClockFrequency(freq);
	return 0;
}

static int lua_getcpu(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	lua_pushinteger(L, scePowerGetArmClockFrequency());
	return 1;
}

static int lua_getbus(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	lua_pushinteger(L, scePowerGetBusClockFrequency());
	return 1;
}

static int lua_getgpu(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	lua_pushinteger(L, scePowerGetGpuClockFrequency());
	return 1;
}

static int lua_getgpu2(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	lua_pushinteger(L, scePowerGetGpuXbarClockFrequency());
	return 1;
}

static int lua_gettime(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	SceDateTime time;
	sceRtcGetCurrentClockLocalTime(&time);
	lua_pushinteger(L,time.hour);
	lua_pushinteger(L,time.minute);
	lua_pushinteger(L,time.second);
	return 3;
}

static int lua_getdate(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	SceDateTime time;
	sceRtcGetCurrentClockLocalTime(&time);
	lua_pushinteger(L, sceRtcGetDayOfWeek(time.year, time.month, time.day));
	lua_pushinteger(L,time.day);
	lua_pushinteger(L,time.month);
	lua_pushinteger(L,time.year);
	return 4;
}

static int lua_nickname(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	SceChar8 nick[SCE_SYSTEM_PARAM_USERNAME_MAXSIZE];
	sceAppUtilSystemParamGetString(SCE_SYSTEM_PARAM_ID_USERNAME, nick, SCE_SYSTEM_PARAM_USERNAME_MAXSIZE);
	lua_pushstring(L,(char*)nick);
	return 1;
}

static int lua_lang(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	int lang;
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, &lang);
	lua_pushinteger(L,lang);
	return 1;
}

static int lua_title(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	char title[256];
	sceAppMgrAppParamGetString(0, 9, title , 256);
	lua_pushstring(L,title);
	return 1;
}

static int lua_titleid(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	char title[16];
	sceAppMgrAppParamGetString(0, 12, title , 256);
	lua_pushstring(L,title);
	return 1;
}

static int lua_model(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	lua_pushinteger(L,sceKernelGetModel());
	return 1;
}

static int lua_ZipExtract(lua_State *L) {
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if(argc != 2 && argc != 3) return luaL_error(L, "wrong number of arguments.");
	#endif
	const char *FileToExtract = luaL_checkstring(L, 1);
	const char *DirTe = luaL_checkstring(L, 2);
	const char *Password = (argc == 3) ? luaL_checkstring(L, 3) : NULL;
	sceIoMkdir(DirTe, 0777);
	Zip *handle = ZipOpen(FileToExtract);
	#ifndef SKIP_ERROR_HANDLING
	if (handle == NULL) luaL_error(L, "error opening ZIP file.");
	#endif
	int result = ZipExtract(handle, Password, DirTe);
	ZipClose(handle);
	lua_pushinteger(L, result);
	return 1;
}

static int lua_getfilefromzip(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if(argc != 3 && argc != 4 ) return luaL_error(L, "wrong number of arguments.");
	#endif
	const char *FileToExtract = luaL_checkstring(L, 1);
	const char *FileToExtract2 = luaL_checkstring(L, 2);
	const char *Dest = luaL_checkstring(L, 3);
	const char *Password = (argc == 4) ? luaL_checkstring(L, 4) : NULL;
	Zip *handle = ZipOpen(FileToExtract);
	#ifndef SKIP_ERROR_HANDLING
	if (handle == NULL) luaL_error(L, "error opening ZIP file.");
	#endif
	ZipFile* file = ZipFileRead(handle, FileToExtract2, Password);
	if (file == NULL) lua_pushboolean(L, false);
	else{
		FILE* f = fopen(Dest,"w");
		fwrite(file->data, 1, file->size, f);
		fclose(f);
		ZipFileFree(file);
		lua_pushboolean(L, true);
	}
	ZipClose(handle);
	return 1;
}

static int lua_executeuri(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	const char *uri_string = luaL_checkstring(L, 1);
	sceAppMgrLaunchAppByUri(0xFFFFF, uri_string);
	return 0;
}

//Register our System Functions
static const luaL_Reg System_functions[] = {

  // Dofile & I/O Library patched functions
  {"doNotUse",							lua_dofile},
  {"doNotOpen",							lua_openfile},
  {"doNotRead",							lua_readfile},
  {"doNotWrite",						lua_writefile},
  {"doNotClose",						lua_closefile},  
  {"doNotSeek",							lua_seekfile},  
  {"doNotSize",							lua_sizefile},  
  
  {"doesFileExist",						lua_checkexist},
  {"exit",								lua_exit},
  {"rename",							lua_rename},
  {"deleteFile",						lua_removef},
  {"deleteDirectory",					lua_removef2},
  {"createDirectory",					lua_newdir},
  {"listDirectory",						lua_dir},
  {"wait",								lua_wait},
  {"isBatteryCharging",					lua_charging},
  {"getBatteryPercentage",				lua_percent},
  {"getBatteryLife",					lua_lifetime},
  {"powerTick",							lua_nopower},
  {"setCpuSpeed",						lua_setcpu},
  {"getCpuSpeed",						lua_getcpu},
  {"setBusSpeed",						lua_setbus},
  {"getBusSpeed",						lua_getbus},
  {"setGpuSpeed",						lua_setgpu},
  {"getGpuSpeed",						lua_getgpu},
  {"setGpuXbarSpeed",					lua_setgpu2},
  {"getGpuXbarSpeed",					lua_getgpu2},
  {"launchEboot",						lua_launch},
  {"getTime",							lua_gettime},
  {"getDate",							lua_getdate},
  {"getUsername",						lua_nickname},
  {"getLanguage",						lua_lang},
  {"getModel",							lua_model},
  {"getTitle",							lua_title},
  {"getTitleID",						lua_titleid},
  {"extractZIP",						lua_ZipExtract},
  {"extractFromZIP",					lua_getfilefromzip},
  {"takeScreenshot",					lua_screenshot},
  {"executeUri",					lua_executeuri},	
  {0, 0}
};

void luaSystem_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, System_functions, 0);
	lua_setglobal(L, "System");
	VariableRegister(L,FREAD);
	VariableRegister(L,FWRITE);
	VariableRegister(L,FCREATE);
	VariableRegister(L,FRDWR);
	VariableRegister(L,SET);
	VariableRegister(L,END);
	VariableRegister(L,CUR);
}
