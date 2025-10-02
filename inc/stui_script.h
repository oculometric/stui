/*  
	A simple header-only library for creating text-based user interfaces
	inside a terminal window.
	Copyright (C) 2024-2025  Jacob Costen (oculometric)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

	Contact me here <mailto://jcostenart@gmail.com>
*/

#ifndef STUI_SCRIPT_H
#define STUI_SCRIPT_H

///////////////////////////////////////////////////////////////////////
//                             INCLUDES
///////////////////////////////////////////////////////////////////////

#include "stui_extensions.h"

#include <iostream>
#include <fstream>

///////////////////////////////////////////////////////////////////////
//                             DEFINES
///////////////////////////////////////////////////////////////////////

#pragma region STUI_DEFS

#define GETNAME_STUB string getName() override
#define BUILD_STUB Component* build(BuilderArgs constructor) override

#pragma endregion STUI_DEFS

namespace stui
{

///////////////////////////////////////////////////////////////////////
//                       COMPONENT BUILDERS
///////////////////////////////////////////////////////////////////////

#pragma region STUI_BUILDERS

/**
 * @brief encapsulates a map of component constructor argument names and values.
 * 
 * provides some helper functions to check if arguments exist and are of a specific
 * type.
 */
class BuilderArgs
{
public:
    /**
     * @brief enumerates possible types for a builder argument, each one having
     * and array counterpart.
     */
    enum ArgType
    {
        STRING,
        INT,
        FLOAT,
        COORDINATE,
        COMPONENT,
        STRING_ARRAY,
        INT_ARRAY,
        FLOAT_ARRAY,
        COORDINATE_ARRAY,
        COMPONENT_ARRAY
    };

    /**
     * @brief structure describing a constructor argument being passed to a builder.
     * 
     * this should be used by `ComponentBuilder` subclasses to be translated into actual
     * constructor arguments for the `Component`s which get built.
     */
    struct Argument
    {
        ArgType type = ArgType::STRING;

        union
        {
            string s_value = "";
            int i_value;
            float f_value;
            Coordinate coo_value;
            Component* com_value;
        };

        vector<string> as_value;
        vector<int> ai_value;
        vector<float> af_value;
        vector<Coordinate> acoo_value;
        vector<Component*> acom_value;

        inline Argument() { }

        inline Argument(const Argument& other)
        {
            type = other.type;
            switch (other.type)
            {
                case STRING: s_value = other.s_value; break;
                case INT: i_value = other.i_value; break;
                case FLOAT: f_value = other.f_value; break;
                case COORDINATE: coo_value = other.coo_value; break;
                case COMPONENT: com_value = other.com_value; break;
                case STRING_ARRAY: as_value = other.as_value; break;
                case INT_ARRAY: ai_value = other.ai_value; break;
                case FLOAT_ARRAY: af_value = other.af_value; break;
                case COORDINATE_ARRAY: acoo_value = other.acoo_value; break;
                case COMPONENT_ARRAY: acom_value = other.acom_value; break;
            }
        }

        inline Argument operator=(const Argument& other)
        {
            type = other.type;
            switch (other.type)
            {
                case STRING: s_value = other.s_value; break;
                case INT: i_value = other.i_value; break;
                case FLOAT: f_value = other.f_value; break;
                case COORDINATE: coo_value = other.coo_value; break;
                case COMPONENT: com_value = other.com_value; break;
                case STRING_ARRAY: as_value = other.as_value; break;
                case INT_ARRAY: ai_value = other.ai_value; break;
                case FLOAT_ARRAY: af_value = other.af_value; break;
                case COORDINATE_ARRAY: acoo_value = other.acoo_value; break;
                case COMPONENT_ARRAY: acom_value = other.acom_value; break;
            }

            return *this;
        }

        inline ~Argument()
        {
            if (type == ArgType::STRING)
                s_value.~string();
        }
    };

    map<string, Argument> arguments;

    BuilderArgs(map<string, Argument> args) : arguments(args) { }

    inline bool has(string name, ArgType type)
    {
        if (arguments.count(name) == 0) return false;
        return arguments[name].type == type;
    }

    inline Argument get(string name) { return arguments[name]; }

    inline void copy(string name, string& to) { if (has(name, ArgType::STRING)) to = get(name).s_value; }
    inline void copy(string name, int& to) { if (has(name, ArgType::INT)) to = get(name).i_value; }
    inline void copy(string name, float& to) { if (has(name, ArgType::FLOAT)) to = get(name).f_value; }
    inline void copy(string name, Coordinate& to) { if (has(name, ArgType::COORDINATE)) to = get(name).coo_value; }
    inline void copy(string name, Component*& to) { if (has(name, ArgType::COMPONENT)) to = get(name).com_value; }
    inline void copy(string name, vector<string>& to) { if (has(name, ArgType::STRING_ARRAY)) to = get(name).as_value; }
    inline void copy(string name, vector<int>& to) { if (has(name, ArgType::INT_ARRAY)) to = get(name).ai_value; }
    inline void copy(string name, vector<float>& to) { if (has(name, ArgType::FLOAT_ARRAY)) to = get(name).af_value; }
    inline void copy(string name, vector<Coordinate>& to) { if (has(name, ArgType::COORDINATE_ARRAY)) to = get(name).acoo_value; }
    inline void copy(string name, vector<Component*>& to) { if (has(name, ArgType::COMPONENT_ARRAY)) to = get(name).acom_value; }
};

class LayoutReader;

/**
 * @brief base for subclasses which each construct a specific `Component` type from
 * a set of arguments parsed from a LayoutScript file.
 * 
 * all subclasses must implement `getName` (which must return the string name of the
 * Component this subclass intends to build), and `build`, which actually performs the
 * construction. there are relevant macros to make this easy.
 * 
 * all built-in STUI `Component` types have matching `ComponentBuilder` types.
 * 
 * you should also name component builders to match component types: for instance, in
 * order for the `Label` component type to be read from a file, there should be a 
 * `LabelBuilder` class defined, where its `getName` returns "Label". to declare a 
 * label inside a LayoutScript file, you can now do something like:
 * ```
 * Label(text = "Hello", alignment = -1)
 * ```
 * and the `build` function should read in the `text` and `alignment` arguments and
 * pass them correctly to the `Label` constructor.
 * 
 * in order for your custom component types to be read in from LayoutScript, you
 * must define your own builders for each custom type, and register them with
 * the `LayoutReader` you're using to read files.
 */
class ComponentBuilder
{
    friend class LayoutReader;

protected:
    /**
     * @brief returns the name (as given in LayoutScript files) of the component
     * type which this builder intends to handle construction of.
     * 
     * @param the name as shown in LayoutScript files 
     */
    virtual string getName() = 0;

