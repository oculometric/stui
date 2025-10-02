### Building Your Own Components

it's relatively easy to write your own STUI `Component` types, to draw whatever specific thing you want. the first step to this is changing how you include the header file. you must add the following line before `#include <stui.h>`:
```
#define STUI_KEEP_DEFINES
```

this will give you access to a collection of macros which make it much easier to write your own `Component`

the next step is to define a new class which inherits from `Component`, or another existing UI element type (*although i cannot vouch for what may happen if you do the latter*):
```
class CustomComponent : public Component, public Utility
{

};
```

you may notice that i also inherited from the `Utility` class. this is a class which encapsulates some static functions which are useful for things like drawing text and copying buffers.

now we need to define what our `CustomComponent` actually does and looks like. the functions we must implement are `Component::render`, `Component::getTypeName`, `Component::getMaxSize`, and `Component::getMinSize`. with the latter two, we can tell the parent `Component` how big this component should be. `getMinSize` should return the smallest size the component expects to be drawn. most other `Components` like the `VerticalBox` will not bother drawing your `CustomComponent` if they cannot find enough space to meet the requirement set by `CustomComponent::getMinSize`. `getMaxSize` returns the maximum size the component wants to be drawn as, though it's possible in some circumstances for the component to be given a buffer bigger than this function asks for. if you want your `CustomComponent` to fill the available space, specify -1 as the size (works in either dimension, independently).

the `render` method is what actually draws our `CustomComponent` into existence, and this is your chance to go wild. **you must be careful to respect the bounds of the buffer into which you are drawing. you WILL corrupt the heap otherwise, and that means bad times for everyone.** you probably also want to include some validation for the size of the buffer, and whether it's a null-pointer or not, just in case some evil person has written a buggy `EvilCustomComponent` which is calling `render` on your `CustomComponent` incorrectly.

the `getTypeName` function should simply return the string representation of the class name. this is currently just used for auto-naming `Component`s in the `Page` component registry.

the only other thing we need to do is specify some properties for our UI widget, and write an appropriate constructor, and we have a working UI element.

now, as far as providing `render`, `getMaxSize` and `getMinSize` implementations goes, you definitely want to use the preprocessor macros i mentioned before. their use is fairly self-explanatory:
```
class CustomComponent : public Component, public Utility
{
public:
    int circle_size;

    CustomComponent(int _circle_size) : circle_size(_circle_size) { }

    RENDER_STUB
    {
        // do your rendering here. make sure to check the buffer is definitely as big as you expect it to be
    }

    GETMAXSIZE_STUB { return Coordinate{ -1,-1 }; }
    GETMINSIZE_STUB { return Coordinate{ 3,3 }; }

    GETTYPENAME_STUB("CustomComponent");
};
```

now, if you want to receive input from the user, you need to also override `Component::isFocusable` and `Component::handleInput`. the first of these is trivial, just return true (though you may want the option to put your component in read-only mode, in which case you could just have an `editable` flag and return that instead). the second is where you do your own input handling, and you should return true if you consumed the input or false if you did not.
```
// ...
    ISFOCUSABLE_STUB { return true; }

    HANDLEINPUT_STUB
    {
        if (!focused) return false;

        // do something with the user input here

        return true;
    }
// ...
```

finally, if your `CustomComponent` has child `Component`s that it draws, you should implement `Component::getAllChildren` and return a list of pointers to those children. this also has a corresponding macro. if you do this, then you should also state the `SETENABLEDRECURSIVE;` macro, as this will allow child `Component`s to be recursively enabled and disabled. you can also use the related stub macro to do your own behaviour.

and you're done! if you want to see how to do more specific things, take a look through the [stui.h](stui.h) header file and see how it's done there. or submit an issue on Github asking for clarification/documentation.

one final note about the output buffer: each `Tixel` in the buffer represents a single character in the terminal. at the end of the rendering process. **don't try and print extended ASCII. it will not display properly.** you can however specify up to 4-byte Unicode characters in the `Tixel::character` field, and they should display correctly (assuming the terminal you're using has support for it. if it doesn't you should probably fix that or something). you can also specify a colour (yes, that spelling, spooky) for the character, which is applied via 8-colour ANSI codes. you can `|` (bitwise OR) two `Tixel::ColourCommand`s together to change both foreground and background, but you MUST only combine one foreground and one background command per `Tixel`. otherwise who knows what might happen.

if you want to be able to deserialise your `CustomComponent` type from LayoutScript files, all you have to do is implement a `CustomComponentBuilder`. this is not hard, and is done in much the same way as you do for the `Component` itself. below is an example (see the [HELP_EX3.md](HELP_EX3.md) for more information about programming with LayoutScript, and [LAYOUTSCRIPT.md](LAYOUTSCRIPT.md) for the LayoutScript specification):
```
#define STUI_IMPLEMENTATION
#define STUI_KEEP_DEFINES
#include <stui_script.h>

using namespace stui;

class CustomComponentBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "CustomComponent"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        CustomComponent* c = new CustomComponent();
        constructor.copy("circle_size", c->circle_size);
        return c;
    }
#endif
    ;
};

int main()
{
    Terminal::configure();

    // method 1:
    LayoutReader reader;
    reader.registerBuilder(new CustomComponentBuilder());

    // method 2:
    // LayoutReader reader = LayoutReader({ builder<CustomComponentBuilder> });
    // 

    Page* page_with_custom_comp = reader->readPage("foo.sls");
// ...
}
```