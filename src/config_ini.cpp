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

	bool config_ini::set_reverse_channel(bool _reverse)
	{
		return set_value(L"REVERSE", _reverse ? L"1" : L"0");
	}

	bool config_ini::get_reverse_channel()
	{
		auto reverse = get_value(L"REVERSE");
		return reverse == L"1" ? true : false;
	}

	bool config_ini::set_volume(UINT32 _v)
	{
		// 0～10に変換して書き込む
		_v = _v / 10;
		if (_v > 10) _v = 10;
		return set_value(L"VOLUME", std::to_wstring(_v));
	}

	UINT32 config_ini::get_volume()
	{
		auto s = get_value(L"VOLUME");
		if      (s == L"0") return  0;
		else if (s == L"1") return 10;
		else if (s == L"2") return 20;
		else if (s == L"3") return 30;
		else if (s == L"4") return 40;
		else if (s == L"5") return 50;
		else if (s == L"6") return 60;
		else if (s == L"7") return 70;
		else if (s == L"8") return 80;
		else if (s == L"9") return 90;
		else return 100;
	}
}
