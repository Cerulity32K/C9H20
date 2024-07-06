/// 
/// \/\/\/\/ Nonane (C9H20) \/\/\/\/
///	 -=* Created by Cerulity32K *=-
/// 
/// A small program meant to fiddle around and screw with the capabilities of WinGDI and WaveOut.
/// 
/// Heavily inspired by Trichloromethane.exe (aka Chloroform.exe), MEMZ, and similar malware.
/// 
/// While these programs are malware (as they overwrite the MBR), this is not.
/// This program is instead just meant to look cool and be what older programs used to be.
/// Although visually and audibly invasive, this program, ***as far as i know***, is completely safe to run.
/// 
/// Although this is a C++ program, it is styled and structured more in the direction of
/// how a C program would be structured, given the low-level nature of the program.
/// 

#pragma region Preprocessor
#include <iostream>
#include <algorithm>
#include <vector>

#include <windows.h>
#pragma endregion

#pragma region Globals
const int screen_width = GetSystemMetrics(0);
const int screen_height = GetSystemMetrics(1);

// Some pre-chosen spam texts.
const char* TEXT_SPAM[] = {
	":3 :3 :3 :3 :3 :3 :3 :3",
	"cerulity32k.github.io",
	"in a trash world, you recycle",
	"meow meow meow meow meow meow meow",
	"who said malware cant be fun?",
	"visual c++? yeah this is pretty visible",
	"go make something, be creative <3",
};
const size_t TEXT_SPAM_COUNT = sizeof(TEXT_SPAM) / sizeof(const char*);
// A pool of icons to rendomly pick from.
static HICON ICON_POOL[] = {
	LoadIcon(nullptr, IDI_WARNING),
	LoadIcon(nullptr, IDI_ERROR),
	LoadIcon(nullptr, IDI_SHIELD),
	LoadIcon(nullptr, IDI_APPLICATION),
	LoadIcon(nullptr, IDI_HAND),
	LoadIcon(nullptr, IDI_INFORMATION),
	LoadIcon(nullptr, IDI_QUESTION),
};
const size_t ICON_POOL_COUNT = sizeof(ICON_POOL) / sizeof(HICON);
#pragma endregion

#pragma region Helpers
// Can be used in place of RGBQUAD or COLORREF, allows for both packed and unpacked RGB manipulation.
typedef union rgb_t {
	COLORREF rgb;
	RGBQUAD quad;
	struct {
		BYTE r, g, b, reserved;
	};
} rgb;
static_assert(sizeof(rgb_t) == sizeof(RGBQUAD), "rgb_t has a different size than RGBQUAD");

// Useful for marking durations.
typedef DWORD MILLISECONDS;
// Queues sound, where the sample source is a function that returns a sample given a sample index.
// That is the definition of Bytebeat, a type/genre of music.
void bytebeat(DWORD samplerate, MILLISECONDS duration, CHAR(*source)(size_t)) {
	HWAVEOUT wave_out_handle = 0;
	WAVEFORMATEX wave_format = { WAVE_FORMAT_PCM, 1, samplerate, samplerate, 1, 8, 0 };
	waveOutOpen(&wave_out_handle, WAVE_MAPPER, &wave_format, 0, 0, CALLBACK_NULL);

	size_t buffer_size = (size_t)samplerate * (size_t)duration / 1000;
	LPSTR buffer = new CHAR[buffer_size];
	for (size_t t = 0; t < buffer_size; ++t) {
		buffer[t] = source(t);
	}

	WAVEHDR header = { buffer, buffer_size * sizeof(CHAR), 0, 0, 0, 0, 0, 0 };
	waveOutPrepareHeader(wave_out_handle, &header, sizeof(WAVEHDR));
	waveOutWrite(wave_out_handle, &header, sizeof(WAVEHDR));
	waveOutUnprepareHeader(wave_out_handle, &header, sizeof(WAVEHDR));
	waveOutClose(wave_out_handle);
}

// Stores a memory DC.
struct mem_dc {
	HDC dc;
	rgb_t* buffer_data;
	HBITMAP bitmap_handle;

