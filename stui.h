#pragma once


// TODO: add a proper license

#include <string>
#include <format>
#include <iostream>
#include <vector>
#include <chrono>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#else
#ifdef __linux__
#include <sys/ioctl.h>
#endif
#endif

using namespace std;

#define RENDER_STUB virtual inline void render(char* output_buffer, Coordinate size) override
#define GETMINSIZE_STUB(w,h) virtual inline Coordinate getMinSize() override { return { w, h }; }
#define GETMAXSIZE_STUB(w,h) virtual inline Coordinate getMaxSize() override { return { w, h }; }
#define COMPONENT_STUB_0(cls) class cls : public Component\
{\
public:\
	cls() { }\

#define COMPONENT_STUB_1(cls,par0_t,par0_n) class cls : public Component\
{\
public:\
	par0_t par0_n;\
	cls(par0_t _##par0_n) : par0_n(_##par0_n) { }\

#define COMPONENT_STUB_2(cls,par0_t,par0_n,par1_t,par1_n) class cls : public Component\
{\
public:\
	par0_t par0_n;\
	par1_t par1_n;\
	cls(par0_t _##par0_n, par1_t _##par1_n) : par0_n(_##par0_n), par1_n(_##par1_n) { }\
	
#define OUTPUT_TARGET cout

#define ANSI_ESCAPE '\033'
#define ANSI_CLEAR_SCREEN ANSI_ESCAPE << "[2J" 
#define ANSI_CLEAR_SCROLL ANSI_ESCAPE << "[3J"
#define ANSI_SETCURSOR(x,y) ANSI_ESCAPE << format("[{};{}H", y, x)
#define ANSI_PANUP(n) ANSI_ESCAPE << format("[{}T", n)
#define ANSI_PANDOWN(n) ANSI_ESCAPE << format("[{}S", n)

namespace stui
{

/**
 * @brief two-dimensional integer coordinate pair.
 * 
 **/
struct Coordinate
{
	int x, y;
};

/**
 * @brief base class from which all UI components inherit.
 * 
 * all `Component` subclasses must override the `render`, `getMaxSize`, and 
 * `getMinSize` methods (ideally using the relevant macros) 
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
	virtual inline void render(char* output_buffer, Coordinate size) { }
	
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
};

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
inline void drawBox(Coordinate box_origin, Coordinate box_size, char* buffer, Coordinate buffer_size)
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

inline vector<string> wrapTextInner(string text, size_t max_width)
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
inline vector<string> wrapText(string text, size_t max_width)
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
inline void drawText(string text, bool wrap, Coordinate text_origin, Coordinate max_size, char* buffer, Coordinate buffer_size)
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
inline char* makeBuffer(Coordinate buffer_size)
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
inline void copyBox(const char* src, Coordinate src_size, Coordinate src_offset, Coordinate area_size, char* dst, Coordinate dst_size, Coordinate dst_offset)
{
	if (src == nullptr || dst == nullptr) return;
	if (area_size.x <= 0 || area_size.y <= 0) return;
	if (src_offset.x < 0 || src_offset.y < 0) return;
	if (dst_offset.x < 0 || dst_offset.y < 0) return;
	if (src_offset.x + area_size.x > src_size.x || src_offset.y + area_size.y > src_size.y) return;
	if (dst_offset.x + area_size.x > dst_offset.x || dst_offset.y + area_size.y > dst_offset.y) return;

	for (int y = 0; y < area_size.y; y++)
	{
		memcpy(dst + dst_offset.x + ((y + dst_offset.y) * dst_size.x), src + src_offset.x + ((y + src_offset.y) * src_size.x), area_size.x);
	}
}

/**
 * @brief single-line text box.
 **/
COMPONENT_STUB_1(Text, string, text)
// TODO: add text alignment (left, center, right)
// TODO: add newline trimming for input string
	RENDER_STUB
	{
		drawText(text, false, Coordinate{ 0,0 }, Coordinate{ size.x, 1 }, output_buffer, size);
	}

	GETMAXSIZE_STUB(-1, 1);
	GETMINSIZE_STUB(static_cast<int>(text.length()), 1);
};

/**
 * @brief multi-line wrapping text area, with a box outline.
 **/
COMPONENT_STUB_1(TextArea, string, text)
	RENDER_STUB
	{
		drawBox(Coordinate{ 0,0 }, size, output_buffer, size);
		drawText(text, true, Coordinate{ 1,1 }, Coordinate{ size.x - 2, size.y - 2 }, output_buffer, size);
	}

	GETMAXSIZE_STUB(-1, -1);
	GETMINSIZE_STUB(3, 3);
};

/**
 * @brief linear progress bar.
 * 
 * progress is expressed as a `fraction` between 0 and 1.
 **/
COMPONENT_STUB_1(ProgressBar, float, fraction)
	RENDER_STUB
	{
		int completed = (int)round((float)size.x * fraction);
			
		for (int i = 0; i < size.x; i++)
			output_buffer[i] = i < completed ? '\xdb' : '\xb0';
	}

	GETMAXSIZE_STUB(-1, 1);
	GETMINSIZE_STUB(1, 1);
};

/**
 * @brief simple activity spinner.
 * 
 * described by its `state` (effectively the current animation frame index), and its `type`
 * (style setting between 0 and 3 inclusive).
 **/
COMPONENT_STUB_2(Spinner, size_t, state, int, type)
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

	GETMAXSIZE_STUB(1, 1);
	GETMINSIZE_STUB(1, 1);
};

/**
 * @brief vertical layout box containing a list of child widgets.
 * 
 * the layout algorithm will attempt to meet the minimum sizes of all children first, then
 * expand each element one-by-one until they either meet their max height or there's no more
 * space to expand into. ensures minimum widths for all elements.
 **/
COMPONENT_STUB_1(VerticalBox, vector<Component*>, children)
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
			children[i]->render(component_buffer, component_size);
			copyBox(component_buffer, component_size, Coordinate{ 0,0 }, component_size, output_buffer, size, Coordinate{ 0,y_offset });
			delete[] component_buffer;
			y_offset += component_size.y;
		}
	}

	virtual inline Coordinate getMaxSize() override
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

	virtual inline Coordinate getMinSize() override
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
COMPONENT_STUB_1(HorizontalBox, vector<Component*>, children)
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
			children[i]->render(component_buffer, component_size);
			copyBox(component_buffer, component_size, Coordinate{ 0,0 }, component_size, output_buffer, size, Coordinate{ x_offset,0 });
			delete[] component_buffer;
			x_offset += component_size.x;
		}
	}

	virtual inline Coordinate getMaxSize() override
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

	virtual inline Coordinate getMinSize() override
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
 * @brief blank spacing element which fills a set amount of space horizontally.
 **/
