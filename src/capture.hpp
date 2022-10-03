#pragma once

#include "common.hpp"

#include "sample_buffer.hpp"

#include <string>

#include <initguid.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>

namespace app
{

	class capture
	{
	private:
		std::vector<IMMDevice*> devices_;
		std::vector<std::wstring> names_;
		IAudioClient* client_;
		IAudioCaptureClient* capture_client_;
		WAVEFORMATEX format_;
		UINT32 buffersize_;

	public:
		capture();
		~capture();

		HANDLE event_;

		// コピー不可
		capture(const capture&) = delete;
		capture& operator = (const capture&) = delete;
		// ムーブ不可
		capture(capture&&) = delete;
		capture& operator = (capture&&) = delete;

		bool init();
		bool start(const std::wstring &);
		void stop();

		bool proc_buffer(sample_buffer &);

		const std::vector<std::wstring> get_names() const;
	};
}
