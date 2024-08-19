# Help

## The Short Version

Simple Text User Interface gives you a lightweight toolkit to build interactive interfaces inside the terminal window.

it's header only, so project setup simply involves adding the [stui.h](stui.h) file to your project somehow and `#include`-ing it (though actually you should probably add the whole project as a git submodule or something). the option [stui_extensions](stui_extensions.h) file gives you a useful class for managing an entire interface page in one class.

the most basic use of the library simply invovles creating a `Component` and calling `Renderer::render` on it. interface layouts can then be built up from a combination of nested elements, and you just call `Renderer::render` on the root element. **however, you must call `Terminal::configure` first for the terminal window to behave correctly.**

this will replace the entire content of the terminal window with the rendered interface.

the next step is to wrap this render call into a `while (true)` loop. you probably want to limit the framerate so you aren't just burning CPU time, which you can do with a call to `Renderer::targetFramerate`.

the final step is receiving input from the user, which you can do using `Renderer::handleInput`. this function takes a `Component` which the input should be applied to, and a list of shortcut bindings.

if you want to use the library *properly* then you should read the rest of this document.

## Something's Not Working!

if you're having problems, the best way to resolve it is to follow these steps, ideally in this order:
1) re-read your code very slowly, including the compiler error if you're getting one
2) write `#define DEBUG` before your `#include <stui.h>` line, and use the `DEBUG_LOG` macro to log stuff to a file
3) read the rest of this document, and/or the relevant documentation comments in `stui.h`/`stui_extensions.h`/`stui_script.h`
4) check the open issues on the repository [here](https://github.com/oculometric/stui) to see if this is a known problem
5) check the [TODO.md](TODO.md) file to see if i have a todo item relating to your problem
6) if you're really sure you're experiencing a bug and not making a mistake, and it isn't already documented somewhere, i'd greatly appreciate you submitting an issue on the Github page for the repository (see above). this includes if you find something that isn't adequately documented

## Walkthrough

### Getting Set Up

the best way to set up your project to use STUI is to add the repository as a submodule using
```
git submodule add https://github.com/oculometric/stui
```
then depending on your project setup, add the `stui/inc` directory as an include path. for instance with `gcc`, just add `-I stui/inc` to your compile command, or with Visual Studio, add the `inc` directory in project settings.

### Basic Component Usage

setup is pretty simple, you start by creating the `Component`s you want, and then render them using the `Page` class:
```
#define STUI_IMPLEMENTATION
#include <stui.h>

// we'll be writing `stui::` a lot otherwise
using namespace stui;

int main()
{
    // configure the terminal window
    Terminal::configure();

    // construct a text widget with the text "Hello, World!", and using with left alignment
    Label text_widget("Hello, World!", -1);

    // display loop
    while (true)
    {
        // display the Component on the screen
        Renderer::render(&text_widget);
    }

    return 0;
}
```

when compiled, this should produce output looking something like this, depending on your terminal size:
```
Hello, World!    





```

now, `Component`s are intended to be as simple and stateless as possible, only using class properties to hold data between instantiation and the `Component` being drawn. this means that the only things a `Component` actually does are:
- report its minimum desired size
- report its maximum desired size
- draw itself into a buffer
- report whether it can be focused for input or not
- handle input sent to it
- report its type name as a string
- report its children, if it has them

not all `Component` subclasses will override all of these; for instance, non-input-receiving components like a `BorderedBox` can leave the default `isFocusable` as-is which returns false, same for the `handleInput` function.

however, your code should probably never call any of these functions directly.

### Improving the Mainloop

now, you may want to limit your framerate in the terminal to something reasonable. after all, we're only building a text UI and currently our code runs at over 3000 frames per second, which is unecessary. we can limit this using built-in behaviour:
```
// ...
    // keep track of the timestamp of the last frame
    auto frame_time = clock_type::now();
        
    // display loop
    while (true)
    {
        // limit the framerate, which also gives us access to useful delta-time statistics
        Renderer::FrameData frame_data = Renderer::targetFramerate(12, frame_time);

        // display the Component on the screen
        Renderer::render(&text_widget);
    }
// ...
```

now we'll wait for up to 1/12 of a second before rendering the next frame, hopefully maintaining a relatively stable framerate.

### Listening to the User

you probably want to read user input at some point in your program, and there's support for that too. STUI gives you control over which `Component` you send your input to, as well as the ability to specify global shortcuts.

