#pragma once

#include <deque>
#include <string>
#include <vector>
#include <glm.hpp>

class Utility
{

public:

	static void ParseString(std::string& string,
		std::vector<std::string>& subStrings, char token);

	static void AddMessage(const std::string& message);
	static std::string ReadMessage();

	struct Ray{
		glm::vec3 origin;
		glm::vec3 direction;
	};

private:

	static std::deque<std::string> s_messages;

};