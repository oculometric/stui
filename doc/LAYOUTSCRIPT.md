# LayoutScript Documentation

## Syntax and Structure

all LayoutScript files must start with by defining the root `Component`; any other token is invalid. there may only be one top-level `Component`.

### Defining Components

`Component`s can be defined simply with the following syntax:
```
ComponentType : "unique_identifier" (arg_name = "arg_value", other_arg = 0)
```
this defines a `Component` of type `ComponentType`, and will cause it to be registered in the `Page` with the identifier `"unique_identifier"` (unless there is a duplicate component, in which case the one registered later one will be renamed).

the identifier is optional, and definitions can be made by omitting the `: "unique_identifier"` part like so:
```
ComponentType (arg_name = "arg_value", other_arg = 0)
```
the `Component` will have its name generated procedurally by the `Page` instead.

the `Component` will be initialised with its properties set to the values specified by the arguments (contained in brackets). all arguments are optional, and the order is irrelevant. arguments which do not match constructor values for the given `Component` class (or more specifically, which the `ComponentBuilder` does not respect) are ignored silently.

### Argument Types

there are five main argument types:
- int (`4`, `-32`)
- float (`4.0`, `-0.16`)
- string (`"Hello, World!"`)
- `Coordinate` (`[ 4, 2 ]`, `[ -1, 48 ]`)
- `Component` (see Nesting Components)

each of these has an array variant (see Arrays).

arguments are formatted in a list, separated by `,` commas. the name of the argument must always be supplied, and the value should be supplied on the other side of an `=` equals sign.

string tokens only respect `"` double-quotes, `'` single-quotes are treated like a normal character. strings may be multi-line and contain invalid characters, though these may not read/display correctly.

`Coordinate` tokens consist of exactly two integers within `[]` square brackets, separated by a `,` comma. floating point values are invalid.

### Nesting Components

many `Component`s accept a child `Component` as a parameter, so nested `Component` definitions for arguments are supported. there is no special syntax for this (follow the regular argument syntax):
```
SizeLimiter
(
    max_size = [ 32, 4 ],
    child = TextBox (text = "Lorem Ipsum Dolor")
)
```
there is no limit to the depth of nesting.

### Arrays

arrays are denoted with `{}` curly braces, and the items are separated with `,` commas.

an array must contain at least one item, and may contain only one type of element (int, float, string, `Coordinate` or `Component`):
```
VerticalBox
(
    children = {
        Label(),
        Button(),
        Button(),
        RadioButton(options = {"option 1", "option 2", "option 3"})
    }
)
```

nested arrays (arrays of arrays) are not permitted.

### Whitespace and Comments

single-line comments are supported:
```
VerticalBox
( // this is a comment
    children = {
        Label()
    }
)
```
everything after the `//` double-forward-slash will be discarded until a newline is encountered.

newlines are completely discarded once tokenisation is complete; tabs and spaces are irrelevant if they are outside a string, unless they are between two primary tokens (any combination of int, float, string, `Component` name, argument name, `Coordinate`). this means that:
```
HorizontalSpacer(width=2)
```
```
SizeLimiter:"limiter"( child=Label(   text=  "some string" ))
```
and
```
Label : "label" ( text = "some string" )
```
are all fine, but
```
Label : "label"0.5()
```
is not (for multiple reasons actually, since a float token is not valid here regardless) fine.

## Error Codes

when you've made a mistake in a LayoutScript file, the `LayoutReader` will catch that mistake and tell you about it. it will do this by throwing a runtime exception with a detailed error message, which looks like this:
```
terminate called after throwing an instance of 'std::runtime_error'
  what():  STUI layout document parsing error:
        coordinates may not be floating-point
        at character 1093 (ln 31, col 28)
        -> '... max_size = [ 30.5, -1 ]...'
                               ^
        terminating parsing.
```
as you can see, the tokeniser has correctly identified my syntax error, and shows precisely where in the original `widgets_demo.sls` file it occurred, along with a preview.

when a parsing exception like this occurs, you may catch it in order to show an error message to the user and exit gracefully, but the return value of `readPage` will be `nullptr`, so don't try to access anything on it.

there are lots of different types of errors which can be thrown, so below is a table of all of them, what stage they happen, and what the message actually means in a little more detail.

