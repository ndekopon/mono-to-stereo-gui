#include "mmdevice-utils.hpp"

#include <functiondiscoverykeys_devpkey.h>
#include <Audioclient.h> 

namespace
{
	std::wstring get_name(IMMDevice* _device)
	{
		HRESULT hr;
		std::wstring result;

		// プロパティストアを開く
		IPropertyStore* store = NULL;
		hr = _device->OpenPropertyStore(STGM_READ, &store);
		if (hr == S_OK)
		{
			PROPVARIANT v;
			::PropVariantInit(&v);
			hr = store->GetValue(PKEY_Device_FriendlyName, &v);
			if (SUCCEEDED(hr))
			{
				if (v.vt == VT_LPWSTR)
				{
					result = v.pwszVal;
				}
			}
			::PropVariantClear(&v);
		}
		store->Release();

		return result;
	}

	std::vector<IMMDevice*> get_all_mmdevices(EDataFlow _dataflow)
	{
		HRESULT hr;
		std::vector<IMMDevice*> devices;
		IMMDeviceEnumerator* enumrator = NULL;
		IMMDeviceCollection* collection = NULL;

		hr = ::CoCreateInstance(__uuidof(::MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&enumrator));
		if (hr != S_OK)
		{
			return devices;
		}

		// コレクション取得
		hr = enumrator->EnumAudioEndpoints(_dataflow, DEVICE_STATE_ACTIVE, &collection);
		if (hr == S_OK)
		{
			UINT count = 0;
			hr = collection->GetCount(&count);
			if (hr == S_OK && count > 0)
			{
				for (ULONG i = 0; i < count; i++)
				{
					IMMDevice* device = NULL;
					hr = collection->Item(i, &device);
					if (hr == S_OK)
					{
						devices.push_back(device);
					}
				}
			}
			collection->Release();
		}
		enumrator->Release();
		return devices;
	}

	bool is_format_supported(IMMDevice *_device, const WAVEFORMATEX& _format)
	{
		bool rc = false;
		HRESULT hr;
		IAudioClient* client = NULL;
		hr = _device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&client);
		if (hr == S_OK)
		{
			WAVEFORMATEX* closestmatch_format = NULL;
			hr = client->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &_format, &closestmatch_format);
			if (closestmatch_format) ::CoTaskMemFree(closestmatch_format);
			if (hr == S_OK) rc = true;
			client->Release();
		}
		return rc;
	}

	bool is_deviceformat_matched(IMMDevice *_device, WORD _channel, WORD _bits_per_sample, DWORD _samples_per_sec)
	{
		bool rc = false;
		HRESULT hr;
		IPropertyStore* store = NULL;
		// プロパティストアを開く
		hr = _device->OpenPropertyStore(STGM_READ, &store);
		if (hr == S_OK)
		{
			PROPVARIANT v;
			::PropVariantInit(&v);
			hr = store->GetValue(PKEY_AudioEngine_DeviceFormat, &v);
			if (SUCCEEDED(hr) && v.vt == VT_BLOB)
			{
				auto format = reinterpret_cast<PWAVEFORMATEX>(v.blob.pBlobData);
				if (format->nChannels == _channel &&
					format->wBitsPerSample == _bits_per_sample &&
					format->nSamplesPerSec == _samples_per_sec)
				{
					rc = true;
				}
			}
			::PropVariantClear(&v);
		}
		store->Release();

		return rc;
	}
}

namespace app
{

	std::vector<IMMDevice*> get_mmdevices(EDataFlow _dataflow, WORD _channel, WORD _bits_per_sample, DWORD _samples_per_sec, const WAVEFORMATEX& _format)
	{
		HRESULT hr;
		std::vector<IMMDevice*> devices;

		for (auto device : get_all_mmdevices(_dataflow))
		{
			if (is_format_supported(device, _format))
			{
				if (_dataflow == eRender)
				{
					devices.push_back(device);
				}
				else if (_dataflow == eCapture)
				{
					if (is_deviceformat_matched(device, _channel, _bits_per_sample, _samples_per_sec))
					{
						devices.push_back(device);
					}
				}
			}

			if (std::find(devices.begin(), devices.end(), device) == devices.end())
			{
				device->Release();
			}
		}
		return devices;
	}

	std::vector<std::wstring> get_mmdevices_name(const std::vector<IMMDevice*>& _devices)
	{
		std::vector<std::wstring> names;

		// 名前の取得
		for (auto device : _devices)
		{
			auto name = get_name(device);
			names.push_back(name);
		}

		return names;
	}
}
