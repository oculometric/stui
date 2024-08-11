#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstring>
#include <cmath>

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#define VC_EXTRALEAN
	#include <Windows.h>
#elif defined(__linux__)
	#include <sys/ioctl.h>
#endif

using namespace std;

#define RENDER_STUB virtual inline void render(char* output_buffer, Coordinate size, bool focused) override
#define GETMINSIZE_STUB virtual inline Coordinate getMinSize() override
#define GETMAXSIZE_STUB virtual inline Coordinate getMaxSize() override
#define HANDLEINPUT_STUB virtual inline bool handleInput(uint8_t input_character) override
#define	ISFOCUSABLE_STUB virtual inline bool isFocusable() override

#define OUTPUT_TARGET cout

#define ANSI_ESCAPE '\033'
#define ANSI_CLEAR_SCREEN ANSI_ESCAPE << "[2J" 
#define ANSI_CLEAR_SCROLL ANSI_ESCAPE << "[3J"
#define ANSI_SETCURSOR(x,y) ANSI_ESCAPE << '[' << y << ';' << x << 'H'
#define ANSI_PANUP(n) ANSI_ESCAPE << '[' << n << 'T'
#define ANSI_PANDOWN(n) ANSI_ESCAPE << '[' << n << 'S'

namespace stui
{

/**
 * @brief removes any invalid characters from a string, as well as other
 * specified illegal characters
 * 
 * @param str string to remove invalid characters from
 * @param others array of additional characters that should be excluded
 * @return repaired string 
 **/
inline string stripNullsAndMore(string str, const char* others)
{
    string result = "";
    for (char c : str)
    {
        if (c < ' ' && c != '\n' && c != '\t') continue;
		bool is_valid = true;
		for (size_t i = 0; others[i] != '\0'; i++)
			if (c == others[i]) is_valid = false;
		if (is_valid)
		{
			if (c == '\t') result += "    ";
			else result += c;
		}
    }
    return result;
}

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
		PASS		= 0b0000,
		FG_BLACK 	= 0b00000001,
		FG_RED   	= 0b00000010,
		FG_GREEN 	= 0b00000100,
		FG_BLUE  	= 0b00001000,
		FG_YELLOW 	= 0b00000110,
		FG_CYAN	    = 0b00001100,
		FG_MAGENTA	= 0b00001010,
		FG_WHITE	= 0b00001110,
		BG_BLACK 	= 0b00010000,
		BG_RED   	= 0b00100000,
		BG_GREEN 	= 0b01000000,
		BG_BLUE  	= 0b10000000,
		BG_YELLOW 	= 0b01100000,
		BG_CYAN	    = 0b11000000,
		BG_MAGENTA	= 0b10100000,
		BG_WHITE	= 0b11100000,
	};

	uint8_t character;
	ColourCommand colour;
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
	/**
	 * @brief draws the `Component` into a character buffer, with a specified size.
	 * 
	 * implementations should assume that the `output_buffer` has been allocated to be the correct
	 * `size`, but should also check that any drawing is performed within the limits of the `size`
	 * (otherwise you'll corrupt the heap, yippee!).
	 * 
	 * @param output_buffer buffer allocated by the caller to be drawn into. ordered left
	 * to right, top to bottom
	 * @param size size of the buffer
	 **/
	virtual inline void render(char* output_buffer, Coordinate size, bool focused) { }
	
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
	virtual inline bool handleInput(uint8_t input_character) { return false; }

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
	static inline void drawBox(Coordinate box_origin, Coordinate box_size, char* buffer, Coordinate buffer_size)
	{
		if (buffer == nullptr) return;
		if (box_size.x <= 0 || box_size.y <= 0) return;

		if (box_origin.y >= 0 && box_origin.y < buffer_size.y)
		{
			for (int x = box_origin.x; x < box_origin.x + box_size.x; x++)
			{
				if (x < 0) continue;
				if (x >= buffer_size.x) break;

				buffer[x + (box_origin.y * buffer_size.x)] = (x == box_origin.x ? '\xc9' : (x == box_origin.x + box_size.x - 1 ? '\xbb' : '\xcd'));
			}
		}

		for (int y = box_origin.y + 1; y < box_origin.y + box_size.y - 1; y++)
		{
			if (y < 0) continue;
			if (y >= buffer_size.y) break;

			if (box_origin.x >= 0 && box_origin.x < buffer_size.x)
				buffer[box_origin.x + (y * buffer_size.x)] = '\xba';

			if (box_origin.x + box_size.x - 1 >= 0 && box_origin.x + box_size.x - 1 < buffer_size.x)
				buffer[box_origin.x + box_size.x + (y * buffer_size.x) - 1] = '\xba';
		}

		if (box_origin.y + box_size.y - 1 >= 0 && box_origin.y + box_size.y - 1 < buffer_size.y)
		{
			for (int x = box_origin.x; x < box_origin.x + box_size.x; x++)
			{
				if (x < 0) continue;
				if (x >= buffer_size.x) break;

				buffer[x + ((box_origin.y + box_size.y - 1) * buffer_size.x)] = (x == box_origin.x ? '\xc8' : (x == box_origin.x + box_size.x - 1 ? '\xbc' : '\xcd'));
			}
		}
	}

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
	static inline vector<string> wrapText(string text, size_t max_width)
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

	static inline vector<string> wrapTextInner(string text, size_t max_width)
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
	static inline void drawText(string text, bool wrap, Coordinate text_origin, Coordinate max_size, char* buffer, Coordinate buffer_size)
	{
		if (buffer == nullptr) return;
		if (buffer_size.x <= 0 || buffer_size.y <= 0) return;

		if (wrap)
		{
			vector<string> lines = wrapText(text, min(max_size.x, buffer_size.x - text_origin.x));
			
			size_t row = 0;
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
			for (size_t i = 0; i < text.length(); i++)
			{
				if (text_origin.x + i < 0) continue;
				if (text_origin.x + i >= buffer_size.x || i >= max_size.x) break;
				if (text_origin.y < 0 || text_origin.y >= buffer_size.y) break;

				buffer[(text_origin.x + i) + (text_origin.y * buffer_size.x)] = text[i];
			}
		}
	}

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
	static inline char* makeBuffer(Coordinate buffer_size)
	{
		if (buffer_size.x <= 0 || buffer_size.y <= 0) return nullptr;

		size_t size = buffer_size.x * buffer_size.y;
		char* buf = new char[size + 1];
		memset(buf, ' ', size);
		buf[size] = '\0';

		return buf;
	}

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
	static inline void copyBox(const char* src, Coordinate src_size, Coordinate src_offset, Coordinate area_size, char* dst, Coordinate dst_size, Coordinate dst_offset)
	{
		if (src == nullptr || dst == nullptr) return;
		if (area_size.x <= 0 || area_size.y <= 0) return;
		if (src_offset.x < 0 || src_offset.y < 0) return;
		if (dst_offset.x < 0 || dst_offset.y < 0) return;
		if (src_offset.x + area_size.x > src_size.x || src_offset.y + area_size.y > src_size.y) return;
		if (dst_offset.x + area_size.x > dst_size.x || dst_offset.y + area_size.y > dst_size.y) return;

		for (int y = 0; y < area_size.y; y++)
		{
			memcpy(dst + dst_offset.x + ((y + dst_offset.y) * dst_size.x), src + src_offset.x + ((y + src_offset.y) * src_size.x), area_size.x);
		}
	}
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
		LEFT_CTRL   = 0b00000001,
		RIGHT_CTRL  = 0b00000010,
		SHIFT       = 0b00000100,
		LEFT_ALT    = 0b00010000,
		RIGHT_ALT   = 0b00100000
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
	/**
	 * @brief queries the system's input buffer and fetches all available key events.
	 * 
	 * only returns the key-down key events (and repeats). translates presence of meta
	 * keys like CTRL, SHIFT, ALT. also moves some relevant key presses down into the
	 * ASCII range to make them easier to read (specifically the arrow keys).
	 * 
	 * @returns list of key-press events
	 **/
	static inline vector<Key> getQueuedKeyEvents()
	{
		vector<Key> events;
#if defined(_WIN32)
		DWORD events_available;
		GetNumberOfConsoleInputEvents(GetStdHandle(STD_INPUT_HANDLE), &events_available);
		if (events_available < 1) return events;

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
				if (control_key & 0x01) k.control_states = (ControlKeys)(k.control_states | ControlKeys::RIGHT_ALT);
				if (control_key & 0x02) k.control_states = (ControlKeys)(k.control_states | ControlKeys::LEFT_ALT);
				if (control_key & 0x04) k.control_states = (ControlKeys)(k.control_states | ControlKeys::RIGHT_CTRL);
				if (control_key & 0x08) k.control_states = (ControlKeys)(k.control_states | ControlKeys::LEFT_CTRL);
				if (control_key & 0x10) k.control_states = (ControlKeys)(k.control_states | ControlKeys::SHIFT);
				events.push_back(k);
			}