| error message                                 | thrown during       | cause/explanation                  |
|-----------------------------------------------|---------------------|------------------------------------|
| the LayoutScript file must contain at least one complete Component | page decoding | self explanatory |
| the LayoutScript file must begin with a Component definition | page decoding | the file does not begin with an identifier token (i.e. the name of a `Component` type) |
| invalid first token | tokenisation | the file begins with a token type other than an identifier or a comment, which is disallowed |
| incomplete comment initiator | tokenisation | a single forward slash is found; comments must be initiated by double forward slash |
| coordinates may not be floating point | tokenisation | a float token was detected inside a `Coordinate` token, which is disallowed |
| incomplete coordinate token | tokenisation | a `Coordinate` token ends too early, for example due to being cut off by a comment or not having both numbers |
| invalid integer token within coordinate token | tokenisation | a `Coordinate` token contains an invalid number which `stoi` cannot understand, or the `Coordinate` contains too many values |
| invalid token within coordinate token | tokenisation | a `Coordinate` token contains a token which is not allowed, such as brackets/curly braces or a string |
| invalid conjoined token | tokenisation | two tokens are adjacent (without being separated by whitespace, such as a space) which are not permitted: identifier and string, identifier and `Coordinate`, identifier and int, identifier and float, string and int, string and float, `Coordinate` and int, or `Coordinate` and float |
| invalid int token | tokenisation | an integer token could not be understood by `stoi` |
| invalid float token | tokenisation | a float token which could not be understood by `stof`, for instance due to having multiple `.` floating points |
| invalid tokeniser state | tokensiation | the tokeniser has reached a condition in a switch statement which should be impossible to reach. if this is ever thrown, it should be reported as a bug, it's not your fault! |
| incomplete component definition | component parsing | the `Component` parser was not given enough tokens to construct a token |
| initial token must be a component name | component parsing | the first token of the `Component` was not an identifier (i.e. a `Component` type name) |
| unrecognised component type | component parsing | the first token of the `Component` was an identifier, but there was no `ComponentBuilder` registered to handle deserialising it |
| invalid token after component type | component parsing | if the token after the `Component` type is a `:` colon, then it must be followed by a string; any other token is invalid |
| component type token name must be followed by either a bracket pair or a colon, a string name in quotes, and then a bracket pair | component parsing | self explanatory (see Defining Components above) |
| incomplete component definition | component parsing | the `Component` definition does not have a matching closing bracket, and/or is lacking a body |
| expected argument identifier | component parsing | the argument list for the `Component` is not formatted correctly, see Defining Components above. the argument list should be a repeating sequence of `argument_name`, `=`, <int/float/string/Coordinate/Component>, `,` tokens. |
| expected '=' following identifier | component parsing | as above |
| expected argument value after '='" | component parsing | an argument has been terminated (e.g. with a `,` comma, or by a premature `)` closing bracket) before at least one value token was provided for it |
| invalid closing bracket | component parsing, argument creation | a `)` closing backet was found when there was no accessible `(` opening bracket to match |
| invalid closing curly brace | component parsing, argument creation | as above, but with a `}` closing curly brace instead |
| unexpected token(s) after int | argument creation | extra tokens were found after an int token (i.e. junk tokens after an int literal argument) |
| unexpected token(s) after float | argument creation | as above but in relation to a float token |
| unexpected token(s) after string | argument creation | as above but in relation to a string token |
| unexpected token(s) after coordinate | argument creation | as above but in relation to a `Coordinate` token |
| unexpected token(s) after closing curly brace | argument creation | as above but the junk tokens are found after the end of an array |
| empty arrays are not permitted | argument creation | self explanatory; array type cannot be inferred from an empty array |
| invalid first token in array | argument creation | the first token within an array may only be an int, float, string, `Coordinate`, or `Component` type name, all others (e.g. `(` opening bracket, etc) are considered invalid |
| arrays may only contain one type of data | argument creation | self explanatory |
| invalid argument decoder state | argument creation | the argument decoder has reached a condition in a switch statement which should be impossible to reach. if this is ever thrown, it should be reported as a bug, it's not your fault! |
| invalid token(s) after '=' | argument creation | the token after the `=` equals in the argument does not represent one of the supported argument types, it may only be an int, float, string, `Coordinate`, `Component`, or array of one of those types |
| missing closing bracket | component parsing, argument creation | the matching `)` closing bracket for a given `(` opening bracket could not be found within the sequence of tokens |
| missing closing curly brace | component parsing, argument creation | as above, but with a `}` closing curly brace |
