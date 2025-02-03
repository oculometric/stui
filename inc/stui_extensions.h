/*  
	A simple header-only library for creating text-based user interfaces
	inside a terminal window.
	Copyright (C) 2024-2025  Jacob Costen (oculometric)

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

#ifndef STUI_EXTENSIONS_H
#define STUI_EXTENSIONS_H

///////////////////////////////////////////////////////////////////////
//                             INCLUDES
///////////////////////////////////////////////////////////////////////

#ifdef STUI_KEEP_DEFINES
#define STUI_TMP_KEEP_DEFS
#endif

#define STUI_KEEP_DEFINES
#include "stui.h"

#ifndef STUI_TMP_KEEP_DEFS
#undef STUI_KEEP_DEFINES
#endif

#include <map>
#include <queue>

namespace stui
{

///////////////////////////////////////////////////////////////////////
//                               PAGE
///////////////////////////////////////////////////////////////////////

#pragma region STUI_PAGE

class Page;

static Page* current_page = nullptr;

/**
 * @brief encapsulates most of the behaviour necessary for managing a page of user interface
 * components.
 * 
 * this class manages component focus for you, and also keeps track of components with unique 
 * names, making it easy to pass an entire block of UI from one place to another in your code.
 * 
 * leaves you with enough control to hopefully not interfere with your own program architecture.
 * 
 * see HELP.md for more information on how to best make use of this.
 */
class Page
{
public:
	vector<Input::Shortcut> shortcuts;
	vector<Component*> focusable_component_sequence;

private:
	map<string, Component*> components;
	Component* root = nullptr;
	size_t focused_component_index = 0;
	clock_type::time_point last_frame;

public:
	Page() { last_frame = clock_type::now(); }

	Page(Page& other) = delete;
	Page operator=(Page& other) = delete;

	/**
	 * @brief checks for user input and sends it to the currently focused component.
	 * 
	 * @returns true if some input was detected, false if none
	 */
	bool checkInput();

	/**
	 * @brief redraws the entire UI tree pointed to by `root` into the terminal.
	 *
	 * you only actually need to do this if something has changed: for instance,
	 * you changed the text of a component, or some input was received from the
	 * user, or the terminal window was resized.
	 */
	void render();

	/**
	 * @brief attempts to maintain the framerate specified, by waiting for
	 * whatever time is left in the current frame.
	 *
	 * you don't necessarily have to re-draw the screen this many times
	 * per second, you can simply use this function for limiting the
	 * rate of any random loop (or for instance, your input checking loop).
	 *
	 * @param fps_target target number of frames/function calls to be
	 * possible per second
	 * @returns information about the duration and active fraction of
	 * the last frame
	 */
	Renderer::FrameData framerate(int fps_target);

	/**
	 * @brief checks that all components in the UI tree are registered with
	 * the page.
	 *
	 * any components which exist in the tree but not in the registry will
	 * be automatically added to the registry with a procedurally generated
	 * unique name.
	 *
	 * any components which don't exist in the tree but are present in the
	 * registry will be automatically removed from the registry.
	 *
	 * other components will be left alone.
	 *
	 * if `root` is null when this is called, the entire registry will be
	 * cleared.
	 */
	void ensureIntegrity();

	/**
	 * @brief get the component from the registry with the specified name.
	 *
	 * throws if there is no component present with the specified name.
	 *
	 * @param identifier name of the component to fetch
	 * @returns component with the given name in the registry
	 */
	inline Component* operator[](string identifier) { return components[identifier]; }

