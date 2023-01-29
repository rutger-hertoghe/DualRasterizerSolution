#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
//#include "Mesh.h"
#include "Renderer.h"
#include "ConsoleColorCtrl.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

void PrintStartMessage()
{
	ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_YELLOW);
	std::cout << "[Key Bindings] - SHARED\n"
		<< "	[F1] Toggle Rasterizer Mode (HARDWARE/SOFTWARE)\n"
		<< "	[F2] Toggle Vehicle Rotation (ON/OFF)\n"
		<< "	[F5] Cycle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR) (*)\n"
		<< "	[F6] Toggle Normal Map (ON/OFF) (*)\n"
		<< "	[F9] Cycle Cull Modes (BACK/FRONT/NONE)\n" 
		<< "	[F10] Toggle Uniform ClearColor [On/Off]\n"
		<< "	[F11] Toggle Print FPS [On/Off]\n\n";

	ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_GREEN);
	std::cout << "[Key Bindings] - HARDWARE\n"
		<< "	[F3] Toggle FireFX (ON/OFF)\n"
		<< "	[F4] Cycle Sampler State (POINT/LINEAR/ANISOTROPIC)\n\n";

	ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_PURPLE);
	std::cout << "[Key Bindings] - SOFTWARE\n"
		<< "	[F7] Toggle DepthBuffer Visualisation (ON/OFF)\n" 
		<< "	[F8] Toggle BoundingBoxVisualisation (ON/OFF)\n\n";

	ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_WHITE);
	std::cout << "(*) = Differs from specification document: have been implemented as shared instead of only software\n";
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"Dual Rasterizer - Rutger Hertoghe (2GD07)",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	PrintStartMessage();

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	bool isPrintingFPS = false;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				//Test for a key
				if(e.key.keysym.scancode == SDL_SCANCODE_F1)
				{
					pRenderer->ToggleRasterizer();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
				{
					pRenderer->ToggleRotateMesh();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F3)
				{
					pRenderer->ToggleFireFX();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F4)
				{
					pRenderer->CycleFilteringTechniques();
				}
				if(e.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					pRenderer->CycleShadingModes();
				}
				if(e.key.keysym.scancode == SDL_SCANCODE_F6)
				{
					pRenderer->ToggleUseNormalMaps();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F7)
				{
					pRenderer->ToggleShowDepthBuffer();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F8)
				{
					pRenderer->ToggleShowBoundingBoxes();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F9)
				{
					pRenderer->CycleCullingMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F10)
				{
					pRenderer->ToggleUniformColor();
				}
				if(e.key.keysym.scancode == SDL_SCANCODE_F11)
				{
					isPrintingFPS = !isPrintingFPS;
				}
				break;
			default: ;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			if (isPrintingFPS)
			{
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}
	}
	pTimer->Stop();

	ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_WHITE);
	ConsoleColorCtrl::Destroy();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}