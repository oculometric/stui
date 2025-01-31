VerticalBox(children = {
    HorizontalBox(children = {
        SizeLimiter(max_size = [ 32, -1 ], child = VerticalBox(children = {
            BorderedBox(name = "input files", child = ListView : "selected_input_files" ()),
            BorderedBox(name = "output file", child = TextInputBox : "output_file" (text = "a.out"))
        })),
        VerticalBox(children = {
            BorderedBox(name = "compiler", child = VerticalBox(children = {
                RadioButton : "compiler_selection" (options = {"g++", "clang++"}),
                HorizontalDivider(),
                TextArea : "compiler_help" ()
            })),
            BorderedBox(name = "compiler options", child = TextInputBox : "options_input" (text = "-Wall"))
        }),
        SizeLimiter(max_size = [ 36, -1 ], child = BorderedBox(name = "include directories", child = ListView : "include_dirs" ()))
    }),
    HorizontalBox(children = {
        Spinner : "activity_indicator" (),
        HorizontalSpacer(width = 1),
        TextArea : "command_output" ()
    }),
    HorizontalDivider(),
    Label : "shortcut_label" ()
})