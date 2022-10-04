#pragma once

#define WINVER       0x0A00 // windows10
#define _WIN32_WINNT 0x0A00 // windows10

#include <WinSDKVer.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_AMD64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_ARM
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='arm' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_ARM64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='arm64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

namespace app {
	constexpr UINT CWM_TASKTRAY = WM_APP + 1;
	constexpr UINT CWM_INIT_COMPLETE = WM_APP + 2;
	constexpr UINT CWM_STATS_UPDATE = WM_APP + 3;

	constexpr UINT32 RENDER_SAMPLES = 48000;
	constexpr UINT32 RENDER_CHANNELS = 2;
	constexpr UINT32 CAPTURE_SAMPLES = 96000;
	constexpr UINT32 CAPTURE_CHANNELS = 1;
}