let's add a component that's capable of receiving user input, a `TextInputBox`. to do this, we need to start building a component tree:
```
// ...
    // this will receive input from the user
    TextInputBox text_field("type something", nullptr, true);
    // we must tell the `TextInputBox` that it's currently focused
    text_field.focused = true;

    // this adds a border around its child `Component`
    BorderedBox text_field_border(&text_field, "input widget");
    // this will contain our other two widgets
    VerticalBox vertical({ &text_widget, &text_field_border });


    // keep track of the timestamp of the last frame
    auto frame_time = clock_type::now();

    while(true)
    {
        // limit the framerate, which also gives us access to useful delta-time statistics
        Renderer::FrameData frame_data = Renderer::targetFramerate(12, frame_time);

        // handle input from the user, and send it to the `TextInputBox`
        Renderer::handleInput(&text_field, { });

        // display the vertical box on the screen instead
        Renderer::render(&vertical);
    }
```

after this you should have a terminal which looks something like this:
```
Hello, World!                                       
┏━━input widget━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃> type something                                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ 




```

try typing in some text! you should see it appear in the box.

### Shortcuts and Callbacks

while the library is primarily immediate-mode, callbacks just make life easier. some widgets implement these, like the `TextInputBox` for instance. in the constructor we included a `nullptr` argument, which we can swap out for a pointer `void`-returning function which will be called when the user presses enter with the box focused:
```
void textBoxCallback()
{
    // do something interesting here
}

// ...
    TextInputBox text_field("type something", textBoxCallback, true);
// ...
```

if we want to make this useful we should probably declare some of our widgets outside `main`, so they can be accessed by callbacks. the easiest way to do that is shown below. lets use this to make the user have a little conversation. to make sure it's clear what we're doing, i'll include the whole program:
```
#define STUI_IMPLEMENTATION
#include <stui.h>

// we'll be writing `stui::` a lot otherwise
using namespace stui;

// this will be called when the user presses enter
void textBoxCallback();

// define our `Component`s
Label text_widget("you must enter the password before you can proceed!", -1);
TextInputBox text_field("type something", textBoxCallback, true);
BorderedBox text_field_border(&text_field, "input widget");
VerticalBox vertical({ &text_widget, &text_field_border });

void textBoxCallback()
{
    if (text_field.text == "secret")
        text_widget.text = "password correct! you may enter my secret lair";
    else
        text_widget.text = "password incorrect! you are forbidden from entering";
}

void ctrlSCallback()
{
    text_widget.text = "gah! you have killed me. i suppose you can now enter my secret lair";
    text_field.enabled = false;
}

int main()
{
    // configure the terminal window
    Terminal::configure();
    
    text_field.focused = true;

    auto frame_time = clock_type::now();
    while(true)
    {
        Renderer::FrameData frame_data = Renderer::targetFramerate(12, frame_time);

        Renderer::handleInput(&text_field,
        { 
            Input::Shortcut{ Input::Key{ 'S', Input::ControlKeys::CTRL }, ctrlSCallback }
        });

        Renderer::render(&vertical);
    }

    return 0;
}
```

you may also notice the change to the `handleInput` call, where i've added a `Shortcut` which links CTRL+S to a callback function. shortcuts always consume the input that meets their requirements (so if you hit CTRL+S, you won't see an S appear in the text box). as you'll see when you test this program, you can either enter the password correctly, enter some random gibberish, or hit CTRL+S to uhhh... bypass the guard i guess.

**important note regarding shortcuts** - since we're working inside the terminal, there are a few limitations we have to consider. while ALT shortcuts work for *most* keys, they are not present for all of them. CTRL shortcuts are only present for A-Z, space, and forward slash (for whatever reason), excluding CTRL+H, CTRL+I, and CTRL+J which generate control codes which overlap with other characters. SHIFT shortcuts are present for most keys, though not space, and some which involve SHIFT+number key may not work properly on all keyboards. CTRL, SHIFT, and ALT are all mutually exclusive, you cannot mix-and-match them. special keys like DEL and ESC are mostly handled correctly. terminal-based input has a lot of limitations, especially on Linux terminals without Windows' surprisingly helpful API. i can only apologise for this. if you discover an issue with this or the input system in general, feel free to submit a Github issue.

### Another Efficiency Improvement

