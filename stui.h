/*  
	A simple header-only library for creating text-based user interfaces
	inside a terminal window.
	Copyright (C) 2024  Jacob Costen (oculometric)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

	Contact me here <mailto://jcostenart@gmail.com>
*/

#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstring>
#include <cmath>
#include <sstream>
#include <iomanip>

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#define VC_EXTRALEAN
	#include <Windows.h>
#elif defined(__linux__)
	#include <sys/ioctl.h>
	#include <signal.h>
	#include <termios.h>
	#include <poll.h>
#endif

using namespace std;

#if defined(_WIN32)
	using clock_type = chrono::steady_clock;
#elif defined(__linux__)
	using clock_type = chrono::system_clock;
#endif

#define RENDER_STUB virtual void render(Tixel* output_buffer, Coordinate size) override
#define GETMINSIZE_STUB virtual inline Coordinate getMinSize() override
#define GETMAXSIZE_STUB virtual inline Coordinate getMaxSize() override
#define HANDLEINPUT_STUB virtual bool handleInput(uint8_t input_character, Input::ControlKeys modifiers) override
#define	ISFOCUSABLE_STUB virtual inline bool isFocusable() override

#define OUTPUT_TARGET cout

#define ANSI_ESCAPE '\033'
#define ANSI_CLEAR_SCREEN ANSI_ESCAPE << "[2J" 
#define ANSI_CLEAR_SCROLL ANSI_ESCAPE << "[3J"
#define ANSI_SET_CURSOR(x,y) ANSI_ESCAPE << '[' << y << ';' << x << 'H'
#define ANSI_HIDE_CURSOR ANSI_ESCAPE << "[?25l"
#define ANSI_SHOW_CURSOR ANSI_ESCAPE << "[?25h"
#define ANSI_PANUP(n) ANSI_ESCAPE << '[' << n << 'T'
#define ANSI_PANDOWN(n) ANSI_ESCAPE << '[' << n << 'S'
#define ANSI_SET_COLOUR(c) ANSI_ESCAPE << '[' << to_string(c) << 'm'

#define UNICODE_BLOCK (uint32_t)0x8896e2
#define UNICODE_BLOCK_1_8 (uint32_t)0x8196e2
#define UNICODE_BLOCK_3_8 (uint32_t)0x8396e2
#define UNICODE_BLOCK_6_8 (uint32_t)0x8696e2
#define UNICODE_BLOCK_6_8 (uint32_t)0x8696e2
#define UNICODE_LIGHT_SHADE (uint32_t)0x9196e2
#define UNICODE_MID_SHADE (uint32_t)0x9296e2
#define UNICODE_DARK_SHADE (uint32_t)0x9396e2
#define UNICODE_BOX_TOPLEFT (uint32_t)0x8f94e2
#define UNICODE_BOX_HORIZONTAL (uint32_t)0x8194e2
#define UNICODE_BOX_TOPRIGHT (uint32_t)0x9394e2
#define UNICODE_BOX_VERTICAL (uint32_t)0x8394e2
#define UNICODE_BOX_BOTTOMLEFT (uint32_t)0x9794e2
#define UNICODE_BOX_BOTTOMRIGHT (uint32_t)0x9b94e2
#define UNICODE_QUADRANT_LOWERLEFT (uint32_t)0x9696e2
#define UNICODE_QUADRANT_TOPLEFT (uint32_t)0x9896e2
#define UNICODE_QUADRANT_TOPRIGHT (uint32_t)0x9d96e2
#define UNICODE_QUADRANT_LOWERRIGHT (uint32_t)0x9796e2
#define UNICODE_BOXLIGHT_UP (uint32_t)0xb595e2
#define UNICODE_BOXLIGHT_UPRIGHT (uint32_t)0x9494e2
#define UNICODE_BOXLIGHT_UPRIGHTDOWN (uint32_t)0x9c94e2
#define UNICODE_BOXLIGHT_UPRIGHTDOWNLEFT (uint32_t)0xbc94e2
#define UNICODE_MIDDLE_DOT (uint32_t)0xb7c2
#define UNICODE_NOT (uint32_t)0xacc2
#define UNICODE_CIRCLE_HOLLOW (uint32_t)0xbe8ce2
#define UNICODE_CIRCLE_FILLED (uint32_t)0x998ae2

namespace stui
{

static ofstream debug_log;

static inline void debug(string str)
{
	if (!debug_log.is_open()) return;
	auto now = chrono::system_clock::now().time_since_epoch();
	auto hours = chrono::duration_cast<chrono::hours>(now);
	auto minutes = chrono::duration_cast<chrono::minutes>(now - hours);
	auto seconds = chrono::duration_cast<chrono::seconds>(now - hours - minutes);
	auto millis = chrono::duration_cast<chrono::milliseconds>(now - hours - minutes - seconds);

	debug_log << '[' << setfill('0') << setw(2) << hours.count() % 24 << ':' << setw(2) << minutes.count() << ':' << setw(2) << seconds.count() << '.' << setw(3) << millis.count() << ']' << ' ' << str << endl;
	debug_log.flush();
}

/**
 * @brief removes any invalid characters from a string, as well as other
 * specified illegal characters
 * 
 * @param str string to remove invalid characters from
 * @param others array of additional characters that should be excluded
 * @return repaired string 
 **/
string stripNullsAndMore(string str, const char* others)
#ifdef STUI_IMPLEMENTATION
{
    string result = "";
    for (char c : str)
    {
		// auto-strip if the character is a non-renderable
        if (c < ' ' && c != '\n' && c != '\t' && c != '\b') continue;
		// check if the current char is inside list to remove
		bool is_valid = true;
		for (size_t i = 0; others[i] != '\0'; i++)
			if (c == others[i]) is_valid = false;
		if (is_valid)
		{
			// if not, keep it, swapping tab for some spaces
			if (c == '\t') result += "    ";
			else result += c;
		}
    }
    return result;
}
#endif
;

/**
 * @brief two-dimensional integer coordinate pair.
 * 
 **/
struct Coordinate
{
	int x, y;
};

/**
 * @brief structure describing the value of a terminal-pixel, which
 * may optionally have a command to change colour specified.
 * 
 * if `PASS` is specified for either side of the colour command (FG/BG),
 * then this `Tixel` will inherit the colour configuration of the
 * previous pixel (for whichever of FG/BG were not specified for this
 * `Tixel`).
 **/
struct Tixel
{
	/**
	 * @brief enumerates possible foreground and background colours
	 * for a `Tixel`.
	 * 
	 * one FG and one BG colour can be or-ed together to set both on
	 * this `Tixel`
	 **/
	enum ColourCommand
	{
		FG_BLACK 	= 0b00000001,
		FG_RED   	= 0b00000010,
		FG_GREEN 	= 0b00000100,
		FG_BLUE  	= 0b00001000,
		FG_YELLOW 	= 0b00000110,
		FG_CYAN	    = 0b00001100,
		FG_MAGENTA	= 0b00001010,
		FG_GRAY		= 0b00001110,
		FG_WHITE	= 0b00001111,
		BG_BLACK 	= 0b00010000,
		BG_RED   	= 0b00100000,
		BG_GREEN 	= 0b01000000,
		BG_BLUE  	= 0b10000000,
		BG_YELLOW 	= 0b01100000,
		BG_CYAN	    = 0b11000000,
		BG_MAGENTA	= 0b10100000,
		BG_GRAY		= 0b11100000,
		BG_WHITE	= 0b11110000
	};

	uint32_t character = ' ';
	ColourCommand colour = (ColourCommand)(ColourCommand::FG_WHITE | ColourCommand::BG_BLUE);

	inline void operator=(uint32_t c) { character = static_cast<uint32_t>(c); }
	inline void operator=(uint8_t c) { character = static_cast<uint32_t>(c); }
	inline void operator=(char c) { character = static_cast<uint32_t>(static_cast<uint8_t>(c)); }

	static int toANSI(ColourCommand c)
#ifdef STUI_IMPLEMENTATION
	{
		switch (c)
		{
		case FG_BLACK: return 30;
		case BG_BLACK: return 40;
		case FG_RED: return 91;
		case BG_RED: return 101;
		case FG_GREEN: return 92;
		case BG_GREEN: return 102;
		case FG_YELLOW: return 93;
		case BG_YELLOW: return 103;
		case FG_BLUE: return 94;
		case BG_BLUE: return 104;
		case FG_MAGENTA: return 95;
		case BG_MAGENTA: return 105;
		case FG_CYAN: return 96;
		case BG_CYAN: return 106;
		case FG_GRAY: return 90;
		case BG_GRAY: return 100;
		case FG_WHITE: return 97;
		case BG_WHITE: return 107;
		default: return 40;
		}
	}
#endif
	;
};

/**
 * @brief class which encapsulates input functionality which is used to receive and handle
 * input in useful ways. another way of encapsulating functionality to hide it from the you!
 * just pretend this isn't here.
 **/
class Input
{
	friend class Page;

public:
	/**
	 * @brief enumerates the possible control key states (shift, alt, and control). 
	 * 
	 * left and right shift are treated as the same on Windows, so for cross-platform-ness i'll
	 * settle for that.
	 **/
	enum ControlKeys
	{
		NONE        = 0b00000000,
		CTRL  		= 0b00000001,
		SHIFT       = 0b00000010,
		ALT    		= 0b00000100
	};

	/**
	 * @brief enumerates the arrow keys for identifying directional key strokes
	 **/
	enum ArrowKeys
	{
		UP          = 0x11,
		DOWN		= 0x12,
		LEFT		= 0x13,
		RIGHT		= 0x14
	};

	/**
	 * @brief describes a key input event. though we only care about key down or key repeat
	 * events.
	 **/
	struct Key
	{
		uint16_t key;
		ControlKeys control_states;
	};

