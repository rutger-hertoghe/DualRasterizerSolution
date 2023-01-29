#pragma once
#include "pch.h"

#define CNSL_GREEN FOREGROUND_GREEN
#define CNSL_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN)
#define CNSL_PURPLE (FOREGROUND_BLUE | FOREGROUND_RED)
#define CNSL_WHITE (FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN)

class ConsoleColorCtrl
{
public:
	void SetConsoleColor(WORD color);

	static ConsoleColorCtrl* GetInstance();
	static void Destroy();

private:
	ConsoleColorCtrl() = default;

	void SetConsoleHandle(HANDLE hConsole);

	HANDLE m_hConsole;

	inline static ConsoleColorCtrl* m_pInstance{nullptr};
};