as it stands, we don't actually need to redraw the screen 12 times per second. in fact, we really only need to redraw it if some input was detected. to do that we can just use the return value of `handleInput` to decide whether or not anything has actually happened. let's do that now:
```
// ...
    auto frame_time = clock_type::now();
    bool dirty = true;
    while (true)
    {
        // ...

        dirty |= Renderer::handleInput(&text_field,
        { 
            Input::Shortcut{ Input::Key{ 'S', Input::ControlKeys::CTRL }, ctrlSCallback }
        });

        if (dirty) Renderer::render(&vertical);
        dirty = false;
    }
// ...
```

you might have your own functionality you want to add here, such as always redrawing the screen at least once per second using a timer variable, with input still being checked 12 times per second. remember that you can call `render` any time you want, so if you change something inside one of your callbacks, feel free to stay in there for a while, updating the screen as necessary.

### Using the Extensions

while the process described above is fine for a simple UI, if you're building a more complex application, things may get complicated. for instance, with many focusable UI elements, you likely need to come up with a mechanism for navigating between them. this is further compounded if you want to have multiple separate 'tabs' or pages within your interface.

i encountered this frustration during development, so i built the [stui_extensions.h](stui_extensions.h) module to help with that. this module also adds a bunch of extra non-essential `Component`s too.

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

