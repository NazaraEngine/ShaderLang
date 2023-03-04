// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/SanitizeVisitor.hpp>

namespace nzsl::Ast
{
	inline ModulePtr SanitizeVisitor::Sanitize(const Module& module, std::string* error)
	{
		return Sanitize(module, {}, error);
	}

	inline ModulePtr Sanitize(const Module& module, std::string* error)
	{
		SanitizeVisitor sanitizer;
		return sanitizer.Sanitize(module, error);
	}

	inline ModulePtr Sanitize(const Module& module, const SanitizeVisitor::Options& options, std::string* error)
	{
		SanitizeVisitor sanitizer;
		return sanitizer.Sanitize(module, options, error);
	}
}
