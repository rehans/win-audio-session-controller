// Copyright 2025 wincrowd

#pragma once

#include <string>
#include <vector>

namespace wincrowd::audio_session_controller {

//------------------------------------------------------------------------
struct AudioSessionInfo
{
    using AudioSessionIdentifier = std::wstring;
    using ProcessID              = long long; // DWORD

    AudioSessionIdentifier session_id;
    ProcessID process_id;
};

using AudioSessionInfos = std::vector<AudioSessionInfo>;
auto collectAudioSessionInfos() -> const AudioSessionInfos;
auto muteAudioSession(const AudioSessionInfo& info, bool state) -> bool;

//------------------------------------------------------------------------
} // namespace wincrowd::audio_session_controller