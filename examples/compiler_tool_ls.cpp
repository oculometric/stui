#define STUI_IMPLEMENTATION
#include <stui_script.h>

using namespace stui;

Page* main_page;

ListView* selected_input_files;
TextInputBox* output_file;

RadioButton* compiler_selection;
TextArea* compiler_help;
TextInputBox* options_input;

ListView* include_dirs;

Spinner* activity_indicator;
TextArea* command_output;
Label* shortcut_label;

Page dialog_page;

TextInputBox add_file("", nullptr, true);
VerticalSpacer vs(3);
HorizontalSpacer hs(3);
HorizontalDivider hdv;
VerticalBox dialog_inner_1({ &vs, &add_file, &vs });
HorizontalBox dialog_inner_2({ &hs, &dialog_inner_1, &hs });
BorderedBox dialog_box(&dialog_inner_2, "");
VerticalSpacer bvs(-1);
Label dialog_shortcuts("confirm [enter]   cancel [esc]", -1);
VerticalBox dialog_root({ &dialog_box, &bvs, &hdv, &dialog_shortcuts });

bool continue_dialog_rendering = false;

void cancelDialog()
{
    continue_dialog_rendering = false;
}

void endAddInputFile()
{
    if (add_file.text != "")
    {
        continue_dialog_rendering = false;
        selected_input_files->elements.push_back(add_file.text);
    }
}

void addInputFileCallback()
{
    add_file.text = "";
    add_file.callback = endAddInputFile;
    dialog_box.name = "add input file";

    continue_dialog_rendering = true;

    while (continue_dialog_rendering)
    {
        dialog_page.framerate(24);
        dialog_page.checkInput();
        dialog_page.render();
    }
}

void removeInputFileCallback()
{
    if (selected_input_files->elements.size() == 0) return;
    for (size_t i = selected_input_files->selected_index; i < selected_input_files->elements.size() - 1; i--)
        selected_input_files->elements[i] = selected_input_files->elements[i + 1];
    selected_input_files->elements.pop_back();
}

void endAddIncludeDir()
{   
    if (add_file.text != "")
    {
        continue_dialog_rendering = false;
        include_dirs->elements.push_back(add_file.text);
    }
}

void addIncludeDirCallback()
{
    add_file.text = "";
    add_file.callback = endAddIncludeDir;
    dialog_box.name = "add include directory";

    continue_dialog_rendering = true;

    while (continue_dialog_rendering)
    {
        dialog_page.framerate(24);
        dialog_page.checkInput();
        dialog_page.render();
    }
}

void removeIncludeDirCallback()
{
    if (include_dirs->elements.size() == 0) return;
    for (size_t i = include_dirs->selected_index; i < include_dirs->elements.size() - 1; i--)
        include_dirs->elements[i] = include_dirs->elements[i + 1];
    include_dirs->elements.pop_back();
}

bool command_running = false;
string command_output_text = "";
void commandThread(string cmd)
{
    char buffer[128];
    command_output_text = "";
    FILE* pipe = popen(("2>&1 " + cmd).c_str(), "r");
    if (!pipe) command_output_text = "unable to open command pipe";
    try {
        while (fgets(buffer, 128, pipe) != nullptr)
        {
            command_output_text += buffer;
        }
    } catch (...)
    {
        command_output_text = "exception capturing command output";
    }
    pclose(pipe);
    command_running = false;
}

void compileCallback()
{
    string cmd = compiler_selection->options[compiler_selection->selected_index];

    cmd += " -o " + output_file->text;

    cmd += " " + options_input->text;

    for (string s : include_dirs->elements)
        cmd += " -I " + s;
    
    for (string s : selected_input_files->elements)
        cmd += " " + s;
    
    command_output->text = "running command '" + cmd + "'...";
    main_page->render();
    
    command_running = true;
    thread t(commandThread, cmd);

    float timer = 0.0f;
    while (command_running)
    {
        auto ft = main_page->framerate(12);
        timer += ft.delta_time;
        if (timer >= 0.25f)
        {
            activity_indicator->state++;
            timer -= 0.25f;
        }

        main_page->render();
    }

    t.join();

    command_output->text = "output: " + command_output_text + "\ndone.";
    main_page->render();
}