#elif defined(__linux__)
		// TODO: linux implementation
#endif
		return events;
	}
	
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
	static inline void processShortcuts(vector<Shortcut> shortcuts, vector<Key>& key_events)
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
	static inline vector<uint8_t> getTextCharacters(vector<Key>& key_events)
	{
		vector<Key> non_processed;
		vector<uint8_t> result;
		for (size_t i = 0; i < key_events.size(); i++)
		{
			Key k = key_events[i];
			if ((k.control_states == ControlKeys::NONE || k.control_states == ControlKeys::SHIFT) && (
				  (k.key >= 32 && k.key <= 127) 
				|| k.key == '\n' 
				|| k.key == '\t' 
				|| k.key == ArrowKeys::UP
				|| k.key == ArrowKeys::DOWN
				|| k.key == ArrowKeys::LEFT
				|| k.key == ArrowKeys::RIGHT))
			{
				result.push_back(static_cast<uint8_t>(k.key));
			}
			else
			{
				non_processed.push_back(k);
			}
		}

		key_events = non_processed;

		return result;
	}

	// TODO: here
	// size_t const BufferSize = 512;
//char buffer[BufferSize];

//if (std::cin.readsome(buffer, BufferSize) >= 1) {
//}
//std::ios_base::sync_with_stdio(false)
/// or if (std::cin.rdbuf() and std::cin.rdbuf()->in_avail() >= 0) {
//}

