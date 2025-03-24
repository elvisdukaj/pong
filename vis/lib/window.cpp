module;

#include <SDL3/SDL.h>

export module vis:window;

import std;

export namespace vis {

class Window {
public:
	using Pointer = std::unique_ptr<Window>;
	static std::expected<Pointer, std::string> create(std::string_view title, int width, int height,
																										SDL_WindowFlags flags) {
		auto window = SDL_CreateWindow(title.data(), width, height, flags);
		if (not window)
			return std::unexpected(std::format("Unable to create window: {}", SDL_GetError()));

		return Pointer{new Window{window}};
	}

	~Window() {
		if (window) {
			SDL_DestroyWindow(window);
			window = nullptr;
		}
	}

	Window(Window&) = delete;
	Window& operator=(Window&) = delete;

	Window(Window&& rhs) : window{rhs.window} {
		rhs.window = nullptr;
	}

	Window& operator=(Window&& rhs) {
		window = rhs.window;
		rhs.window = nullptr;
		return *this;
	};

	operator SDL_Window*() {
		return window;
	}

private:
	Window(SDL_Window* window) : window(window) {
		SDL_ShowWindow(window);
	}

private:
	SDL_Window* window;
};

} // namespace vis

export namespace vis::win {

enum class AppResult { app_continue = SDL_APP_CONTINUE, success = SDL_APP_SUCCESS, failure = SDL_APP_FAILURE };

inline SDL_AppResult to_sdl(AppResult app_result) {
	return static_cast<SDL_AppResult>(app_result);
}

enum class EventType : int {
	quit = SDL_EVENT_QUIT,

	/* These application events have special meaning on iOS and Android, see README-ios.md and README-android.md for
		 details */
	terminating = SDL_EVENT_TERMINATING, /**< The application is being terminated by the OS. This event must be
														handled in a callback set with SDL_AddEventWatch(). Called on iOS in
														applicationWillTerminate() Called on Android in onDestroy()
												 */
	low_memory = SDL_EVENT_LOW_MEMORY, /**< The application is low on memory, free memory if possible. This event must be
													handled in a callback set with SDL_AddEventWatch().  Called on iOS in
													applicationDidReceiveMemoryWarning() Called on Android in onTrimMemory()
											 */
	will_enter_background = SDL_EVENT_WILL_ENTER_BACKGROUND, /**< The application is about to enter the background. This
																		event must be handled in a callback set with SDL_AddEventWatch(). Called on iOS in
																		applicationWillResignActive() Called on Android in onPause()
															 */
	did_enter_background =
			SDL_EVENT_DID_ENTER_BACKGROUND, /**< The application did enter the background and may not get CPU for some time.
							 This event must be handled in a callback set with SDL_AddEventWatch().  Called on iOS in
							 applicationDidEnterBackground()  Called on Android in onPause()
					*/
	will_enter_foreground = SDL_EVENT_WILL_ENTER_FOREGROUND, /**< The application is about to enter the foreground. This
																	 event must be handled in a callback set with SDL_AddEventWatch(). Called on iOS in
																	 applicationWillEnterForeground() Called on Android in onResume()
															*/
	did_enter_foreground = SDL_EVENT_DID_ENTER_FOREGROUND,	 /**< The application is now interactive. This event must be
																		 handled in a callback set		 with SDL_AddEventWatch().  Called on iOS in
																		 applicationDidBecomeActive()  Called on		 Android in onResume()
																*/

	locale_changed = SDL_EVENT_LOCALE_CHANGED, /**< The user's locale preferences have changed. */

	system_theme_changed = SDL_EVENT_SYSTEM_THEME_CHANGED, /**< The system theme changed */

