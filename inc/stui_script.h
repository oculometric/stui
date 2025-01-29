/*  
	A simple header-only library for creating text-based user interfaces
	inside a terminal window.
	Copyright (C) 2024  Jacob Costen (oculometric)

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

#pragma once

#define STUI_KEEP_DEFINES
#include "stui.h"
#include "stui_extensions.h"
#undef STUI_KEEP_DEFINES

#include <iostream>
#include <fstream>

namespace stui
{

class ComponentBuilder
{
// TODO:
};

class LayoutReader
{
private:
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

public:
    static Page* readPage(string file);

private:
    static vector<Token> tokenise(const string content);

    static inline bool isAlphabetic(char c)
	{
		if (c >= 'a' && c <= 'z') return true;
		if (c >= 'A' && c <= 'Z') return true;

		return false;
	}

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

    static inline void reportError(const string err, size_t off, const string& str)
    {
        int32_t extract_start = max(0, (int32_t)off - 16);
        int32_t extract_end = extract_start + 32;
        while (true)
        {
            size_t find = str.find('\n', extract_start);
            if (find > off) break;
            extract_start = find + 1;
        }
        size_t find = str.find('\n', off);
        if (find != string::npos)
        {
            if ((int32_t)find < extract_end)
            extract_end = find;
        }
        string extract = str.substr(extract_start, extract_end - extract_start);

        throw runtime_error("STUI format document parsing error:\n\t" + err
			+ "\n\tat character " + to_string(off)
			+ "\n\t-> '..." + extract + "..."\
			+ "\n\t" + string(7 + ((int32_t)off - extract_start), ' ') + "^"\
			+ "\n\tterminating parsing.");
    }
};

#ifdef STUI_IMPLEMENTATION

Page* LayoutReader::readPage(string file)
{
    cout << "here" << endl;
    ifstream file_data(file, ifstream::ate);
    cout << "here2" << endl;
    if (!file_data.is_open())
    {
    cout << "nofile" << endl;

        return nullptr;
    }
    cout << "read" << endl;

    string file_content = "";
    file_content.resize(file_data.tellg());
    file_data.seekg(ios::beg);
    file_data.read(file_content.data(), file_content.size());

    file_data.close();
    cout << file_content << endl;

    vector<Token> tokens = tokenise(file_content);

    return nullptr;
    // some code
}

vector<LayoutReader::Token> LayoutReader::tokenise(const string content)
{
    size_t offset = 0;
    vector<Token> tokens;
    if (content.length() == 0) return tokens;

    string current_token = "";
    TokenType current_type = getType(content[0]);
    size_t start_offset = 0;
    if (current_type != TokenType::TEXT && current_type != TokenType::COMMENT)
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

#undef PARSE_ERROR

#endif

}