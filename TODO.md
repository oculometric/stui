# To-Dos

## Misc
- [ ] add a proper license, including to the top of each header
- [ ] make some demo files (.layout, .cpp)
- [x] test under linux (and mac, preferably) - it didnt work `:(`
- [x] remove boxes around components and convert to its own element BorderedBox
- [ ] fix displaying in non-windows terminals (UTF8 display in Linux)
- [x] migrate to more complex buffer system
- [x] bordered box add name
- [x] make colours pixel-specific
- [ ] add greyed-out colour and use it for all unfocused selections

## LayoutScript
- [ ] implement quotation escape sequence for strip, extract, matching closing brace, and split comma list functions
- [x] implement all the various functions for the PageManager
- [ ] complete ability to read UI from file
- [ ] implement ensureIntegrity
- [x] change from .stui to .layout

## New Components
- [x] list view
- [x] bordered box
- [x] tab menu to switch between pages (accessed via number keys)
- [x] button
- [x] text entry box
- [ ] radio button
- [ ] check box
- [ ] 3D object renderer component

## Comments
- [ ] add comments inside functions across stui.h
- [x] add doxy comments to the rest of stui.h
- [ ] add doxy and detailed comments to stui_external.h
- [x] doxy comments in Input

## Input
- [x] simple input tracking library (see planetarium)
- [ ] input reading on linux
- [x] ability to add shortcut bindings to callbacks
- [x] input system for components
- [ ] widget focus system (shift-tabbing between focussable views, moving selection inside focussed view)
    - [ ] horizontalbox
    - [ ] verticalbox
    - [ ] sizelimiter
    - [ ] borderedbox
- [x] focus highlighting
- [x] hook up focus component passthrough in Page::render
    - [x] button
    - [ ] text area
    - [ ] text input box
    - [x] list view
    - [x] tree view
    - [x] tab display
- [ ] input for tree view (selection, expansion)
- [ ] fix input for text area
- [ ] fix input for text input box
- [x] action callbacks e.g. for buttons, text boxes