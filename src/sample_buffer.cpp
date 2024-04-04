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
}


namespace app
{

	sample_buffer::sample_buffer(UINT32 _duplicate_threshold, UINT32 _threashold_interval, bool _reverse_channel)
		: buffer_(static_cast<size_t>(RENDER_SAMPLES * 8), 0)
		, last_write_(_reverse_channel ? 0 : 1)
		, last_read_(RENDER_SAMPLES - (RENDER_SAMPLES / 1000) * 30) // 初回は30ms遅延入れておく
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
		const auto w = last_write_ / 2;
		const auto w_s = (w + RENDER_SAMPLES - (RENDER_SAMPLES / 1000) * skip_threshold_) % RENDER_SAMPLES;
		const auto w_d = (w + RENDER_SAMPLES - (RENDER_SAMPLES / 1000) * duplicate_threshold_) % RENDER_SAMPLES;
		const auto next_m = (m + _frames) % RENDER_SAMPLES;

		if (w_s < w_d)
		{
			if (render_lesser(next_m, w_s))
			{
				m += 1;
				m %= RENDER_SAMPLES;
				skip_count_++;
			}
			else if (render_greater(next_m, w_d))
			{
				m += RENDER_SAMPLES - 1;
				m %= RENDER_SAMPLES;
				duplicate_count_++;
			}
		}
		else if (w_d < w_s)
		{
			if (render_greater(next_m, w_d) && render_lesser(next_m, w_s))
			{
				const auto a = w_s - next_m;
				const auto b = next_m - w_d;
				if (a > b)
				{
					m += RENDER_SAMPLES - 1;
					m %= RENDER_SAMPLES;
					duplicate_count_++;
				}
				else
				{
					m += 1;
					m %= RENDER_SAMPLES;
					skip_count_++;
				}
			}
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
