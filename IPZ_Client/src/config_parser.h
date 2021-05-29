#pragma once

#include <iostream>
#include <map>
#include <any>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <functional>

enum class types
{
	String, Int, Float, Bool
};

struct attributes
{
	types attributeType;
	std::string attributeName;
	std::any	attributeValue;
};

class ConfigParser {
	ConfigParser() {};

public:

	std::map<std::string, std::vector<attributes>> myMap;
	static std::map<types, std::function<void(struct attributes const&, std::ofstream* file)>> Actions;

	template <typename T>
	static void writeNumber(struct attributes const& a, std::ofstream* file);

	template <typename T>
	static void writeString(struct attributes const& a, std::ofstream* file);

	template <typename T>
	static void writeBool(struct attributes const& a, std::ofstream* file);

	static void writeVec(struct attributes const& a, std::ofstream* file);

	static int lineLength(char* start);

	static char* nextLine(char* start, char* end);

	static bool isFirstLine(char* start);

	static std::string extractString(char* beginning, char* end);

	static std::string getObjectName(char* ptr);

	static struct attributes getAttribute(char* beginning);

	static void writeConfigFile(std::map<std::string, std::vector<attributes>> myMap);
	static void readConfigFile(std::map<std::string, std::vector<attributes>>* myMap);
};

std::map<types, std::function<void(struct attributes const&, std::ofstream* file)>> ConfigParser::Actions = {
	{ types::Float,				writeNumber<float> },
	{ types::Int,				writeNumber<int> },
	{ types::Bool,				writeBool<bool> },
	{ types::String,			writeString<std::string> },
};