	mem_dc(HDC memory_dc, RGBQUAD* buffer_data, HBITMAP bitmap_handle)
		: dc{ memory_dc }, buffer_data{ (rgb_t*)buffer_data }, bitmap_handle{ bitmap_handle } {}
};
// Creates a memory DC.
mem_dc make_memory_screen() {
	HDC main_screen = GetDC(0);
	HDC memory_dc = CreateCompatibleDC(main_screen);
	BITMAPINFO bitmap_info = { 0 };
	bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFO);
	bitmap_info.bmiHeader.biBitCount = 32;
	bitmap_info.bmiHeader.biPlanes = 1;
	bitmap_info.bmiHeader.biWidth = screen_width;
	bitmap_info.bmiHeader.biHeight = screen_height;
	RGBQUAD* screen_data{ nullptr };
	HBITMAP bitmap_handle = CreateDIBSection(main_screen, &bitmap_info, 0, (void**)&screen_data, nullptr, 0);
	SelectObject(memory_dc, bitmap_handle);

	return mem_dc{
		memory_dc,
		screen_data,
		bitmap_handle,
	};
}
// Destroys a memory DC.
void destroy_memory_screen(mem_dc& memory_dc) {
	DeleteDC(memory_dc.dc);
	DeleteObject(memory_dc.bitmap_handle);
}
// Parameters that are created and passed to every GDI manipulator thread.
// Mostly used to enforce lifetimes. In the future, destructor functions may be attached to GDI manipulators.
struct gdi_manipulator_data {
	mem_dc memory_screen;
	/// allocations created with new (not to be confused with new[]) that are to be deleted on thread termination
	std::vector<void*> allocations;
	/// allocations created with new[] that are to be deleted on thread termination
	std::vector<void*> array_allocations;

	gdi_manipulator_data() : memory_screen{ make_memory_screen() } {}

	~gdi_manipulator_data() {
		destroy_memory_screen(memory_screen);
		for (void* allocation : allocations) {
			delete allocation;
		}
		for (void* allocation : array_allocations) {
			delete[] allocation;
		}
	}
};
#pragma endregion

#pragma region WinGDI Manipulators
// Copies random sections of the screen to other random sections of the screen.
DWORD gdi_rect_jumble(gdi_manipulator_data* data) {
	for (;;) {
		int width = rand() % 100 + 50;
		int height = rand() % 100 + 50;

		int src_x = rand() % (screen_width - width);
		int src_y = rand() % (screen_height - height);
		int dst_x = rand() % (screen_width - width);
		int dst_y = rand() % (screen_height - height);

		HDC dc = GetDC(0);
		BitBlt(dc, dst_x, dst_y, width, height, dc, src_x, src_y, SRCCOPY);
		ReleaseDC(nullptr, dc);
	}
	return 0;
}
// Writes out a bunch of text on the screen.
DWORD gdi_text_spam(gdi_manipulator_data* data) {
	for (;;) {
		const char* text = TEXT_SPAM[rand() % TEXT_SPAM_COUNT];
		int dst_x = rand() % screen_width;
		int dst_y = rand() % screen_height;
		HDC dc = GetDC(0);

		SetBkColor(dc, RGB(rand(), rand(), rand()));
		SetTextColor(dc, RGB(rand(), rand(), rand()));

		TextOutA(dc, dst_x, dst_y, text, strlen(text));
		Sleep(5);
		ReleaseDC(nullptr, dc);
	}
	return 0;
}
// Creates a tweaked XOR fractal (Partially taken from Trichloromethane.exe).
DWORD gdi_xoring(gdi_manipulator_data* data) {
	HDC main_screen = GetDC(0);
	for (;;) {
		BitBlt(data->memory_screen.dc, 0, 0, screen_width, screen_height, main_screen, 0, 0, SRCCOPY);
		for (size_t i = 0; i < screen_width * screen_height; i++) {
			size_t src_x = i % screen_width;
			size_t src_y = i / screen_width;
			data->memory_screen.buffer_data[i].rgb += (src_x ^ src_y) ^ 1020;
			data->memory_screen.buffer_data[i].rgb += 1234;
		}
		BitBlt(main_screen, 0, 0, screen_width, screen_height, data->memory_screen.dc, 0, 0, SRCCOPY);
	}
}
// Dissolves/melts the screen by moving pixels.
// TODO: Optimize, seems like rand() is pretty slow.
DWORD gdi_pixeldissolve(gdi_manipulator_data* data) {
	HDC main_screen = GetDC(0);
	const intptr_t drift_amount = 5;
	for (;;) {
		BitBlt(data->memory_screen.dc, 0, 0, screen_width, screen_height, main_screen, 0, 0, SRCCOPY);
		for (size_t i = 0; i < screen_width * screen_height; i++) {
			size_t src_x = i % screen_width;
			size_t src_y = i / screen_width;

			// clamp to prevent oob
			intptr_t dst_x = (intptr_t)src_x + rand() % (drift_amount * 2 + 1) - drift_amount;
			if (dst_x < 0) dst_x = 0;
			if (dst_x >= screen_width) dst_x = screen_width - 1;

			intptr_t dst_y = (intptr_t)src_y + rand() % (drift_amount * 2 + 1) - drift_amount;
			if (dst_y < 0) dst_y = 0;
			if (dst_y >= screen_height) dst_y = screen_height - 1;

			data->memory_screen.buffer_data[(dst_x + dst_y * screen_width)].rgb = data->memory_screen.buffer_data[(src_x + src_y * screen_width)].rgb;
		}
		BitBlt(main_screen, 0, 0, screen_width, screen_height, data->memory_screen.dc, 0, 0, SRCCOPY);
	}
}
// Draws icons all over the screen.
DWORD gdi_icons(gdi_manipulator_data* data) {
	HDC main_screen = GetDC(0);
	for (;;) {
		DrawIcon(main_screen, rand() % screen_width, rand() % screen_height, ICON_POOL[rand() % ICON_POOL_COUNT]);
		Sleep(5);
	}
}
#pragma endregion

