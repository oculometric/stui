## Basic Component Usage

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

    // construct a text widget with the text "Hello, World!", and using left alignment
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
