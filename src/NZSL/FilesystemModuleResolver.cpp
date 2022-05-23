// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/FilesystemModuleResolver.hpp>
#include <NZSL/ShaderLangParser.hpp>
#include <NZSL/Ast/AstSerializer.hpp>
#include <efsw/efsw.h>
#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>

namespace nzsl
{
	FilesystemModuleResolver::FilesystemModuleResolver()
	{
		m_fileWatcher = efsw_create(0);
		efsw_watch(m_fileWatcher);
	}

	FilesystemModuleResolver::~FilesystemModuleResolver()
	{
		if (m_fileWatcher)
			efsw_release(m_fileWatcher);
	}

	void FilesystemModuleResolver::RegisterModule(const std::filesystem::path& realPath)
	{
		Ast::ModulePtr module;
		try
		{
			std::string ext = realPath.extension().generic_u8string();
			if (ext == CompiledModuleExtension)
			{
				std::ifstream inputFile(realPath, std::ios::in | std::ios::binary);
				if (!inputFile)
					throw std::runtime_error("failed to open " + realPath.generic_u8string());

				inputFile.seekg(0, std::ios::end);

				std::streamsize length = inputFile.tellg();

				inputFile.seekg(0, std::ios::beg);

				std::vector<char> content(length);
				if (length > 0 && !inputFile.read(&content[0], length))
					throw std::runtime_error("failed to read " + realPath.generic_u8string());

				Unserializer unserializer(content.data(), content.size());
				module = Ast::UnserializeShader(unserializer);
			}
			else if (ext == ModuleExtension)
				module = ParseFromFile(realPath);
			else
				throw std::runtime_error("unknown extension " + ext);
		}
		catch (const std::exception& e)
		{
			std::cerr << "failed to register module from file " << realPath.generic_u8string() << ": " << e.what() << std::endl;
			return;
		}

		if (!module)
			return;

		std::string moduleName = module->metadata->moduleName;
		RegisterModule(std::move(module));

		std::filesystem::path canonicalPath = std::filesystem::canonical(realPath);
		m_moduleByFilepath.emplace(canonicalPath.generic_u8string(), std::move(moduleName));
	}

	void FilesystemModuleResolver::RegisterModule(std::string_view moduleSource)
	{
		Ast::ModulePtr module = Parse(moduleSource);
		if (!module)
			return;

		return RegisterModule(std::move(module));
	}

	void FilesystemModuleResolver::RegisterModule(Ast::ModulePtr module)
	{
		assert(module);

		std::string moduleName = module->metadata->moduleName;
		if (moduleName.empty())
			throw std::runtime_error("cannot register anonymous module");

		auto it = m_modules.find(moduleName);
		if (it != m_modules.end())
		{
			it->second = std::move(module);

			OnModuleUpdated(this, moduleName);
		}
		else
			m_modules.emplace(std::move(moduleName), std::move(module));
	}

	void FilesystemModuleResolver::RegisterModuleDirectory(const std::filesystem::path& realPath, bool watchDirectory)
	{
		if (!std::filesystem::is_directory(realPath))
			return;

		auto FileSystemCallback = [](efsw_watcher /*watcher*/, efsw_watchid /*watchid*/, const char* dir, const char* filename, efsw_action action, const char* oldFileName, void* param)
		{
			FilesystemModuleResolver* resolver = static_cast<FilesystemModuleResolver*>(param);

			switch (action)
			{
				case EFSW_ADD:
					resolver->OnFileAdded(dir, filename);
					break;

				case EFSW_DELETE:
					resolver->OnFileRemoved(dir, filename);
					break;

				case EFSW_MODIFIED:
					resolver->OnFileUpdated(dir, filename);
					break;

				case EFSW_MOVED:
					resolver->OnFileMoved(dir, filename, (oldFileName) ? oldFileName : std::string_view());
					break;
			}
		};

		if (watchDirectory)
			efsw_addwatch(m_fileWatcher, realPath.generic_u8string().c_str(), FileSystemCallback, 1, this);

		for (const auto& entry : std::filesystem::recursive_directory_iterator(realPath))
		{
			if (entry.is_regular_file() && CheckExtension(entry.path().generic_u8string()))
			{
				try
				{
					RegisterModule(entry.path());
				}
				catch (const std::exception& e)
				{
					std::cerr << "failed to register module " << entry.path().generic_u8string() << ": " << e.what() << std::endl;
				}
			}
		}
	}

	Ast::ModulePtr FilesystemModuleResolver::Resolve(const std::string& moduleName)
	{
		auto it = m_modules.find(moduleName);
		if (it == m_modules.end())
			return {};

		return it->second;
	}

	void FilesystemModuleResolver::OnFileAdded(std::string_view directory, std::string_view filename)
	{
		if (!CheckExtension(filename))
			return;

		RegisterModule(std::filesystem::path(directory) / filename);
	}

	void FilesystemModuleResolver::OnFileRemoved(std::string_view directory, std::string_view filename)
	{
		if (!CheckExtension(filename))
			return;

		std::filesystem::path canonicalPath = std::filesystem::canonical(std::filesystem::path(directory) / filename);

		auto it = m_moduleByFilepath.find(canonicalPath.generic_u8string());
		if (it != m_moduleByFilepath.end())
		{
			m_modules.erase(it->second);
			m_moduleByFilepath.erase(it);
		}
	}

	void FilesystemModuleResolver::OnFileMoved(std::string_view directory, std::string_view filename, std::string_view oldFilename)
	{
		if (oldFilename.empty() || !CheckExtension(oldFilename))
			return;

		std::filesystem::path canonicalPath = std::filesystem::canonical(std::filesystem::path(directory) / oldFilename);
		auto it = m_moduleByFilepath.find(canonicalPath.generic_u8string());
		if (it != m_moduleByFilepath.end())
		{
			std::filesystem::path newCanonicalPath = std::filesystem::canonical(std::filesystem::path(directory) / filename);

			std::string moduleName = std::move(it->second);
			m_moduleByFilepath.erase(it);

			m_moduleByFilepath.emplace(newCanonicalPath.generic_u8string(), std::move(moduleName));
		}
	}

	void FilesystemModuleResolver::OnFileUpdated(std::string_view directory, std::string_view filename)
	{
		if (!CheckExtension(filename))
			return;

		RegisterModule(std::filesystem::path(directory) / filename);
	}
	
	bool FilesystemModuleResolver::CheckExtension(std::string_view filename)
	{
		auto EndsWith = [](std::string_view lhs, std::string_view rhs)
		{
			if (rhs.size() > lhs.size())
				return false;

			return std::equal(lhs.end() - rhs.size(), lhs.end(), rhs.begin(), rhs.end(), [](char c1, char c2)
			{
				return std::tolower(c1) == std::tolower(c2);
			});
		};

		return EndsWith(filename, ModuleExtension) || EndsWith(filename, CompiledModuleExtension);
	}
}
