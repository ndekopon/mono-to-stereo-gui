#include "main_window.hpp"

#include <imm.h>

#pragma comment(lib, "imm32.lib")

namespace
{
	void menu_checkeditem_create(HMENU _menu, UINT _id, const std::wstring& _str, bool _checked)
	{
		MENUITEMINFO mi = { 0 };
		std::vector<wchar_t> sz(_str.c_str(), _str.c_str() + _str.size() + 1);

		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_ID | MIIM_STATE | MIIM_STRING | MIIM_CHECKMARKS;
		mi.wID = _id;
		mi.dwTypeData = sz.data();
		if (_checked)
			mi.fState = MFS_CHECKED;
		else
			mi.fState = MFS_UNCHECKED;
		::InsertMenuItemW(_menu, -1, TRUE, &mi);
	}

	void menu_separator_create(HMENU _menu)
	{
		MENUITEMINFO mi = { 0 };

		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_FTYPE;
		mi.fType = MFT_SEPARATOR;
		::InsertMenuItemW(_menu, -1, TRUE, &mi);
	}
}

namespace app
{
	constexpr UINT MID_EXIT = 0;
	constexpr UINT MID_RESET = 1;
	constexpr UINT MID_TOGGLE_REVERSE_CHANNEL = 2;
	constexpr UINT MID_RENDER_NONE = 31;
	constexpr UINT MID_RENDER_0 = 32;
	constexpr UINT MID_CAPTURE_NONE = 63;
	constexpr UINT MID_CAPTURE_0 = 64;
	constexpr UINT MID_SET_VOLUME_0 = 96;

	const wchar_t* main_window::window_class_ = L"mono-to-stereo-gui-mainwindow";
	const wchar_t* main_window::window_title_ = L"mono-to-stereo-gui";


	main_window::main_window(HINSTANCE _instance)
		: instance_(_instance)
		, window_(nullptr)
		, nid_({ 0 })
		, taskbar_created_(0)
		, worker_thread_()
		, render_name_()
		, capture_name_()
		, render_names_()
		, capture_names_()
		, reverse_channel_(false)
		, volume_(100)
	{
	}

	main_window::~main_window()
	{
	}

	bool main_window::init()
	{
		disable_ime();

		set_dpi_awareness();

		// create window
		register_window_class();
		if (!create_window())
			return false;

		return true;
	}


	int main_window::loop()
	{
		MSG message;

		while (::GetMessageW(&message, nullptr, 0, 0))
		{
			::TranslateMessage(&message);
			::DispatchMessageW(&message);
		}
		return (int)message.wParam;
	}

	void main_window::disable_ime()
	{
		::ImmDisableIME(-1);
	}

	void main_window::set_dpi_awareness()
	{
		auto desired_context = DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED;
		if (::IsValidDpiAwarenessContext(desired_context))
		{
			auto hr = ::SetProcessDpiAwarenessContext(desired_context);
			if (hr)
				return;
		}
	}

	ATOM main_window::register_window_class()
	{
		WNDCLASSEXW wcex;

		wcex.cbSize = sizeof(WNDCLASSEXW);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = window_proc_common;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = instance_;
		wcex.hIcon = ::LoadIconW(nullptr, IDI_APPLICATION);
		wcex.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = window_class_;
		wcex.hIconSm = ::LoadIconW(nullptr, IDI_APPLICATION);

		return ::RegisterClassExW(&wcex);
	}

	bool main_window::create_window()
	{
		window_ = ::CreateWindowExW(0, window_class_, window_title_, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, instance_, this);

		if (window_ == nullptr)
		{
			return false;
		}

		return true;
	}

	UINT main_window::tasktray_add()
	{
		nid_.cbSize = sizeof(NOTIFYICONDATAW);
		nid_.uFlags = (NIF_ICON | NIF_MESSAGE | NIF_TIP);
		nid_.hWnd = window_;
		nid_.hIcon = ::LoadIconW(nullptr, IDI_APPLICATION);
		nid_.uID = 1;
		nid_.uCallbackMessage = CWM_TASKTRAY;
		nid_.uTimeout = 10000;
		nid_.dwState = NIS_HIDDEN;
		::lstrcpyW(nid_.szTip, window_title_);

		::Shell_NotifyIconW(NIM_ADD, &nid_);

		return ::RegisterWindowMessageW(L"TaskbarCreated");
	}

	void main_window::tasktray_reload()
	{
		::Shell_NotifyIconW(NIM_DELETE, &nid_);
		::Shell_NotifyIconW(NIM_ADD, &nid_);
	}


	void main_window::tasktray_remove()
	{
		::Shell_NotifyIconW(NIM_DELETE, &nid_);
	}