	/**
	 * @brief describes a shortcut linking a desired key-bind to a function that should
	 * be called when the binding is triggered.
	 **/
	struct Shortcut
	{
		Input::Key binding;
		void (*callback)();
	};

private:
#if defined(__linux__)
	static constexpr Key linux_keymap[128] =
	{// 0x_0 / 0x_8      0x_1 / 0x_9      0x_2 / 0x_A      0x_3 / 0x_B      0x_4 / 0x_C      0x_5 / 0x_D      0x_6 / 0x_E      0x_7 / 0x_F
/*0x0_*/{ ' ', CTRL },   { 'A', CTRL },   { 'B', CTRL },   { 'C', CTRL },   { 'D', CTRL },   { 'E', CTRL },   { 'F', CTRL },   { 'G', CTRL },
/*0x0_*/{ '\b', NONE },  { '\t', NONE },  { '\n', NONE },  { 'K', CTRL },   { 'L', CTRL },   { 'M', CTRL },   { 'N', CTRL },   { 'O', CTRL },
/*0x1_*/{ 'P', CTRL },   { 'Q', CTRL },   { 'R', CTRL },   { 'S', CTRL },   { 'T', CTRL },   { 'U', CTRL },   { 'V', CTRL },   { 'W', CTRL },
/*0x1_*/{ 'X', CTRL },   { 'Y', CTRL },   { 'Z', CTRL },   { '\e', NONE },  { '\x1c', NONE },{ '\x1d', NONE },{ '\x1e', NONE },{ '\x1f', NONE },
/*0x2_*/{ ' ', NONE },   { '!', SHIFT },  { '"', SHIFT },  { '#', NONE },   { '$', SHIFT },  { '%', SHIFT },  { '&', SHIFT },  { '\'', NONE },
/*0x2_*/{ '(', SHIFT },  { ')', SHIFT },  { '*', SHIFT },  { '+', SHIFT },  { ',', NONE },   { '-', NONE },   { '.', NONE },   { '/', NONE },
/*0x3_*/{ '0', NONE },   { '1', NONE },   { '2', NONE },   { '3', NONE },   { '4', NONE },   { '5', NONE },   { '6', NONE },   { '7', NONE },
/*0x3_*/{ '8', NONE },   { '9', NONE },   { ':', SHIFT },  { ';', NONE },   { '<', SHIFT },  { '=', NONE },   { '>', SHIFT },  { '?', SHIFT },
/*0x4_*/{ '@', SHIFT },  { 'A', SHIFT },  { 'B', SHIFT },  { 'C', SHIFT },  { 'D', SHIFT },  { 'E', SHIFT },  { 'F', SHIFT },  { 'G', SHIFT },
/*0x4_*/{ 'H', SHIFT },  { 'I', SHIFT },  { 'J', SHIFT },  { 'K', SHIFT },  { 'L', SHIFT },  { 'M', SHIFT },  { 'N', SHIFT },  { 'O', SHIFT },
/*0x5_*/{ 'P', SHIFT },  { 'Q', SHIFT },  { 'R', SHIFT },  { 'S', SHIFT },  { 'T', SHIFT },  { 'U', SHIFT },  { 'V', SHIFT },  { 'W', SHIFT },
/*0x5_*/{ 'X', SHIFT },  { 'Y', SHIFT },  { 'Z', SHIFT },  { '[', NONE },   { '\\', NONE },  { ']', NONE },   { '^', SHIFT },  { '_', SHIFT },
/*0x6_*/{ '`', NONE },   { 'a', NONE },   { 'b', NONE },   { 'c', NONE },   { 'd', NONE },   { 'e', NONE },   { 'f', NONE },   { 'g', NONE },
/*0x6_*/{ 'h', NONE },   { 'i', NONE },   { 'j', NONE },   { 'k', NONE },   { 'l', NONE },   { 'm', NONE },   { 'n', NONE },   { 'o', NONE },
/*0x7_*/{ 'p', NONE },   { 'q', NONE },   { 'r', NONE },   { 's', NONE },   { 't', NONE },   { 'u', NONE },   { 'v', NONE },   { 'w', NONE },
/*0x7_*/{ 'x', NONE },   { 'y', NONE },   { 'z', NONE },   { '{', SHIFT },  { '|', SHIFT },  { '}', SHIFT },  { '~', SHIFT },  { '\b', NONE }
	};
#endif

	/**
	 * @brief queries the system's input buffer and fetches all available key events.
	 * 
	 * only returns the key-down key events (and repeats). translates presence of meta
	 * keys like CTRL, SHIFT, ALT. also moves some relevant key presses down into the
	 * ASCII range to make them easier to read (specifically the arrow keys).
	 * 
	 * @returns list of key-press events
	 **/
	static vector<Key> getQueuedKeyEvents()
#ifdef STUI_IMPLEMENTATION
	{
		vector<Key> events;
#if defined(_WIN32)
		// find available
		DWORD events_available;
		GetNumberOfConsoleInputEvents(GetStdHandle(STD_INPUT_HANDLE), &events_available);
		if (events_available < 1) return events;

		// grab 32 of them
		INPUT_RECORD records[32] = { };
		DWORD records_read;

		if (ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), records, 32, &records_read) == 0)
			throw runtime_error("input error");
		
		for (size_t i = 0; i < records_read; i++)
			if (records[i].EventType == KEY_EVENT && records[i].Event.KeyEvent.bKeyDown)
			{
				Key k{ };
				k.key = records[i].Event.KeyEvent.uChar.AsciiChar;
				if (records[i].Event.KeyEvent.wVirtualKeyCode == VK_UP) k.key = ArrowKeys::UP;
				else if (records[i].Event.KeyEvent.wVirtualKeyCode == VK_DOWN) k.key = ArrowKeys::DOWN;
				else if (records[i].Event.KeyEvent.wVirtualKeyCode == VK_LEFT) k.key = ArrowKeys::LEFT;
				else if (records[i].Event.KeyEvent.wVirtualKeyCode == VK_RIGHT) k.key = ArrowKeys::RIGHT;
				if (k.key == '\r') k.key = '\n';
				k.control_states = ControlKeys::NONE;
				DWORD control_key = records[i].Event.KeyEvent.dwControlKeyState;
				if (control_key & 0x01) k.control_states = (ControlKeys)(k.control_states | ControlKeys::ALT);
				if (control_key & 0x02) k.control_states = (ControlKeys)(k.control_states | ControlKeys::ALT);
				if (control_key & 0x04) k.control_states = (ControlKeys)(k.control_states | ControlKeys::CTRL);
				if (control_key & 0x08)
				{
					k.control_states = (ControlKeys)(k.control_states | ControlKeys::CTRL);
					k.key += 96;
				}
				if (control_key & 0x10) k.control_states = (ControlKeys)(k.control_states | ControlKeys::SHIFT);
				events.push_back(k);
			}
#elif defined(__linux__)
		// check if there are any events to read
		if (kbhit() < 1) return events;

		// read a queue of 64 events (or as many are available)
		uint8_t buffer[64] = { 0 };
		ssize_t bytes_read = read(STDIN_FILENO, buffer, 64);
		for (ssize_t i = 0; i < bytes_read; i++)
		{
			int c = buffer[i];
			// process escape sequences
			if (c == '\e')
			{
				// unless it's actually just escape
				if (i == bytes_read - 1) events.push_back(Key{ '\e', ControlKeys::NONE });
				else if (buffer[i + 1] != '[')
				{
					// if there's no bracket then it's an alt-event
					int c_next = buffer[i + 1];
					events.push_back(Key{ linux_keymap[c_next].key, ControlKeys::ALT });
					// skip to the next char so we don't process the same one twice
					i++;
				}
				else if (i == bytes_read - 2)
				{
					// it could just be alt-bracket, not an escape sequences
					events.push_back(Key{ '[', ControlKeys::ALT });
					i++;
				}
				else
				{
					// otherwise it's an escape sequence
					int c_next = buffer[i + 2];
					if (c_next == 0x41) events.push_back(Key{ ArrowKeys::UP, ControlKeys::NONE });
					else if (c_next == 0x42) events.push_back(Key{ ArrowKeys::DOWN, ControlKeys::NONE });
					else if (c_next == 0x44) events.push_back(Key{ ArrowKeys::LEFT, ControlKeys::NONE });
					else if (c_next == 0x43) events.push_back(Key{ ArrowKeys::RIGHT, ControlKeys::NONE });
					else if (c_next == '1' && buffer[i + 3] == ';' && buffer[i + 4] == '2')
					{
						int c_next_next = buffer[i + 5];
						if (c_next_next == 0x41) events.push_back(Key{ ArrowKeys::UP, ControlKeys::SHIFT });
						else if (c_next_next == 0x42) events.push_back(Key{ ArrowKeys::DOWN, ControlKeys::SHIFT });
						else if (c_next_next == 0x44) events.push_back(Key{ ArrowKeys::LEFT, ControlKeys::SHIFT });
						else if (c_next_next == 0x43) events.push_back(Key{ ArrowKeys::RIGHT, ControlKeys::SHIFT });
						i += 3;
					}
					else
					{
						// debug if it isnt handled
						debug("unhandled ANSI code: " + string((char*)buffer + i));
						break;
					}
					i += 2;
				}
			}
			else if (c < 128)
			{
				// normal keypress, use the keymap as a look-up table
				events.push_back(linux_keymap[c]);
			}
			else
			{
				debug("uh oh! unhandled UTF-8");
				// TODO: handle utf8 oh dear god
				break;
			}
		}
#endif
		return events;
	}
