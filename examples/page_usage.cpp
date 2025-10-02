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
    text_field.setEnabled(false);

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