#include "pch.h"
#include "ConsoleColorCtrl.h"

void ConsoleColorCtrl::SetConsoleHandle(HANDLE hConsole)
{
	m_hConsole = hConsole;
}

void ConsoleColorCtrl::SetConsoleColor(WORD color)
{
	SetConsoleTextAttribute(m_hConsole, color);
}

ConsoleColorCtrl* ConsoleColorCtrl::GetInstance()
{
	if(m_pInstance == nullptr)
	{
		m_pInstance = new ConsoleColorCtrl;
		m_pInstance->SetConsoleHandle(GetStdHandle(STD_OUTPUT_HANDLE));
	}
	return m_pInstance;
}

void ConsoleColorCtrl::Destroy()
{
	delete m_pInstance;
}
