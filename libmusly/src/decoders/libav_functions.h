/**
 * Copyright 2026, Andreas (modified for dynamic loading)
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MUSLY_DECODERS_LIBAV_FUNCTIONS_H_
#define MUSLY_DECODERS_LIBAV_FUNCTIONS_H_

#ifdef MUSLY_DECODER_LIBAV

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/frame.h>
    #include <libavutil/log.h>
}

namespace musly {
namespace decoders {

bool libav_is_available();

extern int (*ptr_av_log_get_level)(void);
extern void (*ptr_av_log_set_level)(int level);
extern void (*ptr_av_log_set_callback)(void (*callback)(void*, int, const char*, va_list));
extern int (*ptr_avformat_open_input)(AVFormatContext **ps, const char *url, const AVInputFormat *fmt, AVDictionary **options);
extern int (*ptr_avformat_find_stream_info)(AVFormatContext *ic, AVDictionary **options);
extern int (*ptr_av_find_best_stream)(AVFormatContext *ic, enum AVMediaType type, int wanted_stream_nb, int related_stream, const AVCodec **decoder_ret, int flags);
extern AVCodecContext *(*ptr_avcodec_alloc_context3)(const AVCodec *codec);
extern int (*ptr_avcodec_parameters_to_context)(AVCodecContext *codec, const AVCodecParameters *par);
extern const AVCodec *(*ptr_avcodec_find_decoder)(enum AVCodecID id);
extern int (*ptr_avcodec_open2)(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);
extern AVFrame *(*ptr_av_frame_alloc)(void);
extern AVPacket *(*ptr_av_packet_alloc)(void);
extern int (*ptr_av_get_bytes_per_sample)(enum AVSampleFormat sample_fmt);
extern int (*ptr_av_sample_fmt_is_planar)(enum AVSampleFormat sample_fmt);
extern int (*ptr_av_seek_frame)(AVFormatContext *s, int stream_index, int64_t timestamp, int flags);
extern void (*ptr_avcodec_flush_buffers)(AVCodecContext *avctx);
extern int (*ptr_av_read_frame)(AVFormatContext *s, AVPacket *pkt);
extern void (*ptr_av_packet_unref)(AVPacket *pkt);
extern void (*ptr_av_frame_unref)(AVFrame *frame);
extern int (*ptr_avcodec_receive_frame)(AVCodecContext *avctx, AVFrame *frame);
extern int (*ptr_avcodec_send_packet)(AVCodecContext *avctx, const AVPacket *avpkt);
extern void (*ptr_av_frame_free)(AVFrame **frame);
extern void (*ptr_avformat_close_input)(AVFormatContext **s);
extern void (*ptr_av_packet_free)(AVPacket **pkt);
extern int (*ptr_avcodec_close)(AVCodecContext *avctx);
extern void (*ptr_avcodec_free_context)(AVCodecContext **avctx);

// Macros to replace original function names with pointers
#define av_log_get_level ptr_av_log_get_level
#define av_log_set_level ptr_av_log_set_level
#define av_log_set_callback ptr_av_log_set_callback
#define avformat_open_input ptr_avformat_open_input
#define avformat_find_stream_info ptr_avformat_find_stream_info
#define av_find_best_stream ptr_av_find_best_stream
#define avcodec_alloc_context3 ptr_avcodec_alloc_context3
#define avcodec_parameters_to_context ptr_avcodec_parameters_to_context
#define avcodec_find_decoder ptr_avcodec_find_decoder
#define avcodec_open2 ptr_avcodec_open2
#define av_frame_alloc ptr_av_frame_alloc
#define av_packet_alloc ptr_av_packet_alloc
#define av_get_bytes_per_sample ptr_av_get_bytes_per_sample
#define av_sample_fmt_is_planar ptr_av_sample_fmt_is_planar
#define av_seek_frame ptr_av_seek_frame
#define avcodec_flush_buffers ptr_avcodec_flush_buffers
#define av_read_frame ptr_av_read_frame
#define av_packet_unref ptr_av_packet_unref
#define av_frame_unref ptr_av_frame_unref
#define avcodec_receive_frame ptr_avcodec_receive_frame
#define avcodec_send_packet ptr_avcodec_send_packet
#define av_frame_free ptr_av_frame_free
#define avformat_close_input ptr_avformat_close_input
#define av_packet_free ptr_av_packet_free
#define avcodec_close ptr_avcodec_close
#define avcodec_free_context ptr_avcodec_free_context

} // namespace decoders
} // namespace musly

#endif // MUSLY_DECODER_LIBAV

#endif // MUSLY_DECODERS_LIBAV_FUNCTIONS_H_
