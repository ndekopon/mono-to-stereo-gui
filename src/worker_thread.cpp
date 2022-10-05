#include "worker_thread.hpp"

#include "log.hpp"

#include "render.hpp"
#include "capture.hpp"

#include <avrt.h>

#pragma comment(lib, "avrt.lib")

namespace app {
	constexpr DWORD RC_WORKER_THREAD_RESET = 1;
	constexpr DWORD RC_WORKER_THREAD_CLOSE = 2;

	worker_thread::worker_thread()
		: window_(NULL)
		, thread_(NULL)
		, mtx_()
		, cfg_mtx_()
		, stats_mtx_()
		, render_names_()
		, capture_names_()
		, event_close_(NULL)
		, event_reset_(NULL)
		, event_stats_(NULL)
		, stats_total_skip_(0)
		, stats_total_duplicate_(0)
	{
	}

	worker_thread::~worker_thread()
	{
		stop();
		if (event_close_) ::CloseHandle(event_close_);
		if (event_reset_) ::CloseHandle(event_reset_);
		if (event_stats_) ::CloseHandle(event_stats_);
	}

	DWORD WINAPI worker_thread::proc_common(LPVOID _p)
	{
		auto p = reinterpret_cast<worker_thread*>(_p);
		return p->proc();
	}

	DWORD worker_thread::proc_render_and_capture()
	{
		DWORD rc = 0;
		render ren;
		capture cap;
		std::wstring render_name;
		std::wstring capture_name;

		if (ren.init() && cap.init())
		{
			set_render_names(ren.get_names());
			set_capture_names(cap.get_names());

			// 初期化完了を通知
			::PostMessageW(window_, CWM_INIT_COMPLETE, NULL, NULL);

			if (cap.start(get_capture_name()))
			{
				if (ren.start(get_render_name()))
				{

					// 優先度を設定
					DWORD task_index = 0;
					HANDLE task = AvSetMmThreadCharacteristicsW(L"Audio", &task_index);
					if (NULL != task)
					{
						sample_buffer buffer;

						HANDLE events[] = {
							event_close_,
							event_reset_,
							ren.event_,
							cap.event_,
							event_stats_
						};

						while (true)
						{

							auto id = WaitForMultipleObjects(ARRAYSIZE(events), events, FALSE, INFINITE);

							if (id == WAIT_OBJECT_0)
							{
								rc = RC_WORKER_THREAD_CLOSE;
								wlog("close received.");
								break;
							}
							else if (id == WAIT_OBJECT_0 + 1)
							{
								rc = RC_WORKER_THREAD_RESET;
								wlog("reset received.");
								break;
							}
							else if (id == WAIT_OBJECT_0 + 2)
							{
								if (!ren.proc_buffer(buffer))
								{
									wlog("render::proc_buffer() failed.");
								}
							}
							else if (id == WAIT_OBJECT_0 + 3)
							{
								if (!cap.proc_buffer(buffer))
								{
									wlog("capture::proc_buffer() failed.");
								}
							}
							else if (id == WAIT_OBJECT_0 + 4)
							{
								bool changed = false;
								auto sc = buffer.get_skip_count();
								auto dc = buffer.get_duplicate_count();

								{
									std::lock_guard<std::mutex> lock(stats_mtx_);
									if (stats_total_skip_ != sc || stats_total_duplicate_ != dc)
									{
										changed = true;
										stats_total_skip_ = sc;
										stats_total_duplicate_ = dc;
									}
								}
								::PostMessageW(window_, CWM_STATS_UPDATE, NULL, NULL);
								if (changed)
								{
									wlog("skip=" + std::to_string(sc) + " duplicate=" + std::to_string(dc));
								}
							}
						}
						AvRevertMmThreadCharacteristics(task);
					}
					ren.stop();
				}
				cap.stop();
			}
		}
		
		// 意図しない終了だった場合、アクションを待つ
		if (rc != RC_WORKER_THREAD_RESET &&
			rc != RC_WORKER_THREAD_CLOSE)
		{
			HANDLE events[] = {
				event_close_,
				event_reset_
			};

			wlog("wait until reset or close.");

			auto id = WaitForMultipleObjects(ARRAYSIZE(events), events, FALSE, INFINITE);

			if (id == WAIT_OBJECT_0)
			{
				rc = RC_WORKER_THREAD_CLOSE;
				wlog("close received.");
			}
			else if (id == WAIT_OBJECT_0 + 1)
			{
				rc = RC_WORKER_THREAD_RESET;
				wlog("reset received.");
			}
		}

		return rc;
	}

