/**
 * Any Speed Cruise Control - key setup tool
 *
 * Small window showing the four cruise control functions. Click "Change", press
 * a keyboard key or a wheel / gamepad button, and the [keys] section of
 * any_speed_cruise.ini (next to this program) is updated.
 *
 * Values written: "0x6E" (virtual-key code) or "joy1.b5" (joystick 1, button 5).
 */

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { ROWS = 4, MAX_JOYSTICKS = 16, ID_CHANGE = 100, ID_CLOSE = 200, TIMER_ID = 1 };

static const char *const ini_keys[ROWS] = { "set", "resume", "increase", "decrease" };
static const char *const labels[ROWS]   = { "Set / hold speed", "Resume", "Increase", "Decrease" };
static const char *const defaults[ROWS] = { "0x6E", "0x6A", "0x6B", "0x6D" };

static char  ini_path[MAX_PATH];
static char  values[ROWS][32];
static HWND  main_window;
static HWND  value_controls[ROWS];
static HWND  status_control;
static int   capturing = -1;               // row waiting for a key, -1 = none
static bool  joy_present[MAX_JOYSTICKS];
static DWORD joy_buttons[MAX_JOYSTICKS];   // buttons already down, to detect new presses

static bool read_joystick(const int id, DWORD &buttons)
{
	JOYINFOEX info;
	memset(&info, 0, sizeof(info));
	info.dwSize = sizeof(info);
	info.dwFlags = JOY_RETURNBUTTONS;
	if (joyGetPosEx(static_cast<UINT>(id), &info) != JOYERR_NOERROR) {
		return false;
	}
	buttons = info.dwButtons;
	return true;
}

static void key_name(const int vk, char *out, const int size)
{
	// The numeric keypad is what most players use, give it unambiguous names.
	static const char *const numpad_operators[] = { "Num *", "Num +", "Num Enter", "Num -", "Num .", "Num /" };
	if ((vk >= VK_NUMPAD0) && (vk <= VK_NUMPAD9)) {
		snprintf(out, size, "Num %d", vk - VK_NUMPAD0);
		return;
	}
	if ((vk >= VK_MULTIPLY) && (vk <= VK_DIVIDE)) {
		snprintf(out, size, "%s", numpad_operators[vk - VK_MULTIPLY]);
		return;
	}

	LONG lparam = static_cast<LONG>(MapVirtualKeyA(static_cast<UINT>(vk), MAPVK_VK_TO_VSC)) << 16;
	switch (vk) {
		case VK_INSERT: case VK_DELETE: case VK_HOME: case VK_END: case VK_PRIOR: case VK_NEXT:
		case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN: case VK_RCONTROL: case VK_RMENU: case VK_NUMLOCK:
			lparam |= (1 << 24); // extended key
			break;
		default:
			break;
	}
	if ((lparam == 0) || (GetKeyNameTextA(lparam, out, size) == 0)) {
		snprintf(out, size, "Key");
	}
}

static void describe(const char *value, char *out, const int size)
{
	int joy = 0;
	int button = 0;
	if (sscanf(value, "joy%d.b%d", &joy, &button) == 2) {
		snprintf(out, size, "Joystick %d - button %d", joy, button);
		return;
	}
	const int vk = static_cast<int>(strtol(value, NULL, 0));
	if (vk <= 0) {
		snprintf(out, size, "(none)");
		return;
	}
	char name[64];
	key_name(vk, name, sizeof(name));
	snprintf(out, size, "%s   (0x%02X)", name, vk);
}

static void refresh(void)
{
	for (int row = 0; row < ROWS; ++row) {
		char text[128];
		if (row == capturing) {
			snprintf(text, sizeof(text), "Press a key or button...  (Esc = cancel)");
		}
		else {
			describe(values[row], text, sizeof(text));
		}
		SetWindowTextA(value_controls[row], text);
	}
}

static void set_status(const char *text)
{
	SetWindowTextA(status_control, text);
}

static void begin_capture(const int row)
{
	capturing = row;
	for (int id = 0; id < MAX_JOYSTICKS; ++id) {
		joy_buttons[id] = 0;
		joy_present[id] = read_joystick(id, joy_buttons[id]);
	}
	SetFocus(main_window);
	set_status("Waiting for a keyboard key or a wheel / gamepad button.");
	refresh();
}

static void end_capture(const char *new_value)
{
	const int row = capturing;
	capturing = -1;
	if (! new_value) {
		set_status("Cancelled.");
		refresh();
		return;
	}

	snprintf(values[row], sizeof(values[row]), "%s", new_value);
	if (! WritePrivateProfileStringA("keys", ini_keys[row], new_value, ini_path)) {
		char text[MAX_PATH + 64];
		snprintf(text, sizeof(text), "Unable to write %s", ini_path);
		set_status(text);
		refresh();
		return;
	}

	const char *message = "Saved. Restart the game to apply. Bind the same key in the game options.";
	for (int other = 0; other < ROWS; ++other) {
		if ((other != row) && (strcmp(values[other], values[row]) == 0)) {
			message = "Saved, but this key is used by two functions: please change one of them.";
		}
	}
	set_status(message);
	refresh();
}

static void poll_joysticks(void)
{
	for (int id = 0; id < MAX_JOYSTICKS; ++id) {
		DWORD buttons = 0;
		if ((! joy_present[id]) || (! read_joystick(id, buttons))) {
			continue;
		}
		const DWORD newly_pressed = buttons & ~joy_buttons[id];
		joy_buttons[id] = buttons;
		if (newly_pressed == 0) {
			continue;
		}
		int button = 0;
		while ((newly_pressed & (1u << button)) == 0) {
			++button;
		}
		char value[32];
		snprintf(value, sizeof(value), "joy%d.b%d", id + 1, button + 1);
		end_capture(value);
		return;
	}
}