	/**
	 * @brief get a component from the registry with the specified name, automatically
	 * cast to the specified `Component` type.
	 * 
	 * returns nullptr if the component name is not present in the registry, or if
	 * the component is present but its type does not match the requested type.
	 * 
	 * @tparam T component type to cast to
	 * @param identifier name of the component to fetch
	 * @returns component with the given name from the registry, or nullptr
	 */
	template<class T>
	inline T* get(string identifier)
	{
    	static_assert(std::is_base_of<Component, T>::value, "T is not a Component type");
		if (components.count(identifier) == 0) return nullptr;
		T* tmp = new T();
		Component* comp = components[identifier];
		if (comp->getTypeName() == tmp->getTypeName())
			return (T*)comp;
		
		return nullptr;
	}

	/**
	 * @brief returns a list of all the names of the components registered in the page.
	 * 
	 * @returns list of component names
	 */
	vector<string> getAllComponents();

	/**
	 * @brief assign a new root component. automatically calls `ensureIntegrity`.
	 *
	 * @param component component to make the new root
	 */
	inline void setRoot(Component* component) { root = component; ensureIntegrity(); }

	/**
	 * @brief get the current root of the UI tree.
	 *
	 * @returns root component
	 */
	inline Component* getRoot() { return root; }

	/**
	 * @brief add a component to the registry, with a specified name.
	 *
	 * if the specified name is not unique, or is an empty string, a
	 * new unique one will be generated. does not check for the
	 * component already being in the registry.
	 *
	 * @param component new component to add to the registry
	 * @param identifier name for the new component, optional if you
	 * don't care about accessing the component by name later
	 * @returns the identifier assigned to the component in the registry
	 */
	string registerComponent(Component* component, string identifier = "");

	/**
	 * @brief remove a component from the registry.
	 *
	 * throws a runtime error if the component does not exist in
	 * the registry.
	 *
	 * @param identifier name of the component to remove
	 */
	Component* unregisterComponent(string identifier);

	/**
	 * @brief checks if a component exists in the registry.
	 *
	 * potentially slow for large pages.
	 *
	 * @param component component to check for
	 * @returns true if the component is already registered, false
	 * if not
	 */
	inline bool isComponentRegistered(Component* component)
	{
		for (pair<string, Component*> p : components)
			if (p.second == component) return true;

		return false;
	}

	/**
	 * @brief destroys all the components the page knows about, and `delete`s them.
	 * 
	 * you should use this before you delete a `Page` (or before it goes out of scope) 
	 * if the components within it are created with `new` or via the `LayoutReader`. any components
	 * you didn't allocate with `new` (or via the `LayoutReader`) you **MUST** name them in the
	 * `exclude_list`, otherwise bad things will happen.
	 * 
	 * components in the exclude list will still be removed from the tree (along with everything else),
	 * but they won't be `delete`d and will instead be returned.
	 * 
	 * @param exclude_list list of component names to exclude from the deletion
	 * @returns list of `Component`s which were not deleted (correspond to the `exclude_list`)
	 */
	vector<Component*> destroyAllComponents(vector<string> exclude_list);

	/**
	 * @brief ensures that only one component is focused. specifically the
	 * one indexed by `focused_component_index`. you should call this if you make
	 * manual changes to the list of of focusable components or the focused component
	 * index.
	 */
	inline void updateFocus()
	{
		for (size_t i = 0; i < focusable_component_sequence.size(); i++)
			focusable_component_sequence[i]->focused = (i == focused_component_index);
	}

	/**
	 * @brief sets the currently focused `Component` index. useful if you just added
	 * a `Component` to the tree, and want to immediately pull it as focus.
	 * 
	 * clamped to the end of the `focusable_component_sequence` array.
	 * 
	 * @param index index of the `Component` to focus in the `focusable_component_sequence`
	 */
	inline void setFocusIndex(size_t index)
	{
		focused_component_index = min((int)index, (int)focusable_component_sequence.size() - 1);
	}

	inline ~Page() { }

private:

	/**
	 * @brief callback for when the shortcut for advancing to the next focusable
	 * component is triggered, usually pressing tab.
	 *
	 * acts on the currently active page, which is set automatically by whichever
	 * page is currently consuming input/rendering.
	 */
	static void advanceFocus();

