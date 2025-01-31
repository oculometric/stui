VerticalBox ( children = {
    HorizontalBox (children = 
    {
        Label : "t1" (text="text widget"),
        HorizontalSpacer (width = 2),
        Spinner : "s1" (type = 0),
        HorizontalSpacer (width = 2),
        Spinner : "s2" (type = 1),
        HorizontalSpacer (width = 2),
        Spinner : "s3" (type = 2),
        HorizontalSpacer (width = 2),
        Spinner : "s4" (type = 3),
        Label (text = "right-aligned text", alignment = 1)
    }),
    HorizontalDivider (),
    ProgressBar : "pb" (progress = 0.2),
    BorderedBox (child = TextInputBox : "tib" (text = "type in me"), name = "demo text input"),
    VerticalSpacer(height= 2),
    HorizontalBox(children = 
    {
        SizeLimiter
        (
            child = VerticalBox
            (
                children = 
                {
                    BorderedBox ( child = ListView : "list" (elements = { "heres the first of a list" }), name = "program args" ),
                    BorderedBox ( child = TreeView : "tree" (), name = "tree demo" )
                }
            )
            max_size = [ 30, -1 ]
        ),
        VerticalBox
        (
            children = 
            {
                ImageView : "image_view" (),
                HorizontalBox
                (
                    children =
                    {
                        BorderedBox(child = TextArea : "ta" (), name = "demo text"),
                        RadioButton : "rb" (options = {"option 1", "option 2", "option 3", "option 4"})
                    }
                )
            }
        )
    })
})