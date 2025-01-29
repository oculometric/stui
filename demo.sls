VerticalBox
(
    children = 
    {
        SizeLimiter 
        (
            child = Text : "text_widget" (text = "Hello, Again!", alignment = -1),
            size = [ 5, 1 ]
        ),
        ProgressBar : "progress_bar" (fraction = 0.3)
    }
)