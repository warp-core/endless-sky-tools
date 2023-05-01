/* color-converter.cpp
Copyright (c) 2023 by warp-core

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

// color-converter: program to convert between the fractional rgb color values used
// by Endless Sky and 24 bit HTML color codes.
// $ g++ --std=c++11 -o color-converter color-converter.cpp
// $ ./color-converter --es-to-hex <file>
// Reads Endless Sky colors from the file at the given path and prints to STDOUT
// a list of the parsed colors with values given as 24 bit hexadecimal HTML color codes.
// $ ./color-converter --hex-to-es <file>
// Reads 24 bit hexadecimal HTML colors from the file at the given path and prints
// to STDOUT a list of the parsed colors in the Endless Sky color format.
// Endless Sky format: color <name> <r#> <g#> <b#> [<a#>]
// 24 bit hex format: color <name> #<0xrr><0xgg><0xbb>
// $ ./color-converter <r#> <g#> <b#> [<a#>]
// Converts the given Endless Sky color to HTML format.
// $ ./color-converter #<0xrr><0xgg><0xbb>
// Converts the given HTML color to Endless Sky format.

#include "shared/DataFile.h"
#include "shared/DataNode.h"
#include "shared/DataWriter.h"

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace std;



void PrintHelp();
void ESFileToHTML(string path);
void HTMLFileToES(string path);
string ESToHTML(const DataNode &node, int index = 0);
vector<double> HTMLToES(const string &hex);
string ToHex(int val);



int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		PrintHelp();
		return 1;
	}

	for(char **it = argv + 1; *it; ++it)
	{
		string arg = *it;
		if(arg == "--es-to-hex" || arg == "--hex-to-es")
		{
			++it;
			if(!*it)
			{
				cout << "Error: expected additional argument:\n\n";
				PrintHelp();
				return 1;
			}
			if(arg == "--es-to-hex")
				ESFileToHTML(*it);
			else if(arg == "--hex-to-es")
				HTMLFileToES(*it);
			return 0;
		}
		else if(arg[0] == '#')
		{
			int index = 0;
			for(double it : HTMLToES(arg))
			{
				if(index++)
					cout << ' ';
				cout << it;
			}
			return 0;
		}
	}

	if(argc > 3)
	{
		vector<DataNode> data = DataNode::AsDataNodes(*argv);
		cout << ESToHTML(data[0], 1);
		return 0;
	}

	return 2;
}



void PrintHelp()
{
	cout << "--es-to-hex <file>: Reads Endless Sky colors from the file at the given path and prints to STDOUT"
			"a list of the parsed colors with values given as 24 bit hexadecimal HTML color codes." << endl;
	cout << "--hex-to-es <file>: Reads 24 bit hexadecimal HTML colors from the file at the given path and prints"
			"to STDOUT a list of the parsed colors in the Endless Sky color format." << endl;
	cout << "Endless Sky format: color <name> <r#> <g#> <b#> [<a#>]" << endl;
	cout << "24 bit hex format: color <name> #<0xrr><0xgg><0xbb>" << endl;
	cout << "<r#> <g#> <b#> [<a#>]: Converts the given Endless Sky color to HTML format." << endl;
	cout << "#<0xrr><0xgg><0xbb>: Converts the given HTML color to Endless Sky format." << endl;
}



void ESFileToHTML(string path)
{
	vector<pair<string, string>> results;

	DataFile in(path);
	for(const DataNode &node : in)
	{
		if(node.Token(0) == "color" && node.Size() >= 5)
		{
			results.emplace_back(node.Token(1), ESToHTML(node, 2));
		}
	}

	for(const auto &it : results)
		cout << "\"" << it.first << "\" " << it.second << '\n';
}



void HTMLFileToES(string path)
{
	vector<pair<string, vector<double>>> results;

	DataFile in(path);

	for(const DataNode &node : in)
	{
		if(node.Token(0) == "color" && node.Size() >= 3)
		{
			results.emplace_back(node.Token(1), HTMLToES(node.Token(2)));
		}
	}

	for(const auto &it : results)
	{
		cout << "color" << '"' << it.first << '"';
		for(const auto &val : it.second)
		{
			cout << ' ' << val;
		}
		cout << '\n';
	}
}



string ESToHTML(const DataNode &node, int index)
{
	string result = "#";

	for(int i = 0; i < 3; ++i)
	{
		int val = node.Value(index + i) * 255;
		val = min(255, val);
		val = max(0, val);
		result += ToHex(val);
	}

	return result;
}



vector<double> HTMLToES(const string &hex)
{
	auto HexToDec = [](const string input) -> int
	{
		int multiplier = 1;
		int result = 0;

		for(auto it = input.rbegin(); it != input.rend(); ++it, multiplier *= 16)
		{
			static const vector<pair<char, char>> converter = {
				{'0', '9'},
				{'A', 'F'},
				{'a', 'f'}
			};

			for(const auto &bounds : converter)
			{
				if(*it >= bounds.first && *it <= bounds.second)
				result += (*it - bounds.first) * multiplier;
			}
		}

		return result;
	};

	vector<double> result;

	if(hex[0] != '#')
		return result;
	if(hex.length() < 7)
		return result;

	for(int i = 1; i < 6; i += 2)
	{
		result.push_back(HexToDec(hex.substr(i, 2)));
		result.back() /= 255;
	}

	return result;
}



string ToHex(int val)
{
	static const vector<char> DecToHex = {
		'0',
		'1',
		'2',
		'3',
		'4',
		'5',
		'6',
		'7',
		'8',
		'9',
		'A',
		'B',
		'C',
		'D',
		'E',
		'F'
	};

	if(val >= 16)
		return DecToHex[val / 16] + ToHex(val % 16);

	string result;
	result += DecToHex[val];
	return result;
}
