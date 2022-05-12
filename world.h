#pragma once


#include "includes.h"


struct world {
	DWORD GameModule = (DWORD)GetModuleHandle("client.dll");
};

struct entity {
public:

	int health;
	int flags;

	DWORD EntityPtr;

	void ForceJump(DWORD GameModule) {
		*(int*)(GameModule + dwForceJump) = 2;
	}


};

#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a,b)STR_MERGE_IMPL(a,b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define DEFINE_MEMBER_N(type, name, offset)struct {unsigned char MAKE_PAD(offset); type name;}

struct Vector32 {
	float x, y, z;
};

class Ent {
public:
	union {
	
		DEFINE_MEMBER_N(bool, isDormant, 0xED);
		DEFINE_MEMBER_N(int, iHealth, 0x100);
		//DEFINE_MEMBER_N(Vector3, vecOrigin, 0x138);
		DEFINE_MEMBER_N(int, iTeamNum, 0xF4);
	};
};

class EntListObj {
public:
	struct Ent* ent;
	char padding[12];
};

class EntList {
public:
	EntListObj ents[32];

};

class Hack {
public:
	Ent* localEnt;
	EntList* entList;
	float viewMatrix[16];
};