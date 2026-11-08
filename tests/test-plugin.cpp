// test-plugin: loads the installed libobs without opening OBS, loads aec-mic.dll, creates the source and
// measures the audio it delivers. Phase 1: 4 s with nothing played by the test. Phase 2: a voice file
// played on the default output (it should NOT show up in the source if echo cancellation works).
//
// Run with the OBS binary folder as working directory and in PATH:
//   cd "C:\Program Files\obs-studio\bin\64bit"
//   <repo>\build\test-plugin.exe <aec-mic.dll> <data dir> <voice.wav> [seconds] [out.wav]
//
// Copyright (C) 2026 dosxnjos
// SPDX-License-Identifier: GPL-2.0-or-later
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdarg>
#include <mutex>
#include <vector>
#include <obs.h>
#include <util/base.h>
#include <util/platform.h>

// Declared in obs-module.h (meant for plugins); exported by obs.dll since OBS 32.
extern "C" __declspec(dllimport) const char *obs_get_module_id(obs_module_t *module);
extern "C" __declspec(dllimport) const char *obs_get_module_version(obs_module_t *module);

static void LogHandler(int lvl, const char *fmt, va_list args, void *)
{
	if (lvl > LOG_INFO)
		return;
	char buf[2048];
	vsnprintf(buf, sizeof buf, fmt, args);
	if (strstr(buf, "aec-mic") || lvl <= LOG_WARNING)
		printf("  [log] %s\n", buf);
}

struct Stats {
	std::mutex m;
	double energy = 0;
	uint64_t frames = 0, totalFrames = 0, calls = 0;
	std::vector<float> rec; // channel 0 of everything the source delivered (48 kHz)
};

static void SaveWav(const wchar_t *path, const std::vector<float> &s)
{
	FILE *f = _wfopen(path, L"wb");
	if (!f)
		return;
	uint32_t rate = 48000, bytes = (uint32_t)(s.size() * 4), riff = 36 + bytes, fs = 16, br = rate * 4;
	uint16_t tag = 3, ch = 1, al = 4, bits = 32;
	fwrite("RIFF", 1, 4, f); fwrite(&riff, 4, 1, f); fwrite("WAVEfmt ", 1, 8, f); fwrite(&fs, 4, 1, f);
	fwrite(&tag, 2, 1, f); fwrite(&ch, 2, 1, f); fwrite(&rate, 4, 1, f); fwrite(&br, 4, 1, f);
	fwrite(&al, 2, 1, f); fwrite(&bits, 2, 1, f); fwrite("data", 1, 4, f); fwrite(&bytes, 4, 1, f);
	fwrite(s.data(), 4, s.size(), f);
	fclose(f);
}

static void OnAudio(void *param, obs_source_t *, const struct audio_data *data, bool)
{
	Stats *st = (Stats *)param;
	const float *ch0 = (const float *)data->data[0];
	if (!ch0)
		return;
	double e = 0;
	for (uint32_t i = 0; i < data->frames; i++)
		e += (double)ch0[i] * ch0[i];
	std::lock_guard<std::mutex> lk(st->m);
	st->energy += e;
	st->frames += data->frames;
	st->totalFrames += data->frames;
	st->calls++;
	st->rec.insert(st->rec.end(), ch0, ch0 + data->frames);
}

static double TakeDb(Stats &st, uint64_t &frames)
{
	std::lock_guard<std::mutex> lk(st.m);
	frames = st.frames;
	double db = st.frames ? 10 * log10(std::max(1e-20, st.energy / st.frames)) : -999;
	st.energy = 0;
	st.frames = 0;
	return db;
}

static std::string Utf8(const wchar_t *w)
{
	char b[MAX_PATH * 3];
	WideCharToMultiByte(CP_UTF8, 0, w, -1, b, sizeof b, nullptr, nullptr);
	return b;
}

