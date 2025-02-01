## Using LayoutScript

big UI trees are often frustrating to manage. while the `Page` class makes that much easier, constructing the UI is still going to involve keeping track of a whole load of variables, the majority of which are probably never going to be referenced again until they're destructed.

you may also want to load UI dynamically at runtime, for whatever reason.

also, if you have an application with multiple `Page`s, LayoutScript files allow you to completely separate them to avoid confusion.

luckily, you can now do that! the [stui_script.h](inc/stui_script.h) module contains the `LayoutReader`, which can translate a block of structured text into a `Page` containing the UI tree described in the text block.

this file is an example, and doesn't go into too much detail about the LayoutScript language itself. see [LAYOUTSCRIPT.md](LAYOUTSCRIPT.md) for detailed information.

### Syntax

lets start with the syntax of LayoutScript. the main rule of LayoutScript is that we ~~never talk about LayoutScript~~ must always start with the file with a `Component`, and that we can **only** have one root `Component` per file. essentially, one file means one `Component`, it's just that we can nest `Component`s to create a UI tree.

we can convert a `Component` initialisation in C++ code like this:
```
Label* l = new Label("Hello, World!", -1);
BorderedBox* bb = new BorderedBox(l, "box name");
```
into a definition in LayoutScript like this:
```
BorderedBox : "bb"
(
    child = Label : "l" (text = "Hello, World!", alignment = -1),
    name = "box name"
)
```

breaking down the syntax of the first line, we have the `Component` type name `BorderedBox`, then `: "bb"` which tells the `LayoutReader` to give the `Component` a name and call it 'bb' in the `Page`s registry. this is optional, we can skip the `: "bb"` entirely if we want and just let the `Page` name the `Component` automatically.

giving `Component`s an identifiable name will allow us to fetch them from the registry later. you'll likely need to do this for any interactive `Component`, like a `Button` that you want to set the callback for.

next we have an `(` open bracket, and until the `)` closing bracket we are now listing the arguments with which to initialise the `BorderedBox`.

these arguments are assigned by name (*the name of the LayoutScript argument for a `Component` should always match the name of the constructor argument in C++*), with the 'identifier = value' syntax. the value part of this can be one of 5 things:
- an int
- a float
- a string
- a `Coordinate`
- a nested `Component`

each of these also has an array variant, but we'll get to that later.

arguments are optional: you can set any, all, or none of the possible constructor arguments for a given `Component` type, and in any order. arguments which do not match a constructor argument in C++ will be ignored.

### Loading LayoutScript

once we've written our UI tree in LayoutScript, we'll need to load that at runtime in C++. we do that by creating a `LayoutReader`, and just calling `readPageFromFile`:
```
#define STUI_IMPLEMENTATION
#include <stui_script.h>

int main()
{
    Terminal::configure();

    LayoutReader reader;
    
    Page* page = reader.readPageFromFile("my_ui.sls")
    if (page == nullptr) return;

    page->render();

    // TODO: destruct page
    return 1;
}
```
once we've loaded our LayoutScript to a UI tree, we can treat it as just a normal `Page`, `render` it, `checkInput` on it, and fetch named items from it if needed.
```
// ...
    page->get<Label>("l")->text = "the text has changed!";
    // or like this
    ((Label*)((*page)["l"]))->text = "the text has changed!";
// ...
```

### Cleaning Up

since we've allocated all the `Component`s in the `Page` dynamically (i.e. with `new`), we should `delete` them when we're done with them. this isn't such a big deal with simple applications, since when the user hits `CTRL+C` all our memory gets freed anyway, but this will **definitely** be important if you're loading/destroying UI dynamically during the course of your program.

when `Page` destructs, it leaves the `Components` it contains intact. after all, it doesn't know if they were created with `new`, or if they're potentially still in use/referenced elsewhere. thus, we have to tell it to `delete` them ourselves.

we do this using the `destroyAllComponents` method. this method takes an argument with a list of names of `Component`s to not `delete`. we may want to do this for either of the reasons listed above, or just because we want to reuse the `Component` later. it will return a list of the `Component`s which haven't been `delete`d.
```
// ...
    page->render();

    page->destroyAllComponents({ });

    return 0;
// ...
```
in our case, all of our `Component`s were created dynamically by the `LayoutReader`, so they all get `delete`d.

`destroyAllComponents` also clears the entire `Page` registry and the root `Component`, since it has no idea which `Component`s may now reference deleted children, and keeping them around is almost certain to lead to a segfault, or potentially double-freeing something later, etc.

### Arrays

an important element of LayoutScript is arrays. various `Component` types use arrays in their arguments; of particular note is the `VerticalBox` and `HorizontalBox`. arrays are written using `{}` curly braces:
```
// from examples/layoutscripts/page_usage.sls
VerticalBox
( children = {
    Label : "text_widget" (text = "you must enter the password before you can proceed!"),
    BorderedBox(child = TextInputBox : "text_field" (text = "type something")),
    Button : "dummy_button" (text = "this does nothing"),
    Button : "disabled_button" (text = "this isn't even focusable", enabled = 0)
})
```
the rules with arrays are simple:
1) array elements are separated by commas
2) arrays must always contain at least one item, remember that if you want to leave a constructor argument blank you can just omit it
3) arrays may only contain one type of element, you cannot mix multiple types in the same array (e.g. an int and a string)
4) array elements may not be blank (i.e. `{ 4, 3, , 5 }` is not valid)

### A Larger Example