	void main_window::submenu_render_create(HMENU _menu)
	{
		int index = -1;
		for (size_t i = 0; i < render_names_.size() && i < 16; ++i)
		{
			if (render_name_ == render_names_.at(i)) index = i;
		}

		for (size_t i = 0; i < render_names_.size() && i < 16; ++i)
		{
			menu_checkeditem_create(_menu, MID_RENDER_0 + i, render_names_.at(i), index == i);
		}
		menu_checkeditem_create(_menu, MID_RENDER_NONE, L"None", index == -1);
	}

	void main_window::submenu_capture_create(HMENU _menu)
	{
		int index = -1;
		for (size_t i = 0; i < capture_names_.size() && i < 16; ++i)
		{
			if (capture_name_ == capture_names_.at(i)) index = i;
		}

		for (size_t i = 0; i < capture_names_.size() && i < 16; ++i)
		{
			menu_checkeditem_create(_menu, MID_CAPTURE_0 + i, capture_names_.at(i), index == i);
		}
		menu_checkeditem_create(_menu, MID_CAPTURE_NONE, L"None", index == -1);
	}

	void main_window::submenu_volume_create(HMENU _menu)
	{
		for (int i = 10; i >= 0; --i)
		{
			menu_checkeditem_create(_menu, MID_SET_VOLUME_0 + i, std::to_wstring(i * 10) + L"%", volume_ == i * 10);
		}
	}

	void main_window::menu_create()
	{
		HMENU menu;
		HMENU menu_render;
		HMENU menu_capture;
		HMENU menu_volume;
		POINT pt;
		MENUITEMINFO mi = { 0 };
		mi.cbSize = sizeof(MENUITEMINFO);

		// メニューハンドル作成
		menu = ::CreatePopupMenu();
		menu_render = ::CreatePopupMenu();
		menu_capture = ::CreatePopupMenu();
		menu_volume = ::CreatePopupMenu();

		// サブメニュー作成
		submenu_render_create(menu_render);
		submenu_capture_create(menu_capture);
		submenu_volume_create(menu_volume);

		// Render
		WCHAR menu_render_string[] = L"render";
		mi.fMask = MIIM_SUBMENU | MIIM_STRING;
		mi.hSubMenu = menu_render;
		mi.dwTypeData = menu_render_string;
		::InsertMenuItemW(menu, -1, TRUE, &mi);

		// Capture
		WCHAR menu_capture_string[] = L"capture";
		mi.fMask = MIIM_SUBMENU | MIIM_STRING;
		mi.hSubMenu = menu_capture;
		mi.dwTypeData = menu_capture_string;
		::InsertMenuItemW(menu, -1, TRUE, &mi);

		// Volume
		WCHAR menu_volume_string[] = L"volume";
		mi.fMask = MIIM_SUBMENU | MIIM_STRING;
		mi.hSubMenu = menu_volume;
		mi.dwTypeData = menu_volume_string;
		::InsertMenuItemW(menu, -1, TRUE, &mi);

		// reverse channel
		menu_checkeditem_create(menu, MID_TOGGLE_REVERSE_CHANNEL, L"reverse channel", reverse_channel_);

		// セパレーター
		menu_separator_create(menu);

		// リセット
		WCHAR menu_reset_string[] = L"reset";
		mi.fMask = MIIM_ID | MIIM_STRING;
		mi.wID = MID_RESET;
		mi.dwTypeData = menu_reset_string;
		::InsertMenuItemW(menu, -1, TRUE, &mi);

		// セパレーター
		menu_separator_create(menu);

		// 終了
		WCHAR menu_exit_string[] = L"exit";
		mi.fMask = MIIM_ID | MIIM_STRING;
		mi.wID = MID_EXIT;
		mi.dwTypeData = menu_exit_string;
		::InsertMenuItemW(menu, -1, TRUE, &mi);

		::GetCursorPos(&pt);
		::SetForegroundWindow(window_);
		::TrackPopupMenu(menu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, window_, NULL);

		// ハンドルは削除
		::DestroyMenu(menu_volume);
		::DestroyMenu(menu_capture);
		::DestroyMenu(menu_render);
		::DestroyMenu(menu);
	}

