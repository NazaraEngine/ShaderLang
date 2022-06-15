// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_SERIALIZER_HPP
#define NZSL_AST_SERIALIZER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/SourceLocation.hpp>
#include <NZSL/Serializer.hpp>
#include <NZSL/Ast/Module.hpp>

namespace nzsl::Ast
{
	class NZSL_API SerializerBase
	{
		public:
			SerializerBase() = default;
			SerializerBase(const SerializerBase&) = delete;
			SerializerBase(SerializerBase&&) = delete;
			~SerializerBase() = default;

			void Serialize(AccessIdentifierExpression& node);
			void Serialize(AccessIndexExpression& node);
			void Serialize(AliasValueExpression& node);
			void Serialize(AssignExpression& node);
			void Serialize(BinaryExpression& node);
			void Serialize(CallFunctionExpression& node);
			void Serialize(CallMethodExpression& node);
			void Serialize(CastExpression& node);
			void Serialize(ConditionalExpression& node);
			void Serialize(ConstantExpression& node);
			void Serialize(ConstantArrayValueExpression& node);
			void Serialize(ConstantValueExpression& node);
			void Serialize(FunctionExpression& node);
			void Serialize(IdentifierExpression& node);
			void Serialize(IntrinsicExpression& node);
			void Serialize(IntrinsicFunctionExpression& node);
			void Serialize(StructTypeExpression& node);
			void Serialize(SwizzleExpression& node);
			void Serialize(TypeExpression& node);
			void Serialize(VariableValueExpression& node);
			void Serialize(UnaryExpression& node);

			void Serialize(BranchStatement& node);
			void Serialize(ConditionalStatement& node);
			void Serialize(DeclareAliasStatement& node);
			void Serialize(DeclareConstStatement& node);
			void Serialize(DeclareExternalStatement& node);
			void Serialize(DeclareFunctionStatement& node);
			void Serialize(DeclareOptionStatement& node);
			void Serialize(DeclareStructStatement& node);
			void Serialize(DeclareVariableStatement& node);
			void Serialize(DiscardStatement& node);
			void Serialize(ExpressionStatement& node);
			void Serialize(ForStatement& node);
			void Serialize(ForEachStatement& node);
			void Serialize(ImportStatement& node);
			void Serialize(MultiStatement& node);
			void Serialize(NoOpStatement& node);
			void Serialize(ReturnStatement& node);
			void Serialize(ScopedStatement& node);
			void Serialize(WhileStatement& node);

			void SerializeExpressionCommon(Expression& expr);
			void SerializeNodeCommon(Ast::Node& node);
			void SerializeStatementCommon(Statement& stmt);

		protected:
			template<typename T> void Container(T& container);
			template<typename T> void Enum(T& enumVal);
			template<typename T> void ExprValue(ExpressionValue<T>& attribute);
			template<typename T> void OptEnum(std::optional<T>& optVal);
			inline void OptType(std::optional<ExpressionType>& optType);
			template<typename T> void OptVal(std::optional<T>& optVal);

			virtual bool IsWriting() const = 0;

			virtual void Node(ExpressionPtr& node) = 0;
			virtual void Node(StatementPtr& node) = 0;

			virtual void SerializeModule(ModulePtr& module) = 0;
			virtual void SharedString(std::shared_ptr<const std::string>& val) = 0;

			inline void SourceLoc(SourceLocation& sourceLoc);

			virtual void Type(ExpressionType& type) = 0;

			virtual void Value(bool& val) = 0;
			virtual void Value(float& val) = 0;
			virtual void Value(std::string& val) = 0;
			virtual void Value(std::int32_t& val) = 0;
			virtual void Value(Vector2f& val) = 0;
			virtual void Value(Vector3f& val) = 0;
			virtual void Value(Vector4f& val) = 0;
			virtual void Value(Vector2i32& val) = 0;
			virtual void Value(Vector3i32& val) = 0;
			virtual void Value(Vector4i32& val) = 0;
			virtual void Value(std::uint8_t& val) = 0;
			virtual void Value(std::uint16_t& val) = 0;
			virtual void Value(std::uint32_t& val) = 0;
			virtual void Value(std::uint64_t& val) = 0;
			inline void SizeT(std::size_t& val);
	};

	class NZSL_API ShaderAstSerializer final : public SerializerBase
	{
		public:
			inline ShaderAstSerializer(AbstractSerializer& stream);
			~ShaderAstSerializer() = default;

			void Serialize(ModulePtr& shader);

		private:
			using SerializerBase::Serialize;

			bool IsWriting() const override;
			void Node(ExpressionPtr& node) override;
			void Node(StatementPtr& node) override;
			void SerializeModule(ModulePtr& module) override;
			void SharedString(std::shared_ptr<const std::string>& val) override;
			void Type(ExpressionType& type) override;
			void Value(bool& val) override;
			void Value(float& val) override;
			void Value(std::string& val) override;
			void Value(std::int32_t& val) override;
			void Value(Vector2f& val) override;
			void Value(Vector3f& val) override;
			void Value(Vector4f& val) override;
			void Value(Vector2i32& val) override;
			void Value(Vector3i32& val) override;
			void Value(Vector4i32& val) override;
			void Value(std::uint8_t& val) override;
			void Value(std::uint16_t& val) override;
			void Value(std::uint32_t& val) override;
			void Value(std::uint64_t& val) override;

			std::unordered_map<std::string, std::uint32_t> m_stringIndices;
			AbstractSerializer& m_serializer;
	};

	class NZSL_API ShaderAstUnserializer final : public SerializerBase
	{
		public:
			ShaderAstUnserializer(AbstractUnserializer& stream);
			~ShaderAstUnserializer() = default;

			ModulePtr Unserialize();

		private:
			using SerializerBase::Serialize;

			bool IsWriting() const override;
			void Node(ExpressionPtr& node) override;
			void Node(StatementPtr& node) override;
			void SerializeModule(ModulePtr& module) override;
			void SharedString(std::shared_ptr<const std::string>& val) override;
			void Type(ExpressionType& type) override;
			void Value(bool& val) override;
			void Value(float& val) override;
			void Value(std::string& val) override;
			void Value(std::int32_t& val) override;
			void Value(Vector2f& val) override;
			void Value(Vector3f& val) override;
			void Value(Vector4f& val) override;
			void Value(Vector2i32& val) override;
			void Value(Vector3i32& val) override;
			void Value(Vector4i32& val) override;
			void Value(std::uint8_t& val) override;
			void Value(std::uint16_t& val) override;
			void Value(std::uint32_t& val) override;
			void Value(std::uint64_t& val) override;

			std::vector<std::shared_ptr<const std::string>> m_strings;
			AbstractUnserializer& m_unserializer;
	};
	
	NZSL_API void SerializeShader(AbstractSerializer& serializer, ModulePtr& shader);
	NZSL_API ModulePtr UnserializeShader(AbstractUnserializer& unserializer);
}

#include <NZSL/Ast/AstSerializer.inl>

#endif // NZSL_AST_SERIALIZER_HPP
