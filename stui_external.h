#pragma once

#include "stui.h"
#include <map>

namespace stui
{

static void reportError(string input, size_t char_index, string summary)
{
	throw runtime_error("STUI format document parsing error:\n\t" + summary 
														 + "\n\tat character " + to_string(char_index) 
														 + "\n\t-> '..." + input.substr(max(0, (int32_t)char_index - 16), 32) + "...'"
														 + "\n\t" + string(min(16, char_index) + 7, ' ') + "^"
														 + "\n\tterminating parsing.");
}

// TODO: implement quotation escape sequence (strip, extract, matching closing brace, split comma list)
static string stripNonCoding(string input)
{
	string result;

	bool is_inside_string = false;
	bool is_inside_line_comment = false;
	bool is_inside_star_comment = false;
	for (size_t i = 0; i < input.size(); i++)
	{
		if (is_inside_line_comment && input[i] == '\n')
			is_inside_line_comment = false;
		else if (is_inside_star_comment && i > 0 && input[i-1] == '*' && input[i] == '/')
		{
			is_inside_star_comment = false;
			i++;
		}
		else if (!is_inside_string && !is_inside_star_comment && i < input.size() - 1 && input[i] == '/' && input[i + 1] == '/')
			is_inside_line_comment = true;
		else if (!is_inside_string && i < input.size() - 1 && input[i] == '/' && input[i + 1] == '*')
		{
			is_inside_line_comment = false;
			is_inside_star_comment = true;
		}

		if (!is_inside_line_comment && !is_inside_star_comment)
		{
			if (is_inside_string) result.push_back(input[i]);
			else if (!(input[i] == ' ' || input[i] == '\t' || input[i] == '\n'))
			{
				result.push_back(input[i]);
			}

			if (input[i] == '"') is_inside_string = !is_inside_string;
		}
	}

	return result;
}

static string extractString(string input, size_t& first_char)
{
	if (input[first_char] != '"')
		reportError(input, first_char, "character is not the start of a string block");

	size_t end = input.find('"', first_char + 1);
	if (end == string::npos)
		reportError(input, first_char, "unclosed quotation mark");
	
	string substr = input.substr(first_char + 1, (end - first_char - 1));
	first_char = end + 1;

	return substr;
}

static size_t findMatchingClosingBrace(string input, size_t opening_brace)
{
	vector<char> bracket_history;

	bool is_in_string = false;
	size_t i = opening_brace;
	char outermost = input[i];
	if (outermost != '{' && outermost != '(') return opening_brace;
	while (i < input.size())
	{
		if (!is_in_string)
		{
			if (input[i] == '{')
				bracket_history.push_back('{');
			else if (input[i] == '(')
				bracket_history.push_back('(');
			else if (input[i] == ')')
			{
				if (bracket_history[bracket_history.size() - 1] == '(')
					bracket_history.pop_back();
				else
					reportError(input, i, "inappropriate closing round bracket");
			}
			else if (input[i] == '}')
			{
				if (bracket_history[bracket_history.size() - 1] == '{')
					bracket_history.pop_back();
				else
					reportError(input, i, "inappropriate closing curly brace");
			}
		}
		
		if (input[i] == '"') is_in_string = !is_in_string;

		if (bracket_history.size() == 0) break;

		i++;
	}

	if (i >= input.size())
		reportError(input, opening_brace, "missing closing brace");

	return i;
}

// TODO: fix broken error indices in functions like this
static vector<pair<size_t, string>> splitCommaSeparatedList(string input)
{
	bool is_inside_string = false;
	int curly_brace_depth = 0;
	int round_brace_depth = 0;
	int angle_brace_depth = 0;

	vector<pair<size_t, string>> result;
	string current;

	size_t last = 0;
	size_t i = 0;
	while (i < input.size())
	{
		current += input[i];

		if (!is_inside_string && curly_brace_depth == 0 && round_brace_depth == 0 && angle_brace_depth == 0 && input[i] == ',')
		{
			current.pop_back();
			result.push_back(pair<size_t, string>(last, current));
			current = "";
			last = i + 1;
			i++;
			continue;
		}

		if (input[i] == '"')
		{
			is_inside_string = !is_inside_string;
		}
		else if (!is_inside_string)
		{
			switch (input[i])
			{
			case '{': curly_brace_depth++; break;
			case '}': curly_brace_depth--; break;
			case '(': round_brace_depth++; break;
			case ')': round_brace_depth--; break;
			case '[': angle_brace_depth++; break;
			case ']': angle_brace_depth--; break;
			}
		}

		i++;
	}

	result.push_back(pair<size_t, string>(last, current));

	if (i == input.size())
	{
		if (curly_brace_depth != 0 || round_brace_depth != 0 || angle_brace_depth != 0)
			reportError(input, 0, "illegal brackets found in comma separated list");
		if (is_inside_string)
			reportError(input, 0, "dangling string block in comma separated list");
	}

	return result;
}

static bool isAlphabetic(char c)
{
	if (c >= 'a' && c <= 'z') return true;
	if (c >= 'A' && c <= 'Z') return true;

	return false;
}

static Component* decodeComponentString(string input)
{
	bool has_name = true;
	size_t i = 0;
	while (input[i] != ':')
	{
		if (i >= input.size() || input[i] == '"') reportError(input, i, "unable to find name separator or paramter list");

		if (input[i] == '(') { has_name = false; break; }

		i++;
	}

	string name;
	if (has_name)
	{
		i++;
		name = extractString(input, i);
		if (name.length() == 0) name = "_component";
	}
	else
	{
		name = "_component";
	}

	if (input[i] != '(') reportError(input, i, "unable to find parameter list");

	size_t j = findMatchingClosingBrace(input, i);

	if (i == j) reportError(input, i, "parameter list not enclosed");
	
	vector<pair<size_t, string>> split_params = splitCommaSeparatedList(input.substr(i + 1, j - i - 1));

	enum ArgumentType
	{
		INT,
		STRING,
		COORDINATE,
		FLOAT,
		COMPONENT,
		INT_ARRAY,
		STRING_ARRAY,
		COORDINATE_ARRAY,
		FLOAT_ARRAY,
		COMPONENT_ARRAY
	};

	struct Argument
	{
		ArgumentType type;
		long long int int_val;
		string string_val;
		Coordinate coordinate_val;
		float float_val;
		Component* component_val;
		vector<long long int> int_arr;
		vector<string> string_arr;
		vector<Coordinate> coordinate_arr;
		vector<float> float_arr;
		vector<Component*> component_arr;
	};

	vector<Argument> arguments;
	for (pair<size_t, string> p : split_params)
	{
		if (p.second.length() < 1)
			reportError(input, p.first + i + 1, "malformed element in comma separated list");

		Argument arg;
		if (p.second[0] == '"') arg.type = ArgumentType::STRING;
		else if (p.second[0] == '[') arg.type = ArgumentType::COORDINATE;
		else if ((p.second[0] >= '0' && p.second[0] <= '9') || p.second[0] == '-')
		{
			if (p.second.find('.') != string::npos)
				arg.type = ArgumentType::FLOAT;
			else
				arg.type = ArgumentType::INT;
		}
		else if (p.second[0] == '{')
		{
			if (p.second.length() < 2) reportError(input, p.first + i + 1, "malformed array argument");
			if (p.second[1] == '{') reportError(input, p.first + i + 1, "nested arrays are not allowed");
			else if (p.second[1] == '}') reportError(input, p.first + i + 1, "empty arrays are not allowed");
			else if (p.second[1] == '"') arg.type = ArgumentType::STRING_ARRAY;
			else if (p.second[1] == '[') arg.type = ArgumentType::COORDINATE_ARRAY;
			else if ((p.second[1] >= '0' && p.second[1] <= '9') || p.second[1] == '-')
			{
				if (p.second.find('.') != string::npos)
					arg.type = ArgumentType::FLOAT_ARRAY;
				else
					arg.type = ArgumentType::INT_ARRAY;
			}
			else if (isAlphabetic(p.second[1])) arg.type = ArgumentType::COMPONENT_ARRAY;
			else reportError(input, p.first + i + 1, "invalid array type");
		}
		else if (isAlphabetic(p.second[0])) arg.type = ArgumentType::COMPONENT;
		else reportError(input, p.first + i + 1, "invalid argument type");

		// TODO: here - get the actual value of each argument
	}
	// TODO: here - construct the component

	return nullptr;
}

// TODO: all of this
class PageManager
{
private:
	vector<Component*> pages;
	map<string, Component*> all_components;

public:
	inline PageManager();
	inline PageManager(string formating_file);

	inline bool ensureIntegrity();
	inline bool readFromFile(string formatting_file);
	inline bool writeToFile(string formatting_file);

	inline Component*& operator[](size_t page);
	inline Component*& operator[](string identifier);

	inline void registerComponent(Component* component, string identifier);
	inline void addPage(Component* page_root);
	inline void unregisterComponent(string identifier);
	inline void removePage(size_t page_index);

	inline ~PageManager();
private:
	inline bool isNameUnique(string name);
	inline string getUniqueName();
};

}