void updateShortcutsFromFocus()
{
    if (selected_input_files->focused)
    {
        shortcut_label->text = "next step [tab]   trigger compile [ctrl b]   navigate [up/down arrows]   add input file [a]   remove input file [r]";
        main_page->shortcuts = 
        {
            Input::Shortcut{ Input::Key{ 'a', Input::ControlKeys::NONE }, addInputFileCallback },
            Input::Shortcut{ Input::Key{ 'r', Input::ControlKeys::NONE }, removeInputFileCallback },
            Input::Shortcut{ Input::Key{ 'B', Input::ControlKeys::CTRL }, compileCallback }
        };
    }
    else if (output_file->focused)
    {
        shortcut_label->text = "next step [tab]   trigger compile [ctrl b]";
        main_page->shortcuts = { Input::Shortcut{ Input::Key{ 'B', Input::ControlKeys::CTRL }, compileCallback } };
    }
    else if (compiler_selection->focused)
    {
        shortcut_label->text = "next step [tab]   trigger compile [ctrl b]   navigate [up/down arrows]   select compiler [enter]";
        main_page->shortcuts = { Input::Shortcut{ Input::Key{ 'B', Input::ControlKeys::CTRL }, compileCallback } };
    }
    else if (compiler_help->focused)
    {
        shortcut_label->text = "next step [tab]   trigger compile [ctrl b]   scroll [up/down arrows]";
        main_page->shortcuts = { Input::Shortcut{ Input::Key{ 'B', Input::ControlKeys::CTRL }, compileCallback } };
    }
    else if (options_input->focused)
    {
        shortcut_label->text = "next step [tab]   trigger compile [ctrl b]";
        main_page->shortcuts = { Input::Shortcut{ Input::Key{ 'B', Input::ControlKeys::CTRL }, compileCallback } };
    }
    else if (include_dirs->focused)
    {
        shortcut_label->text = "next step [tab]   trigger compile [ctrl b]   navigate [up/down arrows]   add include directory [a]   remove include directory [r]";
        main_page->shortcuts = 
        {
            Input::Shortcut{ Input::Key{ 'a', Input::ControlKeys::NONE }, addIncludeDirCallback },
            Input::Shortcut{ Input::Key{ 'r', Input::ControlKeys::NONE }, removeIncludeDirCallback },
            Input::Shortcut{ Input::Key{ 'B', Input::ControlKeys::CTRL }, compileCallback }
        };
    }
    else if (command_output->focused)
    {
        shortcut_label->text = "next step [tab]   trigger compile [ctrl b]   scroll [up/down arrows]";
        main_page->shortcuts = { Input::Shortcut{ Input::Key{ 'B', Input::ControlKeys::CTRL }, compileCallback } };
    }
}

int last_selected_compiler = -1;

void updateCommandHelp()
{
    if (last_selected_compiler != compiler_selection->selected_index)
    {
        last_selected_compiler = compiler_selection->selected_index;

        char buffer[128];
        string result = "";
        FILE* pipe = popen(("2>&1 " + compiler_selection->options[last_selected_compiler] + " --help").c_str(), "r");
        if (!pipe) result = "unable to open command pipe";
        try {
            while (fgets(buffer, 128, pipe) != nullptr)
            {
                result += buffer;
            }
        } catch (...)
        {
            result = "exception capturing command output";
        }
        pclose(pipe);

        compiler_help->text = result;
    }
}

int main()
{
    Terminal::configure("compiler tool", 1.0f);

    LayoutReader reader;
    main_page = reader.readPageFromFile("examples/layoutscripts/compiler_tool.sls");
    selected_input_files = main_page->get<ListView>("selected_input_files");
    output_file = main_page->get<TextInputBox>("output_file");
    compiler_selection = main_page->get<RadioButton>("compiler_selection");
    compiler_help = main_page->get<TextArea>("compiler_help");
    options_input = main_page->get<TextInputBox>("options_input");
    include_dirs = main_page->get<ListView>("include_dirs");
    activity_indicator = main_page->get<Spinner>("activity_indicator");
    command_output = main_page->get<TextArea>("command_output");
    shortcut_label = main_page->get<Label>("shortcut_label");
    main_page->focusable_component_sequence = { selected_input_files, output_file, compiler_selection, compiler_help, options_input, include_dirs, command_output };

    dialog_page.focusable_component_sequence = { &add_file };
    dialog_page.shortcuts = { Input::Shortcut{ Input::Key{ '\e', Input::ControlKeys::NONE }, cancelDialog } };
    dialog_page.setRoot(&dialog_root);

    while (true)
    {
        main_page->framerate(32);
        
        main_page->checkInput();
        Terminal::isTerminalResized();

        updateShortcutsFromFocus();
        updateCommandHelp();
        main_page->render();
    }

    return 0;
}