#pragma region Audio Queuers
// Sierpinski melody (bytebeat)
// Composed by miiro (https://youtu.be/qlrs2Vorw2Y?t=2m14s)
// Found on dollchan.net (https://dollchan.net/bytebeat/)
void waveout_sierpinski_melody(MILLISECONDS duration) {
	bytebeat(8000, duration, [](size_t t) -> CHAR { return 5 * t & t >> 7 | 3 * t & 4 * t >> 10; });
}
// The 42 melody v2 (bytebeat)
// Composed by viznut (http://viznut.fi/demos/unix/bytebeat_formulas.txt)
// Modified by me
// Found on dollchan.net (https://dollchan.net/bytebeat/)
void waveout_42_melody_v2(MILLISECONDS duration) {
	bytebeat(8000, duration, [](size_t t) -> CHAR { return ((((t >> 14) % 8) + 1) * t * (((t >> 10) & (2 | 8) ^ 10))) >> 1; });
}
// Chip arpeggio that eats itself (bytebeat)
// Composed by kb_ (https://www.pouet.net/topic.php?which=8357&page=8#c388898)
// Found on dollchan.net (https://dollchan.net/bytebeat/)
void waveout_chip_arp(MILLISECONDS duration) {
	bytebeat(44100, duration, [](size_t t) -> CHAR {
		return ((t >> 1) * (15 & 0x234568a0 >> (t >> 8 & 28)) | t >> 1 >> (t >> 11) ^ t >> 12) + (t >> 4 & t & 24);
	});
}
// Woimp (bytebeat)
// Composed by me (at least, according to the 3-month old list of bytebeats i have)
void waveout_woimp(MILLISECONDS duration) {
	bytebeat(8000, duration, [](size_t t) -> CHAR {
		return t * ((((t >> 11) % 32) + 1) << ((t >> 8) % 8));
	});
}
// Mosquito (bytebeat)
// Composed by me
void waveout_mosquito(MILLISECONDS duration) {
	bytebeat(8000, duration, [](size_t t) -> CHAR {
		return t * ((((t >> 10) ^ 20) ^ (t >> 9)) & 0x1f);
	});
}
#pragma endregion

#pragma region Main
// An element in an effect sequence.
// Not a payload in the sense of an overwrite of some critical data, instead it is a visual/auditory payload.
struct payload {
	// The graphics function to be dispatched to a separate thread.
	// Should run forever, but pointers to allocations should be placed into the given
	// allocation vectors so that they are removed upon thread termination.
	// TODO: Add graphics destructor function.
	DWORD(WINAPI *graphics)(gdi_manipulator_data*);
	// The audio queuer function.
	// This should return as soon as possible and queue up the given number of milliseconds of audio.
	void(*audio_queuer)(MILLISECONDS);
	// The amount of time to run the payload for.
	MILLISECONDS duration;

	// Runs the graphics and audio associated with this effect payload.
	void run() {
		gdi_manipulator_data data {};
		HANDLE graphics_thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)graphics, &data, 0, 0);
		if (!graphics_thread) {
			std::cerr << "Graphics thread could not be created!\n";
		}
		audio_queuer(duration);
		Sleep(duration);
		if (graphics_thread) {
			// TODO: Clean up threads properly.
			TerminateThread(graphics_thread, 0);
			CloseHandle(graphics_thread);
		}
	}
};

int main() {
	srand(time(nullptr));
	payload payloads[] = {
		payload{ gdi_pixeldissolve, waveout_sierpinski_melody, 20000 },
		payload{ gdi_rect_jumble, waveout_42_melody_v2, 16500 },
		payload{ gdi_xoring, waveout_woimp, 10000, },
		payload{ gdi_icons, waveout_mosquito, 15000 },
		payload{ gdi_text_spam, waveout_chip_arp, 10000 },
	};
	for (payload& payload : payloads) {
		payload.run();
	}
}
#pragma endregion
