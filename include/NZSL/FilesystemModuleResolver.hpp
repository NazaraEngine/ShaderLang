// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_FILESYSTEMMODULERESOLVER_HPP
#define NZSL_FILESYSTEMMODULERESOLVER_HPP

#include <Nazara/Utils/MovablePtr.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/ModuleResolver.hpp>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace nzsl
{
	class NZSL_API FilesystemModuleResolver : public ModuleResolver
	{
		public:
			FilesystemModuleResolver() = default;
			FilesystemModuleResolver(const FilesystemModuleResolver&) = delete;
			FilesystemModuleResolver(FilesystemModuleResolver&&) noexcept = delete;
			~FilesystemModuleResolver();

			void RegisterModule(const std::filesystem::path& realPath);
			void RegisterModule(std::string_view moduleSource);
			void RegisterModule(Ast::ModulePtr module);
			void RegisterModuleDirectory(const std::filesystem::path& realPath, bool watchDirectory = false);

			Ast::ModulePtr Resolve(const std::string& moduleName) override;

			FilesystemModuleResolver& operator=(const FilesystemModuleResolver&) = delete;
			FilesystemModuleResolver& operator=(FilesystemModuleResolver&&) noexcept = delete;

			static constexpr const char* CompiledModuleExtension = ".nzslb";
			static constexpr const char* ModuleExtension = ".nzsl";

		private:
			void OnFileAdded(std::string_view directory, std::string_view filename);
			void OnFileRemoved(std::string_view directory, std::string_view filename);
			void OnFileMoved(std::string_view directory, std::string_view filename, std::string_view oldFilename);
			void OnFileUpdated(std::string_view directory, std::string_view filename);

			static bool CheckExtension(std::string_view filename);

			std::unordered_map<std::string, std::string> m_moduleByFilepath;
			std::unordered_map<std::string, Ast::ModulePtr> m_modules;
			Nz::MovablePtr<void> m_fileWatcher;
			std::recursive_mutex m_moduleLock;
	};
}

#include <NZSL/FilesystemModuleResolver.inl>

#endif // NZSL_FILESYSTEMMODULERESOLVER_HPP
