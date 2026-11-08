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

## License

GPL-2.0-or-later, like OBS Studio. See [LICENSE](LICENSE).
