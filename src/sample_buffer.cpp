#include "sample_buffer.hpp"

#include "log.hpp"

namespace
{
	bool render_lesser(uint64_t _a, uint64_t _b)
	{
		if (_b < 12000 && 36000 < _a) return true;
		if (_a < 12000 && 36000 < _b) return false;
		return _a < _b;
	}
	bool render_greater(uint64_t _a, uint64_t _b)
	{
		if (_a < 12000 && 36000 < _b) return true;
		if (_b < 12000 && 36000 < _a) return false;
		return _a > _b;
	}

	INT32 render_diff(uint64_t _a, uint64_t _b)
	{
		if (render_lesser(_a, _b))
		{
			if (_a < _b) return -static_cast<INT32>(_b - _a);
			else return -static_cast<INT32>(_b + 48000 - _a);
		}
		else
		{
			if (_a > _b) return static_cast<INT32>(_a - _b);
			else return static_cast<INT32>(_a + 48000 - _b);
		}
	}
}


namespace app
{

	sample_buffer::sample_buffer()
		: buffer_(static_cast<size_t>(48000 * 8), 0)
		, last_write_(1)
		, last_read_(48000 - 480 * 3)
		, skip_count_(0)
		, duplicate_count_(0)
	{
	}

	sample_buffer::~sample_buffer()
	{
	}

	void sample_buffer::set(BYTE* _data, UINT32 _frames)
	{
		auto m = last_write_;

		if (m + _frames > 96000)
		{
			auto f2 = (m + _frames) - 96000;
			auto f1 = _frames - f2;
			std::memcpy(&buffer_.at(m * 4), _data, f1 * 4);
			std::memcpy(&buffer_.at(0), _data + f1 * 4, f2 * 4);
			last_write_ = f2;
			last_write_ %= 96000;
		}
		else
		{
			std::memcpy(&buffer_.at(m * 4), _data, _frames * 4);
			last_write_ = m + _frames;
			last_write_ %= 96000;
		}
	}

	void sample_buffer::get(BYTE* _data, UINT32 _frames)
	{
		auto m = last_read_;
		auto w = last_write_ / 2;
		auto wl = (w + 48000 - 960) % 48000;
		auto wg = (w + 48000 - 480) % 48000;
		if (render_lesser((m + _frames) % 48000, wl))
		{
			m += 1;
			m %= 48000;
			skip_count_++;
		}
		else if (render_greater((m + _frames) % 48000, wg))
		{
			m += 48000 - 1;
			m %= 48000;
			duplicate_count_++;
		}

		if (m + _frames > 48000)
		{
			auto f2 = (m + _frames) - 48000;
			auto f1 = _frames - f2;
			std::memcpy(_data, &buffer_.at(m * 8), f1 * 8);
			std::memcpy(_data + f1 * 8, &buffer_.at(0), f2 * 8);
			last_read_ = f2;
			last_read_ %= 48000;
		}
		else
		{
			std::memcpy(_data, &buffer_.at(m * 8), _frames * 8);
			last_read_ = m + _frames;
			last_read_ %= 48000;
		}
	}

	UINT64 sample_buffer::get_skip_count()
	{
		return skip_count_;
	}

	UINT64 sample_buffer::get_duplicate_count()
	{
		return duplicate_count_;
	}
}