// also getch() from conio.h
};

/**
 * @brief single-line text box.
 * 
 * text `alignment` can be specified as < 0 for left-aligned, = 0 for center-aligned, or
 * > 0 for right-aligned
 **/
class Text : public Component, public Utility
{
public:
	string text;
	int alignment;
	
	Text(string _text, int _alignment) : text(_text), alignment(_alignment) { }

	RENDER_STUB
	{
		if (size.y < 1) return;

		Coordinate offset{ 0,0 };
		if (alignment < 0)
			offset.x = 0;
		else if (alignment == 0)
			offset.x = static_cast<int>((size.x - text.length()) / 2);
		else if (alignment > 1)
			offset.x = static_cast<int>(size.x - text.length());

		drawText(stripNullsAndMore(text, "\n\t"), false, offset, Coordinate{ static_cast<int>(text.length()), 1 }, output_buffer, size);
	}

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
	{
		if (size.y < 1) return;

		drawText("> " + text + "< ", false, Coordinate{ 0,0 }, Coordinate{ static_cast<int>(text.length()) + 4,1 }, output_buffer, size);
	}

	GETMAXSIZE_STUB { return Coordinate{ -1,1 }; }
	GETMINSIZE_STUB { return Coordinate{ static_cast<int>(text.length()) + 4,1 }; }