	DWORD worker_thread::proc()
	{
		DWORD rc = 0;

		auto hr = ::CoInitializeEx(0, COINIT_MULTITHREADED);

		while (true)
		{
			rc = proc_render_and_capture();
			if (rc == RC_WORKER_THREAD_RESET) continue;
			break;
		}
		
		::CoUninitialize();

		return rc;
	}

	bool worker_thread::run(HWND _window, const std::wstring &_render_name, const std::wstring &_capture_name)
	{
		window_ = _window;
		render_name_ = _render_name;
		capture_name_ = _capture_name;

		// イベント生成
		if (event_close_ == NULL) event_close_ = ::CreateEventW(NULL, FALSE, FALSE, NULL);
		if (event_close_ == NULL) return false;
		if (event_reset_ == NULL) event_reset_ = ::CreateEventW(NULL, FALSE, FALSE, NULL);
		if (event_reset_ == NULL) return false;
		if (event_stats_ == NULL) event_stats_ = ::CreateEventW(NULL, FALSE, FALSE, NULL);
		if (event_stats_ == NULL) return false;

		// スレッド起動
		thread_ = ::CreateThread(NULL, 0, proc_common, this, 0, NULL);
		return thread_ != NULL;
	}

	void worker_thread::stop()
	{
		if (thread_ != NULL)
		{
			::SetEvent(event_close_);
			::WaitForSingleObject(thread_, INFINITE);
			thread_ = NULL;
		}
	}

	void worker_thread::reset(const std::wstring& _render_name, const std::wstring& _capture_name)
	{
		if (thread_ != NULL)
		{
			{
				std::lock_guard<std::mutex> lock(cfg_mtx_);
				render_name_ = _render_name;
				capture_name_ = _capture_name;
			}
			::SetEvent(event_reset_);
		}
	}

	void worker_thread::stats()
	{
		if (event_stats_)
		{
			::SetEvent(event_stats_);
		}
	}

	std::vector<std::wstring> worker_thread::get_render_names()
	{
		std::lock_guard<std::mutex> lock(cfg_mtx_);
		return render_names_;
	}

	std::vector<std::wstring> worker_thread::get_capture_names()
	{
		std::lock_guard<std::mutex> lock(cfg_mtx_);
		return capture_names_;
	}

	void worker_thread::set_render_names(const std::vector<std::wstring>& _names)
	{
		std::lock_guard<std::mutex> lock(cfg_mtx_);
		render_names_ = _names;
	}

	void worker_thread::set_capture_names(const std::vector<std::wstring>& _names)
	{
		std::lock_guard<std::mutex> lock(cfg_mtx_);
		capture_names_ = _names;
	}

	std::wstring worker_thread::get_render_name()
	{
		std::lock_guard<std::mutex> lock(cfg_mtx_);
		return render_name_;
	}

	std::wstring worker_thread::get_capture_name()
	{
		std::lock_guard<std::mutex> lock(cfg_mtx_);
		return capture_name_;
	}
	
	std::array<UINT64, 2> worker_thread::get_stats()
	{
		std::lock_guard<std::mutex> lock(stats_mtx_);
		return {
			stats_total_skip_,
			stats_total_duplicate_
		};
	}
}
