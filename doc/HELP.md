# Help

## The Short Version

Simple Text User Interface gives you a lightweight toolkit to build interactive interfaces inside the terminal window.

it's header only, so project setup simply involves adding the [stui.h](../inc/stui.h) file to your project somehow and `#include`-ing it (though actually you should probably add the whole project as a git submodule or something). the optional [stui_extensions.h](../inc/stui_extensions.h) file gives you a useful class for managing an entire interface page in one object, and the (also optional) [stui_script.h](../inc/stui_script.h) file introduces a system for deserializing complex UI trees from separately written LayoutScript files.

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

> a known issue currently is that if you `#include` the base `stui.h` file with `STUI_IMPLEMENTATION` defined in one file, then in another, `#include` one of the other modules also with `STUI_IMPLEMENTATION` defined, you will get duplicate symbol linker errors. the best way to fix this without restructuring the project massively is to just ensure that you just `#include` deepest module you intend to use (for reference, `stui_extensions.h` includes `stui.h`, and `stui_script.h` includes `stui_extensions.h`) with `STUI_IMPLEMENTATION` only once. you could do this by creating a dedicated `.cpp` file and just having
> ```
> #define STUI_IMPLEMENTATION
> #include <stui_script.h>
> ```
> and then never defining `STUI_IMPLEMENTATION` anywhere else, to ensure you have the implementations only once (avoiding linker errors).

## Walkthrough

### Getting Set Up

the best way to set up your project to use STUI is to add the repository as a submodule using
```
git submodule add https://github.com/oculometric/stui
```
then depending on your project setup, add the `stui/inc` directory as an include path. for instance with `gcc`, just add `-I stui/inc` to your compile command, or with Visual Studio, add the `inc` directory in project settings.

you can always just skip this and use a `#include "inc/stui.h"` instead of `#include <stui.h>` if you don't wanna do that.

### Examples

- see the [HELP_EX1.md](HELP_EX1.md) file for a great explanation of getting started using STUI.
- see the [HELP_EX2.md](HELP_EX2.md) file for information on using the extensions.
- see the [HELP_EX3.md](HELP_EX3.md) file for an example of using the script module.
- see the [HELP_EX4.md](HELP_EX4.md) file for an example of writing your own custom `Component`s.

### Navigating the STUI Header Files

if you want to know how to use each `Component` class in detail, you probably want to look in [stui.h](../inc/stui.h). each UI element should be well-documented. if it isn't, feel free to open an issue on Github.

likewise for detailed `Page` documentation, look in [stui_extensions.h](../inc/stui_extensions.h).

for information about using LayoutScript and the `LayoutReader` and `ComponentBuilder`s, read through [stui_script.h](../inc/stui_script.h), or [LAYOUTSCRIPT.md](LAYOUTSCRIPT.md).

all the source files are (reasonably) tidy, and divided into organised sections. these sections are defined with `#pragma region`s so you should be able to neatly fold the bits you don't care about.

the files usually start with with `#define`s, then a block of `#include`s, then one or more main content sections, followed by implementations of the functions declared in the main content bit, then lastly some `#undef`s.

most things (functions, classes, structs, enums) are verbosely documented with comments. honestly, ideally one should be able to figure out how to use STUI just from reading the comments.

the heavy use of macros may be a little confusing at first, so i recommend using an editor which can show you what they expand to.

implementations are separated from their definitions, for the most part. i appreciate this may also make following hard, since everything is in one file, but i found it made understanding an overview of classes like `Renderer` much easier, and was also less of an eyesore than everything looking like this:
```
void foo()
#ifdef STUI_IMPLEMENTATION
{
    // some code
}
#endif
;   // necessary for when STUI_IMPLEMENTATION is not defined
```
the exception to this is the `Component` class methods, which i kept within their class definitions (aka in eyesore mode) for maintenance purposes.

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

### Capturing Exit on CTRL+C

you may want a chance to do your own cleanup when the user hits `CTRL+C`. you may want to clean up a `Page`, show some kind of exit message or save message, etc.

since the `Terminal` needs to restore the terminal emulator state, it needs to capture the control event and do some of its own shutdown (specifically, calling `unConfigure`).

if you also want to receive that event, instead of doing platform-specific setup, you simply call `Terminal::registerExitCallback` with a pointer to your exit function. your function will be called immediately before the terminal is `unConfigure`d (so you have time to show some message using STUI) and the program exits.

### Taking Advantage of LayoutScript

LayoutScript is a neat way to represent UIs in a readable way, without creating a million `VerticalBox` and `HorizontalDivider` variables in your code. you can replace that with an external file with a tree of nested UI elements which can be named and have their constructor parameters set.

supports parameter types of `int`, `float`, `string`, `Coordinate`, and `Component` types (as well as array versions of all of these). also supports single-line comments

to avoid me repeating myself, read the [LAYOUTSCRIPT.md](LAYOUTSCRIPT.md) document for full documentation on the scripting language, and [HELP_EX3.md](HELP_EX3.md) for an example of actually using it.

### Interface Design Guidance

1) **use `BorderedBoxes` to make your interface legible** - outline major elements to separate them from one another visually. i tend to avoid doing this with small things like `Label`s and `Button`s, but for something like a text-box, you probably want to. these are also very useful for labelling what bit of your interface does what, as you can give each box a title.
2) **make use of `TabDisplay` to tell the user where they are** - if you have multiple tabs/pages that can be navigated between, use a `TabDisplay` at the top of the screen to show which screen the user is currently looking at.
3) **tell the user about shortcuts** - if you have hotkeys/shortcuts in your UI, put a label at the bottom of the screen summarising what shortcuts are available to the user. or document them some other way.
4) **make use of `SizeLimiter` when appropriate** - some UI elements don't need to be too big, like a text view you want the user to scroll through, or a `ListView` that you know the maximum required size of. you may want to limit the width or height, or both for such `Components`, which will maximise space for other elements.
5) **don't over-fill the screen with stuff** - not only is this horrible to look at, it also won't draw well (if at all) on small terminal windows. use multiple, simpler pages and separate your UI out into its constituent modules.
6) **if your interface is unweidly in code, consider using LayoutScript** - LayoutScript is easy to set up, and allows you to define complex UI trees in an easily readable format. you can reference named `Component`s in your code to maintain the same interactive behaviour as with a pure-C++ approach.

### Useful Tricks

there's nothing saying you can't reuse the exact same `Component` across multiple `Page`s, or even multiple times within the same `Page`! a `TabDisplay` at the top of the screen is a good example of this, you probably want it to be present in all of your UI views.

as demonstrated earlier, you can check for input more frequently than you render the UI. in fact, if your interface doesn't have any moving parts (or only changes on user input), you only need to redraw the screen then.