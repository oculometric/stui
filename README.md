# stui - Simple Text UI

a lightweight, easy-to-use, header-only, semi-immediate-mode TUI framework for building simple interfaces, inside the terminal, across platforms.

## Key Features

i sorta just summarised them above didn't i but here you go again:
- **cross-platform** - tested on Windows and Linux (including WSL)
- **lightweight** - simple components and architectured with efficiency in mind, including giving you tools to limit your framerate and only redraw when necessary
- **immediate-mode** - you can draw the screen, and check for input whenever and wherever you want
- **callback-driven interaction** - callback functions make it easy to add interactivity
- **extensible** - lacking a particular UI element you want? add it yourself by subclassing `Component`
- **header-only** - no extra compiler options, no extra DLLs you need to ship, it all just builds into a single executable
- **supports Unicode glyphs** - assuming your terminal can display them, `Component`s can draw Unicode characters (e.g. for boxes, spinners, etc)
- **modular** - manage you UI behaviour manually, or use the `Page` extension to handle `Component` management and focus for you
- **scriptable** - (WIP) build you UI in a simple structured language, making building complex UI pages much easier and more human-readable

## Purpose

i started writing a tool for debugging FBX files, and i wanted to have a little UI for it displayed in the terminal so you could browse the NodeRecord tree. since i couldn't seem to find any decent, easy-to-use TUI frameworks (my main requirements being header-only and cross-platform), *un*fortunately this cast me into a rabbit hole of building my own library to do that. and now you have this project.

the project concept was partly inspired by [gitui](https://github.com/extrawurst/gitui), an excellent text UI program for git, which i also used extensively while developing this.

## Status

i'm actively developing this at the moment. see the TODO list file for what i'm probably working on. once the project is complete and fully documented, i'll likely not be actively adding stuff. i'll do my best to respond to issues on Github.

## Installation and Usage

see the [HELP.md](help.md) file for a proper explanation, but all you need to do is `git submodule add` this project (or just clone it separately and keep the `inc` directory somewhere handy) and add the `inc` directory as an include path for your compiler. then you're good to go with creating UI elements and drawing them in the terminal. _happy widgeting!_