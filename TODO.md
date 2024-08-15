# To-Dos

## Misc
- [ ] make some demo files (.layout, .cpp)
- [ ] potentially make scrollable views with extra space have a '...' to indicate more content
- [ ] add handling for delete (not backspace)
- [ ] update help (do this last probably)
- [ ] display message when screen too small for UI (4)
- [ ] fix horrendous linux performance (3)

## LayoutScript
- [ ] implement quotation escape sequence for strip, extract, matching closing brace, and split comma list functions
- [x] implement all the various functions for the PageManager
- [ ] complete ability to read UI from file
- [ ] implement ensureIntegrity
- [x] change from .stui to .layout
- [ ] have errors displayed with a line number and an offset along the line (only show the line number)

## New Components

## Comments
- [ ] add doxy and detailed comments to stui_external.h

## Input
- [ ] add editable text area
- [ ] horizontal scrolling for input box
- [ ] update windows input reading to match linux
- [ ] widget focus toolkit

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