#endif
	;

	/**
	 * @brief checks if a key event is equal to another specified key event
	 * 
	 * @param a first key event
	 * @param b second key event
	 * @returns whether or not the key events are identical
	 **/
	static inline bool compare(Key a, Key b)
	{
		return a.key == b.key && a.control_states == b.control_states;
	}
	
	/**
	 * @brief iterate through a list of key events and handle any shortcuts which
	 * are found within it. 
	 * 
	 * when a shortcut instance is found, that key event is consumed and removed
	 * from the input list.
	 * 
	 * @param shortcuts list of keyboard shortcut bindings
	 * @param key_events list of key events to check
	 **/
	static void processShortcuts(vector<Shortcut> shortcuts, vector<Key>& key_events)
#ifdef STUI_IMPLEMENTATION
	{
		vector<Key> non_processed;
		for (size_t i = 0; i < key_events.size(); i++)
		{
			Key k = key_events[i];
			bool consumed = false;
			for (Shortcut s : shortcuts)
			{
				if (compare(k, s.binding)) { consumed = true; s.callback(); }
			}
			if (!consumed) non_processed.push_back(k);
		}

		key_events = non_processed;
	}
#endif
	;
	
	/**
	 * @brief iterate through a list of key events and extract only the text
	 * characters into a list.
	 * 
	 * when a text character is found, it is consumed and removed from the input
	 * list.
	 * 
	 * @param key_events list of key events to check
	 * @returns list of extracted text characters
	 **/
	static vector<pair<uint8_t, ControlKeys>> getTextCharacters(vector<Key>& key_events)
#ifdef STUI_IMPLEMENTATION
	{
		vector<Key> non_processed;
		vector<pair<uint8_t, ControlKeys>> result;
		for (size_t i = 0; i < key_events.size(); i++)
		{
			Key k = key_events[i];
			if ((k.control_states == ControlKeys::NONE || k.control_states == ControlKeys::SHIFT) && (
				  (k.key >= 32 && k.key <= 127) 
				|| k.key == '\n' 
				|| k.key == '\t' 
				|| k.key == '\b'
				|| k.key == ArrowKeys::UP
				|| k.key == ArrowKeys::DOWN
				|| k.key == ArrowKeys::LEFT
				|| k.key == ArrowKeys::RIGHT))
			{
				result.push_back(pair<uint8_t, ControlKeys>(static_cast<uint8_t>(k.key), k.control_states));
			}
			else
			{
				non_processed.push_back(k);
			}
		}

		key_events = non_processed;

		return result;
	}
#endif
	;

	static inline int kbhit()
	{
		pollfd pfd;
		pfd.fd = STDIN_FILENO;
		pfd.events = POLLIN;
		return poll(&pfd, 1, 0);
	}
};

/**
 * @brief base class from which all UI components inherit.
 * 
 * all `Component` subclasses must override the `render`, `getMaxSize`, and 
 * `getMinSize` methods (ideally using the relevant macros).
 **/
class Component
{
public:
	bool focused = false;

	/**
	 * @brief draws the `Component` into a `Tixel` buffer, with a specified size.
	 * 
	 * implementations should assume that the `output_buffer` has been allocated to be the correct
	 * `size`, but should also check that any drawing is performed within the limits of the `size`
	 * (otherwise you'll corrupt the heap, yippee!).
	 * 
	 * @param output_buffer buffer allocated by the caller to be drawn into. ordered left
	 * to right, top to bottom
	 * @param size size of the buffer
	 **/
	virtual inline void render(Tixel* output_buffer, Coordinate size) { }
	
	/**
	 * @brief returns the maximum desired size that this component should be given.
	 * 
	 * note that the component may still be givena buffer larger than this size, so this should 
	 * be handled if appropriate in `render`. either or both of these values can be given the 
	 * value `-1` to indicate no maximum size in that dimension.
	 * 
	 * @returns a `Coordinate` representing the maximum size for the `Component`
	 **/
	virtual inline Coordinate getMaxSize() { return { 0, 0 }; }

	/**
	 * @brief returns the minimum size that this component should be given to draw into.
	 *
	 * functions which call `render` will try to ensure all components have at least
	 * their minimum size requirements met, otherwise if not enough space cannot be allocated,
	 * the `Component` will not be rendered. this function should never return negative values
	 * for either dimension.
	 *
	 * @returns a `Coordinate` representing the minimum size for the `Component`
	 **/
	virtual inline Coordinate getMinSize() { return { 0, 0 }; }

	/**
	 * @brief handle a single key-press event.
	 * 
	 * not all subclasses need to implement this, but if you want to handle input you probably
	 * should.
	 * 
	 * @param input_character the character to handle
	 * @returns true if the input was consumed or false if not
	 **/
	virtual inline bool handleInput(uint8_t input_character, Input::ControlKeys modifiers) { return false; }

	/**
	 * @brief returns whether or not the component can be focused for input.
	 * 
	 * should be overriden by subclasses which want to receive input.
	 * 
	 * @returns true if the component is focusable
	 **/
	virtual inline bool isFocusable() { return false; }
};

/**
 * @brief class which encapsulates the utility functions that many of `Component`s reuse but
 * which probably shouldn't be publicly accessible outside the header file. basically ignore
 * this.
 **/
class Utility
{
protected:
	/**
	 * @brief draws a box outline using IBM box drawing characters from standard extended ASCII.
	 * 
	 * it's up to the caller to ensure `buffer` is allocated to the size specified by `buffer_size`
	 * 
	 * @param box_origin offset of the start of the box from the top-left corner of the buffer.
	 * may be negative, though areas of the box outside the buffer won't be drawn
	 * @param box_size size of the box. must be positive in both axes. can result in a box which
	 * ends outside the bounds of the buffer, but areas outside won't be drawn
	 * @param buffer pointer to a character array ordered left-to-right, top-to-bottom
	 * @param buffer_size size of the allocated buffer, must match with the size of the `buffer`
	 **/
	static void drawBox(Coordinate box_origin, Coordinate box_size, Tixel* buffer, Coordinate buffer_size)
#ifdef STUI_IMPLEMENTATION
	{
		if (buffer == nullptr) return;
		if (box_size.x <= 0 || box_size.y <= 0) return;

		if (box_origin.y >= 0 && box_origin.y < buffer_size.y)
		{
			for (int x = box_origin.x; x < box_origin.x + box_size.x; x++)
			{
				if (x < 0) continue;
				if (x >= buffer_size.x) break;

				buffer[x + (box_origin.y * buffer_size.x)] = (x == box_origin.x ? UNICODE_BOX_TOPLEFT : (x == box_origin.x + box_size.x - 1 ? UNICODE_BOX_TOPRIGHT : UNICODE_BOX_HORIZONTAL));
			}
		}

		for (int y = box_origin.y + 1; y < box_origin.y + box_size.y - 1; y++)
		{
			if (y < 0) continue;
			if (y >= buffer_size.y) break;

			if (box_origin.x >= 0 && box_origin.x < buffer_size.x)
				buffer[box_origin.x + (y * buffer_size.x)] = UNICODE_BOX_VERTICAL;

			if (box_origin.x + box_size.x - 1 >= 0 && box_origin.x + box_size.x - 1 < buffer_size.x)
				buffer[box_origin.x + box_size.x + (y * buffer_size.x) - 1] = UNICODE_BOX_VERTICAL;
		}

		if (box_origin.y + box_size.y - 1 >= 0 && box_origin.y + box_size.y - 1 < buffer_size.y)
		{
			for (int x = box_origin.x; x < box_origin.x + box_size.x; x++)
			{
				if (x < 0) continue;
				if (x >= buffer_size.x) break;

				buffer[x + ((box_origin.y + box_size.y - 1) * buffer_size.x)] = (x == box_origin.x ? UNICODE_BOX_BOTTOMLEFT : (x == box_origin.x + box_size.x - 1 ? UNICODE_BOX_BOTTOMRIGHT : UNICODE_BOX_HORIZONTAL));
			}
		}
	}
#endif
	;

	/**
	 * @brief converts a single string into an array of lines of a given maximum length.
	 * 
	 * respects line breaks, uses word-wrapping where possible, or forcibly breaks words 
	 * if necessary.
	 * 
	 * @param text string on which to perform wrapping
	 * @param max_width maximum number of characters to place in any row
	 * 
	 * @returns list of individual lines of text
	 **/
	static vector<string> wrapText(string text, size_t max_width)
#ifdef STUI_IMPLEMENTATION
	{
		vector<string> result;
		size_t start = 0;
		while (start != string::npos)
		{
			start++;
			size_t end = text.find('\n', start);
			vector<string> tmp = wrapTextInner(text.substr(start, end - start), max_width);
			for (string s : tmp) result.push_back(s);
			start = end;
		}
		//	i hate this
		if (result.size() > 0)
			result[0] = string(1, text[0]) + result[0];
		return result;
	}
#endif
	;

	static vector<string> wrapTextInner(string text, size_t max_width)
#ifdef STUI_IMPLEMENTATION
	{
		vector<string> lines;
		size_t last_index = 0;
		while (last_index != text.length())
		{
			size_t max_end = min(last_index + max_width - 1, text.length() - 1);
			size_t next_end = max_end;
			bool trim_whitespace = true;
			while (text[next_end] != ' ')
			{
				if (next_end == text.length() - 1)
				{
					trim_whitespace = false;
					break;
				}

				if (next_end == last_index)
				{
					next_end = max_end;
					trim_whitespace = false;
					break;
				}
				next_end--;
			}
			lines.push_back(text.substr(last_index, ((next_end - last_index) + 1) - trim_whitespace));
			last_index = next_end + 1;
		}
		return lines;
	}
#endif
	;

