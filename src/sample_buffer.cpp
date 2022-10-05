#include "sample_buffer.hpp"

#include "log.hpp"

namespace
{
	constexpr UINT32 RENDER_LOWER = app::RENDER_SAMPLES / 4 * 1;
	constexpr UINT32 RENDER_HIGHER = app::RENDER_SAMPLES / 4 * 3;

	bool render_lesser(uint64_t _a, uint64_t _b)
	{
		if (_b < RENDER_LOWER && RENDER_HIGHER < _a) return true;
		if (_a < RENDER_LOWER && RENDER_HIGHER < _b) return false;
		return _a < _b;
	}
	bool render_greater(uint64_t _a, uint64_t _b)
	{
		if (_a < RENDER_LOWER && RENDER_HIGHER < _b) return true;
		if (_b < RENDER_LOWER && RENDER_HIGHER < _a) return false;
		return _a > _b;
	}

	INT32 render_diff(uint64_t _a, uint64_t _b)
	{
		if (render_lesser(_a, _b))
		{
			if (_a < _b) return -static_cast<INT32>(_b - _a);
			else return -static_cast<INT32>(_b + app::RENDER_SAMPLES - _a);
		}
		else
		{
			if (_a > _b) return static_cast<INT32>(_a - _b);
			else return static_cast<INT32>(_a + app::RENDER_SAMPLES - _b);
		}
	}
}


namespace app
{

	sample_buffer::sample_buffer(UINT32 _duplicate_threshold, UINT32 _threashold_interval)
		: buffer_(static_cast<size_t>(RENDER_SAMPLES * 8), 0)
		, last_write_(1)
		, last_read_(RENDER_SAMPLES - 480 * 3)
		, skip_count_(0)
		, duplicate_count_(0)
		, skip_threshold_(_duplicate_threshold + _threashold_interval)
		, duplicate_threshold_(_duplicate_threshold)
	{
	}

	sample_buffer::~sample_buffer()
	{
	}

	void sample_buffer::set(BYTE* _data, UINT32 _frames)
	{
		auto m = last_write_;

		if (m + _frames > CAPTURE_SAMPLES)
		{
			auto f2 = (m + _frames) - CAPTURE_SAMPLES;
			auto f1 = _frames - f2;
			std::memcpy(&buffer_.at(m * 4), _data, f1 * 4);
			std::memcpy(&buffer_.at(0), _data + f1 * 4, f2 * 4);
			last_write_ = f2;
		}
		else
		{
			std::memcpy(&buffer_.at(m * 4), _data, _frames * 4);
			last_write_ = m + _frames;
		}
		last_write_ %= CAPTURE_SAMPLES;
	}

	void sample_buffer::get(BYTE* _data, UINT32 _frames)
	{
		auto m = last_read_;
		auto w = last_write_ / 2;
		auto wl = (w + RENDER_SAMPLES - 48 * skip_threshold_) % RENDER_SAMPLES;
		auto wg = (w + RENDER_SAMPLES - 48 * duplicate_threshold_) % RENDER_SAMPLES;
		if (render_lesser((m + _frames) % RENDER_SAMPLES, wl))
		{
			m += 1;
			m %= RENDER_SAMPLES;
			skip_count_++;
		}
		else if (render_greater((m + _frames) % RENDER_SAMPLES, wg))
		{
			m += RENDER_SAMPLES - 1;
			m %= RENDER_SAMPLES;
			duplicate_count_++;
		}

		if (m + _frames > RENDER_SAMPLES)
		{
			auto f2 = (m + _frames) - RENDER_SAMPLES;
			auto f1 = _frames - f2;
			std::memcpy(_data, &buffer_.at(m * 8), f1 * 8);
			std::memcpy(_data + f1 * 8, &buffer_.at(0), f2 * 8);
			last_read_ = f2;
		}
		else
		{
			std::memcpy(_data, &buffer_.at(m * 8), _frames * 8);
			last_read_ = m + _frames;
		}
		last_read_ %= RENDER_SAMPLES;
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
