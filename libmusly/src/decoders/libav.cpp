/**
 * Copyright 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
 *           2014-2016, Jan Schlueter <jan.schlueter@ofai.at>
 *
 * This file is part of Musly, a program for high performance music
 * similarity computation: http://www.musly.org/.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include <vector>
#include <algorithm>

#include "libav_functions.h"

#ifndef MUSLY_DECODER_LIBAV
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/channel_layout.h>
    #include <libswresample/swresample.h>
}
#endif

#include "minilog.h"
#include "resampler.h"
#include "libav.h"


namespace musly {
namespace decoders {

#ifdef MUSLY_DECODER_LIBAV
MUSLY_DECODER_REGIMPL_DYNAMIC(libav, 0, libav_is_available);

class libav_file {
public:
    libav_file(const std::string& filename) : _filename(filename) {}

    ~libav_file() {
        if (_swr) swr_free(&_swr);
        if (_pkt) av_packet_free(&_pkt);
        if (_frame) av_frame_free(&_frame);
        if (_decx) {
            avcodec_close(_decx);
            avcodec_free_context(&_decx);
        }
        if (_fmtx) avformat_close_input(&_fmtx);
    }

    bool open() {
        if (avformat_open_input(&_fmtx, _filename.c_str(), nullptr, nullptr) < 0) return false;
        if (avformat_find_stream_info(_fmtx, nullptr) < 0) return false;

        _stream_idx = av_find_best_stream(_fmtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
        if (_stream_idx < 0) return false;

        AVStream *st = _fmtx->streams[_stream_idx];
        const AVCodec *dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) return false;

        _decx = avcodec_alloc_context3(dec);
        if (!_decx) return false;
        if (avcodec_parameters_to_context(_decx, st->codecpar) < 0) return false;

        if (avcodec_open2(_decx, dec, nullptr) < 0) return false;

        _channels = _decx->channels;
        _sample_rate = _decx->sample_rate;

        if (st->duration != AV_NOPTS_VALUE) {
            _duration = (float)st->duration * st->time_base.num / st->time_base.den;
        }

        _frame = av_frame_alloc();
        _pkt = av_packet_alloc();

        _swr = swr_alloc_set_opts(nullptr,
                                  AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_FLT, _sample_rate,
                                  av_get_default_channel_layout(_channels), _decx->sample_fmt, _sample_rate,
                                  0, nullptr);
        if (!_swr || swr_init(_swr) < 0) return false;

        return true;
    }

    float duration() const { return _duration; }
    int sample_rate() const { return _sample_rate; }
    int channels() const { return 1; }

    bool seek(float seconds) {
        AVStream *st = _fmtx->streams[_stream_idx];
        if (av_seek_frame(_fmtx, _stream_idx, seconds * st->time_base.den / st->time_base.num, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY) < 0) {
            return false;
        }
        avcodec_flush_buffers(_decx);
        return true;
    }

    size_t read(size_t samples, float* buffer) {
        size_t samples_read = 0;
        int avret;

        while (samples_read < samples) {
            if ((avret = av_read_frame(_fmtx, _pkt)) < 0) break;
            if (_pkt->stream_index != _stream_idx) {
                av_packet_unref(_pkt);
                continue;
            }

            avret = avcodec_send_packet(_decx, _pkt);
            while (avret >= 0 && samples_read < samples) {
                avret = avcodec_receive_frame(_decx, _frame);
                if (avret == AVERROR(EAGAIN) || avret == AVERROR_EOF) break;
                if (avret < 0) return samples_read;

                float* out[] = { buffer + samples_read };
                int out_samples = std::min((int)(samples - samples_read), _frame->nb_samples);
                int converted = swr_convert(_swr, (uint8_t**)out, out_samples, (const uint8_t**)_frame->data, _frame->nb_samples);

                if (converted > 0) {
                    samples_read += converted;
                }
            }
            av_packet_unref(_pkt);
        }
        return samples_read;
    }

private:
    std::string _filename;
    AVFormatContext* _fmtx = nullptr;
    AVCodecContext* _decx = nullptr;
    AVFrame* _frame = nullptr;
    AVPacket* _pkt = nullptr;
    SwrContext* _swr = nullptr;
    int _stream_idx = -1;
    int _channels = 0;
    int _sample_rate = 0;
    float _duration = 0;
};
#endif

libav::libav()
{
}

void libav_log_callback(void *ptr, int level, const char *fmt, va_list vargs)
{
    if (level <= av_log_get_level()) {
        vfprintf(FileLogger::get_stream(), fmt, vargs);
    }
}

std::vector<float>
libav::decodeto_22050hz_mono_float(
        const std::string& file,
        float excerpt_length,
        float excerpt_start)
{
    MINILOG(logTRACE) << "Decoding: " << file << " started.";

    // show libav messages only in verbose mode
    if (MiniLog::current_level() >= logTRACE) {
        av_log_set_level(AV_LOG_VERBOSE);
        av_log_set_callback(libav_log_callback);
    }
    else {
        av_log_set_level(AV_LOG_PANIC);
    }

    libav_file audio_file(file);
    if (!audio_file.open()) {
        MINILOG(logERROR) << "Could not open file: " << file;
        return std::vector<float>(0);
    }

    MINILOG(logDEBUG) << "libav: Audio file duration : " << audio_file.duration() << " seconds";

    if (excerpt_length <= 0 || excerpt_length > audio_file.duration()) {
        excerpt_start = 0;
        excerpt_length = audio_file.duration();
    } else if (excerpt_start < 0) {
        excerpt_start = std::min(-excerpt_start, (audio_file.duration() - excerpt_length) / 2);
    } else if (excerpt_length + excerpt_start > audio_file.duration()) {
        excerpt_start = std::max(0.0f, audio_file.duration() - excerpt_length);
        excerpt_length = std::min(audio_file.duration(), audio_file.duration() - excerpt_start);
    }

    MINILOG(logDEBUG) << "libav: Will decode from " << excerpt_start << " to " << (excerpt_start + excerpt_length);
    audio_file.seek(excerpt_start);

    size_t samples_to_read = excerpt_length * audio_file.sample_rate();
    std::vector<float> pcm(samples_to_read);
    size_t samples_read = audio_file.read(samples_to_read, pcm.data());

    if (samples_read < samples_to_read) {
        pcm.resize(samples_read);
    }

    if (audio_file.sample_rate() != 22050) {
        MINILOG(logDEBUG) << "libav: Will downsample audio from " << audio_file.sample_rate() << "Hz to 22050Hz";
        resampler r(audio_file.sample_rate(), 22050);
        return r.resample(pcm.data(), pcm.size());
    }

    MINILOG(logTRACE) << "Decoding: " << file << " finalized.";
    return pcm;
}

} /* namespace decoders */
} /* namespace musly */