    /**
     * @brief converts a set of named arguments into a component instance acording
     * to the constructor of the component type this builder intends to build.
     * 
     * @param constructor collection of arguments to work with
     * @returns newly initialised `Component` of whatever subclass corresponds to
     * this `ComponentBuilder` subclass
     */
    virtual Component* build(BuilderArgs constructor) = 0;

    virtual ~ComponentBuilder() { }; // don't touch this
};

class LabelBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "Label"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        Label* c = new Label();
        constructor.copy("text", c->text);
        constructor.copy("alignment", c->alignment);
        return c;
    }
#endif
    ;
};

class ButtonBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "Button"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        Button* c = new Button();
        constructor.copy("text", c->text);
        int enabled = true;
        constructor.copy("enabled", enabled);
        c->enabled = enabled;
        return c;
    }
#endif
    ;
};

class RadioButtonBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "RadioButton"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        RadioButton* c = new RadioButton();
        constructor.copy("options", c->options);
        int enabled = true;
        constructor.copy("enabled", enabled);
        c->enabled = enabled;
        constructor.copy("selected_index", c->selected_index);
        return c;
    }
#endif
    ;
};

class ToggleButtonBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "ToggleButton"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        ToggleButton* c = new ToggleButton();
        vector<string> options;
        constructor.copy("options", options);
        for (string op : options) c->options.push_back(pair<string, bool>(op, false));
        int enabled = true;
        constructor.copy("enabled", enabled);
        c->enabled = enabled;
        return c;
    }
#endif
    ;
};

class TextInputBoxBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "TextInputBox"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        TextInputBox* c = new TextInputBox();
        constructor.copy("text", c->text);
        int enabled = true;
        constructor.copy("enabled", enabled);
        c->enabled = enabled;
        return c;
    }
#endif
    ;
};

class TextAreaBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "TextArea"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        TextArea* c = new TextArea();
        constructor.copy("text", c->text);
        constructor.copy("scroll", c->scroll);
        return c;
    }
#endif
    ;
};

class ProgressBarBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "ProgressBar"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        ProgressBar* c = new ProgressBar();
        constructor.copy("fraction", c->fraction);
        return c;
    }
#endif
    ;
};

class SliderBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "Slider"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        Slider* c = new Slider();
        constructor.copy("value", c->value);
        return c;
    }
#endif
    ;
};

class SpinnerBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "Spinner"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        Spinner* c = new Spinner();
        int state = 0;
        constructor.copy("state", state);
        c->state = (size_t)state;
        constructor.copy("state", c->type);
        return c;
    }
#endif
    ;
};

class VerticalBoxBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "VerticalBox"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        VerticalBox* c = new VerticalBox();
        constructor.copy("children", c->children);
        return c;
    }
#endif
    ;
};

class HorizontalBoxBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "HorizontalBox"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        HorizontalBox* c = new HorizontalBox();
        constructor.copy("children", c->children);
        return c;
    }
#endif
    ;
};

class VerticalSpacerBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "VerticalSpacer"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        VerticalSpacer* c = new VerticalSpacer();
        constructor.copy("height", c->height);
        return c;
    }
#endif
    ;
};

class HorizontalSpacerBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "HorizontalSpacer"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        HorizontalSpacer* c = new HorizontalSpacer();
        constructor.copy("width", c->width);
        return c;
    }
#endif
    ;
};

class BorderedBoxBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "BorderedBox"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        BorderedBox* c = new BorderedBox();
        constructor.copy("child", c->child);
        constructor.copy("name", c->name);
        return c;
    }
#endif
    ;
};

class ListViewBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "ListView"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        ListView* c = new ListView();
        constructor.copy("elements", c->elements);
        constructor.copy("scroll", c->scroll);
        constructor.copy("selected_index", c->selected_index);
        constructor.copy("show_numbers", c->show_numbers);
        return c;
    }
#endif
    ;
};

class TreeViewBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "TreeView"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        TreeView* c = new TreeView();
        int scroll = 0;
        constructor.copy("scroll", scroll);
        c->scroll = (size_t)scroll;
        int selected_index = 0;
        constructor.copy("selected_index", selected_index);
        c->selected_index = (size_t)selected_index;
        return c;
    }
#endif
    ;
};

class ImageViewBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "ImageView"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        ImageView* c = new ImageView();
        return c;
    }
#endif
    ;
};

class SizeLimiterBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "SizeLimiter"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        SizeLimiter* c = new SizeLimiter();
        constructor.copy("child", c->child);
        constructor.copy("max_size", c->max_size);
        return c;
    }
#endif
    ;
};

class TabDisplayBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "TabDisplay"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        TabDisplay* c = new TabDisplay();
        constructor.copy("tab_descriptions", c->tab_descriptions);
        int current_tab = 0;
        constructor.copy("current_tab", current_tab);
        c->current_tab = current_tab;
        return c;
    }
#endif
    ;
};

class BannerBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "Banner"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        Banner* c = new Banner();
        constructor.copy("text", c->text);
        return c;
    }
#endif
    ;
};

class VerticalDividerBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "VerticalDivider"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        VerticalDivider* c = new VerticalDivider();
        return c;
    }
#endif
    ;
};

class HorizontalDividerBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "HorizontalDivider"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        HorizontalDivider* c = new HorizontalDivider();
        return c;
    }
#endif
    ;
};

class QRCodeViewBuilder : public ComponentBuilder
{
protected:
    inline GETNAME_STUB { return "QRCodeView"; }

    BUILD_STUB
#ifdef STUI_IMPLEMENTATION
    {
        QRCodeView* c = new QRCodeView();
        return c;
    }
#endif
    ;
};

template<class T>
inline ComponentBuilder* builder()
{
    static_assert(std::is_base_of<ComponentBuilder, T>::value, "T is not a ComponentBuilder type");
    T* instance = new T();
    return instance;
}


#pragma endregion STUI_BUILDERS

///////////////////////////////////////////////////////////////////////
//                          LAYOUT READER
///////////////////////////////////////////////////////////////////////

#pragma region STUI_LAYOUTREADER

/**
 * @brief encapsulates functionality for deserialising `Page`s from
 * LayoutScript files.
 * 
 * allows registering of custom builders to support deserialising
 * custom component types. all built-in component types are supported
 * by default.
 */
class LayoutReader
{
private:
    /**
     * @brief enumerates types of token the tokeniser supports.
     * 
     * these don't translate one-to-one to meaningful parts of
     * LayoutScript files (e.g. argument names, Component definitions).
     */
    enum TokenType
    {
        TEXT,
        OPEN_ROUND,
        CLOSE_ROUND,
        OPEN_CURLY,
        CLOSE_CURLY,
        NEWLINE,
        COLON,
        STRING,
        INT,
        FLOAT,
        COMMA,
        COORDINATE,
        EQUALS,
        COMMENT,
        WHITESPACE
    };

    /**
     * @brief stores a token decoded from the raw LayoutScript text
     * input.
     */
    struct Token
    {
        TokenType type;
        union
        {
            string s_value = "";
            int i_value;
            float f_value;
            Coordinate c_value;
        };
        size_t start_offset = 0;

        inline Token(TokenType ttype)
        {
            type = ttype;
            start_offset = 0;

            switch(ttype)
            {
                case TEXT:
                case STRING:
                case COMMENT:
                    s_value = "";
                    break;
                case COORDINATE:
                    c_value = Coordinate{0, 0};
                    break;
                case INT:
                    i_value = 0;
                    break;
                case FLOAT:
                    f_value = 0.0f;
                    break;
                default:
                    break;
            }
        }

        inline Token(const Token& other)
        {
            type = other.type;
            start_offset = other.start_offset;

            switch(type)
            {
                case TEXT:
                case STRING:
                case COMMENT:
                    s_value = other.s_value;
                    break;
                case COORDINATE:
                    c_value = other.c_value;
                    break;
                case INT:
                    i_value = other.i_value;
                    break;
                case FLOAT:
                    f_value = other.f_value;
                    break;
                default:
                    break;
            }
        }

        inline Token operator=(const Token& other)
        {
            type = other.type;
            start_offset = other.start_offset;

            switch(type)
            {
                case TEXT:
                case STRING:
                case COMMENT:
                    s_value = other.s_value;
                    break;
                case COORDINATE:
                    c_value = other.c_value;
                    break;
                case INT:
                    i_value = other.i_value;
                    break;
                case FLOAT:
                    f_value = other.f_value;
                    break;
                default:
                    break;
            }

            return *this;
        }

        inline ~Token()
        {
            switch (type)
            {
                case TEXT:
                case STRING:
                case COMMENT:
                    s_value.~string();
                    break;
                case COORDINATE:
                    c_value.~Coordinate();
                    break;
                default:
                    break;
            };
        }
    };

private:
    map<string, ComponentBuilder*> builders;

public:
    inline LayoutReader() : LayoutReader(vector<ComponentBuilder*(*)(void)>({ })) { };

    /**
     * @brief constructs a `LayoutReader` with an array of pointers
     * to constructor functions for additional (custom) component
     * builder types.
     * 
     * for this to function properly, you should only supply a list
     * of expressions which look like this:
     * ```
     * builder<ThingBuilder>
     * ```
     * where you obviously replace `ThingBuilder` with your custom
     * builder type. this ensures they are registered correctly.
     * 
     * @param additional_builders array of pointers to functions
     * which construct and return instances of specified component
     * builder types
     */
    LayoutReader(vector<ComponentBuilder*(*)(void)> additional_builders);

    LayoutReader operator=(LayoutReader& other) = delete;
    LayoutReader operator=(LayoutReader&& other) = delete;
    LayoutReader(LayoutReader& other) = delete;
    LayoutReader(LayoutReader&& other) = delete; 

    /**
     * @brief adds a new `ComponentBuilder` type to the list of
     * available builder types that this `LayoutReader` knows about.
     * 
     * if you have custom component types you'd like to deserialise,
     * then use this to register their builders.
     * 
     * the builder will replace an existing builder with the same
     * `getName` return value, if present.
     * 
     * @param builder pointer to an instance of the builder type
     * desired. this `LayoutReader` will handle deallocation of
     * all builders, including ones given via this function
     */
    void registerBuilder(ComponentBuilder* builder);