	/* Display events */
	/* 0x150 was SDL_DISPLAYEVENT, reserve the number for sdl2-compat */
	display_orientation = SDL_EVENT_DISPLAY_ORIENTATION, /**< Display orientation has changed to data1 */
	display_added = SDL_EVENT_DISPLAY_ADDED,						 /**< Display has been added to the system */
	display_removed = SDL_EVENT_DISPLAY_REMOVED,				 /**< Display has been removed from the system */
	display_moved = SDL_EVENT_DISPLAY_MOVED,						 /**< Display has changed position */
	display_desktop_mode_changed = SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED, /**< Display has changed desktop mode */
	display_current_mode = SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED,				 /**< Display has changed current mode */
	display_content_scale = SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED,			 /**< Display has changed content scale */
	display_first = SDL_EVENT_DISPLAY_FIRST,
	display_last = SDL_EVENT_DISPLAY_LAST,

	/* Window events */
	/* 0x200 was SDL_WINDOWEVENT, reserve the number for sdl2-compat */
	/* 0x201 was SDL_SYSWMEVENT, reserve the number for sdl2-compat */
	window_shown = SDL_EVENT_WINDOW_SHOWN,		 /**< Window has been shown */
	window_hidden = SDL_EVENT_WINDOW_HIDDEN,	 /**< Window has been hidden */
	window_exposed = SDL_EVENT_WINDOW_EXPOSED, /**< Window has been exposed and should be redrawn, and can be redrawn
																directly from event watchers for this event */
	window_moved = SDL_EVENT_WINDOW_MOVED,		 /**< Window has been moved to data1, data2 */
	window_resized = SDL_EVENT_WINDOW_RESIZED, /**< Window has been resized to data1xdata2 */
	window_pixel_size_changed =
			SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED, /**< The pixel size of the window has changed to data1xdata2 */
	metal_view_resized =
			SDL_EVENT_WINDOW_METAL_VIEW_RESIZED, /**< The pixel size of a Metal view associated with the window has changed */
	window_minimized = SDL_EVENT_WINDOW_MINIMIZED,			 /**< Window has been minimized */
	window_maximized = SDL_EVENT_WINDOW_MAXIMIZED,			 /**< Window has been maximized */
	window_restored = SDL_EVENT_WINDOW_RESTORED,				 /**< Window has been restored to normal size and position */
	window_mouse_enter = SDL_EVENT_WINDOW_MOUSE_ENTER,	 /**< Window has gained mouse focus */
	window_mouse_leave = SDL_EVENT_WINDOW_MOUSE_LEAVE,	 /**< Window has lost mouse focus */
	window_focus_gained = SDL_EVENT_WINDOW_FOCUS_GAINED, /**< Window has gained keyboard focus */
	window_focus_lost = SDL_EVENT_WINDOW_FOCUS_LOST,		 /**< Window has lost keyboard focus */
	window_close_requested =
			SDL_EVENT_WINDOW_CLOSE_REQUESTED,				 /**< The window manager requests that the window be closed */
	window_hit_test = SDL_EVENT_WINDOW_HIT_TEST, /**< Window had a hit test that wasn't SDL_HITTEST_NORMAL */
	window_iccprof_changed = SDL_EVENT_WINDOW_ICCPROF_CHANGED, /**< The ICC profile of the window's display has changed */
	window_display_changed = SDL_EVENT_WINDOW_DISPLAY_CHANGED, /**< Window has been moved to display data1 */
	window_display_scale_changed = SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED, /**< Window display scale has been changed */
	window_safe_area_changed = SDL_EVENT_WINDOW_SAFE_AREA_CHANGED,				 /**< The window safe area has been changed */
	window_window_occluded = SDL_EVENT_WINDOW_OCCLUDED,										 /**< The window has been occluded */
	window_enter_fullscreen = SDL_EVENT_WINDOW_ENTER_FULLSCREEN,					 /**< The window has entered fullscreen mode */
	window_leave_full_screen = SDL_EVENT_WINDOW_LEAVE_FULLSCREEN,					 /**< The window has left fullscreen mode */
	window_destroyed = SDL_EVENT_WINDOW_DESTROYED, /**< The window with the associated ID is being or has been destroyed.
																	 If this message is being handled in an event watcher, the window handle is still
																	 valid and can still be used to retrieve any properties associated with the window.
																	 Otherwise, the handle has already been destroyed and all resources associated with it
																	 are invalid */
	window_hdr_state_changed = SDL_EVENT_WINDOW_HDR_STATE_CHANGED, /**< Window HDR properties have changed */

