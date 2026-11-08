# obs-aec-mic — echo-cancelling microphone for OBS on Windows

[Português](README.pt-BR.md)

Streaming **without headphones**? Your microphone picks up the game and your friends on Discord from
the speakers, so your stream plays them twice: once from desktop audio and once, delayed and echoey,
through your mic.

Voice chat apps solve this with **acoustic echo cancellation (AEC)**: they subtract from the mic what is
playing on the speakers. OBS's microphone source doesn't do that. This plugin adds an audio source
that does, **using the echo cancellation built into Windows**: no extra app, no virtual cable, near-zero
CPU, no GPU requirement.

## How it works

Windows only turns on its echo cancellation (Voice Clarity, or your audio driver's own effects) for apps
that open the mic in the **Communications** processing mode, the one Discord and Teams use. OBS
opens it in the default mode. This plugin opens it in Communications mode, pins the echo reference to
the output you choose, and stops Windows from lowering ("ducking") your other sounds.

Measured on the author's laptop (Intel Smart Sound mic array, Realtek speakers, Windows 11 25H2):

| | AEC | mic level while a voice plays on the speakers |
| --- | --- | --- |
| OBS's own mic source (default mode) | off | -35.7 dBFS |
| this plugin (Communications mode) | on | -90 dBFS (noise floor) |

Your own voice still comes through (with Windows' noise suppression and automatic gain applied).

## Compared to other echo-cancelling options

Each option takes a different route, and the route changes what you have to install and what you can adjust. Figures for the other projects are what their authors report; none were tested here.

| | Uses speaker output as reference? | Runs in / what to install | CPU / GPU cost | Added latency (declared) | Adjustable? |
| --- | --- | --- | --- | --- | --- |
| **This plugin** (Windows Communications mode) | Yes, the output you pick (Windows picks it if the mic doesn't allow choosing) | Windows' audio engine, before OBS gets the signal; nothing extra | ~1% of total CPU in one rough test on the author's laptop; no GPU | Not measured | Only the reference output. Windows also applies its other voice effects (on the test laptop: beamforming, automatic gain, noise suppression), and the app can't turn them off |
| **WebRTC AEC3** as a VST3 ([AEC3-Echo-Canceller](https://github.com/RadegasTCZ/AEC3-Echo-Canceller)) or as an app + virtual cable ([aec-client](https://github.com/samudinzul/aec-client) (AEC3 or neural engines, DTLN by default), [windows-aec-bridge](https://github.com/nVitius/windows-aec-bridge)) | Yes (speaker loopback) | OBS through a VST3 host plugin (atkAudio), or a separate app + virtual cable (e.g. VB-CABLE) | Not published by the projects | ~30 ms reported to the host (VST3) and 30–40 ms (aec-client), per their authors | Yes, depending on the project |
| **Neural filters** ([LocalVQE](https://github.com/localai-org/LocalVQE), [VoiceAec](https://github.com/384615666/VoiceAec)) | Yes, an OBS source (usually Desktop Audio) | OBS filter, on the CPU by default; nothing extra (VoiceAec also has an optional app + virtual cable) | LocalVQE: ~3.2 ms per 16 ms of audio with its default model and 4 threads, or ~1.3 ms with the echo-only v1.4-AEC model on one thread (Ryzen 9 7900, per its README). VoiceAec: ~3–7% per its author | 16 ms algorithmic (LocalVQE, per its README); tens of ms (VoiceAec, per its author) | Yes |
| **NVIDIA Broadcast** | No. NVIDIA's FAQ describes "Room Echo Removal" as removing "echo and reverb that typically occurs in small rooms", with no speaker-reference input | Broadcast app (virtual mic), or OBS's Noise Suppression filter (NVIDIA method) with NVIDIA's Audio Effects SDK | Needs an NVIDIA RTX GPU (RTX 2060 or newer) | Not stated | n/a (no speaker reference) |
| **Noise gate / noise suppression** (OBS's noise gate and noise-suppression filters: RNNoise, Speex) | No | OBS filter; nothing extra | Not compared | Not compared | Yes (thresholds) |

Notes. LocalVQE's README says Windows is "implemented but unverified", and its release ships no binaries (VoiceAec packages a Windows build of the filter). VoiceAec's OBS filter is built with OBS 32.2.2 headers, so it needs OBS 32.2 or newer, per its README (its app + virtual cable route doesn't depend on the OBS version). A noise gate only closes in silence, so the echo leaks through whenever you speak. Noise suppression removes hiss and fans, not voices.

### What you get from Windows' own canceller (when your mic has one)

These apply only if the log shows `echo cancellation: ON` for your mic.

- **The reference comes from inside the audio engine.** Windows mixes what goes to your speakers itself, so its canceller takes the reference there, with no second app and no virtual cable in between. App and filter cancellers capture a loopback copy and estimate the delay themselves (AEC3 does this automatically; some projects also let you set it).
- **It's the canceller your system ships.** It's either Microsoft's Voice Clarity or, on many laptops, the maker's own effects tuned for that mic. On mic arrays it works together with beamforming, which focuses on your voice.
- **Nothing extra to install or route.** You add one OBS source: no separate app, no virtual cable, no GPU. The source follows Windows when the default device changes.
- **Light on the author's laptop.** In one rough test, Windows' audio process (which handles all audio on the PC) went from about 28% to 42% of a single core, out of 14, so roughly 1% of the CPU. Not comparable with the other projects' figures, which come from other machines.
- **It really cancels, it isn't a gate.** The voice from the speakers dropped from -35.7 dBFS (Default mode) to the noise floor (-90 dBFS). With the echo reference pointed at a different output, it came back (-27.9 dBFS). Windows removes what is playing; it doesn't just mute the mic.

Communications mode can make Windows lower other sounds (ducking); the source turns that off. Tested on one laptop (Intel Smart Sound array, Windows 11 25H2) with the level figures above. Talking over the speakers (double-talk) was checked by ear only; latency wasn't measured.

### When another option is better

- **The log says `not reported / off`.** Windows didn't report echo cancellation for that mic, so it probably doesn't offer it (or your Windows version can't tell). This plugin can't add one. Check by ear, and if the echo stays, try an AEC3 or neural option.
- **You want to adjust the canceller.** Other options let you set the echo delay (AEC3-Echo-Canceller, windows-aec-bridge), choose the engine or model (aec-client, LocalVQE, VoiceAec) or set the noise-suppression level (AEC3-Echo-Canceller). Here you only pick the reference output.
- **You want to keep your own voice processing.** Communications mode also turns on Windows' other voice effects, and the app can't turn them off (on the test laptop: automatic gain, noise suppression and beamforming). If you already have a tuned voice chain, an option that only cancels echo fits better.
- **You're not on Windows 11 22H2+.** This plugin needs it. The AEC3 VST3 and aec-client also run on Windows 10, and LocalVQE is tested on Linux.

## Requirements

- Windows 11 22H2 or newer (x64). Choosing the echo reference needs build 22621+.
- A microphone for which Windows (or its driver) offers echo cancellation in Communications mode. Most
  laptops with Windows 11 24H2+ get Microsoft **Voice Clarity**; if not, the log says so (see below).
- OBS Studio 32 or newer.

## Install

1. Download `obs-aec-mic-<version>-windows-x64.zip` from [Releases](../../releases).
2. Close OBS, extract the zip and run `install.bat`. It copies the plugin to
   `C:\ProgramData\obs-studio\plugins\aec-mic` (survives OBS updates). `uninstall.bat` removes it.
3. In OBS, add the source **"Microphone with echo cancellation (Windows)"**.
4. **Turn off your old microphone source** (Settings → Audio → Mic/Auxiliary Audio = Disabled, or remove
   the mic source from your scenes), otherwise your voice is captured twice.

The plugin also appears under **Tools → Plugin Manager**.

### Settings

- **Microphone:** the Windows default communication device, or a specific mic.
- **Cancel the sound playing on:** the output whose sound must be removed from the mic. The game and
  voice chat must play on this same output; sound on another device (e.g. a monitor over HDMI) is not
  cancelled.

### Tips

- Windows Sound settings → *More sound settings* → **Communications** tab → **Do nothing**. The plugin
  already opts out of ducking, but this makes sure nothing else lowers your game audio.
- Keep the speaker volume reasonable: very loud, distorting laptop speakers are harder to cancel.
- Keep Discord on your physical mic: it has its own echo cancellation.

## Is it working?

OBS → **Help → Log Files → View Current Log**, look for:

```
[aec-mic] mic: <your mic> | 48000 Hz, 1 ch | echo cancellation: ON
```

`not reported / off` means Windows doesn't offer echo cancellation for that mic.

## Build

Requirements: Visual Studio Build Tools (C++ workload), Git, OBS installed (or any `obs.dll` x64).

```powershell
./scripts/build.ps1 -Version 0.1.0 -Test
```

It fetches only the `libobs` headers of OBS 32.0.4, generates `obs.lib` from `obs.dll`'s exports and
writes `build/obs-aec-mic-<version>-windows-x64.zip`. Building against 32.0.4 on purpose: OBS refuses
plugins built for a newer libobs, not an older one.

`build/test-plugin.exe` loads the installed libobs **without opening OBS**, loads the plugin, plays a
voice file and measures what the source delivers (see the header of `tests/test-plugin.cpp`).

CI builds every push; tags `v*` publish a release.

## AI disclosure

This plugin was written with AI assistance (Claude Code), under the author's direction. The echo
measurements above were taken on real hardware, the code went through independent reviews, and the
author uses it in his own streams.

## License

GPL-2.0-or-later, like OBS Studio. See [LICENSE](LICENSE).