    /**
     * @brief reads a `Page` of STUI LayoutScript from a text file.
     * 
     * since the `Component`s in the page are constructed using `new`
     * at runtime, you **MUST** call `destroyAllComponents` on the
     * page when you're done using it.
     * 
     * throws an exception if a syntax error is found.
     * 
     * @param file path of the file to read from
     * @returns a `Page` fully constructed with all of the components
     * specified in the file, or `nullptr` if the file was not readable
     */
    Page* readPageFromFile(string file);

    /**
     * @brief reads a `Page` of STUI LayoutScript from a raw block of
     * LayoutScript code.
     * 
     * since the `Component`s in the page are constructed using `new`
     * at runtime, you **MUST** call `destroyAllComponents` on the
     * page when you're done using it.
     * 
     * throws an exception if a syntax error is found.
     * 
     * @param content LayoutScript code to translate into a `Page`
     * @returns a `Page` fully constructed with all of the components
     * specified in the content, or `nullptr`
     */
    Page* readPage(string content);

    ~LayoutReader();

private:
    /**
     * @brief converts a stream of raw LayoutScript text into a
     * sequence of simplified `Token`s.
     * 
     * throws an exception if a syntax error is found.
     * 
     * @param content raw text data to parse
     * @returns list of `Token`s decoded from the raw content
     */
    static vector<Token> tokenise(const string& content);

    /**
     * @brief returns whether a character is within the English
     * alphabet.
     * 
     * @param c character to check
     * @returns true if the character is alphabetic, otherwise false
     */
    static inline bool isAlphabetic(char c)
	{
		if (c >= 'a' && c <= 'z') return true;
		if (c >= 'A' && c <= 'Z') return true;

		return false;
	}

    /**
     * @brief gets an estimate of the `TokenType` of a given
     * character.
     * 
     * note that this is not guaranteed to be accurate, since
     * many token types are differentiated by context. for
     * instance, the letter 'c' within a string would return
     * `TEXT` here, even though it should in that case
     * technically be `STRING`.
     * 
     * @param c character to analyse
     * @returns a guess at the type of token `c` is part of
     */
    static inline TokenType getType(const char c)
    {
        if (isAlphabetic(c)) return TEXT;
        if (c == '(') return OPEN_ROUND;
        if (c == ')') return CLOSE_ROUND;
        if (c == '{') return OPEN_CURLY;
        if (c == '}') return CLOSE_CURLY;
        if (c == '"') return STRING;
        if (c == '\n') return NEWLINE;
        if (c == ':') return COLON;
        if (c == '=') return EQUALS;
        if (c == ',') return COMMA;
        if (c == '[' || c == ']') return COORDINATE;
        if (c == '-' || (c >= '0' && c <= '9')) return INT;
        if (c == '.') return FLOAT;
        if (c == '/') return COMMENT;
        if (c == ' ' || c == '\t') return WHITESPACE;
        return TEXT;
    }

    /**
     * @brief searches through a sequence of tokens to find
     * the closing bracket/brace matching the opening one at
     * the `open_index`.
     * 
     * throws an exception if a syntax error is found.
     * 
     * @param tokens list of tokens to operate on
     * @param open_index index of the opening bracket/brace
     * @param original_content original text content from
     * which the token array was parsed (used for error
     * reporting)
     * @returns index of the matching closing brace/bracket
     */
    static size_t findClosingBrace(const vector<Token>& tokens, size_t open_index, const string& original_content);

    /**
     * @brief converts a sequence of tokens into a `Component`.
     * 
     * throws an exception if a syntax error is found.
     * 
     * @param tokens list of tokens to operate on
     * @param start_index first token of the `Component`
     * definition
     * @param original_content original text content from
     * which the token array was parsed (used for error
     * reporting)
     * @param page the `Page` to which components are to be
     * registered as they are created
     * @returns the fully initialised component
     */
    Component* parseComponent(const vector<Token>& tokens, size_t start_index, const string& original_content, Page* page);

    /**
     * @brief converts a sequence fo tokens into an argument,
     * which will then be used to initialise a component.
     * 
     * starting with the first token, this function will try
     * to understand what type the argument is (string, int
     * float, component, or an array version of these) from
     * the first few tokens, and then organise it into a single
     * `Argument`.
     * 
     * throws an exception if a syntax error is found.
     * 
     * @param tokens list of tokens to operate on
     * @param original_content original text content from
     * which the token array was parsed (used for error
     * reporting)
     * @param page the `Page` to which new components are to
     * be registered, if necessary
     * @returns a finalised argument
     */
    BuilderArgs::Argument createArgument(const vector<Token>& tokens, const string& original_content, Page* page);
    
    /**
     * @brief prints a syntax exception to the debug log,
     * and throws it as an error.
     * 
     * automatically shows a preview of the faulty piece of
     * code, along with a description of the error, and 
     * calculates the line number and column from the 
     * provided offset into the provided string.
     * 
     * the offset is usually taken from the `start_offset`
     * of the `Token` on which the error occurred.
     * 
     * @param err a description of the type of error
     * @param off the offset into `str` where the error
     * occurred
     * @param str the raw LayoutScript text content for
     * generating a preview of the offending code
     */
    static inline void reportError(const string err, size_t off, const string& str)
    {
        int32_t extract_start = max(0, (int32_t)off - 16);
        int32_t extract_end = extract_start + 32;
        while (true)
        {
            size_t find = str.find('\n', extract_start);
            if (find >= off) break;
            extract_start = find + 1;
        }
        size_t find = str.find('\n', off);
        if (find != string::npos)
        {
            if ((int32_t)find < extract_end)
            extract_end = find;
        }
        string extract = str.substr(extract_start, extract_end - extract_start);

        size_t ln = 0;
        size_t last = 0;
        size_t next = 0;
        while (next < off)
        {
            ln++;
            last = next;
            next = str.find('\n', next + 1);
        }
        size_t col = off - last;
        if (ln > 0) col--;
        if (col > 0 && ln > 0) ln--;

        string error = "STUI layout document parsing error:\n\t" + err
			+ "\n\tat character " + to_string(off) + " (ln " + to_string(ln + 1) + ", col " + to_string(col + 1) + ")"
			+ "\n\t-> '..." + extract + "...'"\
			+ "\n\t" + string(7 + ((int32_t)off - extract_start), ' ') + "^"\
			+ "\n\tterminating parsing.";

        DEBUG_LOG(error);

        throw runtime_error(error);
    }
};

