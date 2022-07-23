// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/AstSerializer.hpp>
#include <Nazara/Utils/Algorithm.hpp>

namespace nzsl::Ast
{
	template<typename T>
	void SerializerBase::Container(T& container)
	{
		bool isWriting = IsWriting();

		std::uint32_t size;
		if (isWriting)
			size = Nz::SafeCast<std::uint32_t>(container.size());

		Value(size);
		if (!isWriting)
			container.resize(size);
	}


	template<typename T>
	void SerializerBase::Enum(T& enumVal)
	{
		bool isWriting = IsWriting();

		std::uint32_t value;
		if (isWriting)
			value = Nz::SafeCast<std::uint32_t>(enumVal);

		Value(value);
		if (!isWriting)
			enumVal = static_cast<T>(value);
	}
	
	template<typename T>
	void SerializerBase::ExprValue(ExpressionValue<T>& attribute)
	{
		std::uint32_t valueType;
		if (IsWriting())
		{
			if (!attribute.HasValue())
				valueType = 0;
			else if (attribute.IsExpression())
				valueType = 1;
			else if (attribute.IsResultingValue())
				valueType = 2;
			else
				throw std::runtime_error("unexpected attribute");
		}

		Value(valueType);

		switch (valueType)
		{
			case 0:
				if (!IsWriting())
					attribute = {};

				break;

			case 1:
			{
				if (!IsWriting())
				{
					ExpressionPtr expr;
					Node(expr);

					attribute = std::move(expr);
				}
				else
					Node(const_cast<ExpressionPtr&>(attribute.GetExpression())); //< not used for writing

				break;
			}

			case 2:
			{
				if (!IsWriting())
				{
					T value;
					if constexpr (std::is_enum_v<T>)
						Enum(value);
					else if constexpr (std::is_same_v<T, ExpressionType>)
						Type(value);
					else
						Value(value);

					attribute = std::move(value);
				}
				else
				{
					T& value = const_cast<T&>(attribute.GetResultingValue()); //< not used for writing
					if constexpr (std::is_enum_v<T>)
						Enum(value);
					else if constexpr (std::is_same_v<T, ExpressionType>)
						Type(value);
					else
						Value(value);
				}

				break;
			}
		}
	}

	template<typename T>
	void SerializerBase::OptEnum(std::optional<T>& optVal)
	{
		bool isWriting = IsWriting();

		bool hasValue;
		if (isWriting)
			hasValue = optVal.has_value();

		Value(hasValue);

		if (!isWriting && hasValue)
			optVal.emplace();

		if (optVal.has_value())
			Enum(optVal.value());
	}

	inline void SerializerBase::OptType(std::optional<ExpressionType>& optType)
	{
		bool isWriting = IsWriting();

		bool hasValue;
		if (isWriting)
			hasValue = optType.has_value();

		Value(hasValue);

		if (!isWriting && hasValue)
			optType.emplace();

		if (optType.has_value())
			Type(optType.value());
	}

	inline void SerializerBase::Metadata(Module::Metadata& metadata)
	{
		Value(metadata.moduleName);
		Value(metadata.shaderLangVersion);
		if (IsVersionGreaterOrEqual(2))
		{
			Value(metadata.author);
			Value(metadata.description);
			Value(metadata.license);

			Container(metadata.enabledFeatures);
			for (ModuleFeature& feature : metadata.enabledFeatures)
				Enum(feature);
		}
	}

	template<typename T>
	void SerializerBase::OptVal(std::optional<T>& optVal)
	{
		bool isWriting = IsWriting();

		bool hasValue;
		if (isWriting)
			hasValue = optVal.has_value();

		Value(hasValue);

		if (!isWriting && hasValue)
			optVal.emplace();

		if (optVal.has_value())
		{
			if constexpr (std::is_same_v<T, std::size_t>)
				SizeT(optVal.value());
			else
				Value(optVal.value());
		}
	}

	inline void SerializerBase::SizeT(std::size_t& val)
	{
		bool isWriting = IsWriting();

		std::uint32_t fixedVal;
		if (isWriting)
			fixedVal = Nz::SafeCast<std::uint32_t>(val);

		Value(fixedVal);

		if (!isWriting)
			val = Nz::SafeCast<std::size_t>(fixedVal);
	}

	inline void SerializerBase::SourceLoc(SourceLocation& sourceLoc)
	{
		SharedString(sourceLoc.file);
		Value(sourceLoc.endColumn);
		Value(sourceLoc.endLine);
		Value(sourceLoc.startColumn);
		Value(sourceLoc.startLine);
	}

	inline ShaderAstSerializer::ShaderAstSerializer(AbstractSerializer& serializer) :
	m_serializer(serializer)
	{
	}

	inline ShaderAstUnserializer::ShaderAstUnserializer(AbstractUnserializer& unserializer) :
	m_unserializer(unserializer)
	{
	}
}