void ctrlSCallback()
{
    text_widget.text = "gah! you have killed me. i suppose you can now enter my secret lair";
    text_field.enabled = false;
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

conveniently, when we call `setRoot`, `ensureIntegrity` is also called, which registers any untracked `Component`s into the `Page`'s registry (which since we have just initialised it, is all of them).

when a `Component` is registered, it is given a unique name. however, you can actually name `Component`s yourself, allowing you to access them later by name. this is useful if you don't want to try and track a ton of variables floating around. instead you could do something like this:
```
Page page;
page.registerComponent(new Label("this is my label", -1), "my_label");

// later...
(Label*)(page["my_label"]).text = "i have changed the text!";
```

if you do things this way, just remember to clean up with `delete page.unregisterComponent("my_label")`, or something similar.

### Changing the Splash Screen

you may have noticed that STUI displays a splash screen when it opens in the terminal. this is done as an extension of the license, but you can specify your own text to go there too. the `Terminal::configure` function can take two arguments: a string to place in the splash screen above the copyright notice, and a duration for it to appear for in seconds. if you just want to remove the splash screen entirely, you can set this to 0.
```
Terminal::configure("My Performance Monitoring Tool", 1.5f);
```

```
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                                                           ┃
┃                                                                           ┃
┃                  My Performance Monitoring Tool                           ┃
┃                                                                           ┃
┃                              using                                        ┃
┃          Simple Text UI  Copyright (C) 2024  Jacob Costen                 ┃
┃           This program comes with ABSOLUTELY NO WARRANTY.                 ┃
┃       This is free software, and you are welcome to redistribute it       ┃
┃          under certain conditions; see the license for details.           ┃
┃                                                                           ┃
┃                                                                           ┃
┃                                                                           ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

### Interface Design Guidance

1) **use `BorderedBoxes` to make your interface legible** - outline major elements to separate them from one another visually. i tend to avoid doing this with small things like `Label`s and `Button`s, but for something like a text-box, you probably want to. these are also very useful for labelling what bit of your interface does what, as you can give each box a title.
2) **make use of `TabDisplay` to tell the user where they are** - if you have multiple tabs/pages that can be navigated between, use a `TabDisplay` at the top of the screen to show which screen the user is currently looking at.
3) **tell the user about shortcuts** - if you have hotkeys/shortcuts in your UI, put a label at the bottom of the screen summarising what shortcuts are available to the user. or document them some other way.
4) **make use of `SizeLimiter` when appropriate** - some UI elements don't need to be too big, like a text view you want the user to scroll through, or a `ListView` that you know the maximum required size of. you may want to limit the width or height, or both for such `Components`, which will maximise space for other elements.
5) **don't over-fill the screen with stuff** - not only is this horrible to look at, it also won't draw well (if at all) on small terminal windows. use multiple, simpler pages and separate your UI out into its constituent modules.

### Useful Tricks

there's nothing saying you can't reuse the exact same `Component` across multiple `Page`s, or even multiple times within the same `Page`! a `TabDisplay` at the top of the screen is a good example of this, you probably want it to be present in all of your UI views.

as demonstrated earlier, you can check for input more frequently than you render the UI. in fact, if your interface doesn't have any moving parts (or only changes on user input), you only need to redraw the screen then.

### Building Your Own Components

it's relatively easy to write your own STUI `Component` types, to draw whatever specific thing you want. the first step to this is changing how you include the header file. you must add the following line before `#include <stui.h>`:
```
#define STUI_KEEP_DEFINES
```

this will give you access to a collection of macros which make it much easier to write your own `Component`

the next step is to define a new class which inherits from `Component`, or another existing UI element type (*although i cannot vouch for what may happen if you do*):
```
class CustomComponent : public Component, public Utility
{

};
```

you may notice that i also inherited from the `Utility` class. this is a class which encapsulates some static functions which are useful for things like drawing text and copying buffers.

now we need to define what our `CustomComponent` actually does and looks like. the functions we must implement are `Component::render`, `Component::getTypeName`, `Component::getMaxSize`, and `Component::getMinSize`. with the latter two, we can tell the parent `Component` how big this component should be. `getMinSize` should return the smallest size the component expects to be drawn. most other `Components` like the `VerticalBox` will not bother drawing your `CustomComponent` if they cannot find enough space to meet the requirement set by `CustomComponent::getMinSize`. `getMaxSize` returns the maximum size the component wants to be drawn as, though it's possible in some circumstances for the component to be given a buffer bigger than this function asks for. if you want your `CustomComponent` to fill the available space, specify -1 as the size (works in either dimension, independently).

the `render` method is what actually draws our `CustomComponent` into existence, and this is your chance to go wild. **you must be careful to respect the bounds of the buffer into which you are drawing. you WILL corrupt the heap otherwise, and that means bad times for everyone.** you probably also want to include some validation for the size of the buffer, and whether it's a null-pointer or not, just in case some evil person has written a buggy `EvilCustomComponent` which is calling `render` on your `CustomComponent` incorrectly.

the `getTypeName` function should simply return the string representation of the class name. this is currently just used for auto-naming `Component`s in the `Page` component registry.

the only other thing we need to do is specify some properties for our UI widget, and write an appropriate constructor, and we have a working UI element.

now, as far as providing `render`, `getMaxSize` and `getMinSize` implementations goes, you definitely want to use the preprocessor macros i mentioned before. their use is fairly self-explanatory:
```
class CustomComponent : public Component, public Utility
{
public:
    int circle_size;

    CustomComponent(int _circle_size) : circle_size(_circle_size) { }

    RENDER_STUB
    {
        // do your rendering here. make sure to check the buffer is definitely as big as you expect it to be
    }

    GETMAXSIZE_STUB { return Coordinate{ -1,-1 }; }
    GETMINSIZE_STUB { return Coordinate{ 3,3 }; }

    GETTYPENAME_STUB("CustomComponent");
};
```

now, if you want to receive input from the user, you need to also override `Component::isFocusable` and `Component::handleInput`. the first of these is trivial, just return true (though you may want the option to put your component in read-only mode, in which case you could just have an `editable` flag and return that instead). the second is where you do your own input handling, and you should return true if you consumed the input or false if you did not.
```
// ...
    ISFOCUSABLE_STUB { return true; }

    HANDLEINPUT_STUB
    {
        if (!focused) return false;

        // do something with the user input here

        return true;
    }
// ...
```

finally, if your `CustomComponent` has child `Component`s that it draws, you should implement `Component::getAllChildren` and return a list of pointers to those children. this also has a corresponding macro

and you're done! if you want to see how to do more specific things, take a look through the [stui.h](stui.h) header file and see how it's done there. or submit an issue on Github asking for clarification/documentation.

one final note about the output buffer: each `Tixel` in the buffer represents a single character in the terminal. at the end of the rendering process. **don't try and print extended ASCII. it will not display properly.** you can however specify up to 4-byte Unicode characters in the `Tixel::character` field, and they should display correctly (assuming the terminal you're using has support for it. if it doesn't you should probably fix that or something). you can also specify a colour (yes, that spelling, spooky) for the character, which is applied via 8-colour ANSI codes. you can `|` (bitwise OR) two `Tixel::ColourCommand`s together to change both foreground and background, but you MUST only combine one foreground and one background command per `Tixel`. otherwise who knows what might happen.

## Component Reference

if you want to know how to use each `Component` class, look in [stui.h](stui.h). each UI element should be well-documented. if it isn't, feel free to open an issue on Github.

## LayoutScript Reference

LayoutScript is being developed in another branch. look there for the documentation for now.