#pragma endregion STUI_LAYOUTREADER

///////////////////////////////////////////////////////////////////////
//                          IMPLEMENTATIONS
///////////////////////////////////////////////////////////////////////

#pragma region STUI_IMPLEMENTATIONS

#ifdef STUI_IMPLEMENTATION

LayoutReader::LayoutReader(vector<ComponentBuilder*(*)(void)> additional_builders)
{
    auto copy = additional_builders;
    copy.push_back(builder<LabelBuilder>);
    copy.push_back(builder<ButtonBuilder>);
    copy.push_back(builder<RadioButtonBuilder>);
    copy.push_back(builder<ToggleButtonBuilder>);
    copy.push_back(builder<TextInputBoxBuilder>);
    copy.push_back(builder<TextAreaBuilder>);
    copy.push_back(builder<ProgressBarBuilder>);
    copy.push_back(builder<SliderBuilder>);
    copy.push_back(builder<SpinnerBuilder>);
    copy.push_back(builder<VerticalBoxBuilder>);
    copy.push_back(builder<HorizontalBoxBuilder>);
    copy.push_back(builder<VerticalSpacerBuilder>);
    copy.push_back(builder<HorizontalSpacerBuilder>);
    copy.push_back(builder<BorderedBoxBuilder>);
    copy.push_back(builder<ListViewBuilder>);
    copy.push_back(builder<TreeViewBuilder>);
    copy.push_back(builder<ImageViewBuilder>);
    copy.push_back(builder<SizeLimiterBuilder>);
    copy.push_back(builder<TabDisplayBuilder>);
    copy.push_back(builder<BannerBuilder>);
    copy.push_back(builder<VerticalDividerBuilder>);
    copy.push_back(builder<HorizontalDividerBuilder>);
    copy.push_back(builder<QRCodeViewBuilder>);

    for (auto ptr : copy)
        registerBuilder(ptr());
}

void LayoutReader::registerBuilder(ComponentBuilder* builder)
{
    string name = builder->getName();
    if (builders.count(name) > 0)
    {
        delete builders[name];
        builders[name] = builder;
    }
    builders.emplace(name, builder);
}

Page* LayoutReader::readPageFromFile(string file)
{
    ifstream file_data(file, ifstream::ate);
    if (!file_data.is_open())
    {
        DEBUG_LOG("failed to load script file " + file);
        return nullptr;
    }

    vector<char> data;
    data.resize(file_data.tellg());
    file_data.seekg(ios::beg);
    file_data.read(data.data(), data.size());
    string file_content = string(data.data());

    file_data.close();

    DEBUG_LOG("loaded " + to_string(file_content.size()) + " chars of script from " + file);

    return readPage(file_content);
}

Page* LayoutReader::readPage(string content)
{
    vector<Token> tokens;
    try
    {
        tokens = tokenise(content);
    }
    catch (const runtime_error& e)
    {
        throw runtime_error(e.what());
        return nullptr;
    }

    vector<Token> pruned_tokens;
    for (Token t : tokens)
    {
        if (t.type == TokenType::NEWLINE || t.type == TokenType::COMMENT) continue;
        else pruned_tokens.push_back(t);
    }

    DEBUG_LOG("decoded " + to_string(tokens.size()) + " tokens total, pruned " + to_string(tokens.size() - pruned_tokens.size()) + " useless ones");

    if (pruned_tokens.size() < 3)
    {
        reportError("the LayoutScript file must contain at least one complete Component", 0, content);
    }

    if (pruned_tokens[0].type != TokenType::TEXT)
    {
        reportError("the LayoutScript file must begin with a Component definition", 0, content);
    }

    Page* page = new Page();

    try
    {
        Component* root = parseComponent(pruned_tokens, 0, content, page);
        page->setRoot(root);
    }
    catch (const runtime_error& e)
    {
        page->destroyAllComponents({ });
        delete page;

        throw runtime_error(e.what());
    }

    DEBUG_LOG("successfully built a UI tree of " + to_string(page->getAllComponents().size()) + " components");

    return page;
}

LayoutReader::~LayoutReader()
{
    for(auto pair : builders)
        delete pair.second;
}

