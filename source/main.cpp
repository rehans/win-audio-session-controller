// Copyright 2025 wincrowd

#include "wincrowd/audio_session_controller/audio_session_controller.h"
#include <windows.h>

INT WINAPI wWinMain(HINSTANCE hInstance,
                    HINSTANCE /*hPrevInstance*/,
                    LPWSTR /*lpCmdLine*/,
                    INT /*nCmdShow*/)
{
    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    // Initialize the COM library.
    HRESULT hr =
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
    {
        return 0;
    }

    using namespace wincrowd::audio_session_controller;
    auto audioSessionInfos = collectAudioSessionInfos();

    for (const auto& el : audioSessionInfos)
        muteAudioSession(el, true);

    CoUninitialize();
    return 0;
}