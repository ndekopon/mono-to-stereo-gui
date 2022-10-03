#pragma once

#include "common.hpp"

#include <array>
#include <mutex>
#include <string>
#include <vector>

namespace app
{

	class worker_thread
	{
	private:
		HWND window_;
		HANDLE thread_;
		std::mutex mtx_;
		std::mutex cfg_mtx_;
		std::mutex stats_mtx_;
		std::wstring render_name_;
		std::wstring capture_name_;
		std::vector<std::wstring> render_names_;
		std::vector<std::wstring> capture_names_;
		HANDLE event_reset_;
		HANDLE event_close_;
		HANDLE event_stats_;
		UINT64 stats_total_skip_;
		UINT64 stats_total_duplicate_;

		static DWORD WINAPI proc_common(LPVOID);
		DWORD proc();
		DWORD proc_render_and_capture();
		void set_render_names(const std::vector<std::wstring>&);
		void set_capture_names(const std::vector<std::wstring>&);
	public:
		worker_thread();
		~worker_thread();

		// コピー不可
		worker_thread(const worker_thread&) = delete;
		worker_thread& operator = (const worker_thread&) = delete;
		// ムーブ不可
		worker_thread(worker_thread&&) = delete;
		worker_thread& operator = (worker_thread&&) = delete;

		bool run(HWND, const std::wstring &, const std::wstring &);
		void stop();

		void reset(const std::wstring &, const std::wstring &);

		void stats();
		std::array<UINT64, 2> get_stats();

		std::vector<std::wstring> get_render_names();
		std::vector<std::wstring> get_capture_names();
		std::wstring get_render_name();
		std::wstring get_capture_name();
	};
}
