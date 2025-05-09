// Copyright 2025 wincrowd

#include "wincrowd/audio_session_controller/audio_session_controller.h"

#include "audiopolicy.h"
#include "mmdeviceapi.h"
#include <atlbase.h>
#include <functional>

namespace wincrowd::audio_session_controller {

//------------------------------------------------------------------------------
template <class T>
void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

//------------------------------------------------------------------------------
static auto createDeviceEnumerator() -> IMMDeviceEnumerator*
{
    IMMDeviceEnumerator* pDeviceEnumerator = NULL;
    // Get the enumerator for the audio endpoint devices.
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&pDeviceEnumerator));

    if (FAILED(hr))
    {
        return NULL;
    }

    return pDeviceEnumerator;
}

//------------------------------------------------------------------------
using FuncAudioSessionControl2 =
    const std::function<bool(IAudioSessionControl2*)>;
static auto iterateAudioSessionControl2s(FuncAudioSessionControl2& func) -> bool
{
    IMMDeviceEnumerator* pDeviceEnumerator = createDeviceEnumerator();
    if (!pDeviceEnumerator)
        return false;

    HRESULT hr                                 = S_OK;
    IMMDevice* pDevice                         = NULL;
    IAudioSessionManager* pAudioSessionManager = NULL;

    hr = pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia,
                                                    &pDevice);
    if (FAILED(hr))
    {
        return false;
    }

    const IID IID_IAudioSessionManager2 = __uuidof(IAudioSessionManager2);
    IAudioSessionManager2* pSManager2   = NULL;
    hr = pDevice->Activate(IID_IAudioSessionManager2, CLSCTX_ALL, NULL,
                           (void**)&pSManager2);
    if (FAILED(hr))
    {
        return false;
    }

    IAudioSessionEnumerator* pSEnumerator = NULL;
    hr = pSManager2->GetSessionEnumerator(&pSEnumerator);
    if (FAILED(hr))
    {
        return false;
    }

    int audioSessionCount = 0;
    hr                    = pSEnumerator->GetCount(&audioSessionCount);
    if (FAILED(hr))
    {
        return false;
    }

    for (int i = 0; i < audioSessionCount; ++i)
    {
        IAudioSessionControl* controls = NULL;
        hr                             = pSEnumerator->GetSession(i, &controls);
        if (FAILED(hr))
        {
            continue;
        }

        CComPtr<IAudioSessionControl2> controls2;
        hr = controls->QueryInterface(IID_PPV_ARGS(&controls2));
        if (FAILED(hr))
        {
            continue;
        }

        hr = controls2->IsSystemSoundsSession();
        if (hr == S_OK)
            continue;

        if (func(controls2))
            return true;
    }

    SafeRelease(&pDeviceEnumerator);

    return true;
}

//------------------------------------------------------------------------
static auto
findAudioSessionControl2(const AudioSessionInfo& info) -> IAudioSessionControl2*
{
    IAudioSessionControl2* control = NULL;
    auto find = [&control, info](IAudioSessionControl2* el) {
        LPWSTR sessionIdentifierStr = NULL;
        HRESULT hr = el->GetSessionInstanceIdentifier(&sessionIdentifierStr);
        if (FAILED(hr))
        {
            return false;
        }

        auto tmp = std::wstring{sessionIdentifierStr};
        if (tmp != info.session_id)
            return false;

        control = el;
        return true;
    };

    auto res = iterateAudioSessionControl2s(find);
    return res ? control : NULL;
}

//------------------------------------------------------------------------
auto collectAudioSessionInfos() -> const AudioSessionInfos
{
    AudioSessionInfos infos;
    auto collect = [&infos](IAudioSessionControl2* el) {
        LPWSTR session_id = NULL;
        HRESULT hr        = el->GetSessionInstanceIdentifier(&session_id);
        if (FAILED(hr))
        {
            false;
        }

        DWORD process_id = 0;
        hr               = el->GetProcessId(&process_id);
        if (FAILED(hr))
        {
            false;
        }

        const AudioSessionInfo info = {
            /* .session_id = */ session_id,
            /* . process_id = */ process_id,
        };

        infos.push_back(info);
        return false;
    };

    iterateAudioSessionControl2s(collect);

    return infos;
}

//------------------------------------------------------------------------
// {2715279F-4139-4ba0-9CB1-B351F1B58A4A}
static const GUID AudioSessionVolumeCtx = {
    0x2715279f,
    0x4139,
    0x4ba0,
    {0x9c, 0xb1, 0xb3, 0x51, 0xf1, 0xb5, 0x8a, 0x4a}};

auto muteAudioSession(const AudioSessionInfo& info, bool state) -> bool
{
    auto control = findAudioSessionControl2(info);
    if (!control)
        return false;

    CComPtr<ISimpleAudioVolume> simple;
    HRESULT hr = control->QueryInterface(IID_PPV_ARGS(&simple));
    if (FAILED(hr))
    {
        return true;
    }

    hr = simple->SetMute(state, &AudioSessionVolumeCtx);
    if (FAILED(hr))
    {
        return true;
    }

    return false;
}

//------------------------------------------------------------------------
} // namespace wincrowd::audio_session_controller