	HANDLEINPUT_STUB { if (input_character == '\n' && callback != nullptr) { callback(); return true; } return false; }
	ISFOCUSABLE_STUB { return enabled; }
};

/**
 * @brief simple text entry box.
 **/
class TextInputBox : public Component, public Utility
{
public:
	string text;
	void (*callback)();
	bool enabled;

	TextInputBox(string _text, void (*_callback)(), bool _enabled) : text(_text), callback(_callback), enabled(_enabled) { }

	RENDER_STUB
	{
		if (size.y < 1) return;

		drawText("> " + text, false, Coordinate{ 0,0 }, Coordinate{ static_cast<int>(text.length()) + 2,1 }, output_buffer, size);
	}

	GETMAXSIZE_STUB { return Coordinate{ -1,1 }; }
	GETMINSIZE_STUB { return Coordinate{ 12,1 }; }

	HANDLEINPUT_STUB
	{
		if (input_character == '\n' && callback != nullptr) callback();
		else text += input_character;

		return true;
	}

	ISFOCUSABLE_STUB { return enabled; }
};

/**
 * @brief multi-line wrapping text area, with a box outline.
 **/
class TextArea : public Component, public Utility
{
public:
	string text;
	bool editable;

	TextArea(string _text, bool _editable) : text(_text), editable(_editable) { }

	RENDER_STUB
	{
		if (size.y < 2 || size.x < 2) return;

		drawText(stripNullsAndMore(text, ""), true, Coordinate{ 0,0 }, Coordinate{ size.x, size.y }, output_buffer, size);
	}

	GETMAXSIZE_STUB { return Coordinate{ -1, -1 }; }
	GETMINSIZE_STUB { return Coordinate{ 3, 3 }; }

	HANDLEINPUT_STUB
	{
		if (input_character == '\b') text.pop_back();
		else text.push_back(input_character);

		return true;
	}

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
	{
		if (size.y < 1) return;

		int completed = (int)round((float)size.x * fraction);
			
		for (int i = 0; i < size.x; i++)
			output_buffer[i] = i < completed ? '\xdb' : '\xb0';
	}

	GETMAXSIZE_STUB { return Coordinate{ -1, 1 }; }
	GETMINSIZE_STUB { return Coordinate{ 1, 1 }; }
};

/**
 * @brief simple activity spinner.
 * 
 * described by its `state` (effectively the current animation frame index), and its `type`
 * (style setting between 0 and 3 inclusive).
 **/
class Spinner : public Component
{
public:
	size_t state;
	int type;

	Spinner(size_t _state, int _type) : state(_state), type(_type) { }

	RENDER_STUB
	{
		const char* sequence;
		switch (type)
		{
		case 1:
			sequence = "\xdc\xdd\xdf\xde"; break;
		case 2:
			sequence = "\xbf\xd9\xc0\xda"; break;
		case 3:
			sequence = "\xdf\xdb\xdc\xdb"; break;
		case 0:
		default:
			sequence = "|/-\\"; break;
		} 
		output_buffer[0] = sequence[state % 4];
	}

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
			char* component_buffer = makeBuffer(component_size);
			children[i]->render(component_buffer, component_size, false);
			copyBox(component_buffer, component_size, Coordinate{ 0,0 }, component_size, output_buffer, size, Coordinate{ 0,y_offset });
			delete[] component_buffer;
			y_offset += component_size.y;
		}
	}

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
			char* component_buffer = makeBuffer(component_size);
			children[i]->render(component_buffer, component_size, false);
			copyBox(component_buffer, component_size, Coordinate{ 0,0 }, component_size, output_buffer, size, Coordinate{ x_offset,0 });
			delete[] component_buffer;
			x_offset += component_size.x;
		}
	}

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
	size_t size;
	
	VerticalSpacer(size_t _size) : size(_size) { }
	
	RENDER_STUB
	{ }

	GETMAXSIZE_STUB { return Coordinate{ 1, static_cast<int>(size) }; }
	GETMINSIZE_STUB { return Coordinate{ 1, static_cast<int>(size) }; }
};

