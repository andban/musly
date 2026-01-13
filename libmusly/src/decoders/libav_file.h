#ifndef MUSLY_DECODERS_LIBAV_FILE_H
#define MUSLY_DECODERS_LIBAV_FILE_H

#include <string>

typedef struct AVFormatContext AVFormatContext;
typedef struct AVCodecContext AVCodecContext;
typedef struct AVFrame AVFrame;
typedef struct AVPacket AVPacket;
typedef struct SwrContext SwrContext;

namespace musly::decoders
{
class libav_file
{
public:
    libav_file(const std::string& filename);

    ~libav_file();

    bool
    open();

    float
    duration() const { return _duration; }

    static int
    sample_rate() { return 22050; }

    static int
    channels() { return 1; }

    bool
    seek(float seconds);

    size_t
    read(size_t samples, float* buffer);

private:
    std::string _filename;
    AVCodecContext* _context = nullptr;
    AVFormatContext* _format_context = nullptr;
    AVPacket* _pkt = nullptr;
    AVFrame* _frame = nullptr;
    SwrContext* _swr = nullptr;
    int _stream_idx = -1;
    float _duration = 0;
};
} // musly::decoders

#endif //MUSLY_DECODERS_LIBAV_FILE_H
