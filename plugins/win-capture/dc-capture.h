#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <obs-module.h>

struct dc_capture {
	gs_texture_t *texture;
	bool         texture_written;
	int          x, y;
	uint32_t     width;
	uint32_t     height;

	bool         compatibility;
	HDC          hdc;
	HBITMAP      bmp, old_bmp;
	BYTE         *bits;

	bool         capture_cursor;
	bool         cursor_captured;
	bool         cursor_hidden;
	CURSORINFO   ci;

	bool         valid;
};

extern void dc_capture_init(struct dc_capture *capture, int x, int y,
		uint32_t width, uint32_t height, bool cursor,
		bool compatibility);
extern void dc_capture_free(struct dc_capture *capture);

extern void dc_capture_capture(struct dc_capture *capture, HWND window);
extern void dc_capture_render(struct dc_capture *capture, gs_effect_t *effect);

typedef UINT(WINAPI *GETDPIFORWINDOW)(HWND hwnd);
//
// Get WINAPI function GetDpiForWindow(); since Windows 10, version 1607 (AU)
//
#define GET_WINAPI_DPIFORWINDOW_FUNC(func_name, library_name) \
static UINT func_name(HWND hwnd) \
{ \
	HMODULE phModule = NULL; \
	static GETDPIFORWINDOW FUNC = NULL; \
	static UINT ret = 0; \
	static bool initialized = false; \
\
	if (!initialized) { \
		if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, \
				TEXT(#library_name), &phModule)) \
			FUNC = (GETDPIFORWINDOW)GetProcAddress(phModule, \
					"GetDpiForWindow"); \
\
		initialized = true; \
		} \
\
	if (FUNC) \
		ret = FUNC(hwnd); \
\
	return ret; \
}
//
// call to GetDpiForWindow() function
//
GET_WINAPI_DPIFORWINDOW_FUNC(GetDForWnd, user32)
