# To-Dos

## Misc
- [ ] add a proper license, including to the top of each header
- [ ] make some demo files (.layout, .cpp)
- [x] test under linux (and mac, preferably) - it didnt work `:(`
- [x] remove boxes around components and convert to its own element BorderedBox
- [ ] fix displaying in non-windows terminals (UTF8 display in Linux)

## LayoutScript
- [ ] implement quotation escape sequence for strip, extract, matching closing brace, and split comma list functions
- [ ] implement all the various functions for the PageManager
- [ ] complete ability to read UI from file
- [x] change from .stui to .layout

## New Components
- [x] list view
- [x] bordered box
- [ ] 3D object renderer component
- [ ] tab menu to switch between pages (accessed via number keys)
- [ ] button
- [ ] text entry box
- [ ] radio button
- [ ] check box

## Comments
- [ ] add an extended comment explaining how to write your own components
- [ ] add comments inside functions across stui.h
- [x] add doxy comments to the rest of stui.h
- [ ] add doxy and detailed comments to stui_external.h
- [ ] doxy comments in Input

## Input
- [x] simple input tracking library (see planetarium)
- [ ] input reading on linux
- [x] ability to add shortcut bindings to callbacks
- [ ] widget focus system (shift-tabbing between focussable views, moving selection inside focussed view)
- [ ] input for tree view (selection, expansion)
- [ ] action callbacks e.g. for buttons, text boxes