static BOOL CALLBACK set_font(HWND child, LPARAM font)
{
	SendMessageA(child, WM_SETFONT, static_cast<WPARAM>(font), TRUE);
	return TRUE;
}

static void create_controls(const HWND window)
{
	const HINSTANCE instance = GetModuleHandleA(NULL);
	CreateWindowExA(0, "STATIC",
		"Click Change, then press the key or the wheel / gamepad button you use in the game for that function.",
		WS_CHILD | WS_VISIBLE, 16, 12, 448, 34, window, NULL, instance, NULL);

	for (int row = 0; row < ROWS; ++row) {
		const int y = 56 + (row * 36);
		CreateWindowExA(0, "STATIC", labels[row], WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 16, y, 120, 26, window, NULL, instance, NULL);
		value_controls[row] = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER, 140, y, 232, 26, window, NULL, instance, NULL);
		CreateWindowExA(0, "BUTTON", "Change", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 382, y, 82, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_CHANGE + row)), instance, NULL);
	}

	status_control = CreateWindowExA(0, "STATIC", "", WS_CHILD | WS_VISIBLE, 16, 206, 448, 34, window, NULL, instance, NULL);
	CreateWindowExA(0, "BUTTON", "Close", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 382, 244, 82, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_CLOSE)), instance, NULL);

	NONCLIENTMETRICSA metrics;
	memset(&metrics, 0, sizeof(metrics));
	metrics.cbSize = sizeof(metrics);
	HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
	if (SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0)) {
		const HFONT message_font = CreateFontIndirectA(&metrics.lfMessageFont);
		if (message_font) {
			font = message_font;
		}
	}
	EnumChildWindows(window, set_font, reinterpret_cast<LPARAM>(font));
}

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message) {
		case WM_CREATE:
			create_controls(window);
			SetTimer(window, TIMER_ID, 30, NULL);
			return 0;

		case WM_TIMER:
			if (capturing >= 0) {
				poll_joysticks();
			}
			return 0;

		case WM_COMMAND: {
			const int id = LOWORD(wparam);
			if ((id >= ID_CHANGE) && (id < ID_CHANGE + ROWS)) {
				begin_capture(id - ID_CHANGE);
			}
			else if (id == ID_CLOSE) {
				DestroyWindow(window);
			}
			return 0;
		}

		case WM_ACTIVATE:
			if ((LOWORD(wparam) == WA_INACTIVE) && (capturing >= 0)) {
				end_capture(NULL);
			}
			return 0;

		case WM_CTLCOLORSTATIC:
			SetBkMode(reinterpret_cast<HDC>(wparam), TRANSPARENT);
			return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_BTNFACE));

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		default:
			return DefWindowProcA(window, message, wparam, lparam);
	}
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show)
{
	InitCommonControls();

	// The configuration file sits next to this program.
	GetModuleFileNameA(NULL, ini_path, MAX_PATH);
	char *const slash = strrchr(ini_path, '\\');
	snprintf(slash ? slash + 1 : ini_path, slash ? static_cast<size_t>(MAX_PATH - (slash + 1 - ini_path)) : MAX_PATH, "any_speed_cruise.ini");
	const bool ini_exists = (GetFileAttributesA(ini_path) != INVALID_FILE_ATTRIBUTES);

	for (int row = 0; row < ROWS; ++row) {
		GetPrivateProfileStringA("keys", ini_keys[row], defaults[row], values[row], sizeof(values[row]), ini_path);
	}

	WNDCLASSA window_class;
	memset(&window_class, 0, sizeof(window_class));
	window_class.lpfnWndProc = window_proc;
	window_class.hInstance = instance;
	window_class.hCursor = LoadCursorA(NULL, IDC_ARROW);
	window_class.hIcon = LoadIconA(NULL, IDI_APPLICATION);
	window_class.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	window_class.lpszClassName = "any_speed_cruise_config";
	RegisterClassA(&window_class);

	const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	RECT rect = { 0, 0, 480, 282 };
	AdjustWindowRect(&rect, style, FALSE);
	main_window = CreateWindowExA(0, window_class.lpszClassName, "Any Speed Cruise Control - Keys", style,
		CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, instance, NULL);
	if (! main_window) {
		return 1;
	}
	refresh();
	set_status(ini_exists
		? "Keys currently stored in any_speed_cruise.ini."
		: "any_speed_cruise.ini was not found next to this program: it will be created.");
	ShowWindow(main_window, show);

	MSG msg;
	while (GetMessageA(&msg, NULL, 0, 0) > 0) {
		// While waiting for a key, every key press belongs to us whichever control has the focus.
		if ((msg.message == WM_KEYDOWN) || (msg.message == WM_SYSKEYDOWN)) {
			if (capturing >= 0) {
				if (msg.wParam == VK_ESCAPE) {
					end_capture(NULL);
				}
				else {
					char value[32];
					snprintf(value, sizeof(value), "0x%02X", static_cast<unsigned>(msg.wParam));
					end_capture(value);
				}
				continue;
			}
			if (msg.message == WM_SYSKEYDOWN) {
				continue; // no menu here, Alt and F10 must not freeze the window
			}
		}
		if ((msg.message == WM_SYSKEYUP) || (msg.message == WM_SYSCHAR)) {
			continue;
		}
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
	return 0;
}
