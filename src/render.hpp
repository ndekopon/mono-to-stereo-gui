#pragma once

#include "common.hpp"

#include "sample_buffer.hpp"

#include <string>

#include <initguid.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>

namespace app
{

	class render
	{
	private:
		std::vector<IMMDevice*> devices_;
		std::vector<std::wstring> names_;
		IAudioClient* client_;
		IAudioRenderClient* render_client_;
		ISimpleAudioVolume* volume_;
		WAVEFORMATEX format_;
		UINT32 buffersize_;
		WORD blockalign_;

		bool get_client_and_volume(const std::wstring&);

	public:
		render();
		~render();

		HANDLE event_;

		// コピー不可
		render(const render&) = delete;
		render& operator = (const render&) = delete;
		// ムーブ不可
		render(render&&) = delete;
		render& operator = (render&&) = delete;

		bool init();
		bool start(const std::wstring&, UINT32);
		void stop();

		bool proc_buffer(sample_buffer &);

		void set_volume(UINT32);

		const std::vector<std::wstring> get_names() const;
	};
}