	/**
	 * @brief draws a block of text into a buffer.
	 * 
	 * supports either single-line drawing (out-of-bounds characters are skipped), or
	 * multi-line wrapped drawing, where lines are wrapped to fit into the box specified
	 * by `max_size`. it's up to the caller to ensure `buffer` is allocated to the size
	 * specified by `buffer_size`. newlines, nulls or other special characters should not be
	 * passed into this function, as it will fuck up the output stage at the end of rendering.
	 * 
	 * @param text text to draw to the buffer
	 * @param wrap enable line wrapping to fit text horizontally in the box; if disabled,
	 * single-line mode will be used instead
	 * @param text_origin position in the output buffer of the top-left-most corner of the
	 * text
	 * @param max_size maximum size of the drawn text, measured from the `text_origin`. must 
	 * be positive in both dimensions. text that runs outside this area is not drawn, and 
	 * wrapped text will be wrapped to fit inside this horizontally
	 * @param buffer pointer to a character array ordered left-to-right, top-to-bottom
	 * @param buffer_size size of the allocated buffer, must match with the size of the `buffer`
	 **/
	static void drawText(string text, bool wrap, Coordinate text_origin, Coordinate max_size, Tixel* buffer, Coordinate buffer_size)
#ifdef STUI_IMPLEMENTATION
	{
		if (buffer == nullptr) return;
		if (buffer_size.x <= 0 || buffer_size.y <= 0) return;

		if (wrap)
		{
			vector<string> lines = wrapText(text, min(max_size.x, buffer_size.x - text_origin.x));
			
			int row = 0;
			for (string line : lines)
			{
				for (size_t col = 0; col < line.length(); col++)
				{
					buffer[(text_origin.x + col) + ((text_origin.y + row) * buffer_size.x)] = line[col];
				}
				row++;
				if (row >= max_size.y || row + text_origin.y >= buffer_size.y) break;
			}
		}
		else
		{
			for (int i = 0; i < static_cast<int>(text.length()); i++)
			{
				if (text_origin.x + i < 0) continue;
				if (text_origin.x + i >= buffer_size.x || i >= max_size.x) break;
				if (text_origin.y < 0 || text_origin.y >= buffer_size.y) break;

				buffer[(text_origin.x + i) + (text_origin.y * buffer_size.x)] = text[i];
			}
		}
	}
#endif
	;

	/**
	 * @brief simplifies allocation of 2D text buffers.
	 * 
	 * automatically clears the buffer with spaces and adds an extra null-terminator on the end. this
	 * function allocates memory with `new`, and callers are responsible for `delete[]`ing the buffers
	 * after it is no longer being used.
	 * 
	 * @param buffer_size two-dimensional size of the buffer required. must be positive and non-zero in
	 * both axes
	 * 
	 * @returns pointer to newly-allocated block of memory, or `nullptr` if the buffer size would be
	 * less than 1
	 **/
	static Tixel* makeBuffer(Coordinate buffer_size)
#ifdef STUI_IMPLEMENTATION
	{
		if (buffer_size.x <= 0 || buffer_size.y <= 0) return nullptr;

		size_t size = buffer_size.x * buffer_size.y;
		Tixel* buf = new Tixel[size + 1];
		for (size_t i = 0; i < size; i++)
			buf[i] = Tixel{ ' ', getDefaultColour() };
		buf[size] = Tixel{ '\0', getDefaultColour() };

		return buf;
	}
#endif
	;

	/**
	 * @brief copy an area from one buffer to another.
	 * 
	 * useful for transferring an area into a larger, differently-shaped buffer for combining elements.
	 * it's up to the caller to ensure `src` is allocated to the size specified by `src_size`, and to
	 * ensure 'dst' is allocated to the size specified by `dst_size`. also, this function does not
	 * clamp the bounds of the box, so trying to copy a box which overflows outside `src` or `dst`, or
	 * both at once, is likely to corrupt the heap or cause an exception.
	 * 
	 * @param src source buffer to copy characters from. must be appropriately sized and allocated, see
	 * above
	 * @param src_size size of the source buffer, must match with the allocated size of `src`
	 * @param src_offset position of the top-left corner of the box to copy in the source buffer. must
	 * be positive in both dimensions
	 * @param area_size size of the area to be copied between buffers. must fit inside both buffers
	 * @param dst destination buffer to copy characters into. must be appropriately sized and allocated,
	 * see above
	 * @param dst_size size of the destination buffer, must match with the allocated size of `dst`
	 * @param dst_offset position of the top-left corner of the box to copy to in the destination buffer.
	 * must be positive in both dimensions
	 **/
	static void copyBox(const Tixel* src, Coordinate src_size, Coordinate src_offset, Coordinate area_size, Tixel* dst, Coordinate dst_size, Coordinate dst_offset)
#ifdef STUI_IMPLEMENTATION
	{
		if (src == nullptr || dst == nullptr) return;
		if (area_size.x <= 0 || area_size.y <= 0) return;
		if (src_offset.x < 0 || src_offset.y < 0) return;
		if (dst_offset.x < 0 || dst_offset.y < 0) return;
		if (src_offset.x + area_size.x > src_size.x || src_offset.y + area_size.y > src_size.y) return;
		if (dst_offset.x + area_size.x > dst_size.x || dst_offset.y + area_size.y > dst_size.y) return;

		for (int y = 0; y < area_size.y; y++)
		{
			memcpy(dst + dst_offset.x + ((y + dst_offset.y) * dst_size.x), src + src_offset.x + ((y + src_offset.y) * src_size.x), area_size.x * sizeof(Tixel));
		}
	}
#endif
	;

	/**
	 * @brief get the current default colour configuration for the interface.
	 * 
	 * @returns the current colour configuration
	 **/
	static inline Tixel::ColourCommand getDefaultColour()
	{
		return (Tixel::ColourCommand)(Tixel::ColourCommand::BG_BLACK | Tixel::ColourCommand::FG_WHITE);
	}

	/**
	 * @brief get the current highlighted colour configuration for the interface.
	 * 
	 * @returns the current highlighted colour configuration
	 **/
	static inline Tixel::ColourCommand getHighlightedColour()
	{
		return (Tixel::ColourCommand)(Tixel::ColourCommand::FG_BLACK | Tixel::ColourCommand::BG_WHITE);
	}

	/**
	 * @brief get the current unfocused colour configuration for the interface.
	 * 
	 * @returns the current unfocused colour configuration
	 **/
	static inline Tixel::ColourCommand getUnfocusedColour()
	{
		return (Tixel::ColourCommand)(Tixel::ColourCommand::FG_BLACK | Tixel::ColourCommand::BG_GRAY);
	}

	/**
	 * @brief fill a specified box area with a particular colour command.
	 * 
	 * @param colour colour command to fill the area with
	 * @param origin starting offset of the area
	 * @param size size of the area. must be positive in both axes
	 * @param buffer output buffer into which the filled box should be drawn
	 * @param buffer_size size of the output buffer
	 **/
	static void fillColour(Tixel::ColourCommand colour, Coordinate origin, Coordinate size, Tixel* buffer, Coordinate buffer_size)
#ifdef STUI_IMPLEMENTATION
	{
		if (size.x <= 0 || size.y <= 0) return;
		if (buffer == nullptr) return;
		if (buffer_size.x <= 0 || buffer_size.y <= 0) return;

		for (int y = 0; y < size.y; y++)
		{
			if (y + origin.y < 0) continue;
			if (y + origin.y >= buffer_size.y) break;

			for (int x = 0; x < size.x; x++)
			{
				if (x + origin.x < 0) continue;
				if (x + origin.x >= buffer_size.x) break;
				buffer[x + origin.x + ((y + origin.y) * buffer_size.x)].colour = colour;
			}
		}
	}
#endif
	;
};

/**
 * @brief single-line text box.
 * 
 * text `alignment` can be specified as < 0 for left-aligned, = 0 for center-aligned, or
 * > 0 for right-aligned
 **/
class Label : public Component, public Utility
{
public:
	string text;
	int alignment;
	
	Label(string _text, int _alignment) : text(_text), alignment(_alignment) { }

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (size.y < 1) return;

		Coordinate offset{ 0,0 };
		if (alignment < 0)
			offset.x = 0;
		else if (alignment == 0)
			offset.x = static_cast<int>((size.x - text.length()) / 2);
		else if (alignment > 0)
			offset.x = static_cast<int>(size.x - text.length());

		drawText(stripNullsAndMore(text, "\n\t"), false, offset, Coordinate{ static_cast<int>(text.length()), 1 }, output_buffer, size);
	}
#endif
	;

	GETMAXSIZE_STUB { return Coordinate{ -1, 1 }; }
	GETMINSIZE_STUB { return Coordinate{ static_cast<int>(text.length()), 1 }; }
};

/**
 * @brief simple clickable button.
 **/
class Button : public Component, public Utility
{
public:
	string text;
	void (*callback)();
	bool enabled;

	Button(string _text, void (*_callback)(), bool _enabled) : text(_text), callback(_callback), enabled(_enabled) { }

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (size.y < 1) return;

		drawText("> " + text + " <", false, Coordinate{ 0,0 }, Coordinate{ static_cast<int>(text.length()) + 4,1 }, output_buffer, size);
		if (focused)
			fillColour(enabled ? getHighlightedColour() : getUnfocusedColour(), Coordinate{ 0,0 }, Coordinate{ static_cast<int>(text.length()) + 4,1 }, output_buffer, size);
	}
#endif
	;

	GETMAXSIZE_STUB { return Coordinate{ -1,1 }; }
	GETMINSIZE_STUB { return Coordinate{ static_cast<int>(text.length()) + 4,1 }; }

	HANDLEINPUT_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (input_character == '\n' && callback != nullptr && focused && enabled) { callback(); return true; }
		return false;
	}
#endif
	;

	ISFOCUSABLE_STUB { return enabled; }
};

/**
 * @brief list of options from which the user can select one by navigating up/down and
 * pressing enter/space.
 **/
