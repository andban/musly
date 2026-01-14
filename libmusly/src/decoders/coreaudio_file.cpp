//
// Created by Andreas on 13.01.26.
//

#include "coreaudio_file.h"

#include "minilog.h"

#include <AudioToolbox/AudioToolbox.h>
#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudioTypes.h>

namespace musly::decoders
{
coreaudio_file::coreaudio_file(const std::string& filename) : decoder_file(filename)
{
}

coreaudio_file::~coreaudio_file()
{
    ExtAudioFileDispose(_audio_file);
}

bool
coreaudio_file::open()
{
    if (_audio_file)
    {
        ExtAudioFileDispose(_audio_file);
        _audio_file = nullptr;
    }

    MINILOG(logTRACE) << "coreaudio: Decoding: " << _filename << " started.\n";

    OSStatus err;
    UInt32 parameter_size;

    CFStringRef url_str = CFStringCreateWithCString(kCFAllocatorDefault, _filename.c_str(), kCFStringEncodingUTF8);
    CFURLRef url_ref = CFURLCreateWithFileSystemPath(nullptr, url_str, kCFURLPOSIXPathStyle, false);

    err = ExtAudioFileOpenURL(url_ref, &_audio_file);
    CFRelease(url_ref);
    CFRelease(url_str);;
    if (err != noErr)
    {
        MINILOG(logERROR) << "coreaudio: Could not open file";
        return false;
    }

    AudioStreamBasicDescription input_format;
    parameter_size = sizeof(input_format);
    err = ExtAudioFileGetProperty(_audio_file, kExtAudioFileProperty_FileDataFormat, &parameter_size, &input_format);
    if (err != noErr)
    {
        MINILOG(logERROR) << "coreaudio: Could not get input audio format";
        return false;
    }

    SInt64 total_fames;
    parameter_size = sizeof(total_fames);
    err = ExtAudioFileGetProperty(_audio_file, kExtAudioFileProperty_FileLengthFrames, &parameter_size, &total_fames);
    if (err != noErr)
    {
        MINILOG(logERROR) << "coreaudio: Could not get input audio file length";
        return false;
    }

    AudioStreamBasicDescription output_format = {0};
    output_format.mSampleRate = TARGET_SAMPLE_RATE;
    output_format.mFormatID = kAudioFormatLinearPCM;
    output_format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
    output_format.mChannelsPerFrame = TARGET_CHANNELS;
    output_format.mBytesPerFrame = sizeof(float);
    output_format.mBitsPerChannel = output_format.mBytesPerFrame * 8;
    output_format.mFramesPerPacket = TARGET_CHANNELS;
    output_format.mBytesPerPacket = output_format.mBytesPerFrame * output_format.mFramesPerPacket;

    parameter_size = sizeof(output_format);
    err = ExtAudioFileSetProperty(_audio_file, kExtAudioFileProperty_ClientDataFormat, parameter_size, &output_format);
    if (err != noErr)
    {
        MINILOG(logERROR) << "coreaudio: Could not set output audio format";
        return false;
    }

    AudioConverterRef audio_converter;
    parameter_size = sizeof(AudioConverterRef);
    err = ExtAudioFileGetProperty(_audio_file, kExtAudioFileProperty_AudioConverter, &parameter_size, &audio_converter);
    if (err != noErr)
    {
        MINILOG(logERROR) << "coreaudio: Could not get audio converter";
        return false;
    }

    AudioConverterPrimeInfo prime_info = {0};
    parameter_size = sizeof(AudioConverterPrimeInfo);
    err = AudioConverterGetProperty(audio_converter, kAudioConverterPrimeInfo, &parameter_size, &prime_info);
    if (err != kAudioConverterErr_PropertyNotSupported)
    {
        _offset = prime_info.leadingFrames;
    }

    _duration = static_cast<float>(total_fames) / static_cast<float>(input_format.mSampleRate);
    seek(0.0);

    return true;
}

bool
coreaudio_file::seek(float position)
{
    SInt64 start = static_cast<SInt64>(position * TARGET_SAMPLE_RATE) + _offset;
    if (ExtAudioFileSeek(_audio_file, start) != noErr)
    {
        MINILOG(logERROR) << "coreaudio: failed to seek in audio file";
        return false;
    }

    return true;
}

int64_t
coreaudio_file::read(size_t samples, float* target)
{
    OSStatus err;
    auto buffer = target;

    UInt32 total_frames_to_read = samples;
    UInt32 frames_read = 0;

    UInt32 frames_read_into_buffer = 0;

    while (frames_read < total_frames_to_read)
    {
        UInt32 frames_to_read = total_frames_to_read - frames_read;

        AudioBufferList buffer_list;
        buffer_list.mNumberBuffers = 1;
        buffer_list.mBuffers[0].mNumberChannels = TARGET_CHANNELS;
        buffer_list.mBuffers[0].mDataByteSize = frames_to_read * sizeof(float);
        buffer_list.mBuffers[0].mData = (void*)(&buffer[frames_read]);

        frames_read_into_buffer = frames_to_read;
        err = ExtAudioFileRead(_audio_file, &frames_read_into_buffer, &buffer_list);
        if (err != noErr)
        {
            MINILOG(logERROR) << "coreaudio: failed to read audio file";
            break;
        }
        if (!frames_read_into_buffer)
        {
            break;
        }
        frames_read += frames_read_into_buffer;
    }

    return frames_read;
}

} // musly::decoders