vector<LayoutReader::Token> LayoutReader::tokenise(const string& content)
{
    size_t offset = 0;
    vector<Token> tokens;
    if (content.length() == 0) return tokens;

    string current_token = "";
    TokenType current_type = getType(content[0]);
    size_t start_offset = 0;
    if (current_type != TokenType::TEXT && current_type != TokenType::COMMENT && current_type != TokenType::WHITESPACE && current_type != TokenType::NEWLINE)
        reportError("invalid first token", offset, content);

    current_type = TokenType::WHITESPACE;

    while (offset < content.length())
    {
        char cur = content[offset];
        TokenType new_type = getType(cur);

        if (current_type == TokenType::STRING)
        {
            if (new_type == TokenType::STRING)
            {
                Token finished_token = Token(current_type);
                finished_token.start_offset = start_offset;
                finished_token.s_value = current_token;
                tokens.push_back(finished_token);

                start_offset = ++offset;
                current_token = "";
                current_type = TokenType::WHITESPACE;
                continue;
            }

            current_token += cur;
        }
        else if (current_type == TokenType::COMMENT)
        {
            if (new_type != TokenType::COMMENT && current_token.length() < 2)
                reportError("incomplete comment initiator", offset, content);
            else if (new_type == TokenType::NEWLINE)
            {
                Token finished_token = Token(TokenType::COMMENT);
                finished_token.start_offset = start_offset;
                finished_token.s_value = current_token.substr(2);
                tokens.push_back(finished_token);

                Token extra_token = Token(new_type);
                extra_token.start_offset = offset;
                tokens.push_back(extra_token);

                current_token = "";
                
                start_offset = offset + 1;
                current_type = TokenType::WHITESPACE;
            }
            else
            {
                current_token += cur;
            }
        }
        else if (current_type == TokenType::COORDINATE)
        {
            size_t comma;
            switch(new_type)
            {
                case INT:
                case COMMA:
                    current_token += cur;
                    break;
                case FLOAT:
                    reportError("coordinates may not be floating-point", offset, content);
                    break;
                case COMMENT:
                    reportError("incomplete coordinate token", offset, content);
                    break;
                case WHITESPACE:
                    break;
                case COORDINATE:
                    comma = current_token.find(',');
                    if (comma == string::npos)
                        reportError("incomplete coordinate token", offset, content);
                    else
                    {
                        string cx = current_token.substr(0, comma);
                        string cy = current_token.substr(comma + 1);
                        try
                        {
                            int co_x = stoi(cx);
                            int co_y = stoi(cy);

                            Token finished_token = Token(TokenType::COORDINATE);
                            finished_token.start_offset = start_offset;
                            finished_token.c_value = Coordinate{ co_x, co_y };
                            tokens.push_back(finished_token);

                            current_token = "";
                            current_type = TokenType::WHITESPACE;
                        }
                        catch(const exception& _)
                        {
                            reportError("invalid integer token within coordinate token", start_offset, content);
                        }
                    }
                    break;
                default:
                    reportError("invalid token within coordinate token", offset, content);
                    break;
            }
        }
        else if (new_type != current_type)
        {
            if ((current_type == TokenType::FLOAT && new_type == TokenType::INT) || (current_type == TokenType::INT && new_type == TokenType::FLOAT))
            {
                current_type = TokenType::FLOAT;
                current_token += cur;

                offset++;
                continue;
            }

            if (current_type != TokenType::WHITESPACE)
            {
                Token finished_token = Token(current_type);
                finished_token.start_offset = start_offset;
                switch(current_type)
                {
                    case TEXT:
                        if (new_type == TokenType::INT || new_type == TokenType::FLOAT || new_type == TokenType::STRING || new_type == TokenType::COORDINATE)
                            reportError("invalid conjoined token", offset, content);
                        finished_token.s_value = current_token;
                        break;
                    case INT:
                        if (new_type == TokenType::TEXT || new_type == TokenType::STRING || new_type == TokenType::COORDINATE)
                            reportError("invalid conjoined token", offset, content);
                        try
                        {
                            finished_token.i_value = stoi(current_token);
                        }
                        catch (const exception& _)
                        {
                            reportError("invalid int token", start_offset, content);
                        }
                        break;
                    case FLOAT:
                        if (new_type == TokenType::TEXT || new_type == TokenType::STRING || new_type == TokenType::COORDINATE)
                            reportError("invalid conjoined token", offset, content);
                        try
                        {
                            finished_token.f_value = stof(current_token);
                        }
                        catch (const exception& _)
                        {
                            reportError("invalid float token", start_offset, content);
                        }
                        break;
                    default:
                        reportError("invalid tokeniser state", offset, content);
                }
                tokens.push_back(finished_token);
            }

            start_offset = offset;
            current_token = "";
            if (!(new_type == TokenType::STRING || new_type == TokenType::COORDINATE || new_type == TokenType::WHITESPACE)) current_token += cur;

            Token extra_token = Token(new_type);
            switch(new_type)
            {
                case OPEN_ROUND:
                case CLOSE_ROUND:
                case OPEN_CURLY:
                case CLOSE_CURLY:
                case EQUALS:
                case COLON:
                case COMMA:
                case NEWLINE:
                    extra_token.start_offset = offset;
                    tokens.push_back(extra_token);

                    start_offset = offset + 1;
                    current_type = TokenType::WHITESPACE;
                    break;
                default:
                    current_type = new_type;
                    break;
            }
        }
        else if (current_type != TokenType::WHITESPACE)
        {
            current_token += cur;
        }

        offset++;
    }

    return tokens;
}

