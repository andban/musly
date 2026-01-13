#include "mediafoundation_file.h"
#include "minilog.h"

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <propvarutil.h>
#include <algorithm>

namespace {

template<typename T> void releaseHandle(T **handle)
{
    if (*handle)
    {
        (*handle)->Release();
        *handle = nullptr;
    }
}

} // namespace

namespace musly::decoders
{

mediafoundation_file::mediafoundation_file(const std::string& filename)
    : decoder_file(filename)
{
}

mediafoundation_file::~mediafoundation_file()
{
    releaseHandle(&_reader);
}

bool
mediafoundation_file::open()
{
    MINILOG(logTRACE) << "mediafoundation: Decoding " << _filename << " started";

    wchar_t url_str[MAX_PATH + 1];
    MultiByteToWideChar(CP_UTF8, 0, _filename.c_str(), -1, (LPWSTR)url_str, MAX_PATH);

    HRESULT res(S_OK);

    MINILOG(logTRACE) << "mediafoundation: Init COM";
    res = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: Could not initialize COM";
        return false;
    }

    MINILOG(logTRACE) << "mediafoundation: Init MediaFoundation";
    res = MFStartup(MF_VERSION);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: Could not initialize MediaFoundation";
        return false;
    }

    MINILOG(logTRACE) << "mediafoundation: Open file";
    
    res = MFCreateSourceReaderFromURL(url_str, nullptr, &_reader);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: Could not open file";
        return false;
    }

    res = _reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: failure deselecting all streams";
        return false;
    }

    res = _reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: failure selecting first audio stream";
        return false;
    }

    IMFMediaType* source_media_type;
    res = _reader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &source_media_type);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: failure reading source media type";
        return false;
    }

    uint32_t channels;
    res = source_media_type->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: failure reading number of source audio channels";
        return false;
    }

    uint32_t sample_rate;
    res = source_media_type->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sample_rate);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: failure reading source sample rate";
        return false;
    }

    IMFMediaType* target_media_type;
    res = MFCreateMediaType(&target_media_type);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: failure creating target media type";
        return false;
    }

    target_media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    target_media_type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    target_media_type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 22050);
    target_media_type->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE);
    target_media_type->SetUINT32(MF_MT_SAMPLE_SIZE, sizeof(uint16_t));
    target_media_type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 1);
    target_media_type->SetUINT32(MF_MT_AUDIO_CHANNEL_MASK, SPEAKER_FRONT_CENTER);
    
    res = _reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, target_media_type);
    releaseHandle(&target_media_type);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: failure setting target media type";
        return false;
    }

    res = _reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &target_media_type);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: failure re-loading media type";
        return false;
    }

    res = _reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: failure re-setting first audio stream";
        return false;
    }

    PROPVARIANT prop_value;
    res = _reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &prop_value);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: failure getting audio file duration";
        return false;
    }

    _duration = static_cast<float>(prop_value.hVal.QuadPart / 1e7);

    seek(0.0f);

    return true;
}

bool
mediafoundation_file::seek(float position)
{
    int64_t position_nsec = std::max(int64_t(0), (int64_t)(position * 1e7));

    HRESULT res(S_OK);  
    PROPVARIANT prop_value;
    res = InitPropVariantFromInt64(position_nsec, &prop_value);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: Could not initialize seek property value";
        return false;
    }

    res = _reader->Flush(MF_SOURCE_READER_FIRST_AUDIO_STREAM);
    if (FAILED(res))
    {
        MINILOG(logWARNING) << "mediafoundation: Could not flush stream before seeking";
    }

    res = _reader->SetCurrentPosition(GUID_NULL, prop_value);
    if (FAILED(res))
    {
        MINILOG(logERROR) << "mediafoundation: Failure while seeking position in stream";
    }
    PropVariantClear(&prop_value);
    _reader->Flush(MF_SOURCE_READER_FIRST_AUDIO_STREAM);
    return true;
}

int64_t
mediafoundation_file::read(size_t num_samples, float* samples)
{
    HRESULT res(S_OK);
    DWORD flags(0);
    int64_t timestamp;
    IMFSample* sample(nullptr);
    IMFMediaBuffer *buffer(nullptr);

    size_t total_frames_to_read = num_samples;
    size_t frames_to_read = total_frames_to_read;
    size_t frames_read = 0;
    uint16_t* audio_buffer(nullptr);
    size_t audio_buffer_length = 0;

    std::vector<uint16_t> short_samples(total_frames_to_read);

    bool had_error = false;

    while (frames_read < total_frames_to_read)
    {
        frames_to_read  = total_frames_to_read - frames_read;

        res = _reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags, &timestamp, &sample);
        if (FAILED(res))
        {
            MINILOG(logWARNING) << "mediafoundation: failed to read sample";
            had_error = true;
            goto release_sample;

        }
        
        if (flags & MF_SOURCE_READERF_ERROR)
        {
            MINILOG(logERROR) << "mediafoundation: failed to read sample because of reader error";
            had_error = true;
            goto release_sample;
        }
        else if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
        {
            MINILOG(logTRACE) << "mediafoundation: failed to read sample because EOF";
            had_error = true;
        }
        else if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
        {
            MINILOG(logERROR) << "mediafoundation: failed to read sample because format not supported by PCM format";
            had_error = true;
            goto release_sample;
        }
        
        if (sample == nullptr)
        {
            MINILOG(logERROR) << "mediafoundation: failed to read sample because no data was returned";
            break;
        }

        res = sample->ConvertToContiguousBuffer(&buffer);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: failed to convert sample into buffer";
            had_error = true;
            goto release_sample;
        }

        res = buffer->Lock(reinterpret_cast<BYTE**>(&audio_buffer), nullptr, reinterpret_cast<DWORD*>(&audio_buffer_length));
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: failed to read audio data";
            had_error = true;
            goto release_buffer;
        }

        audio_buffer_length /= (sizeof(int16_t));
        if (audio_buffer_length > frames_to_read)
        {
            audio_buffer_length = frames_to_read;
        }

        memcpy(&short_samples[frames_read], audio_buffer, audio_buffer_length * sizeof(int16_t));

        res = buffer->Unlock();
        audio_buffer = nullptr;
        if (FAILED(res))
        {
            had_error = true;
            goto release_buffer;
        }

        frames_read += audio_buffer_length;
release_buffer:
        releaseHandle(&buffer);
release_sample:
        releaseHandle(&sample);

        if (had_error)
        {
            break;
        }
    }

    size_t samples_read = frames_read;
    const int sample_max = std::numeric_limits<int16_t>::max();

    std::vector<float> result(samples_read);
    std::transform(
        short_samples.begin(), 
        short_samples.begin() + samples_read, 
        samples, 
        [=](int16_t sample) { return static_cast<float>(sample) / (float)sample_max; });

    return samples_read;
}

} // namespace musly::decoders