COMPONENT_STUB_1(HorizontalSpacer, size_t, size)
	RENDER_STUB
	{ }

	GETMAXSIZE_STUB(static_cast<int>(size), 1);
	GETMINSIZE_STUB(static_cast<int>(size), 1);
};

/**
 * @brief blank spacing element which fills a set amount of space vertically.
 **/
COMPONENT_STUB_1(VerticalSpacer, size_t, size)
	RENDER_STUB
	{ }

	GETMAXSIZE_STUB(1, static_cast<int>(size));
	GETMINSIZE_STUB(1, static_cast<int>(size));
};

/**
 * @brief complex display element capable of visualising tree structures as a heirarchy
 * with nested, expandable nodes.
 * 
 * nodes can be given a descriptive name, and an identifier (which could be used to
 * index into your own array of proprietary nodes). expansion of nodes can be toggled;
 * if a node is expanded then its children will be shown, otherwise they will not.
 **/
class TreeDisplay : public Component
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
	TreeDisplay(Node* _root, size_t _scroll) : root(_root), scroll(_scroll) { }

	RENDER_STUB
	{
		drawBox(Coordinate{ 0,0 }, size, output_buffer, size);
		if (root == nullptr) return;
		int top = 1 - static_cast<int>(scroll);
		printNode(root, 0, top, output_buffer, size);
	}

	GETMAXSIZE_STUB(-1, -1);
	GETMINSIZE_STUB(10, 3);

