/**
 * Any Speed Cruise Control - plugin for Euro Truck Simulator 2 / American Truck Simulator
 *
 * The game refuses to engage cruise control below 30 km/h (20 mph in ATS).
 * That limit is hardcoded in the executable, so this plugin works around it
 * with the official SCS SDK only (no memory patching):
 *
 *  - telemetry API: reads speed, the game's cruise control state, brake input...
 *  - input API:     a "semantical" device that can press the throttle and the
 *                   cruise control keys on the player's behalf.
 *
 * Below the game's limit the plugin holds the speed itself with the throttle.
 * As soon as the game accepts it, the game's own cruise control takes over.
 */

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scssdk_telemetry.h"
#include "scssdk_input.h"
#include "eurotrucks2/scssdk_eut2.h"
#include "eurotrucks2/scssdk_telemetry_eut2.h"
#include "eurotrucks2/scssdk_input_eut2.h"
#include "amtrucks/scssdk_ats.h"
#include "amtrucks/scssdk_telemetry_ats.h"
#include "amtrucks/scssdk_input_ats.h"

#define PLUGIN_VERSION "1.1.0"
#define LOG_PREFIX     "[any_speed_cruise] "

static const float KMH = 1.0f / 3.6f; // km/h -> m/s

// ---------------------------------------------------------------------------
// Configuration (any_speed_cruise.ini, next to the DLL)
// ---------------------------------------------------------------------------

/**
 * @brief One watched control: a keyboard key ("0x6E") or a joystick button ("joy1.b5").
 *
 * Must match what is bound to the same function in the game.
 */
struct binding_t
{
	int vk;     // virtual-key code, 0 = none
	int joy;    // zero-based joystick id, -1 = keyboard
	int button; // zero-based joystick button
};

enum { BIND_SET, BIND_RESUME, BIND_INCREASE, BIND_DECREASE, BIND_COUNT };

struct config_t
{
	binding_t bindings[BIND_COUNT];
	float step;         // m/s, increase/decrease step while the plugin holds the speed
	float handover;      // m/s, speed above which the game's cruise control is asked to take over
	float max_throttle;  // 0..1
	float min_set_speed; // m/s, "set" is ignored below this speed
	int   log_events;
};

static config_t cfg;
static char     ini_path[MAX_PATH] = "";
static bool     is_ats = false;

static float ini_float(const char *section, const char *key, const float fallback)
{
	char buffer[64];
	GetPrivateProfileStringA(section, key, "", buffer, sizeof(buffer), ini_path);
	if (buffer[0] == 0) {
		return fallback;
	}
	return static_cast<float>(atof(buffer));
}

static int ini_int(const char *section, const char *key, const int fallback)
{
	char buffer[64];
	GetPrivateProfileStringA(section, key, "", buffer, sizeof(buffer), ini_path);
	if (buffer[0] == 0) {
		return fallback;
	}
	return static_cast<int>(strtol(buffer, NULL, 0)); // accepts 110 or 0x6E
}

static float clampf(const float v, const float lo, const float hi)
{
	return (v < lo) ? lo : ((v > hi) ? hi : v);
}

static binding_t ini_binding(const char *key, const int fallback_vk)
{
	binding_t binding = { fallback_vk, -1, 0 };
	char buffer[64];
	GetPrivateProfileStringA("keys", key, "", buffer, sizeof(buffer), ini_path);
	if (buffer[0] == 0) {
		return binding;
	}
	int joy = 0;
	int button = 0;
	if (sscanf(buffer, "joy%d.b%d", &joy, &button) == 2) {
		if ((joy >= 1) && (joy <= 16) && (button >= 1) && (button <= 32)) {
			binding.vk = 0;
			binding.joy = joy - 1;
			binding.button = button - 1;
		}
		return binding;
	}
	binding.vk = static_cast<int>(strtol(buffer, NULL, 0)); // accepts 110 or 0x6E
	return binding;
}