class RadioButton : public Component, public Utility
{
	int highlighted_index = 0;

public:
	vector<string> options;
	int selected_index;
	bool enabled;

	RadioButton(vector<string> _options, int _selected_index, bool _enabled) : options(_options), selected_index(_selected_index), enabled(_enabled) { }

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (size.y < 1) return;

		for (int line = 0; line < static_cast<int>(options.size()); line++)
		{
			if (line >= size.y) break;
			drawText("[ ] " + options[line], false, Coordinate{ 0,line }, Coordinate{ size.x,1 }, output_buffer, size);
			output_buffer[(line * size.x) + 1] = selected_index == line ? '*' : ' ';
			if (line == highlighted_index && enabled)
				fillColour(focused ? getHighlightedColour() : getUnfocusedColour(), Coordinate{ 0,line }, Coordinate{ size.x,1}, output_buffer, size);
		}
	}
#endif
	;

	GETMAXSIZE_STUB { return Coordinate{ -1,-1 }; }
	GETMINSIZE_STUB { return Coordinate{ 5, static_cast<int>(options.size()) }; }

	ISFOCUSABLE_STUB { return true; }

	HANDLEINPUT_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (!focused || !enabled) return false;

		if (input_character == Input::ArrowKeys::UP && highlighted_index > 0) highlighted_index--;
		if (input_character == Input::ArrowKeys::DOWN && highlighted_index + 1 < static_cast<int>(options.size())) highlighted_index++;
		if (input_character == Input::ArrowKeys::LEFT) highlighted_index = 0;
		if (input_character == Input::ArrowKeys::RIGHT) highlighted_index = max(0, static_cast<int>(options.size()) - 1);
		if (input_character == ' ' || input_character == '\n') selected_index = highlighted_index;
		return false;
	}
#endif
	;
};

/**
 * @brief list of options from which the user can select any, all, or none by navigating
 * up/down and pressing enter/space.
 **/
class ToggleButton : public Component, public Utility
{
	int highlighted_index = 0;

public:
	vector<pair<string, bool>> options;
	bool enabled;

	ToggleButton(vector<pair<string, bool>> _options, bool _enabled) : options(_options), enabled(_enabled) { }

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (size.y < 1) return;

		for (int line = 0; line < static_cast<int>(options.size()); line++)
		{
			if (line >= size.y) break;
			drawText("[ ] " + options[line].first, false, Coordinate{ 0,line }, Coordinate{ size.x,1 }, output_buffer, size);
			output_buffer[(line * size.x) + 1] = options[line].second ? '*' : ' ';
			if (line == highlighted_index && enabled)
				fillColour(focused ? getHighlightedColour() : getUnfocusedColour(), Coordinate{ 0,line }, Coordinate{ size.x,1}, output_buffer, size);
		}
	}
#endif
	;

	GETMAXSIZE_STUB { return Coordinate{ -1,-1 }; }
	GETMINSIZE_STUB { return Coordinate{ 5, static_cast<int>(options.size()) }; }

	ISFOCUSABLE_STUB { return true; }

	HANDLEINPUT_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (!focused || !enabled) return false;

		if (input_character == Input::ArrowKeys::UP && highlighted_index > 0) highlighted_index--;
		if (input_character == Input::ArrowKeys::DOWN && highlighted_index + 1 < static_cast<int>(options.size())) highlighted_index++;
		if (input_character == Input::ArrowKeys::LEFT) highlighted_index = 0;
		if (input_character == Input::ArrowKeys::RIGHT) highlighted_index = max(0, static_cast<int>(options.size()) - 1);
		if (input_character == ' ' || input_character == '\n') options[highlighted_index].second = !options[highlighted_index].second;
		return false;
	}
#endif
	;
};

/**
 * @brief simple text entry box.
 **/
class TextInputBox : public Component, public Utility
{
	size_t cursor_index = 0;
	int last_rendered_width = 0;
public:
	string text;
	void (*callback)();
	bool enabled;

	TextInputBox(string _text, void (*_callback)(), bool _enabled) : text(_text), callback(_callback), enabled(_enabled) { }

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (size.y < 1) return;
		last_rendered_width = size.x;

		drawText("> " + text, false, Coordinate{ 0,0 }, Coordinate{ size.x - 3,1 }, output_buffer, size);
		if (enabled) output_buffer[cursor_index + 2].colour = focused ? getHighlightedColour() : getUnfocusedColour();
	}
#endif
	;

	GETMAXSIZE_STUB { return Coordinate{ -1,1 }; }
	GETMINSIZE_STUB { return Coordinate{ 6,1 }; }

	HANDLEINPUT_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (!focused || !enabled) return false;
		if (input_character == '\n') { if (callback != nullptr) callback(); }
		else if (input_character == Input::ArrowKeys::LEFT) { if (cursor_index > 0) cursor_index--; }
		else if (input_character == Input::ArrowKeys::RIGHT) { if (cursor_index < text.length()) cursor_index++; }
		else if (input_character == Input::ArrowKeys::UP) cursor_index = 0;
		else if (input_character == Input::ArrowKeys::DOWN) cursor_index = text.length();
		else if (input_character == '\b')
		{ if (cursor_index > 0)
		{
			for (size_t first = cursor_index - 1; first < text.length() - 1; first++)
				text[first] = text[first + 1];
			text.pop_back();
			cursor_index--;
		} }
		else if (input_character == '\t') return false;
		else if (cursor_index < last_rendered_width - 3)
		{
			text.push_back(' ');
			for (size_t first = text.length() - 1; first > cursor_index; first--)
				text[first] = text[first - 1];
			text[cursor_index] = input_character;
			cursor_index++;
		}

		return true;
	}
#endif
	;

	ISFOCUSABLE_STUB { return enabled; }
};

/**
 * @brief multi-line wrapping text area.
 **/
class TextArea : public Component, public Utility
{
	size_t cursor_index = 0;
	size_t line_index = 0 ;
public:
	string text;
	bool editable;

	TextArea(string _text, bool _editable) : text(_text), editable(_editable) { }

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (size.y < 2 || size.x < 2) return;

		drawText(stripNullsAndMore(text, ""), true, Coordinate{ 0,0 }, Coordinate{ size.x, size.y }, output_buffer, size);
		if (editable)
			output_buffer[cursor_index + (line_index  * size.x)].colour = focused ? getHighlightedColour() : getUnfocusedColour();
	}
#endif
	;

	GETMAXSIZE_STUB { return Coordinate{ -1, -1 }; }
	GETMINSIZE_STUB { return Coordinate{ 3, 3 }; }

	HANDLEINPUT_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (!editable || !focused) return false;
		// TODO: apply characters in the right place, and move cursor...
		if (input_character == '\b') text.pop_back();
		else if (input_character == Input::ArrowKeys::RIGHT) cursor_index++;
		else if (input_character == Input::ArrowKeys::LEFT && cursor_index > 0) cursor_index--;
		// TODO: implement moving the cursor here...
		else text.push_back(input_character);

		return true;
	}
#endif
	;

	ISFOCUSABLE_STUB { return editable; }
};

/**
 * @brief linear progress bar.
 * 
 * progress is expressed as a `fraction` between 0 and 1.
 **/
class ProgressBar : public Component
{
public:
	float fraction;
	
	ProgressBar(float _fraction) : fraction(_fraction) { }

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (size.y < 1) return;

		int completed = (int)round((float)size.x * fraction);
			
		for (int i = 0; i < size.x; i++)
			output_buffer[i] = i < completed ? UNICODE_BLOCK : UNICODE_LIGHT_SHADE;
	}
#endif
	;

	GETMAXSIZE_STUB { return Coordinate{ -1, 1 }; }
	GETMINSIZE_STUB { return Coordinate{ 1, 1 }; }
};

/**
 * @brief user-interactible slider widget.
 **/
class Slider : public Component, public Utility
{
public:
	float value;

	Slider(float _value) : value(_value) { }

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (size.x < 3 || size.y < 1) return;

		output_buffer[0] = '[';
		output_buffer[size.x - 1] = ']';
		for (int x = 1; x < size.x - 1; x++)
			output_buffer[x] = '-';
		output_buffer[(int)round(value * (size.x - 2.0f)) + 1].colour = focused ? getHighlightedColour() : getUnfocusedColour();
	}
#endif
	;

	GETMAXSIZE_STUB { return Coordinate{ -1,1 }; }
	GETMINSIZE_STUB { return Coordinate{ 5,1 }; }

	ISFOCUSABLE_STUB { return true; }

	HANDLEINPUT_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (!focused) return false;
		float difference = 0.0;
		if (input_character == Input::ArrowKeys::LEFT) difference = -0.01f;
		if (input_character == Input::ArrowKeys::RIGHT) difference = 0.01f;
		if (modifiers & Input::ControlKeys::SHIFT) difference *= 5.0f;
		value = max(0.0f, min(value + difference, 1.0f));
		return true;
	}
#endif
	;
};

/**
 * @brief simple activity spinner.
 * 
 * described by its `state` (effectively the current animation frame index), and its `type`
 * (style setting between 0 and 3 inclusive).
 **/
class Spinner : public Component
{
	static constexpr int types = 4;
	static constexpr uint32_t sequences[types][4]
	{
		{ '|', '/', '-', '\\' },
		{ UNICODE_QUADRANT_LOWERLEFT, UNICODE_QUADRANT_TOPLEFT, UNICODE_QUADRANT_TOPRIGHT, UNICODE_QUADRANT_LOWERRIGHT },
		{ UNICODE_BOXLIGHT_UP, UNICODE_BOXLIGHT_UPRIGHT, UNICODE_BOXLIGHT_UPRIGHTDOWN, UNICODE_BOXLIGHT_UPRIGHTDOWNLEFT },
		{ UNICODE_BLOCK_1_8, UNICODE_BLOCK_3_8, UNICODE_BLOCK_6_8, UNICODE_BLOCK }
	};
public:
	size_t state;
	int type;

