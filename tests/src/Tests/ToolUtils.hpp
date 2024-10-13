#pragma once

#ifndef NAZARA_UNITTESTS_TOOLS_TOOLUTILS_HPP
#define NAZARA_UNITTESTS_TOOLS_TOOLUTILS_HPP

#include <filesystem>
#include <string>

void CheckHeaderMatch(const std::filesystem::path& originalFilepath);
void ExecuteCommand(const std::string& command, const std::string& pattern = {}, std::string expectedOutput = {});

#endif