	/* Keyboard events */
	key_down = SDL_EVENT_KEY_DOWN = 0x300,											 /**< Key pressed */
	key_up = SDL_EVENT_KEY_UP,																	 /**< Key released */
	key_editing = SDL_EVENT_TEXT_EDITING,												 /**< Keyboard text editing (composition) */
	key_text_input = SDL_EVENT_TEXT_INPUT,											 /**< Keyboard text input */
	keymap_changed = SDL_EVENT_KEYMAP_CHANGED,									 /**< Keymap changed due to a system event such as an
																										input language or keyboard layout change. */
	keyboard_added = SDL_EVENT_KEYBOARD_ADDED,									 /**< A new keyboard has been inserted into the system */
	kayboard_removed = SDL_EVENT_KEYBOARD_REMOVED,							 /**< A keyboard has been removed */
	text_editing_candidates = SDL_EVENT_TEXT_EDITING_CANDIDATES, /**< Keyboard text editing candidates */

	/* Mouse events */
	mouse_motion = SDL_EVENT_MOUSE_MOTION = 0x400,	 /**< Mouse moved */
	mouse_button_down = SDL_EVENT_MOUSE_BUTTON_DOWN, /**< Mouse button pressed */
	mouse_button_up = SDL_EVENT_MOUSE_BUTTON_UP,		 /**< Mouse button released */
	mouse_wheel = SDL_EVENT_MOUSE_WHEEL,						 /**< Mouse wheel motion */
	mouse_added = SDL_EVENT_MOUSE_ADDED,						 /**< A new mouse has been inserted into the system */
	mouse_removed = SDL_EVENT_MOUSE_REMOVED,				 /**< A mouse has been removed */

	/* Joystick events */
	joystick_axis_motion = SDL_EVENT_JOYSTICK_AXIS_MOTION = 0x600, /**< Joystick axis motion */
	joystick_ball_motion = SDL_EVENT_JOYSTICK_BALL_MOTION,				 /**< Joystick trackball motion */
	joystick_hat_motion = SDL_EVENT_JOYSTICK_HAT_MOTION,					 /**< Joystick hat position change */
	joystick_button_down = SDL_EVENT_JOYSTICK_BUTTON_DOWN,				 /**< Joystick button pressed */
	joystick_button_up = SDL_EVENT_JOYSTICK_BUTTON_UP,						 /**< Joystick button released */
	joystick_added_SDL_EVENT_JOYSTICK_ADDED,			 /**< A new joystick has been inserted into the system */
	joystick_removed = SDL_EVENT_JOYSTICK_REMOVED, /**< An opened joystick has been removed */

	/* Gamepad events */
	gamepad_axis_motion = SDL_EVENT_GAMEPAD_AXIS_MOTION = 0x650, /**< Gamepad axis motion */
	gamepad_button_down = SDL_EVENT_GAMEPAD_BUTTON_DOWN,				 /**< Gamepad button pressed */
	gamepad_button_up = SDL_EVENT_GAMEPAD_BUTTON_UP,						 /**< Gamepad button released */
	gamepad_added = SDL_EVENT_GAMEPAD_ADDED,										 /**< A new gamepad has been inserted into the system */
	gamepad_removed = SDL_EVENT_GAMEPAD_REMOVED,								 /**< A gamepad has been removed */
	gamepad_remapped = SDL_EVENT_GAMEPAD_REMAPPED,							 /**< The gamepad mapping was updated */
	gamepad_touchpad_down = SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN,		 /**< Gamepad touchpad was touched */
	gamepad_touchpad_motion = SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION, /**< Gamepad touchpad finger was moved */
	gamepad_touchpad_up = SDL_EVENT_GAMEPAD_TOUCHPAD_UP,				 /**< Gamepad touchpad finger was lifted */
	gamepad_sensor_update = SDL_EVENT_GAMEPAD_SENSOR_UPDATE,		 /**< Gamepad sensor was updated */
	gamepad_update_complete = SDL_EVENT_GAMEPAD_UPDATE_COMPLETE, /**< Gamepad update is complete */
	gamepad_steam_handle_update = SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED, /**< Gamepad Steam handle has changed */

