#pragma once

#include "common.hpp"

#include <string>

namespace app
{

	class config_ini
	{
	private:
		std::wstring path_;

		bool set_value(const std::wstring& _key, const std::wstring& _value);
		std::wstring get_value(const std::wstring& _key);
		UINT get_intvalue(const std::wstring& _key);

	public:
		config_ini();
		~config_ini();

		bool set_render_device(const std::wstring& _device);
		std::wstring get_render_device();

		bool set_capture_device(const std::wstring& _device);
		std::wstring get_capture_device();
	};
}
