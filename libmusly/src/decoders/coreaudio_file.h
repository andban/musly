//
// Created by Andreas on 13.01.26.
//

#ifndef MUSLY_DECODERS_COREAUDIO_FILE_H
#define MUSLY_DECODERS_COREAUDIO_FILE_H

#include <decoder.h>
#include <AudioToolbox/AudioToolbox.h>

#include <string>

namespace musly::decoders
{
class coreaudio_file : public decoder_file
{
public:
    coreaudio_file(const std::string& filename);

    ~coreaudio_file() override;

    float
    duration() const override
    {
        return _duration;
    }

    operator ExtAudioFileRef()
    {
        return _audio_file;
    }

    bool
    open() override;

    bool
    seek(float position) override;

    int64_t
    read(size_t samples, float* buffer) override;

private:
    ExtAudioFileRef   _audio_file = nullptr;
    SInt32            _offset = 0;
    float             _duration = 0;
};

} // musly::decoders

#endif //MUSLY_DECODERS_COREAUDIO_FILE_H