/**
 * @brief blank spacing element which fills a set amount of space horizontally.
 **/
class HorizontalSpacer : public Component
{
public:
	size_t size;
	
	HorizontalSpacer(size_t _size) : size(_size) { }

	RENDER_STUB
	{ }

	GETMAXSIZE_STUB { return Coordinate{ static_cast<int>(size), 1 }; }
	GETMINSIZE_STUB { return Coordinate{ static_cast<int>(size), 1 }; }
};

/**
 * @brief draws a border around another component using IBM box characters.
 **/
class BorderedBox : public Component, public Utility
{
public:
	Component* child;

	BorderedBox(Component* _child) : child(_child) { }

	RENDER_STUB
	{
		if (size.x < 3 || size.y < 3) return;
		drawBox(Coordinate{ 0,0 }, size, output_buffer, size);
		if (child == nullptr) return;

		Coordinate component_size{ size.x - 2, size.y - 2 };
		char* component_buffer = makeBuffer(component_size);
		child->render(component_buffer, component_size, false);
		copyBox(component_buffer, component_size, Coordinate{ 0,0 }, component_size, output_buffer, size, Coordinate{ 1,1 });
		delete[] component_buffer;
	}

	GETMAXSIZE_STUB { return (child == nullptr) ? Coordinate{ -1,-1 } : child->getMaxSize(); }
	GETMINSIZE_STUB { return (child == nullptr) ? Coordinate{ 2,2 } : child->getMinSize(); }
};

/**
 * @brief element which displays a list of strings.
 * 
 * allows scrolling.
 * 
 **/
class ListView : public Component, public Utility
{
public:
	vector<string> elements;
	size_t scroll;

	ListView(vector<string> _elements, size_t _scroll) : elements(_elements), scroll(_scroll) { }

	RENDER_STUB
	{
		if (size.x < 2 || size.y < 2) return;
		int row = -1 - static_cast<int>(scroll);
		int index = -1;
		for (string element : elements)
		{
			index++;
			row++;
			if (row < 0) continue;
			if (row >= size.y - 2) break;

			drawText(stripNullsAndMore(element, "\n\t"), false, Coordinate{ 0, row }, Coordinate{ size.x, 1 }, output_buffer, size);
			string index_str = " (" + to_string(index) + ")";
			drawText(index_str, false, Coordinate{ size.x - static_cast<int>(index_str.length()), row }, Coordinate{ size.x, 1 }, output_buffer, size);
		}
	}

	GETMAXSIZE_STUB { return Coordinate{ -1, -1 }; }
	GETMINSIZE_STUB { return Coordinate{ 10, 3 }; }
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

	TreeView(Node* _root, size_t _scroll) : root(_root), scroll(_scroll) { }

	RENDER_STUB
	{
		if (size.x < 2 || size.y < 2) return;
		if (root == nullptr) return;
		int top = 0 - static_cast<int>(scroll);
		printNode(root, 0, top, output_buffer, size);
	}