	/* Touch events */
	finger_down = SDL_EVENT_FINGER_DOWN = 0x700,
	finger_up = SDL_EVENT_FINGER_UP,
	finger_motion = SDL_EVENT_FINGER_MOTION,
	finger_canceled = SDL_EVENT_FINGER_CANCELED,

	/* 0x800, 0x801, and 0x802 were the Gesture events from SDL2. Do not reuse these values! sdl2-compat needs them! */

	/* Clipboard events */
	clipboard_update = SDL_EVENT_CLIPBOARD_UPDATE = 0x900, /**< The clipboard or primary selection changed */

	/* Drag and drop events */
	drop_file = SDL_EVENT_DROP_FILE = 0x1000, /**< The system requests a file open */
	drop_text = SDL_EVENT_DROP_TEXT,					/**< text/plain drag-and-drop event */
	drop_begin = SDL_EVENT_DROP_BEGIN,				/**< A new set of drops is beginning (NULL filename) */
	drop_complete = SDL_EVENT_DROP_COMPLETE,	/**< Current set of drops is now complete (NULL filename) */
	drop_position = SDL_EVENT_DROP_POSITION,	/**< Position while moving over the window */

	/* Audio hotplug events */
	audio_device_added = SDL_EVENT_AUDIO_DEVICE_ADDED = 0x1100, /**< A new audio device is available */
	audio_device_removed = SDL_EVENT_AUDIO_DEVICE_REMOVED,			/**< An audio device has been removed. */
	audio_device_format_changed =
			SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED, /**< An audio device's format has been changed by the system. */

	/* Sensor events */
	sensor_update = SDL_EVENT_SENSOR_UPDATE = 0x1200, /**< A sensor was updated */

	/* Pressure-sensitive pen events */
	pen_proximity_in = SDL_EVENT_PEN_PROXIMITY_IN = 0x1300, /**< Pressure-sensitive pen has become available */
	pen_proximity_out = SDL_EVENT_PEN_PROXIMITY_OUT,				/**< Pressure-sensitive pen has become unavailable */
	pen_down = SDL_EVENT_PEN_DOWN,													/**< Pressure-sensitive pen touched drawing surface */
	pen_up = SDL_EVENT_PEN_UP,									 /**< Pressure-sensitive pen stopped touching drawing surface */
	pen_button_down = SDL_EVENT_PEN_BUTTON_DOWN, /**< Pressure-sensitive pen button pressed */
	pen_button_up = SDL_EVENT_PEN_BUTTON_UP,		 /**< Pressure-sensitive pen button released */
	pen_motion = SDL_EVENT_PEN_MOTION,					 /**< Pressure-sensitive pen is moving on the tablet */
	pen_axis = SDL_EVENT_PEN_AXIS,							 /**< Pressure-sensitive pen angle/pressure/etc changed */

	/* Camera hotplug events */
	camera_device_added = SDL_EVENT_CAMERA_DEVICE_ADDED = 0x1400, /**< A new camera device is available */
	camera_device_removed = SDL_EVENT_CAMERA_DEVICE_REMOVED,			/**< A camera device has been removed. */
	camera_device_approved =
			SDL_EVENT_CAMERA_DEVICE_APPROVED,									 /**< A camera device has been approved for use by the user. */
	camera_device_denied = SDL_EVENT_CAMERA_DEVICE_DENIED, /**< A camera device has been denied for use by the user. */

	/* Render events */
	render_targets_resetSDL_EVENT_RENDER_TARGETS_RESET =
			0x2000, /**< The render targets have been reset and their contents need to be updated */
	render_device_reset =
			SDL_EVENT_RENDER_DEVICE_RESET, /**< The device has been reset and all textures need to be recreated */
	render_device_lost = SDL_EVENT_RENDER_DEVICE_LOST, /**< The device has been lost and can't be recovered. */

