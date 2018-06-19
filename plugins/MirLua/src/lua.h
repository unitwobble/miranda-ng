#pragma once

class CMLua
{
	friend struct CMPlugin;

private:
	lua_State *L;

public:
	CMLua();
	~CMLua();

	lua_State* GetState();

	void Load();
};
