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

#define STUI_KEEP_DEFINES
#include <stui.h>
#undef STUI_KEEP_DEFINES

#include <map>

namespace stui
{

class Page;

static Page* current_page = nullptr;

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
    Page() { }

    bool checkInput()
#ifdef STUI_IMPLEMENTATION
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
#endif
    ;

    void render()
#ifdef STUI_IMPLEMENTATION
    {
        current_page = this;

        if (root == nullptr) return;
        updateFocus();
        Renderer::render(root);
    }
#endif
    ;

    Renderer::FrameData framerate(int fps_target)
#ifdef STUI_IMPLEMENTATION
    {
        return Renderer::targetFramerate(fps_target, last_frame);
    }
#endif
    ;

	void ensureIntegrity()
#ifdef STUI_IMPLEMENTATION
    {
        // TODO: search the tree, register unregistered, unregister and delete unreferenced
    }
#endif
    ;
	
    inline Component* operator[](string identifier) { return components[identifier]; }

    inline void setRoot(Component* component) { registerComponent(component, ""); root = component; }
    inline Component* getRoot() { return root; }

    void registerComponent(Component* component, string identifier)
#ifdef STUI_IMPLEMENTATION
	{
        if (isComponentRegistered(component)) return;
		if (isNameUnique(identifier)) components.insert(pair<string, Component*>(identifier, component));
		else components.insert(pair<string, Component*>(getUniqueName(), component));
	}
#endif
    ;

	void unregisterComponent(string identifier)
#ifdef STUI_IMPLEMENTATION
	{
		if (components.count(identifier) != 0) { delete components[identifier]; components.erase(identifier); }
		else throw runtime_error("no component registered with that name");
	}
#endif
    ;

    inline bool isComponentRegistered(Component* component)
    {
        for (pair<string, Component*> p : components)
            if (p.second == component) return true;
        
        return false;
    }

private:
    inline void updateFocus()
    {
        for (size_t i = 0; i < current_page->focusable_component_sequence.size(); i++)
            current_page->focusable_component_sequence[i]->focused = (i == current_page->focused_component_index);
    }

    static void advanceFocus()
#ifdef STUI_IMPLEMENTATION
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
#endif
    ;

    inline bool isNameUnique(string name)
	{
		if (name.length() < 1) return false;

		return components.count(name) == 0;
	}

	inline string getUniqueName()
	{
		size_t i = 0;

		while (components.count("__component_" + to_string(i)) != 0)
			i++;
		
		return "__component_" + to_string(i);
	}
};

}

#define STUI_ONLY_UNDEFS
#include <stui.h>
#undef STUI_ONLY_UNDEFS