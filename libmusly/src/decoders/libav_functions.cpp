/**
 * Copyright 2026, Andreas (modified for dynamic loading)
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "libav_functions.h"

#ifdef MUSLY_DECODER_LIBAV

#ifdef _WIN32
#include <windows.h>
#define dlopen(name, flags) LoadLibraryA(name)
#define dlsym(handle, name) GetProcAddress((HMODULE)handle, name)
#define dlclose(handle) FreeLibrary((HMODULE)handle)
#else
#include <dlfcn.h>
#endif

#include <mutex>

#include <vector>
#include <string>


#define BIND(h, name) \
    ptr_ ## name = (decltype(ptr_ ## name))dlsym(h, #name); \
    if (!ptr_ ## name) return;


namespace musly {
namespace decoders {

int (*ptr_av_log_get_level)(void) = nullptr;
void (*ptr_av_log_set_level)(int level) = nullptr;
void (*ptr_av_log_set_callback)(void (*callback)(void*, int, const char*, va_list)) = nullptr;
int (*ptr_avformat_open_input)(AVFormatContext **ps, const char *url, const AVInputFormat *fmt, AVDictionary **options) = nullptr;
int (*ptr_avformat_find_stream_info)(AVFormatContext *ic, AVDictionary **options) = nullptr;
int (*ptr_av_find_best_stream)(AVFormatContext *ic, enum AVMediaType type, int wanted_stream_nb, int related_stream, const AVCodec **decoder_ret, int flags) = nullptr;
AVCodecContext *(*ptr_avcodec_alloc_context3)(const AVCodec *codec) = nullptr;
int (*ptr_avcodec_parameters_to_context)(AVCodecContext *codec, const AVCodecParameters *par) = nullptr;
const AVCodec *(*ptr_avcodec_find_decoder)(enum AVCodecID id) = nullptr;
int (*ptr_avcodec_open2)(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options) = nullptr;
AVFrame *(*ptr_av_frame_alloc)(void) = nullptr;
AVPacket *(*ptr_av_packet_alloc)(void) = nullptr;
int (*ptr_av_get_bytes_per_sample)(enum AVSampleFormat sample_fmt) = nullptr;
int (*ptr_av_sample_fmt_is_planar)(enum AVSampleFormat sample_fmt) = nullptr;
int (*ptr_av_seek_frame)(AVFormatContext *s, int stream_index, int64_t timestamp, int flags) = nullptr;
void (*ptr_avcodec_flush_buffers)(AVCodecContext *avctx) = nullptr;
int (*ptr_av_read_frame)(AVFormatContext *s, AVPacket *pkt) = nullptr;
void (*ptr_av_packet_unref)(AVPacket *pkt) = nullptr;
void (*ptr_av_frame_unref)(AVFrame *frame) = nullptr;
int (*ptr_avcodec_receive_frame)(AVCodecContext *avctx, AVFrame *frame) = nullptr;
int (*ptr_avcodec_send_packet)(AVCodecContext *avctx, const AVPacket *avpkt) = nullptr;
void (*ptr_av_frame_free)(AVFrame **frame) = nullptr;
void (*ptr_avformat_close_input)(AVFormatContext **s) = nullptr;
void (*ptr_av_packet_free)(AVPacket **pkt) = nullptr;
int (*ptr_avcodec_close)(AVCodecContext *avctx) = nullptr;
void (*ptr_avcodec_free_context)(AVCodecContext **avctx) = nullptr;
int64_t (*ptr_av_get_default_channel_layout)(int nb_channels) = nullptr;
struct SwrContext *(*ptr_swr_alloc_set_opts)(struct SwrContext *s, int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate, int64_t in_ch_layout, enum AVSampleFormat in_sample_fmt, int in_sample_rate, int log_offset, void *log_ctx) = nullptr;
int (*ptr_swr_init)(struct SwrContext *s) = nullptr;
void (*ptr_swr_free)(struct SwrContext **s) = nullptr;
int (*ptr_swr_convert)(struct SwrContext *s, uint8_t **out, int out_count, const uint8_t **in, int in_count) = nullptr;

static void* handle_avutil = nullptr;
static void* handle_avcodec = nullptr;
static void* handle_avformat = nullptr;
static void* handle_swresample = nullptr;

static const int supported_avutil_versions[] = { 56, 57, 58, 0 };
static const int supported_avcodec_versions[] = { 58, 59, 60, 0 };
static const int supported_avformat_versions[] = { 58, 59, 60, 0 };
static const int supported_swresample_versions[] = { 3, 4, 0 };

static std::vector<std::string>
get_library_names(const char* libname, const int versions[])
{
    std::vector<std::string> names;
#ifdef _WIN32
    for (int i = 0; versions[i]; ++i) {
        names.push_back(std::string(libname) + "-" + std::to_string(versions[i]) + ".dll");
    }
#elif defined(__APPLE__)
    for (int i = 0; versions[i]; ++i) {
        names.push_back(std::string("lib") + libname + "." + std::to_string(versions[i]) + ".dylib");
    }
    names.push_back(std::string("lib") + libname + ".dylib");
#else
    for (int i = 0; versions[i]; ++i) {
        names.push_back(std::string("lib") + libname + ".so." + std::to_string(versions[i]));
    }
    names.push_back(std::string("lib") + libname + ".so");
#endif
    return names;

}

static void*
load_library(const char* name, const int versions[])
{
    auto names = get_library_names(name, versions);

    void* h = nullptr;
    for (const auto& name : names) {
#ifdef _WIN32
        h = (void*)dlopen(name.c_str(), 0);
#else
        h = dlopen(name.c_str(), RTLD_LAZY | RTLD_GLOBAL);
#endif
        if (h) break;
    }
    return h;
};


static bool load_libav() {
    static std::once_flag flag;
    static bool success = false;

    std::call_once(flag, []() {

        handle_avutil = load_library("avutil", supported_avutil_versions);
        handle_avcodec = load_library("avcodec", supported_avcodec_versions);
        handle_avformat = load_library("avformat", supported_avformat_versions);
        handle_swresample = load_library("swresample", supported_swresample_versions);

        if (!handle_avutil || !handle_avcodec || !handle_avformat || !handle_swresample) return;

        BIND(handle_avutil, av_log_get_level);
        BIND(handle_avutil, av_log_set_level);
        BIND(handle_avutil, av_log_set_callback);
        BIND(handle_avutil, av_get_bytes_per_sample);
        BIND(handle_avutil, av_sample_fmt_is_planar);
        BIND(handle_avutil, av_frame_alloc);
        BIND(handle_avutil, av_frame_unref);
        BIND(handle_avutil, av_frame_free);
        BIND(handle_avutil, av_get_default_channel_layout);

        BIND(handle_avcodec, avcodec_alloc_context3);
        BIND(handle_avcodec, avcodec_parameters_to_context);
        BIND(handle_avcodec, avcodec_find_decoder);
        BIND(handle_avcodec, avcodec_open2);
        BIND(handle_avcodec, av_packet_alloc);
        BIND(handle_avcodec, avcodec_flush_buffers);
        BIND(handle_avcodec, avcodec_receive_frame);
        BIND(handle_avcodec, avcodec_send_packet);
        BIND(handle_avcodec, av_packet_unref);
        BIND(handle_avcodec, av_packet_free);
        BIND(handle_avcodec, avcodec_close);
        BIND(handle_avcodec, avcodec_free_context);

        BIND(handle_avformat, avformat_open_input);
        BIND(handle_avformat, avformat_find_stream_info);
        BIND(handle_avformat, av_find_best_stream);
        BIND(handle_avformat, av_seek_frame);
        BIND(handle_avformat, av_read_frame);
        BIND(handle_avformat, avformat_close_input);

        BIND(handle_swresample, swr_alloc_set_opts);
        BIND(handle_swresample, swr_init);
        BIND(handle_swresample, swr_free);
        BIND(handle_swresample, swr_convert);

        success = true;
    });

    return success;
}

bool libav_is_available() {
    return load_libav();
}

} // namespace decoders
} // namespace musly

#endif // MUSLY_DECODER_LIBAV