	event_user = SDL_EVENT_USER,
};
inline SDL_EventType to_sdl(EventType t) {
	return static_cast<SDL_EventType>(t);
}

struct KeyboardEvent {
	SDL_EventType type; /**< SDL_EVENT_KEY_DOWN or SDL_EVENT_KEY_UP */
	Uint32 reserved;
	Uint64 timestamp;			 /**< In nanoseconds, populated using SDL_GetTicksNS() */
	SDL_WindowID windowID; /**< The window with keyboard focus, if any */
	SDL_KeyboardID which;	 /**< The keyboard instance id, or 0 if unknown or virtual */
	SDL_Scancode scancode; /**< SDL physical key code */
	SDL_Keycode key;			 /**< SDL virtual key code */
	SDL_Keymod mod;				 /**< current key modifiers */
	Uint16 raw;						 /**< The platform dependent scancode for this event */
	bool down;						 /**< true if the key is pressed */
	bool repeat;					 /**< true if this is a key repeat */
};
SDL_KeyboardEvent to_sdl(KeyboardEvent e) { return static_cast<SDL_KeyboardEvent>(e) };

struct Event {
	EventType type;						/**< Event type, shared with all events, Uint32 to cover user events which are not in the
														SDL_EventType					 enumeration */
	SDL_CommonEvent common;		/**< Common event data */
	SDL_DisplayEvent display; /**< Display event data */
	SDL_WindowEvent window;		/**< Window event data */
	SDL_KeyboardDeviceEvent kdevice;								/**< Keyboard device change event data */
	SDL_KeyboardEvent key;													/**< Keyboard event data */
	SDL_TextEditingEvent edit;											/**< Text editing event data */
	SDL_TextEditingCandidatesEvent edit_candidates; /**< Text editing candidates event data */
	SDL_TextInputEvent text;												/**< Text input event data */
	SDL_MouseDeviceEvent mdevice;										/**< Mouse device change event data */
	SDL_MouseMotionEvent motion;										/**< Mouse motion event data */
	SDL_MouseButtonEvent button;										/**< Mouse button event data */
	SDL_MouseWheelEvent wheel;											/**< Mouse wheel event data */
	SDL_JoyDeviceEvent jdevice;											/**< Joystick device change event data */
	SDL_JoyAxisEvent jaxis;													/**< Joystick axis event data */
	SDL_JoyBallEvent jball;													/**< Joystick ball event data */
	SDL_JoyHatEvent jhat;														/**< Joystick hat event data */
	SDL_JoyButtonEvent jbutton;											/**< Joystick button event data */
	SDL_JoyBatteryEvent jbattery;										/**< Joystick battery event data */
	SDL_GamepadDeviceEvent gdevice;									/**< Gamepad device event data */
	SDL_GamepadAxisEvent gaxis;											/**< Gamepad axis event data */
	SDL_GamepadButtonEvent gbutton;									/**< Gamepad button event data */
	SDL_GamepadTouchpadEvent gtouchpad;							/**< Gamepad touchpad event data */
	SDL_GamepadSensorEvent gsensor;									/**< Gamepad sensor event data */
	SDL_AudioDeviceEvent adevice;										/**< Audio device event data */
	SDL_CameraDeviceEvent cdevice;									/**< Camera device event data */
	SDL_SensorEvent sensor;													/**< Sensor event data */
	SDL_QuitEvent quit;															/**< Quit request event data */
	SDL_UserEvent user;															/**< Custom event data */
	SDL_TouchFingerEvent tfinger;										/**< Touch finger event data */
	SDL_PenProximityEvent pproximity;								/**< Pen proximity event data */
	SDL_PenTouchEvent ptouch;												/**< Pen tip touching event data */
	SDL_PenMotionEvent pmotion;											/**< Pen motion event data */
	SDL_PenButtonEvent pbutton;											/**< Pen button event data */
	SDL_PenAxisEvent paxis;													/**< Pen axis event data */
	SDL_RenderEvent render;													/**< Render event data */
	SDL_DropEvent drop;															/**< Drag and drop event data */
	SDL_ClipboardEvent clipboard;										/**< Clipboard event data */
};

} // namespace vis::win