static void load_config(void)
{
	cfg.bindings[BIND_SET]      = ini_binding("set",      VK_DECIMAL);  // numpad .
	cfg.bindings[BIND_RESUME]   = ini_binding("resume",   VK_MULTIPLY); // numpad *
	cfg.bindings[BIND_INCREASE] = ini_binding("increase", VK_ADD);      // numpad +
	cfg.bindings[BIND_DECREASE] = ini_binding("decrease", VK_SUBTRACT); // numpad -

	cfg.step          = clampf(ini_float("cruise", "step_kmh", 5.0f), 0.5f, 20.0f) * KMH;
	cfg.max_throttle  = clampf(ini_float("cruise", "max_throttle", 1.0f), 0.1f, 1.0f);
	cfg.min_set_speed = clampf(ini_float("cruise", "min_set_speed_kmh", 2.0f), 0.5f, 30.0f) * KMH;
	cfg.log_events    = ini_int("cruise", "log", 1);

	// 0 = automatic: just above the game's own limit (30 km/h in ETS2, 20 mph in ATS).
	const float handover_kmh = ini_float("cruise", "handover_kmh", 0.0f);
	cfg.handover = ((handover_kmh > 0.0f) ? handover_kmh : (is_ats ? 32.8f : 30.5f)) * KMH;
}

// ---------------------------------------------------------------------------
// Logging
// ---------------------------------------------------------------------------

static scs_log_t game_log = NULL;

static void log_line(const scs_log_type_t type, const char *format, ...)
{
	if (! game_log) {
		return;
	}
	char buffer[512];
	const size_t prefix_length = strlen(LOG_PREFIX);
	memcpy(buffer, LOG_PREFIX, prefix_length);
	va_list args;
	va_start(args, format);
	vsnprintf(buffer + prefix_length, sizeof(buffer) - prefix_length, format, args);
	va_end(args);
	game_log(type, buffer);
}

// ---------------------------------------------------------------------------
// Telemetry state
// ---------------------------------------------------------------------------

static struct
{
	float speed;          // m/s, negative when reversing
	float game_cruise;    // m/s, 0 when the game's cruise control is off
	float brake_input;    // 0..1, what the player presses
	bool  parking_brake;
	bool  engine_on;
	int   gear;           // displayed gear: >0 forward, 0 neutral, <0 reverse
	bool  paused;
} tele = { 0.0f, 0.0f, 0.0f, false, false, 0, true };

SCSAPI_VOID telemetry_store_float(const scs_string_t, const scs_u32_t, const scs_value_t *const value, const scs_context_t context)
{
	if (value) {
		*static_cast<float *>(context) = value->value_float.value;
	}
}

SCSAPI_VOID telemetry_store_bool(const scs_string_t, const scs_u32_t, const scs_value_t *const value, const scs_context_t context)
{
	if (value) {
		*static_cast<bool *>(context) = (value->value_bool.value != 0);
	}
}

SCSAPI_VOID telemetry_store_s32(const scs_string_t, const scs_u32_t, const scs_value_t *const value, const scs_context_t context)
{
	if (value) {
		*static_cast<int *>(context) = value->value_s32.value;
	}
}

SCSAPI_VOID telemetry_pause(const scs_event_t event, const void *const, const scs_context_t)
{
	tele.paused = (event == SCS_TELEMETRY_EVENT_paused);
}

// ---------------------------------------------------------------------------
// Cruise logic
// ---------------------------------------------------------------------------

// Inputs of our semantical device. The names are the names of the mixes in controls.sii.
enum { INPUT_THROTTLE, INPUT_CC_SET, INPUT_CC_RESUME, INPUT_COUNT };

static const scs_input_device_input_t device_inputs[INPUT_COUNT] = {
	{ "aforward",     "Throttle",              SCS_VALUE_TYPE_float, 0 },
	{ "cruiectrl",    "Cruise control",        SCS_VALUE_TYPE_bool,  0 },
	{ "cruiectrlres", "Cruise control resume", SCS_VALUE_TYPE_bool,  0 },
};

static struct
{
	bool      holding;            // the plugin currently controls the throttle
	float     target;             // m/s, speed being held
	bool      target_from_game;   // target is the speed remembered by the game itself -> hand over with "resume"
	float     remembered;         // m/s, speed "resume" goes back to
	bool      remembered_from_game;

	float     throttle;           // current output 0..1
	float     integral;

	ULONGLONG pending_set_at;     // key presses wait a moment to see whether the game reacts by itself
	ULONGLONG pending_resume_at;

	ULONGLONG pulse_release_at;   // injected key press in progress (0 = none)
	int       pulse_input;
	ULONGLONG next_handover_at;
	int       failed_resumes;

	bool      key_down[4];
	ULONGLONG last_tick;

	// What the game currently knows about our device.
	float     sent_throttle;
	bool      sent_button[INPUT_COUNT];
	bool      resend_all;

	scs_input_event_t queue[INPUT_COUNT];
	int               queue_size;
} st;