	Spinner(size_t _state, int _type) : state(_state), type(_type) { }

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		output_buffer[0] = sequences[type % types][state % 4];
	}
#endif
	;

	GETMAXSIZE_STUB { return Coordinate{ 1, 1 }; }
	GETMINSIZE_STUB { return Coordinate{ 1, 1 }; }
};

/**
 * @brief vertical layout box containing a list of child widgets.
 * 
 * the layout algorithm will attempt to meet the minimum sizes of all children first, then
 * expand each element one-by-one until they either meet their max height or there's no more
 * space to expand into. ensures minimum widths for all elements.
 **/
class VerticalBox : public Component, public Utility
{
public:
	vector<Component*> children;
	
	VerticalBox(vector<Component*> _children) : children(_children) { }
	
	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{	
		vector<int> min_heights(children.size());
		vector<int> max_heights(children.size());
		vector<int> calculated_heights(children.size());
		int total_height = 0;
		for (size_t i = 0; i < children.size(); i++)
		{
			min_heights[i] = children[i]->getMinSize().y;
			max_heights[i] = children[i]->getMaxSize().y;
			calculated_heights[i] = min_heights[i];
			total_height += calculated_heights[i];
		}

		int budget = size.y - total_height;
		if (budget < 0) return;

		while (budget > 0)
		{
			for (size_t i = 0; i < children.size(); i++)
			{
				if (calculated_heights[i] >= max_heights[i] && max_heights[i] != -1) continue;

				calculated_heights[i]++;

				budget--;
				if (budget == 0) break;
			}
		}

		int y_offset = 0;
		for (size_t i = 0; i < children.size(); i++)
		{
			Coordinate component_size{ children[i]->getMaxSize().x == -1 ? size.x : min(size.x, children[i]->getMaxSize().x), calculated_heights[i] };
			Tixel* component_buffer = makeBuffer(component_size);
			children[i]->render(component_buffer, component_size);
			copyBox(component_buffer, component_size, Coordinate{ 0,0 }, component_size, output_buffer, size, Coordinate{ 0,y_offset });
			delete[] component_buffer;
			y_offset += component_size.y;
		}
	}
#endif
	;

	GETMAXSIZE_STUB
	{
		Coordinate max_size{ 0,0 };
		for (Component* child : children)
		{
			Coordinate c_max = child->getMaxSize();
			if (max_size.x != -1 && (c_max.x > max_size.x || c_max.x == -1))
				max_size.x = c_max.x;

			if (c_max.y == -1) max_size.y = -1;
			else if (max_size.y != -1)
				max_size.y += c_max.y;
		}

		return max_size;
	}

	GETMINSIZE_STUB
	{
		Coordinate min_size{ 0,0 };
		for (Component* child : children)
		{
			Coordinate c_min = child->getMinSize();
			if (c_min.x > min_size.x)
				min_size.x = c_min.x;

			min_size.y += c_min.y;
		}

		return min_size;
	}
};

/**
 * @brief horizontal layout box containing a list of child widgets.
 * 
 * the layout algorithm will attempt ot meet the minimum sizes of all children first, then
 * expand each element one-by-one until they either meet their max width or there's no more
 * space to expand into. ensures minimum heights for all elements.
 **/
class HorizontalBox : public Component, public Utility
{
public:
	vector<Component*> children;
	
	HorizontalBox(vector<Component*> _children) : children(_children) { }

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		vector<int> min_widths(children.size());
		vector<int> max_widths(children.size());
		vector<int> calculated_widths(children.size());
		int total_width = 0;
		for (size_t i = 0; i < children.size(); i++)
		{
			min_widths[i] = children[i]->getMinSize().x;
			max_widths[i] = children[i]->getMaxSize().x;
			calculated_widths[i] = min_widths[i];
			total_width += calculated_widths[i];
		}

		int budget = size.x - total_width;
		if (budget < 0) return;

		while (budget > 0)
		{
			for (size_t i = 0; i < children.size(); i++)
			{
				if (calculated_widths[i] >= max_widths[i] && max_widths[i] != -1) continue;

				calculated_widths[i]++;

				budget--;
				if (budget == 0) break;
			}
		}

		int x_offset = 0;
		for (size_t i = 0; i < children.size(); i++)
		{
			Coordinate component_size{ calculated_widths[i], children[i]->getMaxSize().y == -1 ? size.y : min(size.y, children[i]->getMaxSize().y) };
			Tixel* component_buffer = makeBuffer(component_size);
			children[i]->render(component_buffer, component_size);
			copyBox(component_buffer, component_size, Coordinate{ 0,0 }, component_size, output_buffer, size, Coordinate{ x_offset,0 });
			delete[] component_buffer;
			x_offset += component_size.x;
		}
	}
#endif
	;

	GETMAXSIZE_STUB
	{
		Coordinate max_size{ 0,0 };
		for (Component* child : children)
		{
			Coordinate c_max = child->getMaxSize();
			if (c_max.x == -1) max_size.x = -1;
			else if (max_size.x != -1)
				max_size.x += c_max.x;

			if (max_size.y != -1 && (c_max.y > max_size.y || c_max.y == -1))
				max_size.y = c_max.y;
		}

		return max_size;
	}

	GETMINSIZE_STUB
	{
		Coordinate min_size{ 0,0 };
		for (Component* child : children)
		{
			Coordinate c_min = child->getMinSize();
			min_size.x += c_min.x;

			if (c_min.y > min_size.y)
				min_size.y = c_min.y;
		}

		return min_size;
	}
};

/**
 * @brief blank spacing element which fills a set amount of space vertically.
 **/
class VerticalSpacer : public Component
{
public:
	size_t height;
	
	VerticalSpacer(size_t _size) : height(_size) { }

	GETMAXSIZE_STUB { return Coordinate{ 1, static_cast<int>(height) }; }
	GETMINSIZE_STUB { return Coordinate{ 1, static_cast<int>(height) }; }
};

/**
 * @brief blank spacing element which fills a set amount of space horizontally.
 **/
class HorizontalSpacer : public Component
{
public:
	size_t width;
	
	HorizontalSpacer(size_t _size) : width(_size) { }

	GETMAXSIZE_STUB { return Coordinate{ static_cast<int>(width), 1 }; }
	GETMINSIZE_STUB { return Coordinate{ static_cast<int>(width), 1 }; }
};

/**
 * @brief draws a border around another component using IBM box characters.
 **/
class BorderedBox : public Component, public Utility
{
public:
	Component* child;
	string name;

	BorderedBox(Component* _child, string _name) : child(_child), name(_name) { }

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (size.x < 3 || size.y < 3) return;
		drawBox(Coordinate{ 0,0 }, size, output_buffer, size);
		if (name.size() > 0)
			drawText(name, false, Coordinate{ 3,0 }, Coordinate{ size.x - 6,1 }, output_buffer, size);
		if (child == nullptr) return;

		Coordinate component_size{ size.x - 2, size.y - 2 };
		Tixel* component_buffer = makeBuffer(component_size);
		child->render(component_buffer, component_size);
		copyBox(component_buffer, component_size, Coordinate{ 0,0 }, component_size, output_buffer, size, Coordinate{ 1,1 });
		delete[] component_buffer;
	}
#endif
	;

	GETMAXSIZE_STUB { return (child == nullptr) ? Coordinate{ -1,-1 } : child->getMaxSize(); }
	GETMINSIZE_STUB { return (child == nullptr) ? Coordinate{ 2,2 } : Coordinate{ child->getMinSize().x + 2, child->getMinSize().y + 2 }; }
};

/**
 * @brief element which displays a list of strings.
 * 
 * allows scrolling.
 * 
 **/
class ListView : public Component, public Utility
{
	int last_render_height = 0;

public:
	vector<string> elements;
	int scroll;
	int selected_index = 0;

	ListView(vector<string> _elements, int _scroll, int _selected_index) : elements(_elements), scroll(_scroll), selected_index(_selected_index) { }

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (size.x < 2 || size.y < 2) return;
		last_render_height = size.y;
		int row = -1 - static_cast<int>(scroll);
		int index = -1;
		selected_index = min(selected_index, static_cast<int>(elements.size()) - 1);
		for (string element : elements)
		{
			index++;
			row++;
			if (row < 0) continue;
			if (row >= size.y) break;

			drawText(stripNullsAndMore(element, "\n\t"), false, Coordinate{ 0, row }, Coordinate{ size.x, 1 }, output_buffer, size);
			string index_str = " (" + to_string(index) + ")";
			drawText(index_str, false, Coordinate{ size.x - static_cast<int>(index_str.length()), row }, Coordinate{ size.x, 1 }, output_buffer, size);
			if (index == selected_index)
				fillColour(focused ? getHighlightedColour() : getUnfocusedColour(), Coordinate{ 0, row }, Coordinate{ size.x,1 }, output_buffer, size);
		}
	}
#endif
	;

	GETMAXSIZE_STUB { return Coordinate{ -1, -1 }; }
	GETMINSIZE_STUB { return Coordinate{ 10, 3 }; }

	ISFOCUSABLE_STUB { return true; }

	HANDLEINPUT_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (input_character == Input::ArrowKeys::DOWN && selected_index < static_cast<int>(elements.size()) - 1)
		{
			selected_index++;
			if (selected_index - scroll >= last_render_height)
				scroll++;
		}
		else if (input_character == Input::ArrowKeys::UP && selected_index > 0)
		{
			selected_index--;
			if (selected_index - scroll < 0 && scroll > 0)
				scroll--;
		}
		else return false;

		return true;
	}
#endif
	;
};

