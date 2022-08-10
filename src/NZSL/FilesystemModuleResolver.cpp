// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/FilesystemModuleResolver.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Ast/AstSerializer.hpp>
#ifdef NZSL_EFSW
#include <efsw/efsw.h>
#endif
#include <fmt/format.h>
#include <cassert>
#include <cctype>
#include <fstream>

namespace nzsl
{
	FilesystemModuleResolver::~FilesystemModuleResolver()
	{
#ifdef NZSL_EFSW
		if (m_fileWatcher)
			efsw_release(m_fileWatcher);
#endif
	}

	void FilesystemModuleResolver::RegisterModule(const std::filesystem::path& realPath)
	{
		Ast::ModulePtr module;
		try
		{
			std::ifstream inputFile(realPath, std::ios::in | std::ios::binary);
			if (!inputFile)
				throw std::runtime_error("failed to open " + Nz::PathToString(realPath));

			inputFile.seekg(0, std::ios::end);

			std::streamsize length = inputFile.tellg();
			if (length == 0)
				return; //< ignore empty files

			inputFile.seekg(0, std::ios::beg);

			std::vector<char> content(Nz::SafeCast<std::size_t>(length));
			if (!inputFile.read(&content[0], length))
				throw std::runtime_error("failed to read " + Nz::PathToString(realPath));

			std::string ext = Nz::PathToString(realPath.extension());
			if (ext == CompiledModuleExtension)
			{
				Unserializer unserializer(content.data(), content.size());
				module = Ast::UnserializeShader(unserializer);
			}
			else if (ext == ModuleExtension)
				module = Parse(std::string_view(content.data(), content.size()), Nz::PathToString(realPath));
			else
				throw std::runtime_error("unknown extension " + ext);
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(fmt::format("failed to register module {}: {}", Nz::PathToString(realPath), e.what()));
		}

		if (!module)
			return;

		std::lock_guard lock(m_moduleLock);

		std::string moduleName = module->metadata->moduleName;
		RegisterModule(std::move(module));

		std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(realPath);
		m_moduleByFilepath.emplace(Nz::PathToString(canonicalPath), std::move(moduleName));
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

		std::lock_guard lock(m_moduleLock);

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

		if (watchDirectory)
		{
#ifdef NZSL_EFSW
			if (!m_fileWatcher)
			{
				m_fileWatcher = efsw_create(0);
				efsw_watch(m_fileWatcher);
			}

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
		
			efsw_addwatch(m_fileWatcher, Nz::PathToString(realPath).c_str(), FileSystemCallback, 1, this);
#else
			throw std::runtime_error("nzsl was built without filesystem watch feature");
#endif
		}

		for (const auto& entry : std::filesystem::recursive_directory_iterator(realPath))
		{
			if (entry.is_regular_file() && CheckExtension(Nz::PathToString(entry.path())))
			{
				try
				{
					RegisterModule(entry.path());
				}
				catch (const std::exception& e)
				{
					throw std::runtime_error(fmt::format("failed to register module {}: {}", Nz::PathToString(entry.path()), e.what()));
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

		std::filesystem::path filepath = std::filesystem::path(directory) / filename;

		try
		{
			RegisterModule(filepath);
		}
		catch (const std::exception& e)
		{
			fmt::print(stderr, "failed to register module from new file {}: {}", Nz::PathToString(filepath), e.what());
		}
	}

	void FilesystemModuleResolver::OnFileRemoved(std::string_view directory, std::string_view filename)
	{
		if (!CheckExtension(filename))
			return;

		std::lock_guard lock(m_moduleLock);

		std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(std::filesystem::path(directory) / filename);

		auto it = m_moduleByFilepath.find(Nz::PathToString(canonicalPath));
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

		std::lock_guard lock(m_moduleLock);

		std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(std::filesystem::path(directory) / oldFilename);
		auto it = m_moduleByFilepath.find(Nz::PathToString(canonicalPath));
		if (it != m_moduleByFilepath.end())
		{
			std::filesystem::path newCanonicalPath = std::filesystem::weakly_canonical(std::filesystem::path(directory) / filename);

			std::string moduleName = std::move(it->second);
			m_moduleByFilepath.erase(it);

			m_moduleByFilepath.emplace(Nz::PathToString(newCanonicalPath), std::move(moduleName));
		}
	}

	void FilesystemModuleResolver::OnFileUpdated(std::string_view directory, std::string_view filename)
	{
		if (!CheckExtension(filename))
			return;

		std::filesystem::path filepath = std::filesystem::path(directory) / filename;

		try
		{
			RegisterModule(std::filesystem::path(directory) / filename);
		}
		catch (const std::exception& e)
		{
			fmt::print(stderr, "failed to update module from {}: {}", Nz::PathToString(filepath), e.what());
		}
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