static const DWORD KEY_DECISION_DELAY_MS = 250;
static const DWORD PULSE_LENGTH_MS       = 120;
static const DWORD HANDOVER_RETRY_MS     = 1000;

static void stop_holding(const char *reason)
{
	if (! st.holding) {
		return;
	}
	st.holding = false;
	st.throttle = 0.0f;
	st.integral = 0.0f;
	st.remembered = st.target;
	st.remembered_from_game = st.target_from_game;
	if (cfg.log_events) {
		log_line(SCS_LOG_TYPE_message, "released (%s)", reason);
	}
}

static bool can_hold(void)
{
	return (! tele.paused) && tele.engine_on && (! tele.parking_brake) && (tele.gear >= 0) && (tele.speed > -0.3f);
}

static void start_holding(const float target, const bool from_game)
{
	st.holding = true;
	st.target = target;
	st.target_from_game = from_game;
	st.integral = 0.0f;
	st.throttle = 0.0f;
	st.failed_resumes = 0;
	st.next_handover_at = 0;
	if (cfg.log_events) {
		log_line(SCS_LOG_TYPE_message, "holding %.0f km/h (%.0f mph)", target * 3.6f, target * 2.23694f);
	}
}

static bool game_has_focus(void)
{
	const HWND window = GetForegroundWindow();
	if (! window) {
		return false;
	}
	DWORD process = 0;
	GetWindowThreadProcessId(window, &process);
	return process == GetCurrentProcessId();
}

/**
 * @brief Is the bound key or joystick button currently down?
 */
static bool binding_down(const binding_t &binding, const ULONGLONG now)
{
	if (binding.joy < 0) {
		return (binding.vk > 0) && ((GetAsyncKeyState(binding.vk) & 0x8000) != 0);
	}

	// Asking for an unplugged device can be slow, so back off after a failure.
	static ULONGLONG retry_at[16];
	if (now < retry_at[binding.joy]) {
		return false;
	}
	JOYINFOEX info;
	memset(&info, 0, sizeof(info));
	info.dwSize = sizeof(info);
	info.dwFlags = JOY_RETURNBUTTONS;
	if (joyGetPosEx(static_cast<UINT>(binding.joy), &info) != JOYERR_NOERROR) {
		retry_at[binding.joy] = now + 2000;
		return false;
	}
	return (info.dwButtons & (1u << binding.button)) != 0;
}

static void start_pulse(const int input, const ULONGLONG now)
{
	st.pulse_input = input;
	st.pulse_release_at = now + PULSE_LENGTH_MS;
}

/**
 * @brief Runs once per frame: reads the keys, updates the state, computes the outputs.
 */
