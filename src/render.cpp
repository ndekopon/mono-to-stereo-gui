#include "render.hpp"

#include "log.hpp"
#include "mmdevice-utils.hpp"

namespace app {


	render::render()
		: devices_()
		, names_()
		, client_(nullptr)
		, render_client_(nullptr)
		, format_({ WAVE_FORMAT_IEEE_FLOAT, 2, 48000, 384000, 8, 32, 0 })
		, event_(NULL)
		, buffersize_(0)
		, blockalign_(0)
	{
	}

	render::~render()
	{
		if (render_client_) render_client_->Release();
		if (client_) client_->Release();
		for (auto& device : devices_) device->Release();
		if (event_) ::CloseHandle(event_);
	}

	bool render::init()
	{
		wlog("render::init");
		devices_ = get_mmdevices(eRender, 2, 16, 48000, format_);
		names_ = get_mmdevices_name(devices_);
		event_ = ::CreateEventW(NULL, FALSE, FALSE, NULL);
		if (event_ == NULL)
		{
			wlog("  CreateEvent() failed.");
			return false;
		}

		if (devices_.size() == 0)
		{
			wlog("  render device not found.");
			return false;
		}
		wlog("  success.");
		return true;
	}

	const std::vector<std::wstring> render::get_names() const
	{
		return names_;
	}

	bool render::start(const std::wstring& _name)
	{
		wlog("render::start");
		int index = -1;
		for (size_t i = 0; i < names_.size(); ++i)
		{
			if (names_.at(i) == _name)
			{
				wlog(L"  Name: " + _name);
				index = i;
				break;
			}
		}
		if (index == -1)
		{
			wlog("  render device is not matched or None.");
			return false;
		}

		HRESULT hr;
		hr = devices_.at(index)->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&client_);
		if (hr != S_OK)
		{
			wlog("  IMMDevice::Activate() failed.");
			return false;
		}

		// Period取得
		REFERENCE_TIME period_default;
		REFERENCE_TIME period_minimum;
		hr = client_->GetDevicePeriod(&period_default, &period_minimum);
		if (hr != S_OK)
		{
			wlog("  IAudioClient::GetDevicePeriod() failed.");
			return false;
		}
		wlog("  DefaultDevicePeriod: " + std::to_string(period_default));
		wlog("  MinimumDevicePeriod: " + std::to_string(period_minimum));

		// 初期化
		hr = client_->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 0, 0, &format_, 0);
		if (hr != S_OK)
		{
			wlog("  IAudioClient::Initialize() failed.");
			return false;
		}

		// イベントハンドル関連付け
		hr = client_->SetEventHandle(event_);
		if (hr != S_OK)
		{
			wlog("  IAudioClient::SetEventHandle() failed.");
			return false;
		}

		// バッファサイズの取得
		hr = client_->GetBufferSize(&buffersize_);
		if (hr != S_OK)
		{
			wlog("  IAudioClient::GetBufferSize() failed.");
			return false;
		}
		wlog("  BufferSize: " + std::to_string(buffersize_));

		// IAudioCaptureClient取得
		hr = client_->GetService(__uuidof(IAudioRenderClient), (void**)&render_client_);
		if (hr != S_OK)
		{
			wlog("  IAudioClient::GetService() failed.");
			return false;
		}

		// 開始
		hr = client_->Start();
		if (hr != S_OK)
		{
			wlog("  IAudioClient::Start() failed.");
			false;
		}

		wlog("  started.");
		return true;
	}

	void render::stop()
	{
		if (render_client_ && client_)
		{
			client_->Stop();
		}
	}


	bool render::proc_buffer(sample_buffer &_buffer)
	{
		HRESULT hr;
		UINT32 readed;
		UINT32 padding;
		UINT32 available;
		DWORD flags;
		BYTE* data;

		hr = client_->GetCurrentPadding(&padding);
		if (hr != S_OK)
		{
			return false;
		}
		available = buffersize_ - padding;
		hr = render_client_->GetBuffer(available, &data);
		if (hr != S_OK)
		{
			return false;
		}

		_buffer.get(data, available);

		hr = render_client_->ReleaseBuffer(available, 0);
		if (hr != S_OK)
		{
			return false;
		}

		return true;
	}
}
