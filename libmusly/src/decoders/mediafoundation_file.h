#ifndef MUSLY_DECODERS_MEDIAFOUNDATION_FILE_H_
#define MUSLY_DECODERS_MEDIAFOUNDATION_FILE_H_

#include "decoder.h"

class IMFSourceReader;

namespace musly::decoders
{

class mediafoundation_file : public decoder_file
{
public:
    mediafoundation_file(const std::string& filename);

    ~mediafoundation_file();

    bool
    open() override;

    bool
    seek(float position) override;

    int64_t
    read(size_t samples, float* buffer) override;

    float
    duration() const override
    {
        return _duration;
    }

private:
    IMFSourceReader*  _reader;
    uint32_t          _channels;
    uint32_t          _sample_rate;
    float             _duration;
};

} // namespace musly::decoders

#endif // !MUSLY_DECODERS_MEDIAFOUNDATION_FILE_H_