	LRESULT main_window::window_proc(UINT _message, WPARAM _wparam, LPARAM _lparam)
	{
		switch (_message)
		{
		case WM_CREATE:
			// タスクトレイ追加
			taskbar_created_ = tasktray_add();

			// configから読み出し
			render_name_ = config_ini_.get_render_device();
			capture_name_ = config_ini_.get_capture_device();
			reverse_channel_ = config_ini_.get_reverse_channel();
			volume_ = config_ini_.get_volume();

			// スレッド開始
			if (!worker_thread_.run(window_, render_name_, capture_name_, reverse_channel_, volume_)) return -1;

			// タイマー設定
			::SetTimer(window_, 1, 1000, nullptr);

			return 0;

		case CWM_INIT_COMPLETE:
			render_names_ = worker_thread_.get_render_names();
			capture_names_ = worker_thread_.get_capture_names();
			return 0;

		case WM_DESTROY:
			// スレッド停止
			worker_thread_.stop();

			// タスクトレイ削除
			tasktray_remove();

			::PostQuitMessage(0);

			return 0;

		case CWM_TASKTRAY:
			switch (_lparam)
			{
			case WM_RBUTTONUP:
				menu_create();
				break;
			}
			break;

		case WM_COMMAND:
			if (HIWORD(_wparam) == 0 && _lparam == 0)
			{
				WORD id = LOWORD(_wparam);
				if (id == MID_EXIT)
				{
					::DestroyWindow(window_);
				}
				if (id == MID_RESET)
				{
					worker_thread_.reset(render_name_, capture_name_, reverse_channel_, volume_);
				}
				if (id == MID_TOGGLE_REVERSE_CHANNEL)
				{
					reverse_channel_ = !reverse_channel_;
					config_ini_.set_reverse_channel(reverse_channel_);
					worker_thread_.reset(render_name_, capture_name_, reverse_channel_, volume_);
				}
				if (MID_RENDER_NONE <= id && id < MID_RENDER_0 + render_names_.size())
				{
					int index = id - MID_RENDER_0;
					std::wstring new_device = L"";
					if (index >= 0)
					{
						new_device = render_names_.at(index);
					}
					if (render_name_ != new_device)
					{
						render_name_ = new_device;
						config_ini_.set_render_device(render_name_);
						worker_thread_.reset(render_name_, capture_name_, reverse_channel_, volume_);
					}
				}
				if (MID_CAPTURE_NONE <= id && id < MID_CAPTURE_0 + capture_names_.size())
				{
					int index = id - MID_CAPTURE_0;
					std::wstring new_device = L"";
					if (index >= 0)
					{
						new_device = capture_names_.at(index);
					}
					if (capture_name_ != new_device)
					{
						capture_name_ = new_device;
						config_ini_.set_capture_device(capture_name_);
						worker_thread_.reset(render_name_, capture_name_, reverse_channel_, volume_);
					}
				}
				if (MID_SET_VOLUME_0 <= id && id <= MID_SET_VOLUME_0 + 10)
				{
					UINT32 volume = (id - MID_SET_VOLUME_0) * 10;
					if (volume_ != volume)
					{
						volume_ = volume;
						config_ini_.set_volume(volume_);
						worker_thread_.set_volume(volume_);
					}
				}
			}
			break;

		case WM_TIMER:

			if (_wparam == 1)
			{
				worker_thread_.stats();
				return 0;
			}
			break;

		case CWM_STATS_UPDATE:
		{
			auto stats = worker_thread_.get_stats();
			std::wstring tip = L"";
			tip += L"skip " + std::to_wstring(stats.at(0)) + L"\r\n";
			tip += L"duplicate " + std::to_wstring(stats.at(1));
			nid_.uFlags = NIF_TIP;
			::lstrcpyW(nid_.szTip, tip.c_str());
			::Shell_NotifyIconW(NIM_MODIFY, &nid_);
			break;
		}

		default:
			if (_message == taskbar_created_)
			{
				tasktray_reload();
			}
		}

		return ::DefWindowProcW(window_, _message, _wparam, _lparam);
	}

	LRESULT CALLBACK main_window::window_proc_common(HWND _window, UINT _message, WPARAM _wparam, LPARAM _lparam)
	{
		if (_message == WM_NCCREATE)
		{
			// createwindowで指定したポイントからインスタンスを取得
			auto cs = reinterpret_cast<CREATESTRUCTW*>(_lparam);
			auto instance = reinterpret_cast<main_window*>(cs->lpCreateParams);

			instance->window_ = _window;

			// USERDATAにポインタ格納
			::SetWindowLongPtrW(_window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(instance));
		}

		// 既にデータが格納されていたらインスタンスのプロシージャを呼び出す
		if (auto ptr = reinterpret_cast<main_window*>(::GetWindowLongPtrW(_window, GWLP_USERDATA)))
		{
			return ptr->window_proc(_message, _wparam, _lparam);
		}

		return ::DefWindowProcW(_window, _message, _wparam, _lparam);
	}
}
