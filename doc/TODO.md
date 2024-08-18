# To-Dos

## Misc
- [ ] figure out some update-loop system to integrate with page?                                                            (v0.4)

## LayoutScript
- [ ] implement quotation escape sequence for strip, extract, matching closing brace, and split comma list functions        (v0.4)
- [ ] complete ability to read UI from file                                                                                 (v0.4)
- [ ] have errors displayed with a line number and an offset along the line (only show the line number)                     (v0.4)
- [ ] migrate away from PageManager                                                                                         (v0.4)

## New Components
- [ ] add editable text area                                                                                                (v0.5)
- [ ] QR code renderer                                                                                                      (v0.3)

## Comments
- [ ] add doxy and detailed comments to stui_script.h                                                                       (v0.4)

## Input

# Completed v0.3
- [x] allow the user to configure callbacks for program exit                                                                (v0.3)
- [x] make it possible to un-configure the terminal                                                                         (v0.3)
- [x] potentially make scrollable views with extra space have a '...' to indicate more content                              (v0.3)
- [x] horizontal scrolling for input box                                                                                    (v0.3)
- [x] update windows input reading to match linux                                                                           (v0.3)

# Completed v0.2
- [x] fix framerate targeting                                                                                               (v0.2)
- [x] make the multi-line text area scrollable when focused                                                                 (v0.2)
- [x] add handling for delete (not backspace)                                                                               (v0.2)
- [x] implement ensureIntegrity                                                                                             (v0.2)
- [x] add doxy comments to stui_extensions.h                                                                                (v0.2)
- [x] organise project structure better                                                                                     (v0.2)
- [x] update help (do this last probably)                                                                                   (v0.2)
- [x] make some demo files (.layout, .cpp)                                                                                  (v0.2)

# Completed
- [x] list view
- [x] bordered box
- [x] tab menu to switch between pages (accessed via number keys)
- [x] button
- [x] text entry box
- [x] radio button
- [x] slider
- [x] check box
- [x] remove boxes around components and convert to its own element BorderedBox
- [x] fix displaying in non-windows terminals (UTF8 display in Linux) by widening the character buffer type
- [x] migrate to more complex buffer system
- [x] bordered box add name
- [x] make colours pixel-specific
- [x] add greyed-out colour and use it for all unfocused selections
- [x] test under linux (and mac, preferably) - it didnt work `:(`
- [x] intercept Ctrl C
- [x] make everything non-inline, switch to #define-d implementation
- [x] fix right-aligned label
- [x] ability to add shortcut bindings to callbacks
- [x] input system for components
- [x] widget focus system (shift-tabbing between focussable views, moving selection inside focussed view) ?
- [x] focus highlighting
- [x] hook up focus component passthrough in Page::render
    - [x] button
    - [x] text area
    - [x] text input box
    - [x] list view
    - [x] tree view
    - [x] tab display
- [x] simple input tracking library (see planetarium)
- [x] input reading on linux
- [x] fix the rest of input reading on linux
- [x] input for tree view (selection, expansion)
- [x] fix input for text input box
- [x] action callbacks e.g. for buttons, text boxes
- [x] add comments inside functions across stui.h
- [x] add doxy comments to the rest of stui.h
- [x] add a proper license, including to the top of each header
- [x] doxy comments in Input
- [x] make a function which checks for the terminal being resized
- [x] make handleInput return true for input detected
- [x] fix horrendous linux performance (3)
- [x] display message when screen too small for UI (4)
- [x] change from .stui to .layout
- [x] implement all the various functions for the PageManager
- [x] widget focus toolkit