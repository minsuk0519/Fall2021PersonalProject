#pragma once

//standard library
#include <vector>
#include <fstream>
#include <chrono>

namespace Helper
{
	static std::vector<char> readFile(const std::string filename)
	{
		//ate : start reading at the end of the file
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		
		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	//return millisecond, true->reset timer
	static float GetDeltaTime(bool timestamp = false)
	{
		static auto prev = std::chrono::high_resolution_clock::now();
		static auto current = std::chrono::high_resolution_clock::now();

		current = std::chrono::high_resolution_clock::now();
		
		float result = (float)(std::chrono::duration<double, std::milli>(current - prev).count());

		if(timestamp) prev = current;

		return result;
	}
}