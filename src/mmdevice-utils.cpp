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
}

namespace app
{

	std::vector<IMMDevice*> get_mmdevices(EDataFlow _dataflow, WORD _channel, WORD _bits_per_sample, DWORD _samples_per_sec, const WAVEFORMATEX& _format)
	{
		std::vector<IMMDevice*> devices;
		HRESULT hr;
		IMMDeviceEnumerator* enumrator = NULL;

		hr = ::CoCreateInstance(__uuidof(::MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&enumrator));
		if (hr == S_OK)
		{
			// キャプチャーデバイスを列挙
			IMMDeviceCollection* collection = NULL;
			hr = enumrator->EnumAudioEndpoints(_dataflow, DEVICE_STATE_ACTIVE, &collection);
			if (hr == S_OK)
			{
				UINT count = 0;
				hr = collection->GetCount(&count);
				if (!FAILED(hr) && count > 0)
				{
					for (ULONG i = 0; i < count; i++)
					{
						IMMDevice* device = NULL;
						bool matched = false;
						hr = collection->Item(i, &device);
						if (hr == S_OK)
						{
							// デバイスがフォーマットをサポートしているか
							IAudioClient* client = NULL;
							hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&client);
							if (hr == S_OK)
							{
								WAVEFORMATEX *closestmatch_format = NULL;
								hr = client->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &_format, &closestmatch_format);
								if (closestmatch_format) ::CoTaskMemFree(closestmatch_format);
								if (hr == S_OK)
								{

									if (_dataflow == eRender)
									{
										devices.push_back(device);
										matched = true;
									}
									else if (_dataflow == eCapture)
									{
										// プロパティストアを開く
										IPropertyStore* store = NULL;
										hr = device->OpenPropertyStore(STGM_READ, &store);
										if (hr == S_OK)
										{
											PROPVARIANT v;
											::PropVariantInit(&v);
											hr = store->GetValue(PKEY_AudioEngine_DeviceFormat, &v);
											if (SUCCEEDED(hr))
											{
												auto format = reinterpret_cast<PWAVEFORMATEX>(v.blob.pBlobData);
												if (format->nChannels == _channel &&
													format->wBitsPerSample == _bits_per_sample &&
													format->nSamplesPerSec == _samples_per_sec)
												{
													devices.push_back(device);
													matched = true;
												}
											}
											::PropVariantClear(&v);
										}
										store->Release();
									}
								}
								client->Release();
							}



							// 一致しなかったデバイスは解放
							if (!matched) device->Release();
						}
					}
				}
				if (collection) collection->Release();
			}
			if (enumrator) enumrator->Release();
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
