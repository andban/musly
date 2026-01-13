#include "libav_file.h"
#include "libav_functions.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
}

namespace musly::decoders
{
libav_file::libav_file(const std::string& filename) : decoder_file(filename)
{
}

libav_file::~libav_file()
{
    if (_swr)
    {
        swr_free(&_swr);
    }

    av_packet_free(&_pkt);
    av_frame_free(&_frame);

    if (_context)
    {
        avcodec_close(_context);
        avcodec_free_context(&_context);
    }
    if (_format_context)
    {
        avformat_close_input(&_format_context);
    }
}

bool
libav_file::open()
{
    if (avformat_open_input(&_format_context, _filename.c_str(), nullptr, nullptr) < 0)
    {
        return false;
    }
    if (avformat_find_stream_info(_format_context, nullptr) < 0)
    {
        return false;
    }

    _stream_idx = av_find_best_stream(_format_context, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (_stream_idx < 0)
    {
        return false;
    }

    const AVStream* stream = _format_context->streams[_stream_idx];
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder)
    {
        return false;
    }

    _context = avcodec_alloc_context3(decoder);
    if (!_context)
    {
        return false;
    }
    if (avcodec_parameters_to_context(_context, stream->codecpar) < 0)
    {
        return false;
    }
    if (avcodec_open2(_context, decoder, nullptr) < 0)
    {
        return false;
    }

    if (stream->duration != AV_NOPTS_VALUE)
    {
        _duration = (float)stream->duration * stream->time_base.num / stream->time_base.den;
    }

    _frame = av_frame_alloc();
    _pkt = av_packet_alloc();

    _swr = swr_alloc_set_opts(nullptr,
                              AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_FLT, 22050,
                              av_get_default_channel_layout(_context->channels), _context->sample_fmt,
                              _context->sample_rate,
                              0, nullptr);

    if (!_swr || swr_init(_swr) < 0)
    {
        return false;
    }

    return true;
}

bool
libav_file::seek(float seconds)
{
    AVStream* st = _format_context->streams[_stream_idx];
    if (av_seek_frame(_format_context, _stream_idx, seconds * st->time_base.den / st->time_base.num,
                      AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY) < 0)
    {
        return false;
    }
    avcodec_flush_buffers(_context);
    return true;
}

int64_t
libav_file::read(size_t samples, float* buffer)
{
    size_t samples_read = 0;
    int avret;

    while (samples_read < samples)
    {
        if ((avret = av_read_frame(_format_context, _pkt)) < 0) break;
        if (_pkt->stream_index != _stream_idx)
        {
            av_packet_unref(_pkt);
            continue;
        }

        avret = avcodec_send_packet(_context, _pkt);
        while (avret >= 0 && samples_read < samples)
        {
            avret = avcodec_receive_frame(_context, _frame);
            if (avret == AVERROR(EAGAIN) || avret == AVERROR_EOF) break;
            if (avret < 0) return samples_read;

            float* out[] = {buffer + samples_read};
            int out_samples = std::min((int)(samples - samples_read), _frame->nb_samples);
            int converted = swr_convert(_swr, (uint8_t**)out, out_samples, (const uint8_t**)_frame->data,
                                        _frame->nb_samples);

            if (converted > 0)
            {
                samples_read += converted;
            }
        }
        av_packet_unref(_pkt);
    }
    return samples_read;
}
} // musly::decoders
