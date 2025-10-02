## Using the Extensions

while the process described in the previous example is fine for a simple UI, if you're building a more complex application, things may get complicated. for instance, with many focusable UI elements, you likely need to come up with a mechanism for navigating between them. this is further compounded if you want to have multiple separate 'tabs' or pages within your interface.

i encountered this frustration during development, so i built the [stui_extensions.h](inc/stui_extensions.h) module to help with that. this module also adds a bunch of extra non-essential `Component`s too.

from this file we get the `Page` class, which allows us to easily manage all of the `Components`, `Shortcuts`, and focus state for a whole page of UI.

the `Page` class does a few things:
1) keeps track of all your `Component`s. any `Component` in the UI tree should be present in the `Page`'s registry with a unique name, which you can specify yourself if you want to retreive a `Component` later by name.
2) keeps track of your `Shortcuts` and automatically applies them when you check for input
3) manages `Component` focus. `Page` will add its own `Shortcut` bound to TAB which when triggered will advance to the next focusable `Component`. focusable `Components` are described in the `focusable_component_sequence` member, and the order of these is respected as long as the next `Component` in the list is currently focusable (if not, it will be skipped)
4) manage tracking the frame time for you.

to use the `Page` class, create your `Component`s as normal, then tell the `Page` to make your root `Component` root. then, use the wrapper functions in `Page` to create your mainloop as before:
```
#define STUI_IMPLEMENTATION
#include <stui.h>
#include <stui_extensions.h>

using namespace stui;

void textBoxCallback();

Page page;

Label text_widget("you must enter the password before you can proceed!", -1);
TextInputBox text_field("type something", textBoxCallback, true);
Button dummy_button("this does nothing", nullptr, true);
Button disabled_button("this isn't even focusable", nullptr, false);
BorderedBox text_field_border(&text_field, "input widget");
VerticalBox vertical({ &text_widget, &text_field_border, &dummy_button, &disabled_button });

void textBoxCallback()
{
    if (text_field.text == "secret")
        text_widget.text = "password correct! you may enter my secret lair";
    else
        text_widget.text = "password incorrect! you are forbidden from entering";
}

bool ctrlSCallback()
{
    text_widget.text = "gah! you have killed me. i suppose you can now enter my secret lair";
    text_field.enabled = false;

    return true;
}

int main()
{
    // configure the terminal window
    Terminal::configure("My Performance Monitoring Tool", 1.5f);
    
    page.setRoot(&vertical);
    page.focusable_component_sequence.push_back(&text_field);
    page.focusable_component_sequence.push_back(&dummy_button);
    page.focusable_component_sequence.push_back(&disabled_button);
    page.shortcuts.push_back(Input::Shortcut
    {
        Input::Key{ 'S', Input::ControlKeys::CTRL },
        ctrlSCallback
    });
    
    bool dirty = true;
    while(true)
    {
        Renderer::FrameData frame_data = page.framerate(12);

        dirty |= page.checkInput();

        if (dirty) page.render();
        dirty = false;
    }

    return 0;
}
```

### Registry Integrity

conveniently, when we call `setRoot`, `ensureIntegrity` is also called, which registers any untracked `Component`s into the `Page`'s registry (which since we have just initialised it, is all of them).

when a `Component` is registered, it is given a unique name. however, you can actually name `Component`s yourself, allowing you to access them later by name. this is useful if you don't want to try and track a ton of variables floating around. instead you could do something like this:
```
Page page;
page.registerComponent(new Label("this is my label", -1), "my_label");

// later...
(Label*)(page["my_label"])->text = "i have changed the text!";

// or slightly cleaner:
page->get<Label>("my_label")->text = "i have changed the text, with less janky casting!";
```

### Cleaning Up

in the ain example above, we've used stack-allocated `Component`s and `Page`s, which means there's nothing to clean up. however, what about if we've made a `new Page()` instead?

in this case, we must `delete` the `Page` when done of course, **however, this does not `delete` the `Component`s it contains**. this is because those `Component`s may be still in use elsewhere, or they may not have been allocated with `new`. `Page` doesn't know, so it doesn't make assumptions.

however, when you do need to clean up `Component`s, you **must** clear the `Page` it came from! you can use the `destroyAllComponents` function of `Page` to do this. this function will `delete` all everything in the `Page`, and clear the registry. if there are `Component`s you need to prevent being `delete`d (for the reasons mentioned above), you can pass in a list of their names and get out a list of pointers to them. in this case, the registry will still be cleared, but these specific `Component`s will not be `delete`d.

keeping the `Page` tree structure valid is difficult. if you want to `delete` a single `Component` but keep the rest of the tree, you should do something along these lines:

```
// this is the label we are going to destroy
Label* lab = page->get<Label>("useless_label");

// this approach relies on you knowing what the label's parent is in advance, and naming it
VerticalBox* thing_which_uses_lab = page->get<VerticalBox>("vertical_box");
thing_which_uses_lab->children.pop_back(); // assume we know that `lab` is the last item

// remove it from the registry, could also be done by calling ensureIntegrity
page->unregisterComponent("useless_label");

delete lab;
```

there are some obvious caveats with this; we need to know about the `Label` and its parent in advance, we need to be certain nothing else will be referencing the `Label` elsewhere, etc. generally i would recommend just not doing things this way where possible.