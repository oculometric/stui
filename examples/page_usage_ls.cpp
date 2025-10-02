#define STUI_IMPLEMENTATION
#include <stui_script.h>

using namespace stui;

void textBoxCallback();

Label* text_widget;
TextInputBox* text_field;

void textBoxCallback()
{
    if (text_field->text == "secret")
        text_widget->text = "password correct! you may enter my secret lair";
    else
        text_widget->text = "password incorrect! you are forbidden from entering";
}

bool ctrlSCallback()
{
    text_widget->text = "gah! you have killed me. i suppose you can now enter my secret lair";
    text_field->enabled = false;

    return true;
}

int main()
{
    // configure the terminal window
    Terminal::configure("My Performance Monitoring Tool", 1.5f);
    
    LayoutReader reader;

    Page* page = reader.readPageFromFile("examples/layoutscripts/page_usage.sls");
    text_widget = page->get<Label>("text_widget");
    text_field = page->get<TextInputBox>("text_field");
    text_field->callback = textBoxCallback;
    page->focusable_component_sequence = { text_field, (*page)["dummy_button"], (*page)["disabled_button"] };

    page->shortcuts.push_back(Input::Shortcut
    {
        Input::Key{ 'S', Input::ControlKeys::CTRL },
        ctrlSCallback
    });
    
    bool dirty = true;
    while(true)
    {
        Renderer::FrameData frame_data = page->framerate(12);

        dirty |= page->checkInput();

        if (dirty) page->render();
        dirty = false;
    }

    return 0;
}