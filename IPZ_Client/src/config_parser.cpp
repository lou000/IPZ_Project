#pragma once
#include "config_parser.h"


template <typename T>
void ConfigParser::writeNumber(struct attributes const& a, std::ofstream* file)
{
	*file << "\t" << a.attributeName << ": " << std::any_cast<T>(a.attributeValue) << "\n";
}

template <typename T>
void ConfigParser::writeString(struct attributes const& a, std::ofstream* file)
{
	*file << "\t" << a.attributeName << ": " << "\"" << std::any_cast<T>(a.attributeValue) << "\"\n";
}

template <typename T>
void ConfigParser::writeBool(struct attributes const& a, std::ofstream* file)
{
	*file << "\t" << a.attributeName << ": ";
	if (std::any_cast<T>(a.attributeValue))
		*file << "true\n";
	else
		*file << "false\n";
}

void ConfigParser::writeVec(struct attributes const& a, std::ofstream* file)
{
	if (auto it = Actions.find(a.attributeType); it != Actions.end())
	{
		it->second(a, file);
		return;
	}
}

void ConfigParser::writeConfigFile(std::map<std::string, std::vector<attributes>> myMap)
{
	std::ofstream file;

	file.open("example.txt");
	if (file.fail())
	{
		// TO DO
	}

	for (auto it = myMap.begin(); it != myMap.end(); ++it) {
		file << "#" << it->first << "\n";

		for (const auto& value : it->second) {
			writeVec(value, &file);
		}
		file << "\n";
	}

	file.close();
}

int ConfigParser::lineLength(char* start)
{
	int count = 0;
	while (start[0] != '\n')
	{
		start++;
		count++;
	}
	return count;
}

char* ConfigParser::nextLine(char* start, char* end)
{
	while (start[0] != '\n' && start != end)
		start++;
	start++;
	return start;   // pointer to next line
}

bool ConfigParser::isFirstLine(char* start)
{
	if (start[0] == '#')
		return true;
	return false;
}

std::string ConfigParser::extractString(char* beginning, char* end)
{
	std::string temp(beginning, end);
	return temp;
}

std::string ConfigParser::getObjectName(char* ptr)
{
	return 	ConfigParser::extractString(ptr + 1, ptr + ConfigParser::lineLength(ptr) - 1);
}

struct attributes ConfigParser::getAttribute(char* beginning)
{
	struct attributes attribute {};
	int len = lineLength(beginning);
	std::string strValue;
	std::string::size_type sz;     // alias of size_t
	std::string type; // TO DO  or enum

	std::any tempAny;
	//TO DO extract object from first line with ## ro whatever
	char* ptr = beginning + 1;
	char* beg = beginning + 1;
	int count = 0;
	while (ptr[0] != ':')
	{
		ptr++;
		count++;
	}
	attribute.attributeName = extractString(beg, ptr);

	ptr += 2;

	if (ptr[0] == '"')
	{
		strValue = extractString(ptr + 1, beginning + lineLength(beginning) - 2);
		//tempAny = std::make_any<std::string>(strValue);
		attribute.attributeValue = std::make_any<std::string>(strValue);
		attribute.attributeType = types::String;
	}
	else
	{
		char* ptrEnd = beginning + lineLength(beginning) - 1;
		int a = 2;
		strValue = extractString(ptr, ptrEnd);
		if (std::isalpha(strValue[0]))
		{
			if (strValue[0] == 't')
				attribute.attributeValue = std::make_any<bool>(true);
			//tempAny = std::make_any<bool>(true);

			else if (strValue[0] == 'f')
				attribute.attributeValue = std::make_any<bool>(false);
			//tempAny = std::make_any<bool>(false);
			attribute.attributeType = types::Bool;
		}
		else if (strValue.find(".") == -1)
		{
			int tempInt = std::stoi(strValue, &sz);
			attribute.attributeValue = std::make_any<int>(tempInt);
			//tempAny = std::make_any<int>(tempInt);
			attribute.attributeType = types::Int;
		}
		else
		{
			float tempFloat = std::stof(strValue, &sz);
			attribute.attributeValue = std::make_any<float>(tempFloat);
			//tempAny = std::make_any<float>(tempFloat);
			attribute.attributeType = types::Float;
		}
	}

	return attribute;
}

void ConfigParser::readConfigFile(std::map<std::string, std::vector<attributes>>* myMap)
{
	//std::ifstream file;

	//file.open("example.txt");
	//if (file.fail())
	//{
	//	// TO DO
	//}
	//file.close();
	//std::map<std::string, std::vector<attributes>> myMap;
	std::ifstream is("example.txt", std::ifstream::binary);  // poprawic jakos na wzor wyzej
	if (is) {
		// get length of file:
		is.seekg(0, is.end);
		int length = is.tellg();
		is.seekg(0, is.beg);
		std::cout << "TEST LLENGTH OF FILE CHRS READ: " << length << "\n";
		// allocate memory:
		char* buffer = new char[length];

		// read data as a block:
		is.read(buffer, length);

		is.close();

		std::string objectName;
		std::vector<attributes> attributesVec;
		attributes att;

		char* end = buffer + length;
		char* beginning = buffer;

		int limiter = 0;
		while (limiter < length) {
			int l = lineLength(beginning);
			//if (isFirstLine(beginning))
			//{
			objectName = getObjectName(beginning);
			beginning += l + 1;
			limiter += l + 1;
			//}
			//else
			//{
			l = lineLength(beginning);
			while (l != 1)
			{
				attributesVec.push_back(getAttribute(beginning));
				beginning += l + 1;
				limiter += l + 1;
				l = lineLength(beginning);
			}
			//}

			myMap->insert({ objectName, attributesVec });
			attributesVec.clear();
			objectName.clear();
			beginning = nextLine(beginning, beginning + length);
			limiter += l + 1;
		}

		delete[] buffer;
	}
}