/**
 * @brief complex display element capable of visualising tree structures as a heirarchy
 * with nested, expandable nodes.
 * 
 * nodes can be given a descriptive name, and an identifier (which could be used to
 * index into your own array of proprietary nodes). expansion of nodes can be toggled;
 * if a node is expanded then its children will be shown, otherwise they will not.
 **/
class TreeView : public Component, public Utility
{
	int last_render_height = 0;

public:
	struct Node
	{
		string name = "empty tree node";
		vector<Node*> children;
		uint32_t id = 0;
		bool expanded = true;
	};

	Node* root;
	size_t scroll;
	size_t selected_index = 0;

	TreeView(Node* _root, size_t _scroll, size_t _selected_index) : root(_root), scroll(_scroll), selected_index(_selected_index) { }

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (size.x < 2 || size.y < 2) return;
		last_render_height = size.y;
		if (root == nullptr) return;
		int top = 0 - static_cast<int>(scroll);
		printNode(root, 0, top, output_buffer, size);
	}
#endif
	;

	GETMAXSIZE_STUB { return Coordinate{ -1, -1 }; }
	GETMINSIZE_STUB { return Coordinate{ 10, 3 }; }

	ISFOCUSABLE_STUB { return true; }

	HANDLEINPUT_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (!focused) return false;

		vector<Node*> parents;
		vector<size_t> indices_within_parents;
		int actual_offset;
		Node* node = findNodeWithID(selected_index, parents, indices_within_parents, actual_offset);
		if (node == nullptr)
		{
			if (root != nullptr) selected_index = root->id;
			return true;
		}

		if (input_character == Input::ArrowKeys::DOWN)
		{
			size_t steps_up = 0;

			Node* tmp_parent = node;
			size_t next_index = 0;
			while (!(tmp_parent->children.size() > next_index && tmp_parent->expanded))
			{
				steps_up++;
				if (steps_up > parents.size()) return true;
				tmp_parent = parents[parents.size() - steps_up];
				next_index = indices_within_parents[parents.size() - steps_up] + 1;
			}
			if (steps_up == 0)
				selected_index = tmp_parent->children[0]->id;
			else
				selected_index = tmp_parent->children[next_index]->id;
			
			if (actual_offset - static_cast<int>(scroll) >= last_render_height)
				scroll++;
		}
		else if (input_character == Input::ArrowKeys::UP)
		{
			if (parents.size() < 1) return true;
			else if (indices_within_parents[parents.size() - 1] == 0)
				selected_index = parents[parents.size() - 1]->id;
			else
			{
				Node* tmp_parent = parents[parents.size() - 1]->children[indices_within_parents[parents.size() - 1] - 1];
				while (tmp_parent->expanded && tmp_parent->children.size() > 0)
					tmp_parent = tmp_parent->children[tmp_parent->children.size() - 1];
				selected_index = tmp_parent->id;
			}

			if (actual_offset - static_cast<int>(scroll) <= 0 && scroll > 0)
				scroll--;
		}
		else if (input_character == Input::ArrowKeys::RIGHT)
		{
			node->expanded = true;
		}
		else if (input_character == Input::ArrowKeys::LEFT)
		{
			node->expanded = false;
		}
		else return false;

		return true;
	}
#endif
	;

private:
	inline Node* findNodeWithID(uint32_t id, vector<Node*>& parents, vector<size_t>& indices, int& actual_offset)
	{
		Node* current_node = root;
		parents.clear();
		indices.clear();
		actual_offset = 0;
		if (root == nullptr) return nullptr;
		int level = 0;
		int parent_expansion_level = root->expanded ? 0 : -1;
		while (current_node->id != id)
		{
		// TODO: actual_offset is still not correct
			if (!current_node->children.empty())
			{
				parents.push_back(current_node);
				indices.push_back(0);
				level++;
				current_node = current_node->children[0];

				if (parent_expansion_level == (level - 1))
				{
					actual_offset++;
					if (current_node->expanded)
						parent_expansion_level++;
				}
			}
			else
			{
				if (level == 0) return nullptr;
				if (indices[level - 1] < parents[level - 1]->children.size() - 1)
				{
					indices[level - 1]++;
					if (parent_expansion_level == level)
					{
						if (current_node->expanded)
							parent_expansion_level--;
					}
					
					if (parent_expansion_level == (level - 1))	
						actual_offset++;

					current_node = parents[level - 1]->children[indices[level - 1]];
					
					if (parent_expansion_level == (level - 1) && current_node->expanded)
						parent_expansion_level++;
				}
				else
				{
					if (parent_expansion_level == level)
					{
						actual_offset++;
						if (current_node->expanded)
							parent_expansion_level--;
					}
					level--;
					indices.pop_back();
					parents.pop_back();
				}
			}
		}

		return current_node;
	}

	void printNode(Node* node, int depth, int& top, Tixel* output_buffer, Coordinate buffer_size)
#ifdef STUI_IMPLEMENTATION
	{
		if (top >= buffer_size.y) return;

		if (top >= 0)
		{
			drawText((node->expanded ? "  " : "> ") + stripNullsAndMore(node->name, "\n\t"), false, Coordinate{ depth, top }, Coordinate{ buffer_size.x - 2 - depth, 1 }, output_buffer, buffer_size);
			if (node->expanded) output_buffer[depth + (top * buffer_size.x)] = UNICODE_NOT;
			string id_desc = " [" + to_string(node->children.size()) + "]";
			drawText(id_desc, false, Coordinate{ buffer_size.x - (int)id_desc.length(), top }, Coordinate{ (int)id_desc.length(), 1 }, output_buffer, buffer_size);
			if (selected_index == node->id)
				fillColour(focused ? getHighlightedColour() : getUnfocusedColour(), Coordinate{ 0,top, }, Coordinate{ buffer_size.x,1 }, output_buffer, buffer_size);
		}
		if (node->expanded)
		{
			for (Node* child : node->children)
			{
				top++;
				printNode(child, depth + 1, top, output_buffer, buffer_size);
			}
		}
	}
#endif
	;
};

/**
 * @brief complex display element capable of displaying simple low-resolution grayscale
 * images.
 * 
 * images will be displayed at twice the width they are stored at to compensate for the
 * aspect ratio of terminal characters. each pixel should be described by a single byte.
 * the size of the image buffer must be allocated to match the image size parameter.
 **/
class ImageView : public Component
{
public:
	uint8_t* grayscale_image;
	Coordinate image_size;
	
	ImageView(uint8_t* _grayscale_image, Coordinate _image_size) : grayscale_image(_grayscale_image), image_size(_image_size) { }
	
	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (grayscale_image == nullptr) return;

		for (int y = 0; y < size.y; y++)
		{
			for (int x = 0; x < size.x; x++)
			{
				uint32_t out = ' ';
				uint8_t in = grayscale_image[(x / 2) + (y * image_size.x)];
				if (in >= 192) out = UNICODE_BLOCK;
				else if (in >= 128) out = UNICODE_DARK_SHADE;
				else if (in >= 96) out = UNICODE_MID_SHADE;
				else if (in >= 64) out = UNICODE_LIGHT_SHADE;
				else if (in >= 32) out = UNICODE_MIDDLE_DOT;
				else out = ' ';
				output_buffer[x + (y * size.x)] = out;
			}
		}
	}
#endif
	;

	GETMAXSIZE_STUB { return Coordinate{ image_size.x * 2, image_size.y }; }
	GETMINSIZE_STUB { return Coordinate{ 1, 1 }; }
};

/**
 * @brief container which limits the maximum size of the child component
 * 
 * the specified `max_size` should probably be at least as large as the minimum
 * size of the `child`
 **/
class SizeLimiter : public Component
{
public:
	Component* child;
	Coordinate max_size;
	
	SizeLimiter(Component* _child, Coordinate _max_size) : child(_child), max_size(_max_size) { }
	
	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (child == nullptr) return;
		child->render(output_buffer, size);
	}
#endif
	;

	GETMAXSIZE_STUB { return Coordinate{ max_size.x, max_size.y }; }

	GETMINSIZE_STUB
	{
		if (child == nullptr) return Coordinate{ 0,0 };
		return child->getMinSize();
	}
};

/**
 * @brief shows a list of tabs (or just items of any kind really) arranged horizontally,
 * where the selection can be shown.
 */
class TabDisplay : public Component, public Utility
{
public:
	vector<string> tab_descriptions;
	size_t current_tab;

	TabDisplay(vector<string> _tab_descriptions, size_t _current_tab) : tab_descriptions(_tab_descriptions), current_tab(_current_tab) { }

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		if (size.y < 1) return;
		int offset = 0;
		for (size_t i = 0; i < tab_descriptions.size(); i++)
		{
			string tab_text = "[" + to_string(i+1) + " - " + tab_descriptions[i] + "]";
			drawText(tab_text, false, Coordinate{ offset,0 }, Coordinate{ size.x,1 }, output_buffer, size);
			if (i == current_tab)
				fillColour(getUnfocusedColour(), Coordinate{ offset,0 }, Coordinate{ static_cast<int>(tab_text.length()),1 }, output_buffer, size);
			offset += static_cast<int>(tab_text.length() + 1);
		}
	}
#endif
	;

	GETMAXSIZE_STUB { return Coordinate{ -1,1 }; }
	GETMINSIZE_STUB { return Coordinate{ 10,1 }; }
};

#ifdef STUI_IMPLEMENTATION
static termios original_termios;
#endif

/**
 * @brief encapsulates some functionality relating to control of the terminal window.
 * 
 * before you start drawing things using `Page`, you should definitely call `configure`,
 * although it's not strictly necessary.
 */
