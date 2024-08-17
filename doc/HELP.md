# Help

## The Short Version

Simple Text User Interface gives you a lightweight toolkit to build interactive interfaces inside the terminal window.

it's header only, so project setup simply involves adding the [stui.h](stui.h) file to your project somehow and `#include`-ing it (though actually you should probably add the whole project as a git submodule or something). the option [stui_extensions](stui_extensions.h) file gives you a useful class for managing an entire interface page in one class. there is also the optional [stui_script.h](stui_script.h) file which provides support for reading LayoutScript files into a functional UI tree.

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
4) check the open issues on the repository [here](https://github.com/oculometric/stui) to see if this is a known issue
5) if you're really sure you're experiencing a bug and not making a mistake, please submit an issue on the Github page for the repository (see above). this includes if you find something that isn't adequately documented

## Walkthrough

### Getting Set Up

the best way to set up your project to use STUI is to add the repository as a submodule using
```
git submodule add https://github.com/oculometric/stui
```
then depending on your project setup, add the `stui/inc` directory as an include path. for instance with `gcc`, just add `-I stui/inc` to your compile command, or with Visual Studio, add the `inc` directory in project settings.

### Basic Component Usage

// TODO: from here onwards...

setup is pretty simple, you simply start creating the `Component`s you want, and then render them using the `Page` class:
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

now, you may want to limit your framerate to something reasonable. after all, we're only building a text UI and currently our code runs at over 3000 frames per second, which is unecessary. we can limit this using built-in behaviour:

```
// ...
    // keep track of the timestamp of the last frame
    auto frame_time = chrono::high_resolution_clock::now();
        
    // display loop
    while (true)
    {
        // limit the framerate, which also gives us access to useful delta-time statistics
        stui::Page::FrameData frame_data = stui::Page::targetFramerate(12, frame_time);

        // actually display the Component on the screen
        stui::Page::render(&text_widget);
    }
// ...
```
now we'll wait for up to 1/12 of a second before rendering the next frame.

the next step is to build a more complex UI. some `Component`s will give us the ability to have child `Component`s, such as the `VerticalBox` `Component`s:
```
// ...
    stui::Text text_widget("Hello, World!", 0);
    stui::TextArea text_area("Well, isn't this grand! we can do loooooong lines (and they'll automatically multiline themselves) and even line breaks.\n\nthere, see?\nin fact, you can fill this box with text to your heart's content and it should all wrap nicely (until we run out of space of course)");

    // now use a vertical box to lay them out together
    stui::VerticalBox vertical_box({ &text_widget, &text_area });

    // ...
        // actually display the Component on the screen
        stui::Page::render(&vertical_box);
// ...
```
the next step is receiving input from the user, which isn't implemented yet. // TODO: input documentation

now you're ready to step out on your own. there's a selection of `Component`s to play with. remember that you can change the state of any of your `Component`s at any time, and the result will be visible the next time you call `render`.

## Component Reference

if you want to know how to use each `Component` class, look in [stui.h](stui.h). each UI element should be well-documented. if it isn't, feel free to open an issue.

## LayoutScript Reference

LayoutScript is a simple, structured language for defining the layout of a user interface page, based vaguely on C syntax. it consists of a tree of `Component` definitions. it is loaded by the `PageManager` class which also manages all the resources of the Page for you.

### LayoutScript Syntax Summary

`Component`s are defined as follows:
```
Text : "text_widget" ("Hello, World!, -1)
```
easy right? here's a breakdown:

`Text` - class name of the `Component` type being defined

`"text_widget"` - string identifier for the `Component`s

`("Hello, World!, -1)` - initialiser parameter list, consisting of a string and an integer

in this case, we're creating a Text `Component`, with the identifier _text_widget_, and initialising it with the text "Hello, World!", and alignment of -1 (i.e. left-aligned).

some `Component`s take other `Component`s as initialisation arguments, in which case we can do something like:
```
SizeLimiter 
(
    Text : "text_widget" ("Hello, Again!, -1),
    [ 5, 1 ]
)
```
as you can see, we can nest component definitions, and the inner `Component`s will be handled recursively.

another thing you may notice here is that the `SizeLimiter` does not have an identifier. this is perfectly legal, as the loader will generate a unique name for it at load time, but it will make it hard to get access to from the C++ side if we don't know its name.

we've also used another argument type here: a `Coordinate`, as specified by the `[ x, y ]` syntax.

some `Component`s require arrays for initialisation. we can do that too!
```
VerticalBox
({
    SizeLimiter 
    (
        Text : "text_widget" ("Hello, Again!, -1),
        [ 5, 1 ]
    ),
    ProgressBar : "progress_bar" (0.3)
})
```
as you can see, arrays are denoted using `{...}`. arrays must always contain a single type of element, but can be formed of any of the other fundamental types. **you cannot form arrays of arrays.**

in this example we've also discovered another fundamental type: floating point numbers. unlike C++, these should not have an 'f' at the end. as long as it is formatted as `a.bcd` (or however many decimals you require) it will be interpreted as a float instead of an integer.

**note that all whitespacing is optional in LayoutScript.** you're free to format your code in whatever abomination of a scheme you prefer (or no scheme at all). all whitespace outside strings is stripped during parsing.

### Component Identifiers

indentifiers make it easy to reference elements defined in your layout file from within your C++ code. for instance, you might want to access our `"text_widget"` `Component` to change its text later.

for some `Component`s these are very important, as something like an `ImageView` can't be properly initialised in the layout file.

however, identifiers are optional, and for `Component`s that you don't care about changing later (e.g. if you have a `HorizontalBox` which will always hold a text box and an image and will never have new `Component`s added or removed, then it doesn't necessarily need an identifier). the loader will automatically assign unique identifiers to every `Component` that doesn't have one specified, in the format `"__component_n"` where `n` is the next number after the previous identifier.

the loader will also automatically resolve naming collisions: if you have two `Component`s with the same identifier, one will have its identifier replaced with a unique identifier in the format above.

### Fundamental Types

| type        | example     | notes                                                          |
|------       |---------    |-------                                                         |
|string       |`"hello"`    |                                                                |
|int          |`-4`         |signed                                                          |
|float        |`-0.1`       |does not need an 'f' signifier at the end                       |
|Component    |`Text(...)`  |see above for more detailed syntax                              |
|Coordinate   |`[ 3,-1 ]`   |both components must be present and must be ints                |
|array        |`{...}`      |must consist of a single type of item, which cannot be another array |

### Comments

the loader supports comments, both whole-line comments of this format
```
SizeLimiter 
( // everything after the double-slash will be removed, up until a newline
    Text : "text_widget" ("Hello, World!, -1),
    [ 5, 1 ]
    // same for this
)
```
and mid-line comments of this format
```
Text : /* things between these symbols will be removed */ "text_widget" ("Hello, World!" /* even this */, -1)
```

### Error Reporting

the loader reports fairly descriptive errors if it encounters invalid syntax while parsing. however, more detailed explanations of errors and how to remedy them are below.

// TODO: error reporting documentation

### Integrating with C++

the `PageManager` keeps track of your entire UI tree for you, how great is that! the `PageManager` handles loading LayoutScript files for you. actually nevermind, i haven't actually written this bit of code yet so i won't document it yet either // TODO: ls/c++ integration documentation

## PageManager