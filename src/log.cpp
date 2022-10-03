#include "log.hpp"

#include <ctime>
#include <vector>
#include <iomanip>
#include <sstream>

namespace
{
	std::wstring logpath = L"";

	std::wstring get_log_filename()
	{
		auto t = std::time(nullptr);
		struct tm lt;
		auto error = localtime_s(&lt, &t);

		std::wostringstream oss;
		oss << L"log_";
		oss << std::put_time(&lt, L"%Y%m%d_%H%M%S");
		oss << L".txt";
		return oss.str();
	}

	std::wstring get_log_path()
	{
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
		r += get_log_filename();
		return r;
	}

	std::string get_datestring()
	{
		auto t = std::time(nullptr);
		struct tm lt;
		auto error = localtime_s(&lt, &t);

		std::ostringstream oss;
		oss << std::put_time(&lt, "%Y/%m/%d %H:%M:%S");
		return oss.str();
	}

	std::string utf16_to_utf8(const std::wstring& _src)
	{
		// calc buffer size
		auto size = ::WideCharToMultiByte(CP_UTF8, 0, _src.c_str(), -1, nullptr, 0, nullptr, nullptr);

		// convert
		std::vector<char> buffer(size + 1, '\0');
		::WideCharToMultiByte(CP_UTF8, 0, _src.c_str(), -1, buffer.data(), size + 1, nullptr, nullptr);

		return buffer.data();
	}
}

namespace app
{
	void write_to_logfile(const std::string& _text)
	{
		HANDLE file;

		// logpathの確認
		if (logpath == L"")
		{
			logpath = get_log_path();
		}

		file = ::CreateFileW(logpath.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (file == INVALID_HANDLE_VALUE)
		{
			return;
		}
		DWORD written = 0;
		::WriteFile(file, _text.c_str(), (DWORD)_text.size(), &written, NULL);
		::CloseHandle(file);
	}

	void wlog(const std::string& _text)
	{
		auto date = get_datestring();
		write_to_logfile(date + " " + _text + "\r\n");
	}

	void wlog(const std::wstring& _text)
	{
		wlog(utf16_to_utf8(_text));
	}
}