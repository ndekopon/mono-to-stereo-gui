#include "config_ini.hpp"

#include <vector>

namespace {
	std::wstring get_ini_path() {
		WCHAR drivebuff[_MAX_DRIVE];
		std::vector<WCHAR> fullbuff(32767, L'\0');

		// モジュールインスタンスからDLLパスを取得
		auto loaded = ::GetModuleFileNameW(::GetModuleHandleW(nullptr), fullbuff.data(), fullbuff.size());

		// フルパスを分解
		std::vector<WCHAR> dirbuff(loaded, L'\0');
		::_wsplitpath_s(fullbuff.data(), drivebuff, _MAX_DRIVE, dirbuff.data(), dirbuff.size(), nullptr, 0, nullptr, 0);

		// パスの合成
		std::wstring r;
		r = drivebuff;
		r += dirbuff.data();
		r += L"config.ini";
		return r;
	}
}


namespace app {

	const WCHAR section_name[] = L"MAIN";

	config_ini::config_ini()
		: path_(get_ini_path())
	{
	}

	config_ini::~config_ini()
	{
	}

	bool config_ini::set_value(const std::wstring& _key, const std::wstring& _value)
	{
		auto r = ::WritePrivateProfileStringW(section_name, _key.c_str(), _value.c_str(), path_.c_str());
		return r == TRUE;
	}

	std::wstring config_ini::get_value(const std::wstring& _key)
	{
		std::vector<WCHAR> buffer(32767, L'\0');
		auto readed = ::GetPrivateProfileStringW(section_name, _key.c_str(), L"", buffer.data(), buffer.size(), path_.c_str());
		return buffer.data();
	}

	UINT config_ini::get_intvalue(const std::wstring& _key)
	{
		return ::GetPrivateProfileIntW(section_name, _key.c_str(), -1, path_.c_str());
	}

	bool config_ini::set_render_device(const std::wstring& _device)
	{
		return set_value(L"RENDER", _device);
	}

	std::wstring config_ini::get_render_device()
	{
		return get_value(L"RENDER");
	}


	bool config_ini::set_capture_device(const std::wstring& _device)
	{
		return set_value(L"CAPTURE", _device);
	}

	std::wstring config_ini::get_capture_device()
	{
		return get_value(L"CAPTURE");
	}
}
