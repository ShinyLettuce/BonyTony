#include "GameWorld.h"

#include <tge/input/InputManager.h>
#include <tge/scene/Scene.h>
#include <tge/scene/SceneSerialize.h>
#include <tge/settings/settings.h>
#include <tge/error/ErrorManager.h>
#include <tge/input/XInput.h>

#include "InputMapper.h"
#include "Timer.h"
#include "Go.h"
#include "SceneLoader.h"
#include <tge/scene/Scene.h>
#include "Profiler.h"

static Timer* locTimer = nullptr;
static Tga::InputManager* locInput = nullptr;
static Tga::XInput* locXInput = nullptr;

static bool& GetShouldCaptureMouseFlag()
{
	static bool gShouldCaptureMouse = true;
	return gShouldCaptureMouse;
}

void SetMouseCaptureEnabled(bool aEnabled) 
{
	GetShouldCaptureMouseFlag() = aEnabled; 
	if (!locInput)
		return; 

	if (aEnabled) 
	{
		locInput->CaptureMouse(); 
		::SetCursor(nullptr);     
	}
	else 
	{
		locInput->ReleaseMouse();                          
		::SetCursor(::LoadCursor(nullptr, IDC_ARROW));     
	}
}

LRESULT WinProc([[maybe_unused]]HWND hWnd, UINT message, [[maybe_unused]]WPARAM wParam, [[maybe_unused]]LPARAM lParam)
{
	const bool isCaptureControlMessage =
		(message == WM_SETFOCUS) ||
		(message == WM_KILLFOCUS) ||
		(message == WM_SIZE) ||
		(message == WM_SETCURSOR) ||
		(message == WM_ACTIVATE);

	if (!isCaptureControlMessage) 
	{
		if (locInput->UpdateEvents(message, wParam, lParam))
		{
			return 0;
		}
	}
	else 
	{
		locInput->UpdateEvents(message, wParam, lParam); 
	}

	switch (message)
	{
		// this message is read when the window is closed
	case WM_MOVING:
	{
		locTimer->SetIsPaused(true);
		return 0;
	}
	case WM_MOVE:
	{
		locTimer->SetIsPaused(false);
		return 0;
	}
	case WM_DESTROY:
	{
		// close the application entirely
		PostQuitMessage(0);
		return 0;
	}
	case WM_SETCURSOR: 
	{
		if (GetShouldCaptureMouseFlag()) 
		{
			::SetCursor(nullptr); 
		}
		else
		{
			::SetCursor(::LoadCursor(nullptr, IDC_ARROW)); 
		}
		return TRUE; 
	}
	case WM_SETFOCUS:
	{
		if (GetShouldCaptureMouseFlag()) 
		{
			locInput->CaptureMouse(); 
			::SetCursor(nullptr);
		}
		else
		{
			locInput->ReleaseMouse(); 
			::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
		}
		return 0;
	}
	case WM_KILLFOCUS:
	{
		locInput->ReleaseMouse();
		::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
		return 0;
	}
	case WM_SIZE:
	{
		if (GetShouldCaptureMouseFlag()) 
		{
			locInput->CaptureMouse(); 
			::SetCursor(nullptr);
		}
		else
		{
			locInput->ReleaseMouse(); 
			::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
		}
		return 0;
	}
	}
	return 0;
}

// STÄNG AV FÖR SLUTINLÄMNING
#define REBUILD_PACK

void Go()
{
	Tga::LoadSettings(TGE_PROJECT_SETTINGS_FILE);
	
	Tga::EngineConfiguration& cfg = Tga::Settings::GetEngineConfiguration();
	
	Timer timer;
	locTimer = &timer;
	Tga::InputManager input{ nullptr };
	locInput = &input;
	Tga::XInput xInput;
	locXInput = &xInput;
	
	InputMapper inputMapper(locInput, locXInput);

	cfg.myWinProcCallback = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {return WinProc(hWnd, message, wParam, lParam); };
#ifdef _DEBUG
	cfg.myActivateDebugSystems = Tga::DebugFeature::Fps | Tga::DebugFeature::Mem | Tga::DebugFeature::Filewatcher | Tga::DebugFeature::Cpu | Tga::DebugFeature::Drawcalls | Tga::DebugFeature::OptimizeWarnings;
#else
	cfg.myActivateDebugSystems = Tga::DebugFeature::Filewatcher;
#endif

	if (!Tga::Engine::Start())
	{
		ERROR_PRINT("Fatal error! Engine could not start!");
		system("pause");
		return;
	}

	SceneLoader::Init();

	SceneLoader::StartPreloadProcess();

#if defined (REBUILD_PACK)
	SceneLoader::PackScene("levels/Level1.tgs");
	SceneLoader::PackScene("levels/Level2.tgs");
	SceneLoader::PackScene("levels/BossRoomScene.tgs");
#endif

	{
		GameWorld gameWorld;
		gameWorld.Init(&inputMapper, &timer);

		Tga::Engine& engine = *Tga::Engine::GetInstance();
		
		input.SetWindowHandle( *engine.GetHWND() );
		input.CaptureMouse();
		input.HideMouse();

		SetMouseCaptureEnabled(false);

		while (engine.BeginFrame())
		{
			locTimer->Update();
			locInput->Update();
			locXInput->Refresh();
			inputMapper.Update(locTimer->GetDeltaTime());
			gameWorld.Update();
			gameWorld.Render();

			engine.EndFrame();
		}
	}

	SceneLoader::KillPreloadProcess();

	Tga::Engine::GetInstance()->Shutdown();

	Profiler* profiler = Profiler::Get();
	profiler->GetProfile();
}

