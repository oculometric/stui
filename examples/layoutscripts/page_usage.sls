VerticalBox
( children = {
    Label : "text_widget" (text = "you must enter the password before you can proceed!"),
    BorderedBox(child = TextInputBox : "text_field" (text = "type something")),
    Button : "dummy_button" (text = "this does nothing"),
    Button : "disabled_button" (text = "this isn't even focusable", enabled = 0)
})