#include "capture.hpp"

#include "log.hpp"
#include "mmdevice-utils.hpp"

namespace app {


	capture::capture()
		: devices_()
		, names_()
		, client_(nullptr)
		, capture_client_(nullptr)
		, format_({ WAVE_FORMAT_IEEE_FLOAT, CAPTURE_CHANNELS, CAPTURE_SAMPLES, CAPTURE_SAMPLES * CAPTURE_CHANNELS * 4, CAPTURE_CHANNELS * 4, 32, 0 })
		, buffersize_(0)
		, event_(NULL)
	{
	}

	capture::~capture()
	{
		if (capture_client_) capture_client_->Release();
		if (client_) client_->Release();
		for (auto& device : devices_) device->Release();
		if (event_) ::CloseHandle(event_);
	}

	bool capture::init()
	{
		wlog("capture::init");
		devices_ = get_mmdevices(eCapture, CAPTURE_CHANNELS, 16, CAPTURE_SAMPLES, format_);
		names_ = get_mmdevices_name(devices_);
		event_ = ::CreateEventW(NULL, FALSE, FALSE, NULL);
		if (event_ == NULL)
		{
			wlog("  IAudioClient::GetBufferSize() failed.");
			return false;
		}

		if (devices_.size() == 0)
		{
			wlog("  capture device not found.");
			return false;
		}
		wlog(" success.");
		return true;
	}

	const std::vector<std::wstring> capture::get_names() const
	{
		return names_;
	}

	bool capture::start(const std::wstring &_name)
	{
		wlog("capture::start");    
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
			wlog("  capture device is not matched or None.");
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
		hr = client_->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 0, period_minimum, &format_, 0);
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
		hr = client_->GetService(__uuidof(IAudioCaptureClient), (void**)&capture_client_);
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

	void capture::stop()
	{
		if (capture_client_ && client_)
		{
			client_->Stop();
		}
	}


	bool capture::proc_buffer(sample_buffer &_buffer)
	{
		HRESULT hr;
		UINT32 packetsize;
		UINT32 readed;
		DWORD flags;
		BYTE *data;
		UINT64 pcposition;
		auto blockalign = format_.nBlockAlign;
		hr = capture_client_->GetNextPacketSize(&packetsize);
		if (hr != S_OK)
		{
			return false;
		}

		if (packetsize == 0)
		{
			return false;
		}

		hr = capture_client_->GetBuffer(&data, &readed, &flags, NULL, &pcposition);
		if (hr != S_OK)
		{
			return false;
		}
		_buffer.set(data, readed);

		hr = capture_client_->ReleaseBuffer(readed);
		if (hr != S_OK)
		{
			return false;
		}

		return true;
	}
}