let's take a larger example, and combine it with C++ to see how the two combine. if you look in [widgets_demo.cpp](examples/widgets_demo.cpp), you'll see that we do a lot of boilerplate initialisation and the structure of the UI isnt immediately obvious. we have a ton of random 'box0' variables floating around. i went ahead and recreated this in LayoutScript in [widgets_demo.sls](examples/layoutscript/widgets_demo.sls), but here's a copy:
```
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
```
hopefully the structure of the UI tree is clearer now. you'll also notice that most of our UI components are unnamed, all the nested `BorderedBox`es and `SizeLimiter`s, we never cared about keeping track of.

now in C++, we grab references to things we care about and set whatever properties need setting, for instance the `ImageView` (data omitted for clarity):
```
// ...
    stui::ImageView* image_view = page->get<stui::ImageView>("image_view");
    image_view->grayscale_image = image;
    image_view->image_size = stui::Coordinate{ 12, 12 };
// ...
```
then we configure the list of focusable `Component`s within the `Page`, and we just `render` it and update things as before:
```
// ...
    page->focusable_component_sequence = { (*page)["rb"], (*page)["ta"], tib, (*page)["list"], tree };
    
    while (true)
    {
        stui::Renderer::FrameData delta = page->framerate(24);
        timer += delta.delta_time;

        pb->fraction = delta.active_fraction;//fmod(timer, 1.0f);
        t1->text = "fps: " + to_string(1.0f / delta.delta_time);
        s1->state = (timer * 8);
        s2->state = s1->state;
        s3->state = s1->state;
        s4->state = s1->state;

        if (timer > 2.0f)
        {
            timer -= 2.0f;
        }

        page->checkInput();
        page->render();
    }
// ...
```

if you want more examples, i've actually converted **all** of the examples in the [examples](examples) folder to LayoutScript, so you can compare between the two.

### Errors

if you fuck up something in your LayoutScript file, the `LayoutReader` will pick up on it, and throw a (hopefully) meaningful error message. it will tell you where the error occurred, and what was wrong, and show you a preview and so on. for example:
```
terminate called after throwing an instance of 'std::runtime_error'
  what():  STUI layout document parsing error:
        coordinates may not be floating-point
        at character 1093 (ln 31, col 28)
        -> '... max_size = [ 30.5, -1 ]...'
                               ^
        terminating parsing.
```
you _can_ catch these exceptions, but remember that the `Page` returned by the `readPageFromFile`/`readPage` **will** be `nullptr` if an exception has occurred.

for a comprehensive explanation of *all* the different error types, see [LAYOUTSCRIPT.md](LAYOUTSCRIPT.md).

### Using LayoutScript as a Sub-Component

this is a little trick you can do which may be useful to some people. say you have a `BorderedBox`, and you want to put different things inside it depending on what your application is doing. well, with LayoutScript, you could potentially have a LayoutScript file which describes just part of your UI tree,such as the thing you want to swap into the `BorderedBox`.

to do this, you would load the UI from the file like normal, grab the root node out of the `Page`, and then just throw away the `Page`:
```
LayoutReader reader;

// our existing stuff
Page* my_main_page = /* our initialisation of this, who cares */;
BorderedBox* box = my_main_page->get<BorderedBox>("box");

// now we load our sub-UI
Page* tmp = reader->readPageFromFile("sub_ui_thing.sls");

// here we would extract any named `Component`s we want to interact with, like text boxes or buttons

// pull out the root component, and throw away the page
Component* sub_ui_root = tmp->getRoot();
delete tmp;

// ...
// now, somewhere later down in the program, say we want to make our sub-UI the content of the bordered box
box->child = sub_ui_root;
// we should also really call ensureIntegrity on my_main_page any time we modify the UI tree at all
```

### Putting Multiple Root Components in One File

okay, so even though i _said_ we can only have one root `Component` per file, we can technically get around that.

imagine you have a bunch of small bits of UI that you want to swap in and out of the main UI tree, but you don't want to have variables floating around and you don't want to have a million different `.sls` files. what do you do?

since we can nest `Component`s, we can use a similar trick to what we did above, and nest all of our small bits of UI inside a single `VerticalBox`, or `HorizontalBox` (which one is irrelevant, since we're going to discard it).

now, once we've loaded our file into a `Page` using the `LayoutReader`, we extract the list of children from the `VerticalBox`/`HorizontalBox`, and then delete both the `Page` and the `VerticalBox`.

and hey look! we managed to get multiple different UI trees in a single LayoutScript file.

### Additional Stuff

you may have already noticed, but comments exist! single line comments starting with a `//` are supported, but multiline or enclosed comments (i.e. the `/* */` kind) are not.

it may be useful to note that the LayoutScript text doesn't just have to be loaded from a file. if you want to keep things really minimal or _avoid having extra files floating about at runtime_, you can just write your LayoutScript code in a string within your C++ file, and then use `readPage` instead of `readPageFromFile`.
the best way to do this is probably to write your LayoutScript in a dedicated header file contained in a single string and then `#include` that when you want to convert it into a UI tree.

the way the `LayoutReader` translates LayoutScript tokens into `Component`s is via a builder system. each `Component` type has a corresponding `ComponentBuilder`, which looks to see if any of the constructor arguments it expects for its matching `Component` type are present, and then assigns them if they are (otherwise it uses the default values).
if you've created a custom `Component` type, you may want to be able to read it from LayoutScript files. in this case, you'll want to create a matching `ComponentBuilder` class, and then register it with the `LayoutReader` before you try to read your LayoutScript file.
this is why you have to actually instantiate the `LayoutReader` class, rather than `readPageFromFile` just being a static method.