private:
	inline void printNode(Node* node, int depth, int& top, char* output_buffer, Coordinate buffer_size)
	{
		if (top >= buffer_size.y - 1) return;

		if (top > 0)
		{
			drawText((node->expanded ? "\xaa " : "> ") + node->name, false, Coordinate{ 1 + depth, top }, Coordinate{ buffer_size.x - 4 - depth, 1 }, output_buffer, buffer_size);
			string id_desc = " (" + to_string(node->id) + ")";
			drawText(id_desc, false, Coordinate{ buffer_size.x - (int)id_desc.length() - 1, top }, Coordinate{ (int)id_desc.length(), 1 }, output_buffer, buffer_size);
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
COMPONENT_STUB_2(ImageView, uint8_t*, grayscale_image, Coordinate, image_size)
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

	GETMAXSIZE_STUB(image_size.x * 2, image_size.y);
	GETMINSIZE_STUB(1, 1);
};

// TODO: more error checking (checking for bounds size, etc)
// TODO: transition away from component stubs
// TODO: add lots of comments
// TODO: tab menu, tabs
// TODO: reading UI layout from file
// TODO: input
// TODO: focus zone (i.e. you need to hit tab to switch between focus zones
// TODO: input library, bindings?
// TODO: more advanced sizing and formatting boxes
// TODO: object renderer

class Page
{
public:
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
		root_component->render(root_component_buffer, root_component_size);
		copyBox(root_component_buffer, root_component_size, Coordinate{ 0,0 }, root_component_size, root_staging_buffer, screen_size, Coordinate{ 0,0 });
		delete[] root_component_buffer;

		OUTPUT_TARGET << ANSI_CLEAR_SCROLL;
		setCursorPosition(Coordinate{ 0,0 });
		OUTPUT_TARGET << root_staging_buffer;

		delete[] root_staging_buffer;
	}

	struct FrameData
	{
		float delta_time;
		float active_fraction;
	};

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
	static inline int getConstrainedSize(int available, int max, int min)
	{
		int size = 0;
		if (max == -1) size = available;
		else size = min(available, max);

		if (size < min) return -1;
		else return max(size, min);
	}

	static inline void clear()
	{
		OUTPUT_TARGET << ANSI_CLEAR_SCREEN << ANSI_CLEAR_SCROLL;
	}

	static inline Coordinate getScreenSize()
	{
#ifdef _WIN32
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
		return Coordinate{ info.dwSize.X, info.dwSize.Y };
#else
#ifdef __linux__
		struct winsize size;
		ioctl(fileno(stdout), TIOCGWINSZ, &size);
		return Coordinate{ (int)size.ws_col, (int)size.ws_row };
#endif
#endif
	}

	static inline void setCursorPosition(Coordinate position)
	{
		OUTPUT_TARGET << ANSI_SETCURSOR(position.x, position.y);
	}

	static inline void setCursorVisible(bool visible)
	{
#ifdef _WIN32
		CONSOLE_CURSOR_INFO info;
		GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
		info.bVisible = visible;
		SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
#endif
		// TODO: linux implementation
	}

};

}

#ifndef STUI_KEEP_DEFINES

#undef RENDER_STUB
#undef GETMAXSIZE_STUB
#undef GETMINSIZE_STUB
#undef COMPONENT_STUB_0
#undef COMPONENT_STUB_1
#undef COMPONENT_STUB_2
#undef OUTPUT_TARGET
#undef ANSI_ESCAPE
#undef ANSI_CLEAR_SCREEN
#undef ANSI_CLEAR_SCROLL
#undef ANSI_PANDOWN
#undef ANSI_PANUP

#endif