class Terminal
{
	friend class Page;
	friend class Input;

public:
	static void configure()
#ifdef STUI_IMPLEMENTATION
	{
		debug_log.open("stui_debug.log");
		debug("STUI logging started");
#if defined(_WIN32)
		SetConsoleCtrlHandler(windowsControlHandler, true);
#elif defined(__linux__)
		signal(SIGINT, linuxControlHandler);
		signal(SIGQUIT, linuxControlHandler);
		signal(SIGTSTP, linuxControlHandler);
		termios new_termios;
		tcgetattr(STDIN_FILENO, &new_termios);
		tcgetattr(STDIN_FILENO, &original_termios);

		new_termios.c_iflag &= ~(IGNBRK | BRKINT | IXON);
		new_termios.c_lflag &= ~(ICANON | ECHO);
        new_termios.c_cc[VMIN] = 1;
		new_termios.c_cc[VSUSP] = 255;
		tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
#endif
	}
#endif
	;

private:
#if defined(_WIN32)
	static int WINAPI windowsControlHandler(DWORD control_type)
#ifdef STUI_IMPLEMENTATION
	{
		if (control_type == 0)
		{
			setCursorVisible(true);
			clear();
			setCursorPosition(Coordinate{ 0,0 });
			debug("STUI logging stopped");
			debug_log.close();
			exit(0);
		}

		return 1;
	}
#endif
	;

#elif defined(__linux__)
	static void linuxControlHandler(int control_type)
#ifdef STUI_IMPLEMENTATION
	{
		tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);

		setCursorVisible(true);
		clear();
		setCursorPosition(Coordinate{ 0,0 });
		debug("STUI logging stopped");
		debug_log.close();
		exit(0);
	}
#endif
	;

#endif

	/**
	 * @brief clear the entire terminal, including scrollback and onscreen buffers
	 **/
	static inline void clear()
	{
		OUTPUT_TARGET << ANSI_CLEAR_SCREEN << ANSI_CLEAR_SCROLL;
	}

	/**
	 * @brief get the size of the terminal window in characters
	 * 
	 * @return size of the terminal window 
	 **/
	static inline Coordinate getScreenSize()
	{
#if defined(_WIN32)
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
		return Coordinate{ info.dwSize.X, info.dwSize.Y };
#elif defined(__linux__)
		struct winsize size;
		ioctl(fileno(stdout), TIOCGWINSZ, &size);
		return Coordinate{ (int)size.ws_col, (int)size.ws_row };
#endif
	}

	/**
	 * @brief move the cursor to a given position in the terminal window
	 * 
	 * @param position desired position of the cursor
	 **/
	static inline void setCursorPosition(Coordinate position)
	{
		OUTPUT_TARGET << ANSI_SET_CURSOR(position.x, position.y);
	}

	/**
	 * @brief toggle terminal cursor visibility
	 * 
	 * @param visible whether or not the cursor should be visible
	 **/
	static inline void setCursorVisible(bool visible)
	{
#if defined(_WIN32)
		CONSOLE_CURSOR_INFO info;
		GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
		info.bVisible = visible;
		SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
#elif defined(__linux__)
		if (visible) OUTPUT_TARGET << ANSI_SHOW_CURSOR;
		else OUTPUT_TARGET << ANSI_HIDE_CURSOR;
#endif
	}

	/**
	 * @brief forces the current terminal to respect UTF-8
	 */
	static inline void enableUTF8()
	{
#if defined(_WIN32)
		SetConsoleOutputCP(CP_UTF8);
#endif
	}
};

/**
 * @brief purely static class which encapsulates code for rendering a page
 * to the terminal. a page just consists of a `Component` tree
 * 
 **/
class Page : public Utility
{
public:
	/**
	 * @brief draws a `Component` into the terminal via `std::cout`.
	 * 
	 * if the `Component` has children, their drawing will be handled automatically
	 * by the `Component` itself. the root `Component` is always drawn to fill the
	 * terminal, which is completely cleared to prevent scrolling jank.
	 * 
	 * @param root_component element to draw into the terminal
	 **/
	static void render(Component* root_component)
#ifdef STUI_IMPLEMENTATION
	{
		Terminal::setCursorVisible(false);
		Terminal::enableUTF8();

		Coordinate screen_size = Terminal::getScreenSize();

		Tixel* root_staging_buffer = makeBuffer(screen_size);

		Coordinate root_component_size
		{
			getConstrainedSize(screen_size.x, root_component->getMaxSize().x, root_component->getMinSize().x),
			getConstrainedSize(screen_size.y, root_component->getMaxSize().y, root_component->getMinSize().y)
		};
		Tixel* root_component_buffer = makeBuffer(root_component_size);
		root_component->render(root_component_buffer, root_component_size);
		copyBox(root_component_buffer, root_component_size, Coordinate{ 0,0 }, root_component_size, root_staging_buffer, screen_size, Coordinate{ 0,0 });
		delete[] root_component_buffer;

		OUTPUT_TARGET << ANSI_CLEAR_SCROLL;
		OUTPUT_TARGET << ANSI_SET_COLOUR(Tixel::toANSI(Tixel::ColourCommand::FG_WHITE));
		OUTPUT_TARGET << ANSI_SET_COLOUR(Tixel::toANSI(Tixel::ColourCommand::BG_BLACK));
		Terminal::setCursorPosition(Coordinate{ 0,0 });

		string output;
		output.reserve(4 * screen_size.x * screen_size.y);

		Tixel::ColourCommand foreground = (Tixel::ColourCommand)0;
		Tixel::ColourCommand background = (Tixel::ColourCommand)0;

		for (size_t i = 0; i < static_cast<size_t>(screen_size.x * screen_size.y); i++)
		{
			Tixel::ColourCommand new_foreground = (Tixel::ColourCommand)(root_staging_buffer[i].colour & Tixel::ColourCommand::FG_WHITE);
			Tixel::ColourCommand new_background = (Tixel::ColourCommand)(root_staging_buffer[i].colour & Tixel::ColourCommand::BG_WHITE);
			stringstream s;
			if (foreground != new_foreground)
				s << ANSI_SET_COLOUR(Tixel::toANSI(new_foreground));

			if (background != new_background)
				s << ANSI_SET_COLOUR(Tixel::toANSI(new_background));
			output += s.str();
			
			foreground = new_foreground;
			background = new_background;

			uint32_t chr = root_staging_buffer[i].character;
			output.push_back(chr & 0xFF);
			if (chr & 0x80) output.push_back((chr >> 8) & 0xFF);
			if (chr & 0x8000) output.push_back((chr >> 16) & 0xFF);
			if (chr & 0x800000) output.push_back((chr >> 24) & 0xFF);
		}

		OUTPUT_TARGET << output;

		delete[] root_staging_buffer;
	}
#endif
	;

	/**
	 * @brief check for queued input, handle shortcut triggers, and send remaining
	 * input to the specified component. order of input event is preserved.
	 * 
	 * @param focused_component component to send input to
	 * @param shortcut_bindings list of shortcuts to check for
	 **/
	static void handleInput(Component* focused_component, vector<Input::Shortcut> shortcut_bindings)
#ifdef STUI_IMPLEMENTATION
	{
		auto keys = Input::getQueuedKeyEvents();
		Input::processShortcuts(shortcut_bindings, keys);
		auto text_keys = stui::Input::getTextCharacters(keys);

		if (focused_component == nullptr) return;
		for (pair<uint8_t, Input::ControlKeys> k : text_keys)
			focused_component->handleInput(k.first, k.second);
	}
#endif
	;

	/**
	 * @brief stores information about a frame-wait which just happened
	 * 
	 **/
	struct FrameData
	{
		float delta_time;		// time since the last time `targetFramerate` was called
		float active_fraction;	// fraction of the delta_time which was taken up by rendering
	};

	/**
	 * @brief maintains the desired framerate by waiting for the remainder of the frame's 
	 * duration.
	 * 
	 * requires a variable in which to store the last time the function was called in order
	 * to work correctly.
	 * 
	 * @param fps desired framerate in frames per second
	 * @param last_frame_time persistent storage for the time the function was last called
	 * @return information about the frame: the delta time since the previous call, and
	 * the fraction of the delta time which was taken up by the time between calls (the
	 * remaining fraction being occupied by the `targetFramerate` function idling)
	 **/
	static FrameData targetFramerate(int fps, clock_type::time_point& last_frame_time)
#ifdef STUI_IMPLEMENTATION
	{
		chrono::duration<float> active_frame_duration = chrono::high_resolution_clock::now() - last_frame_time;

		auto next_frame_time = last_frame_time + chrono::duration<float>(1.0f / (float)fps);
		this_thread::sleep_until(next_frame_time);

		chrono::duration<float> total_frame_duration = chrono::high_resolution_clock::now() - last_frame_time;
		last_frame_time = chrono::high_resolution_clock::now();

		return FrameData{ total_frame_duration.count(), active_frame_duration.count() / total_frame_duration.count() };
	}
#endif
	;

private:
	/**
	 * @brief calculate an appropriate size for a `Component` (or, one dimension of it)
	 * 
	 * @param available amount of space available to use
	 * @param max maximum desired size of the subject
	 * @param min minimum desired size of the subject
	 * @return resulting constrained size
	 **/
	static inline int getConstrainedSize(int available, int _max, int _min)
	{
		int size = 0;
		if (_max == -1) size = available;
		else size = min(available, _max);

		if (size < _min) return -1;
		else return max(size, _min);
	}

	
};

}

#ifndef STUI_KEEP_DEFINES

#undef RENDER_STUB
#undef GETMAXSIZE_STUB
#undef GETMINSIZE_STUB
#undef OUTPUT_TARGET
#undef ANSI_ESCAPE
#undef ANSI_CLEAR_SCREEN
#undef ANSI_CLEAR_SCROLL
#undef ANSI_PANDOWN
#undef ANSI_PANUP

#endif