static void update(void)
{
	const ULONGLONG now = GetTickCount64();
	float dt = static_cast<float>(now - st.last_tick) * 0.001f;
	st.last_tick = now;
	dt = clampf(dt, 0.0f, 0.1f);

	const bool game_cruise_on = (tele.game_cruise > 0.1f);

	// Remember what the game's cruise control was set to, so that "resume" works at any speed.

	if (game_cruise_on) {
		st.remembered = tele.game_cruise;
		st.remembered_from_game = true;
		if (st.holding) {
			stop_holding("game cruise control took over");
		}
	}

	// Keys. We only watch them, the game still receives them as usual.

	bool pressed[BIND_COUNT] = { false, false, false, false };
	const bool focus = game_has_focus() && (! tele.paused);
	for (int i = 0; i < BIND_COUNT; ++i) {
		const bool down = focus && binding_down(cfg.bindings[i], now);
		pressed[i] = down && (! st.key_down[i]);
		st.key_down[i] = down;
	}

	if (pressed[0]) { // set
		if (st.holding) {
			stop_holding("cruise key");
		}
		else if (! game_cruise_on) {
			st.pending_set_at = now + KEY_DECISION_DELAY_MS;
		}
	}
	if (pressed[1] && (! st.holding) && (! game_cruise_on)) { // resume
		st.pending_resume_at = now + KEY_DECISION_DELAY_MS;
	}
	if (st.holding && (pressed[2] || pressed[3])) { // increase / decrease
		st.target += pressed[2] ? cfg.step : -cfg.step;
		if (st.target < cfg.min_set_speed) {
			st.target = cfg.min_set_speed;
		}
		st.target_from_game = false;
		if (cfg.log_events) {
			log_line(SCS_LOG_TYPE_message, "target %.0f km/h (%.0f mph)", st.target * 3.6f, st.target * 2.23694f);
		}
	}

	// The game had a moment to react to the key. If its cruise control is still off,
	// the speed is below its limit and the key is ours.

	if (st.pending_set_at && (now >= st.pending_set_at)) {
		st.pending_set_at = 0;
		if ((! game_cruise_on) && (! st.holding) && can_hold() && (tele.speed >= cfg.min_set_speed)) {
			start_holding(tele.speed, false);
		}
	}
	if (st.pending_resume_at && (now >= st.pending_resume_at)) {
		st.pending_resume_at = 0;
		if ((! game_cruise_on) && (! st.holding) && can_hold() && (st.remembered >= cfg.min_set_speed)) {
			start_holding(st.remembered, st.remembered_from_game);
		}
	}

	// Injected key press in progress?

	if (st.pulse_release_at && (now >= st.pulse_release_at)) {
		st.pulse_release_at = 0;
	}

	if (! st.holding) {
		st.throttle = 0.0f;
		return;
	}

	// Safety: anything unusual gives the control back to the player.

	if (tele.brake_input > 0.05f) {
		stop_holding("brake");
		return;
	}
	if (! can_hold()) {
		stop_holding("paused, engine off, parking brake or reverse gear");
		return;
	}

	// Throttle: simple PI controller on the speed error.

	const float error = st.target - tele.speed;
	const float proportional = 0.6f * error;
	const float unclamped = proportional + st.integral;
	if (((unclamped < cfg.max_throttle) || (error < 0.0f)) && ((unclamped > 0.0f) || (error > 0.0f))) {
		st.integral = clampf(st.integral + 0.15f * error * dt, 0.0f, cfg.max_throttle);
	}
	const float wanted = clampf(proportional + st.integral, 0.0f, cfg.max_throttle);
	const float max_change = 1.5f * dt; // smooth pedal movement
	st.throttle = clampf(wanted, st.throttle - max_change, st.throttle + max_change);

	// Hand over to the game's own cruise control once it can accept it.

	if ((st.target >= cfg.handover) && (tele.speed >= cfg.handover) && (st.pulse_release_at == 0) && (now >= st.next_handover_at)) {
		if (st.target_from_game && (st.failed_resumes < 3)) {
			// The game still remembers this speed: its "resume" restores it exactly.
			start_pulse(INPUT_CC_RESUME, now);
			++st.failed_resumes; // reset when holding starts; a success ends the holding
			st.next_handover_at = now + HANDOVER_RETRY_MS;
		}
		else if (tele.speed >= st.target - 0.5f * KMH) {
			// Target reached: let the game "set" the current speed.
			start_pulse(INPUT_CC_SET, now);
			st.next_handover_at = now + HANDOVER_RETRY_MS;
		}
	}
}

/**
 * @brief Queues the events needed to bring the game's view of our device up to date.
 */
static void build_events(void)
{
	st.queue_size = 0;

	if (st.resend_all || (st.throttle != st.sent_throttle)) {
		scs_input_event_t &event = st.queue[st.queue_size++];
		event.input_index = INPUT_THROTTLE;
		event.value_float.value = st.throttle;
		st.sent_throttle = st.throttle;
	}

	for (int input = INPUT_CC_SET; input < INPUT_COUNT; ++input) {
		const bool wanted = (st.pulse_release_at != 0) && (st.pulse_input == input);
		if (st.resend_all || (wanted != st.sent_button[input])) {
			scs_input_event_t &event = st.queue[st.queue_size++];
			event.input_index = static_cast<scs_u32_t>(input);
			event.value_bool.value = wanted ? 1 : 0;
			st.sent_button[input] = wanted;
		}
	}

	st.resend_all = false;
}

SCSAPI_RESULT input_event_callback(scs_input_event_t *const event_info, const scs_u32_t flags, const scs_context_t)
{
	if (flags & SCS_INPUT_EVENT_CALLBACK_FLAG_first_in_frame) {
		if (flags & SCS_INPUT_EVENT_CALLBACK_FLAG_first_after_activation) {
			st.resend_all = true;
		}
		update();
		build_events();
	}

	if (st.queue_size == 0) {
		return SCS_RESULT_not_found;
	}
	*event_info = st.queue[--st.queue_size];
	return SCS_RESULT_ok;
}