Component* LayoutReader::parseComponent(const vector<Token>& tokens, size_t start_index, const string& original_content, Page* page)
{
    if (start_index + 2 >= tokens.size())
        reportError("incomplete component definition", tokens[start_index].start_offset, original_content);
    
    if (tokens[start_index].type != TokenType::TEXT)
        reportError("initial token must be a component name", tokens[start_index].start_offset, original_content);

    string component_type_name = tokens[start_index].s_value;

    if (builders.count(component_type_name) == 0)
        reportError("unrecognised component type", tokens[start_index].start_offset, original_content);
    
    string component_nickname = "";
    bool has_name = false;
    if (tokens[start_index + 1].type == TokenType::COLON)
    {
        if (tokens[start_index + 2].type == TokenType::STRING)
        {
            component_nickname = tokens[start_index + 2].s_value;
            has_name = true;
        }
        else
            reportError("invalid token after component type", tokens[start_index + 2].start_offset, original_content);
    }
    
    size_t bracket_open_index = start_index + (has_name ? 3 : 1);

    if (bracket_open_index >= tokens.size() || tokens[bracket_open_index].type != TokenType::OPEN_ROUND)
        reportError("component type token name must be followed by either a bracket pair or a colon, a string name in quotes, and then a bracket pair", tokens[start_index].start_offset, original_content);

    if (bracket_open_index + 1 >= tokens.size())
        reportError("incomplete component definition", tokens[start_index].start_offset, original_content);

    size_t bracket_close_index = findClosingBrace(tokens, bracket_open_index, original_content);
    
    map<string, BuilderArgs::Argument> args;
    int arg_finder_state = 0; // 0 - reading name, 1 - looking for equals, 2 - looking for comma
    string current_arg_identifier = "";
    vector<Token> current_arg_value;
    vector<Token> bracket_stack;
    for (size_t index = bracket_open_index + 1; index < bracket_close_index; index++)
    {
        switch (arg_finder_state)
        {
            case 0:
                if (tokens[index].type == TokenType::TEXT)
                {
                    current_arg_identifier = tokens[index].s_value;
                    arg_finder_state = 1;
                }
                else
                    reportError("expected argument identifier", tokens[index].start_offset, original_content);
                break;
            case 1:
                if (tokens[index].type == TokenType::EQUALS)
                    arg_finder_state = 2;
                else
                    reportError("expected '=' following identifier", tokens[index].start_offset, original_content);
                break;
            case 2:
                if (tokens[index].type == TokenType::COMMA && bracket_stack.size() == 0)
                {
                    if (current_arg_value.size() == 0)
                        reportError("expected argument value after '='", tokens[index - 1].start_offset, original_content);
                    BuilderArgs::Argument arg = createArgument(current_arg_value, original_content, page);
                    args.emplace(current_arg_identifier, arg);
                    current_arg_value.clear();
                    current_arg_identifier = "";
                    arg_finder_state = 0;
                    break;
                }
                current_arg_value.push_back(tokens[index]);
                switch (tokens[index].type)
                {
                    case OPEN_ROUND:
                    case OPEN_CURLY:
                        bracket_stack.push_back(tokens[index]);
                        break;
                    case CLOSE_ROUND:
                        if (!bracket_stack.empty() && bracket_stack[bracket_stack.size() - 1].type == TokenType::OPEN_ROUND)
                            bracket_stack.pop_back();
                        else
                            reportError("invalid closing bracket", tokens[index].start_offset, original_content);
                        break;
                    case CLOSE_CURLY:
                        if (!bracket_stack.empty() && bracket_stack[bracket_stack.size() - 1].type == TokenType::OPEN_CURLY)
                            bracket_stack.pop_back();
                        else
                            reportError("invalid closing curly brace", tokens[index].start_offset, original_content);
                        break;
                    default: break;
                }
                break;
            default: break;
        }
    }
    if (arg_finder_state == 1)
        reportError("expected '=' following identifier", tokens[bracket_close_index - 1].start_offset, original_content);
    else if (arg_finder_state == 2)
    {
        if (current_arg_value.size() == 0)
            reportError("expected argument value after '='", tokens[bracket_close_index - 1].start_offset, original_content);
        BuilderArgs::Argument arg = createArgument(current_arg_value, original_content, page);
        args.emplace(current_arg_identifier, arg);
    }

    ComponentBuilder* comp_builder = builders[component_type_name];
    Component* component = comp_builder->build(BuilderArgs(args));

    page->registerComponent(component, component_nickname);

    return component;
}