	/**
	 * @brief check if a name already exsists in the registry or not.
	 *
	 * @param name name to search for
	 * @returns true if the name was not found in the registry (and therefore)
	 * is unique, false if the name already exists
	 */
	inline bool isNameUnique(string name)
	{
		if (name.length() < 1) return false;

		return components.count(name) == 0;
	}

	/**
	 * @brief generate a unique name for a given component type.
	 *
	 * @param type type name of the component this identifier will be
	 * used for
	 * @returns a unique identifier
	 */
	string getUniqueName(string type);
};

#pragma endregion STUI_PAGE

///////////////////////////////////////////////////////////////////////
//                         EXTRA COMPONENTS
///////////////////////////////////////////////////////////////////////

#pragma region STUI_COMPONENTS

/**
 * @brief renders a QR code inside the terminal. data buffer must be an array
 * of booleans, sized to provide enough data for the QR code version selected.
 */
class QRCodeView : public Component
{
public:
	enum QRVersion
	{
		VER_1 = 11,      // 21x21 pixels
		VER_2 = 13,      // 25x25 pixels
		VER_3 = 15,      // 29x29 pixels
		VER_4 = 17,      // 33x33 pixels
		VER_10 = 29,     // 57x57 pixels
	};

	bool* data;
	QRVersion version;

	QRCodeView(bool* _data = nullptr, QRVersion _version = QRVersion::VER_1) : data(_data), version(_version) { }

	GETTYPENAME_STUB("QRCodeView");

	RENDER_STUB
#ifdef STUI_IMPLEMENTATION
	{
		const uint32_t chars[4] =
		{
			' ',
			UNICODE_QUADRANT_TOP,
			UNICODE_QUADRANT_LOWER,
			UNICODE_BLOCK
		};

		if (size.x < (version * 2) - 1 || size.y < version) return;

		int bytes_length = (version * 2) - 1;

		for (int y = 0; y < version; y++)
		{
			for (int x = 0; x < (version * 2) - 1; x++)
			{
				bool top       =                     data[x + (((y * 2) + 0) * bytes_length)];
				bool bottom    = (y < version - 1) ? data[x + (((y * 2) + 1) * bytes_length)] : false;

				uint8_t index = (bottom << 1) | (top << 0);
				output_buffer[x + (y * ((version * 2) - 1))] = chars[index];
			}
		}
	}
#endif
	;

	GETMAXSIZE_STUB{ return Coordinate{ (version * 2) - 1, version }; }
	GETMINSIZE_STUB{ return Coordinate{ (version * 2) - 1, version }; }
};


#pragma endregion STUI_COMPONENTS

///////////////////////////////////////////////////////////////////////
//                         IMPLEMENTATIONS
///////////////////////////////////////////////////////////////////////

#pragma region STUI_IMPLEMENTATIONS

#ifdef STUI_IMPLEMENTATION

bool Page::checkInput()
{
	current_page = this;

	Component* focused_component = nullptr;
	if (focused_component_index < focusable_component_sequence.size() && root != nullptr) focused_component = focusable_component_sequence[focused_component_index];

	vector<Input::Shortcut> shortcuts_tmp = shortcuts;
	shortcuts_tmp.push_back(Input::Shortcut
	{
		Input::Key{ '\t', Input::ControlKeys::NONE },
		advanceFocus
	});

	return Renderer::handleInput(focused_component, shortcuts_tmp);
}

void Page::render()
{
	current_page = this;

	if (root == nullptr) return;
	updateFocus();
	Renderer::render(root);
}

Renderer::FrameData Page::framerate(int fps_target)
{
	return Renderer::targetFramerate(fps_target, last_frame);
}