	GETMAXSIZE_STUB { return Coordinate{ -1, -1 }; }
	GETMINSIZE_STUB { return Coordinate{ 10, 3 }; }

private:
	static inline void printNode(Node* node, int depth, int& top, char* output_buffer, Coordinate buffer_size)
	{
		if (top >= buffer_size.y) return;

		if (top >= 0)
		{
			drawText((node->expanded ? "\xaa " : "> ") + stripNullsAndMore(node->name, "\n\t"), false, Coordinate{ depth, top }, Coordinate{ buffer_size.x - 2 - depth, 1 }, output_buffer, buffer_size);
			string id_desc = " [" + to_string(node->children.size()) + "]";
			drawText(id_desc, false, Coordinate{ buffer_size.x - (int)id_desc.length(), top }, Coordinate{ (int)id_desc.length(), 1 }, output_buffer, buffer_size);
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
	{
		if (grayscale_image == nullptr) return;

		for (int y = 0; y < size.y; y++)
		{
			for (int x = 0; x < size.x; x++)
			{
				char out = ' ';
				uint8_t in = grayscale_image[(x / 2) + (y * image_size.x)];
				if (in >= 192) out = '\xdb';
				else if (in >= 128) out = '\xb2';
				else if (in >= 96) out = '\xb1';
				else if (in >= 64) out = '\xb0';
				else if (in >= 32) out = '\xfa';
				else out = ' ';
				output_buffer[x + (y * size.x)] = out;
			}
		}
	}

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
	{
		if (child == nullptr) return;
		child->render(output_buffer, size, false);
	}

	GETMAXSIZE_STUB { return Coordinate{ max_size.x, max_size.y }; }

	GETMINSIZE_STUB
	{
		if (child == nullptr) return Coordinate{ 0,0 };
		return child->getMinSize();
	}
};

class TabDisplay : public Component, public Utility
{
public:
	vector<string> tab_descriptions;
	size_t current_tab;

	TabDisplay(vector<string> _tab_descriptions, size_t _current_tab) : tab_descriptions(_tab_descriptions), current_tab(_current_tab) { }

	RENDER_STUB
	{
		if (size.y < 1) return;
		int offset = 0;
		for (size_t i = 0; i < tab_descriptions.size(); i++)
		{
			string tab_text = " [" + to_string(i) + " - " + tab_descriptions[i] + "]";
			drawText(tab_text, false, Coordinate{ offset,0 }, Coordinate{ size.x,1 }, output_buffer, size);
			offset += tab_text.length();
		}
	}

	GETMAXSIZE_STUB { return Coordinate{ -1,1 }; }
	GETMINSIZE_STUB { return Coordinate{ 10,1 }; }
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
	static inline void render(Component* root_component)
	{
		setCursorVisible(false);
		Coordinate screen_size = getScreenSize();

		char* root_staging_buffer = makeBuffer(screen_size);

		Coordinate root_component_size
		{
			getConstrainedSize(screen_size.x, root_component->getMaxSize().x, root_component->getMinSize().x),
			getConstrainedSize(screen_size.y, root_component->getMaxSize().y, root_component->getMinSize().y)
		};
		char* root_component_buffer = makeBuffer(root_component_size);
		root_component->render(root_component_buffer, root_component_size, false);
		copyBox(root_component_buffer, root_component_size, Coordinate{ 0,0 }, root_component_size, root_staging_buffer, screen_size, Coordinate{ 0,0 });
		delete[] root_component_buffer;

		OUTPUT_TARGET << ANSI_CLEAR_SCROLL;
		setCursorPosition(Coordinate{ 0,0 });
		OUTPUT_TARGET << root_staging_buffer;

		delete[] root_staging_buffer;
	}

	static inline void handleInput(Component* focused_component, vector<Input::Shortcut> shortcut_bindings)
	{
		auto keys = Input::getQueuedKeyEvents();
		Input::processShortcuts({}, keys);
		auto text_keys = stui::Input::getTextCharacters(keys);

		if (focused_component == nullptr) return;
		// TODO: find the focussed component and send input
	}

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
	static inline FrameData targetFramerate(int fps, chrono::steady_clock::time_point& last_frame_time)
	{
		chrono::duration<float> active_frame_duration = chrono::high_resolution_clock::now() - last_frame_time;

		auto next_frame_time = last_frame_time + chrono::duration<float>(1.0f / (float)fps);
		this_thread::sleep_until(next_frame_time);

		chrono::duration<float> total_frame_duration = chrono::high_resolution_clock::now() - last_frame_time;
		last_frame_time = chrono::high_resolution_clock::now();

		return FrameData{ total_frame_duration.count(), active_frame_duration.count() / total_frame_duration.count() };
	}

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
		OUTPUT_TARGET << ANSI_SETCURSOR(position.x, position.y);
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
		// TODO: linux implementation
#endif
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