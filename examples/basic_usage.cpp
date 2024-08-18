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
    Terminal::configure("My Performance Monitoring Tool", 1.5f);
    
    text_field.focused = true;

    auto frame_time = clock_type::now();
    bool dirty = true;
    while(true)
    {
        Renderer::FrameData frame_data = Renderer::targetFramerate(12, frame_time);

        dirty |= Renderer::handleInput(&text_field,
        { 
            Input::Shortcut{ Input::Key{ 'S', Input::ControlKeys::CTRL }, ctrlSCallback }
        });

        if (dirty) Renderer::render(&vertical);
        dirty = false;
    }

    return 0;
}