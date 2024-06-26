﻿#pragma once

#include "common.hpp"

#include <vector>

namespace app
{

	class sample_buffer
	{
	private:
		std::vector<BYTE> buffer_;
		UINT32 last_write_;
		UINT32 last_read_;
		UINT64 skip_count_;
		UINT64 duplicate_count_;
		UINT32 skip_threshold_;
		UINT32 duplicate_threshold_;

	public:
		sample_buffer(UINT32, UINT32, bool);
		~sample_buffer();

		void set(BYTE *, UINT32);
		void get(BYTE *, UINT32);

		UINT64 get_skip_count();
		UINT64 get_duplicate_count();
	};
}