BuilderArgs::Argument LayoutReader::createArgument(const vector<Token>& tokens, const string& original_content, Page* page)
{
    BuilderArgs::Argument arg = BuilderArgs::Argument();

    vector<BuilderArgs::Argument> array_elements;
    vector<Token> current_tokens;
    vector<Token> bracket_stack;
    TokenType array_token_type;
    int array_splitter_state;
    size_t end_bracket_index;

    switch (tokens[0].type)
    {
        case INT:
            if (tokens.size() > 1)
                reportError("unexpected token(s) after int", tokens[1].start_offset, original_content);
            arg.type = BuilderArgs::ArgType::INT;
            arg.i_value = tokens[0].i_value;
            break;
        case FLOAT:
            if (tokens.size() > 1)
                reportError("unexpected token(s) after float", tokens[1].start_offset, original_content);
            arg.type = BuilderArgs::ArgType::FLOAT;
            arg.f_value = tokens[0].f_value;
            break;
        case STRING:
            if (tokens.size() > 1)
                reportError("unexpected token(s) after string", tokens[1].start_offset, original_content);
            arg.type = BuilderArgs::ArgType::STRING;
            arg.s_value = tokens[0].s_value;
            break;
        case COORDINATE:
            if (tokens.size() > 1)
                reportError("unexpected token(s) after coordinate", tokens[1].start_offset, original_content);
            arg.type = BuilderArgs::ArgType::COORDINATE;
            arg.coo_value = tokens[0].c_value;
            break;
        case OPEN_CURLY:
            end_bracket_index = findClosingBrace(tokens, 0, original_content);
            if (end_bracket_index != tokens.size() - 1)
                reportError("unexpected token(s) after closing curly brace", tokens[end_bracket_index].start_offset, original_content);
            if (end_bracket_index == 1)
                reportError("empty arrays are not permitted", tokens[1].start_offset, original_content);

            BuilderArgs::ArgType array_arg_type;
            array_splitter_state = 0; // 0 - looking for first token of element, 1 - looking for comma

            for (size_t index = 1; index < end_bracket_index; index++)
            {
                if (array_elements.empty() && current_tokens.empty())
                {
                    array_token_type = tokens[index].type;
                    switch (tokens[index].type)
                    {
                        case INT: array_arg_type =  BuilderArgs::ArgType::INT_ARRAY; break;
                        case FLOAT: array_arg_type =  BuilderArgs::ArgType::FLOAT_ARRAY; break;
                        case STRING: array_arg_type =  BuilderArgs::ArgType::STRING_ARRAY; break;
                        case COORDINATE: array_arg_type =  BuilderArgs::ArgType::COORDINATE_ARRAY; break;
                        case TEXT: array_arg_type =  BuilderArgs::ArgType::COMPONENT_ARRAY; break;
                        default:
                            reportError("invalid first token in array", tokens[index].start_offset, original_content);
                    }
                }

                if (array_splitter_state == 0)
                {
                    if (tokens[index].type != array_token_type)
                        reportError("arrays may only contain one type of data", tokens[index].start_offset, original_content);
                    current_tokens.push_back(tokens[index]);
                    array_splitter_state = 1;
                }
                else if (array_splitter_state == 1)
                {
                    if (tokens[index].type == TokenType::COMMA && bracket_stack.size() == 0)
                    {
                        BuilderArgs::Argument arg2 = createArgument(current_tokens, original_content, page);
                        array_elements.push_back(arg2);
                        array_splitter_state = 0;
                        current_tokens.clear();
                    }
                    else
                    {
                        current_tokens.push_back(tokens[index]);
                        switch (tokens[index].type)
                        {
                            case OPEN_ROUND:
                            case OPEN_CURLY:
                                bracket_stack.push_back(tokens[index]);
                                break;
                            case CLOSE_ROUND:
                                if (!bracket_stack.empty() && bracket_stack[bracket_stack.size() - 1].type == TokenType::OPEN_ROUND)
                                    bracket_stack.pop_back();
                                else
                                    reportError("invalid closing bracket", tokens[index].start_offset, original_content);
                                break;
                            case CLOSE_CURLY:
                                if (!bracket_stack.empty() && bracket_stack[bracket_stack.size() - 1].type == TokenType::OPEN_CURLY)
                                    bracket_stack.pop_back();
                                else
                                    reportError("invalid closing curly brace", tokens[index].start_offset, original_content);
                                break;
                            default: break;
                        }
                    }
                }
            }

            if (array_splitter_state == 1)
            {
                BuilderArgs::Argument arg2 = createArgument(current_tokens, original_content, page);
                array_elements.push_back(arg2);
            }

            arg.type = array_arg_type;
            switch (array_arg_type)
            {
                case BuilderArgs::ArgType::INT_ARRAY:
                    for (BuilderArgs::Argument a : array_elements)
                        arg.ai_value.push_back(a.i_value);
                    break;
                case BuilderArgs::ArgType::FLOAT_ARRAY:
                    for (BuilderArgs::Argument a : array_elements)
                        arg.af_value.push_back(a.f_value);
                    break;
                case BuilderArgs::ArgType::STRING_ARRAY:
                    for (BuilderArgs::Argument a : array_elements)
                        arg.as_value.push_back(a.s_value);
                    break;
                case BuilderArgs::ArgType::COORDINATE_ARRAY:
                    for (BuilderArgs::Argument a : array_elements)
                        arg.acoo_value.push_back(a.coo_value);
                    break;
                case BuilderArgs::ArgType::COMPONENT_ARRAY:
                    for (BuilderArgs::Argument a : array_elements)
                        arg.acom_value.push_back(a.com_value);
                    break;
                default:
                    reportError("invalid argument decoder state", tokens[0].start_offset, original_content);
            }
            break;
        case TEXT:
            arg.type = BuilderArgs::ArgType::COMPONENT;
            arg.com_value = parseComponent(tokens, 0, original_content, page);
            break;
        default:
            reportError("invalid token(s) after '='", tokens[0].start_offset, original_content);
            break;
    }

    return arg;
}

size_t LayoutReader::findClosingBrace(const vector<Token>& tokens, size_t open_index, const string& original_content)
{
    vector<Token> brackets;
    size_t index = open_index;

    while (index < tokens.size())
    {
        switch(tokens[index].type)
        {
            case OPEN_ROUND:
            case OPEN_CURLY:
                brackets.push_back(tokens[index]);
                break;
            case CLOSE_ROUND:
                if (!brackets.empty() && brackets[brackets.size() - 1].type == TokenType::OPEN_ROUND)
                    brackets.pop_back();
                else
                    reportError("invalid closing bracket", tokens[index].start_offset, original_content);
                break;
            case CLOSE_CURLY:
                if (!brackets.empty() && brackets[brackets.size() - 1].type == TokenType::OPEN_CURLY)
                    brackets.pop_back();
                else
                    reportError("invalid closing curly brace", tokens[index].start_offset, original_content);
                break;
            default:
                break;
        }

        if (brackets.size() == 0)
            break;

        index++;
    }

    if (index >= tokens.size())
        reportError("missing closing " + string(tokens[open_index].type == TokenType::OPEN_ROUND ? "bracket" : "curly brace"), tokens[open_index].start_offset, original_content);

    return index;
}

#endif

#pragma endregion STUI_IMPLEMENTATIONS

}

#endif

///////////////////////////////////////////////////////////////////////
//                           UNDEFINES
///////////////////////////////////////////////////////////////////////

#pragma region STUI_UNDEFS

#ifndef STUI_KEEP_DEFINES

#undef GETNAME_STUB
#undef BUILD_STUB

#endif

#pragma endregion STUI_UNDEFS