SCSAPI_VOID input_active_callback(const scs_u8_t active, const scs_context_t)
{
	if (! active) {
		stop_holding("input device deactivated");
		st.pulse_release_at = 0;
		st.pending_set_at = 0;
		st.pending_resume_at = 0;
	}
}

// ---------------------------------------------------------------------------
// SDK entry points
// ---------------------------------------------------------------------------

static void common_init(const scs_sdk_init_params_v100_t &common)
{
	game_log = common.log;
	is_ats = (strcmp(common.game_id, SCS_GAME_ID_ATS) == 0);
	load_config();
}

SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version, const scs_telemetry_init_params_t *const params)
{
	if (version != SCS_TELEMETRY_VERSION_1_01) {
		return SCS_RESULT_unsupported;
	}
	const scs_telemetry_init_params_v101_t *const version_params = static_cast<const scs_telemetry_init_params_v101_t *>(params);
	common_init(version_params->common);

	const bool events_ok =
		(version_params->register_for_event(SCS_TELEMETRY_EVENT_paused, telemetry_pause, NULL) == SCS_RESULT_ok) &&
		(version_params->register_for_event(SCS_TELEMETRY_EVENT_started, telemetry_pause, NULL) == SCS_RESULT_ok);

	// Without speed and cruise control state the plugin cannot work at all.
	const bool required_ok =
		(version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_speed, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float, &tele.speed) == SCS_RESULT_ok) &&
		(version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_cruise_control, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float, &tele.game_cruise) == SCS_RESULT_ok) &&
		(version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_input_brake, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float, &tele.brake_input) == SCS_RESULT_ok) &&
		(version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_parking_brake, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_bool, &tele.parking_brake) == SCS_RESULT_ok) &&
		(version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_engine_enabled, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_bool, &tele.engine_on) == SCS_RESULT_ok) &&
		(version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_displayed_gear, SCS_U32_NIL, SCS_VALUE_TYPE_s32, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_s32, &tele.gear) == SCS_RESULT_ok);

	if ((! events_ok) || (! required_ok)) {
		log_line(SCS_LOG_TYPE_error, "unable to register telemetry callbacks, plugin disabled");
		return SCS_RESULT_unsupported;
	}

	log_line(SCS_LOG_TYPE_message, "version " PLUGIN_VERSION " loaded for %s, handover at %.1f km/h", is_ats ? "ATS" : "ETS2", cfg.handover * 3.6f);
	return SCS_RESULT_ok;
}

SCSAPI_VOID scs_telemetry_shutdown(void)
{
	stop_holding("shutdown");
	tele.paused = true;
	game_log = NULL;
}

SCSAPI_RESULT scs_input_init(const scs_u32_t version, const scs_input_init_params_t *const params)
{
	if (version != SCS_INPUT_VERSION_1_00) {
		return SCS_RESULT_unsupported;
	}
	const scs_input_init_params_v100_t *const version_params = static_cast<const scs_input_init_params_v100_t *>(params);
	common_init(version_params->common);

	memset(&st, 0, sizeof(st));
	st.last_tick = GetTickCount64();
	st.resend_all = true;

	scs_input_device_t device_info;
	memset(&device_info, 0, sizeof(device_info));
	device_info.name = "any_speed_cruise";
	device_info.display_name = "Any Speed Cruise Control";
	device_info.type = SCS_INPUT_DEVICE_TYPE_semantical;
	device_info.input_count = INPUT_COUNT;
	device_info.inputs = device_inputs;
	device_info.input_active_callback = input_active_callback;
	device_info.input_event_callback = input_event_callback;
	device_info.callback_context = NULL;

	if (version_params->register_device(&device_info) != SCS_RESULT_ok) {
		log_line(SCS_LOG_TYPE_error, "unable to register the input device, plugin disabled");
		return SCS_RESULT_generic_error;
	}
	return SCS_RESULT_ok;
}

SCSAPI_VOID scs_input_shutdown(void)
{
	st.holding = false;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason_for_call, LPVOID)
{
	if (reason_for_call == DLL_PROCESS_ATTACH) {
		// The configuration file sits next to the DLL.
		const DWORD length = GetModuleFileNameA(module, ini_path, MAX_PATH);
		char *const dot = (length > 0) ? strrchr(ini_path, '.') : NULL;
		if (dot && (static_cast<size_t>(dot - ini_path) + 5 <= MAX_PATH)) {
			strcpy(dot, ".ini");
		}
		else {
			ini_path[0] = 0;
		}
		DisableThreadLibraryCalls(module);
	}
	return TRUE;
}