int wmain(int argc, wchar_t **argv)
{
	if (argc < 4) {
		printf("usage: test-plugin.exe <aec-mic.dll> <data dir> <voice.wav> [seconds] [out.wav]\n");
		return 2;
	}
	const int voiceSecs = argc > 4 ? _wtoi(argv[4]) : 12;
	int failures = 0;
	base_set_log_handler(LogHandler, nullptr);
	if (!obs_startup("pt-BR", nullptr, nullptr)) {
		printf("FAIL obs_startup\n");
		return 1;
	}
	obs_audio_info ai = {48000, SPEAKERS_STEREO};
	if (!obs_reset_audio(&ai)) {
		printf("FAIL obs_reset_audio\n");
		return 1;
	}
	std::string dll = Utf8(argv[1]), data = Utf8(argv[2]);
	obs_module_t *mod = nullptr;
	int r = obs_open_module(&mod, dll.c_str(), data.c_str());
	if (r != MODULE_SUCCESS || !obs_init_module(mod)) {
		printf("FAIL loading the plugin (obs_open_module=%d)\n", r);
		return 1;
	}
	printf("module name: %s\n", obs_get_module_name(mod));
	printf("description: %s\n", obs_get_module_description(mod));
	printf("manifest id/version: %s / %s\n", obs_get_module_id(mod) ? obs_get_module_id(mod) : "(none)",
	       obs_get_module_version(mod) ? obs_get_module_version(mod) : "(none)");
	printf("source name: %s\n", obs_source_get_display_name("aec_comm_mic_input"));

	obs_properties_t *props = obs_get_source_properties("aec_comm_mic_input");
	if (!props) {
		printf("FAIL: source aec_comm_mic_input not registered\n");
		return 1;
	}
	for (const char *k : {"mic_id", "ref_id"}) {
		obs_property_t *p = obs_properties_get(props, k);
		size_t n = obs_property_list_item_count(p);
		printf("list %s (%zu):", k, n);
		for (size_t i = 0; i < n; i++)
			printf(" [%s]", obs_property_list_item_name(p, i));
		printf("\n");
	}
	obs_properties_destroy(props);

	Stats st;
	obs_source_t *src = obs_source_create("aec_comm_mic_input", "test", nullptr, nullptr);
	if (!src) {
		printf("FAIL obs_source_create\n");
		return 1;
	}
	obs_source_add_audio_capture_callback(src, OnAudio, &st);

	uint64_t fr;
	printf("\nphase 1: nothing played by the test (whatever else plays on the PC keeps playing)\n");
	for (int s = 1; s <= 4; s++) {
		Sleep(1000);
		double db = TakeDb(st, fr);
		printf("  s%-2d %7.1f dBFS  (%llu frames)\n", s, db, (unsigned long long)fr);
	}
	printf("phase 2: voice playing on the default output (%d s)\n", voiceSecs);
	PlaySoundW(argv[3], nullptr, SND_FILENAME | SND_ASYNC);
	double worst = -999;
	for (int s = 1; s <= voiceSecs; s++) {
		Sleep(1000);
		double db = TakeDb(st, fr);
		worst = std::max(worst, db);
		printf("  s%-2d %7.1f dBFS  (%llu frames)\n", s, db, (unsigned long long)fr);
		if (fr < 40000)
			failures++;
	}
	PlaySoundW(nullptr, nullptr, 0);
	if (argc > 5) {
		std::lock_guard<std::mutex> lk(st.m);
		SaveWav(argv[5], st.rec);
	}
	printf("loudest second while the voice played: %.1f dBFS (leak if close to the voice level)\n", worst);

	// Change settings on the fly to a missing mic: must fall back to the default and keep delivering.
	obs_data_t *set = obs_data_create();
	obs_data_set_string(set, "mic_id", "does-not-exist");
	obs_source_update(src, set);
	obs_data_release(set);
	Sleep(1500);
	TakeDb(st, fr);
	Sleep(1000);
	TakeDb(st, fr);
	printf("after switching to a missing mic (falls back to default): %llu frames in 1 s\n", (unsigned long long)fr);
	if (fr < 40000)
		failures++;

	obs_source_remove_audio_capture_callback(src, OnAudio, &st);
	ULONGLONG t0 = GetTickCount64();
	obs_source_release(src);
	printf("source released in %llu ms; total frames %llu\n", (unsigned long long)(GetTickCount64() - t0),
	       (unsigned long long)st.totalFrames);
	obs_shutdown();
	printf(failures ? "RESULT: FAIL (%d)\n" : "RESULT: OK\n", failures);
	return failures ? 1 : 0;
}