void Page::ensureIntegrity()
{
	if (root == nullptr)
	{
		destroyAllComponents({ });
		DEBUG_LOG("ensure integrity called, root was null so the registry was cleared");
	}

	map<Component*, int> discovered_nodes;
	queue<Component*> to_check;
	to_check.push(root);
	while (!to_check.empty())
	{
		Component* comp = to_check.front();
		to_check.pop();
		discovered_nodes.insert_or_assign(comp, 0);

		vector<Component*> children = comp->getAllChildren();
		for (Component* c : children)
			if (c != nullptr)
				to_check.push(c);
	}

	map<Component*, string> known_nodes;
	vector<Component*> new_nodes;
	size_t ignored_nodes = 0;
	for (auto p : components) known_nodes.insert(pair<Component*, string>(p.second, p.first));

	for (auto p : discovered_nodes)
	{
		Component* c = p.first;
		if (known_nodes.count(c) != 0) { known_nodes.erase(c); ignored_nodes++; }
		else new_nodes.push_back(c);
	}

	for (auto p : known_nodes) unregisterComponent(p.second);
	for (auto c : new_nodes) registerComponent(c, "");

#ifdef DEBUG
	DEBUG_LOG("ensure integrity called, registered " + to_string(new_nodes.size()) + " new nodes, ignored " + to_string(ignored_nodes) + " existing nodes, unregistered " + to_string(known_nodes.size()) + " no-longer-referenced nodes.");
	string dbg = "component registry now looks like this:";
	for (auto p : components)
		dbg += "\n\t" + p.first + " : " + to_string((size_t)p.second);
	DEBUG_LOG(dbg);
#endif
}

vector<string> Page::getAllComponents()
{
	vector<string> comps;
	for (auto pair : components)
		comps.push_back(pair.first);
	return comps;
}

string Page::registerComponent(Component* component, string identifier)
{
	string name = identifier;
	if (isNameUnique(name)) components.insert(pair<string, Component*>(name, component));
	else
	{
		name = getUniqueName(component->getTypeName());
		components.insert(pair<string, Component*>(name, component));
	}
	return name;
}

Component* Page::unregisterComponent(string identifier)
{
	if (components.count(identifier) != 0)
	{
		Component* c = components[identifier];
		components.erase(identifier);
		return c;
	}
	else throw runtime_error("no component registered with that name");
	return nullptr;
}

vector<Component*> Page::destroyAllComponents(vector<string> exclude_list)
{
	vector<Component*> remainders;
	vector<string> all_names;
	for (auto pair : components)
		all_names.push_back(pair.first);

	for (string name : all_names)
	{
		unregisterComponent(name);

		bool to_exclude = false;
		for (string s : exclude_list)
		{
			if (s == name)
			{
				to_exclude = true;
				break;
			}
		}
		
		if (to_exclude)
			remainders.push_back(components[name]);
		else
			delete components[name];
	}

	root = nullptr;
	focusable_component_sequence.clear();

	return remainders;
}

void Page::advanceFocus()
{
	if (current_page == nullptr) return;

	if (current_page->focusable_component_sequence.empty()) return;

	current_page->focused_component_index %= current_page->focusable_component_sequence.size();

	bool was_any_focusable = false;
	size_t old_focus = current_page->focused_component_index;
	do
	{
		current_page->focused_component_index++;
		current_page->focused_component_index %= current_page->focusable_component_sequence.size();
		if (current_page->focusable_component_sequence[current_page->focused_component_index]->isFocusable())
		{
			was_any_focusable = true;
			break;
		}
	}
	while (current_page->focused_component_index != old_focus);

	if (!was_any_focusable) current_page->focused_component_index = (size_t)-1;

	current_page->updateFocus();
}

string Page::getUniqueName(string type)
{
	size_t i = 0;

	while (components.count("__component_" + type + '_' + to_string(i)) != 0)
		i++;

	return "__component_" + type + '_' + to_string(i);
}

#endif

#pragma endregion STUI_IMPLEMENTATIONS

}

#define STUI_ONLY_UNDEFS
#include "stui.h"

#endif