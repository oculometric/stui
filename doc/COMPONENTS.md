# Component Reference

| components                                |                                           |
|-------------------------------------------|-------------------------------------------|
| [Label](#label)                           | [Bordered Box](#borderedbox)              |
| [Button](#button)                         | [Vertical Box](#verticalbox)              |
| [Radio Button](#radiobutton)              | [Horizontal Box](#horizontalbox)          |
| [Toggle Button](#togglebutton)            | [Vertical Spacer](#verticalspacer)        |
| [Text Input Box](#textinputbox)           | [Horizontal Spacer](#horizontalspacer)    |
| [Text Area](#textarea)                    | [Vertical Divider](#verticaldivider)      |
| [Progress Bar](#progressbar)              | [Horizontal Divider](#horizontaldivider)  |
| [Slider](#slider)                         | [Size Limiter](#sizelimiter)              |
| [Spinner](#spinner)                       | [Tab Display](#tabdisplay)                |
| [List View](#listview)                    | [Banner](#banner)                         |
| [Tree View](#treeview)                    |
| [Image View](#imageview)                  |
| [QR Code View](#qrcodeview)               |

Each `Component` type is given a summary, a table describing its parameters, and notes on input behaviour, parameter values, and sizing behaviour. The title and parameter names are the mirrored in LayoutScript.


## Label
A single-line text box.

| type     | identifier       | default |
|----------|------------------|---------|
| string   | text             | ""      |
| int      | alignment        | -1      |

`alignment` parameter may be -1 (left-aligned), 0 (center-aligned), or 1 (right aligned). Expands horizontally but will limit itself to single-line vertically.


## Button
A simple clickable button.

| type     | identifier       | default  |
|----------|------------------|----------|
| string   | text             | "Button" |
| void* () | callback         | nullptr  |
| bool     | enabled          | true     |

Only receives input if `enabled` is set to true. On enter or spacebar, `callback` is called (unless `callback` is `nullptr`). Limited to one line vertically; expands horizontally and is always centered. `callback` may not be initialised in LayoutScript.


## RadioButton
A list of options which can be navigated, and only one may be selected at a time.

| type          | identifier        | default   |
|---------------|-------------------|-----------|
| string array  | options           | { }       |
| int           | selected_index    | 0         |
| bool          | enabled           | true      |

Only receives input if `enabled` is set to true. Arrow keys are used to navigate between items, and enter/spacebar marks the current item as selected. Always expects to have space for all of its items vertically.


## ToggleButton
A list of options which can be navigated, and any number of items can be selected at a time.

| type                      | identifier        | default   |
|---------------------------|-------------------|-----------|
| string-bool pair array    | options           | { }       |
| bool                      | enabled           | true      |

Only receives input if `enabled` is set to true. Arrow keys are used to navigate between items, and enter/spacebar toggles the state of the current item. Always expects to have space for all of its items vertically. `options` parameter is initialised as a string array when using LayoutScript.


## TextInputBox
A simple text entry box.

| type          | identifier    | default   |
|---------------|---------------|-----------|
| string        | text          | ""        |
| void* ()      | callback      | nullptr   |
| bool          | enabled       | true      |

Only receives input if `enabled` is set to true. Arrow keys can be used to navigate along the entered text. Enter triggers `callback`, but only if it is not `nullptr`. Supports horizontal scrolling for long text. Limited to one line vertically; expands horizontally. `callback` may not be initialised in LayoutScript.


## TextArea
A muilti-line text box.

| type          | identifier    | default   |
|---------------|---------------|-----------|
| string        | text          | ""        |
| int           | scroll        | 0         |

Wraps text horizontally (though is not aware of spaces), and supports vertical scrolling via arrow keys. Does not support editing.


## ProgressBar
A linear progress bar.

| type          | identifier    | default   |
|---------------|---------------|-----------|
| float         | fraction      | 0.5       |

Progress is expressed as a fraction between 0 and 1.


## Slider
A movable slider widget.

| type          | identifier    | default   |
|---------------|---------------|-----------|
| float         | value         | 0.5       |
| float         | step_size     | 0.1       |
| bool          | enabled       | true      |

Only receives input if `enabled` is set to true. Arrow keys can be used to increase/decrease the value; holding shift changes the value by 5 times as much. The step size controls how much the value changes by upon each input. Limits itself to a single line tall.


## Spinner
An activity spinner.

| type          | identifier    | default   |
|---------------|---------------|-----------|
| size_t        | state         | 0         |
| int           | type          | 0         |

`state` is used as an index into a sequence of characters (the character sequence to use is selected using `type`) to show activity. It should be incremented every frame that you are performing activity. `type` may be 0-3 (inclusive). Limits itself to be 1x1 characters.


## VerticalBox
A layout box which arranges a list of child widgets vertically.

| type              | identifier    | default   |
|-------------------|---------------|-----------|
| Component array   | children      | { }       |

When arranging children, the box will attempt to meet each child's minimum height first, then expands each one incrementally (up to its maximum height) to fill the available vertical space evenly. Horizontal size will match the maximum/minimum of contained children.


## HorizontalBox
A layout box which arranges a list of child widgets horizontally.

| type              | identifier    | default   |
|-------------------|---------------|-----------|
| Component array   | children      | { }       |

When arranging children, the box will attempt to meet each child's minimum width first, then expands each one incrementally (up to its maximum width) to fill the available horizontal space evenly. Vertical size will match the maximum/minimum of contained children.


## VerticalSpacer
An empty spacing element which fills a specified amount of space vertically.

| type              | identifier    | default   |
|-------------------|---------------|-----------|
| int               | height        | 1         |

Always fills 1 character horizontally.


## HorizontalSpacer
An empty spacing element which fills a specified amount of space horizontally.

| type              | identifier    | default   |
|-------------------|---------------|-----------|
| int               | width         | 1         |

Always fills 1 character vertically.


## BorderedBox
A titled container component which draws a box around its child, using IBM box characters.

| type              | identifier    | default   |
|-------------------|---------------|-----------|
| Component         | child         | nullptr   |
| string            | name          | ""        |

`name` will be drawn inline with the top border of the box. The box will always be 2 characters larger in both dimensions to accomodate the border.


## ListView
A scrollable view showing a list of text items.

| type              | identifier        | default   |
|-------------------|-------------------|-----------|
| string array      | elements          | { }       |
| int               | scroll            | 0         |
| int               | selected_index    | 0         |
| int               | show_numbers      | 1         |
| void* ()          | callback          | nullptr   |

The arrow keys can be used to navigate between items; `selected_index` represents which item is currently highlighted. When enter/spacebar is pressed, `callback` will be fired, unless `callback` is `nullptr`. `callback` cannot be initialised in LayoutScript. `show_numbers` controls whether the index of each item is shown beside it; accepted values are 0 (no numbers), 1 (zero-based indices), 2 (one-based indices), etc.


## TreeView
A scrollable, expandable view showing a tree of text items.

| type              | identifier        | default   |
|-------------------|-------------------|-----------|
| TreeView::Node*   | root              | nullptr   |
| size_t            | scroll            | 0         |
| size_t            | selected_index    | 0         |

The arrow keys can be used to navigate between and expand/collapse items. `root` may not be initialised in LayoutScript.


## ImageView
A janky element capable of kinda displaying low resolution greyscale image buffers.

| type              | identifier        | default   |
|-------------------|-------------------|-----------|
| uint8_t*          | grayscale_image   | nullptr   |
| Coordinate        | image_size        | [ 0, 0 ]  |

Neither of the parameters may be initialised in LayoutScript. Thh image will be displayed at twice the width it is stored at to compensate for the aspect ratio of terminal characters. Each pixel should be described by a single byte. The size of the image buffer must be allocated to match the image size parameter.


## SizeLimiter
A container element which limits the maximum size of its child component.

| type              | identifier        | default       |
|-------------------|-------------------|---------------|
| Component         | child             | nullptr       |
| Coordinate        | max_size          | [ -1, -1 ]    |

`max_size` should be at least as large as the minimum size of the child. The value -1 may be used to indicate unlimited size in a given dimension.


## TabDisplay
A widget for displaying a list of tabs or other switchable items arranged horizontally.

| type              | identifier        | default       |
|-------------------|-------------------|---------------|
| string array      | tab_descriptions  | { }           |
| size_t            | current_tab       | 0             |

Limits its size to a single line vertically.


## Banner
A block of text displayed in the center of its area.

| type              | identifier        | default       |
|-------------------|-------------------|---------------|
| string            | text              | ""            |

`text` does not obey wrapping, but line breaks (`\n`) are respected.


## VerticalDivider
A simple vertical line element, useful for splitting up items in a HorizontalBox.

Has no parameters. Expands to fill space vertically; limits itself to one character horizontally.


## HorizontalDivider
A simple horizontal line element, useful for splitting up items in a VerticalBox.

Has no parameters. Expands to fill space horizontal; limits itself to one line vertically.


## QRCodeView
An image-like view which renders a pre-generated QR code into the terminal.

| type          | identifier            | default       |
|---------------|-----------------------|---------------|
| bool array    | data                  | nullptr       |
| QRVersion     | version               | 11            |

Unfortunately you have to generate your own QR codes still. Neither parameter may be initialised in LayoutScript.