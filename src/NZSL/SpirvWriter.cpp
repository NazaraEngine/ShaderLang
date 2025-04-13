// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/SpirvWriter.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NazaraUtils/FixedVector.hpp>
#include <NazaraUtils/PathUtils.hpp>
#include <NZSL/Enums.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Ast/ConstantPropagationVisitor.hpp>
#include <NZSL/Ast/EliminateUnusedPassVisitor.hpp>
#include <NZSL/Ast/RecursiveVisitor.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <NZSL/SpirV/SpirvAstVisitor.hpp>
#include <NZSL/SpirV/SpirvBlock.hpp>
#include <NZSL/SpirV/SpirvConstantCache.hpp>
#include <NZSL/SpirV/SpirvData.hpp>
#include <NZSL/SpirV/SpirvGenData.hpp>
#include <NZSL/SpirV/SpirvSection.hpp>
#include <fmt/format.h>
#include <frozen/unordered_map.h>
#include <tsl/ordered_map.h>
#include <tsl/ordered_set.h>
#include <cassert>
#include <fstream>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace nzsl
{
	namespace NAZARA_ANONYMOUS_NAMESPACE
	{
		template<typename T>
		struct IsVector : std::bool_constant<false> {};

		template<typename T>
		struct IsVector<std::vector<T>> : std::bool_constant<true> {};
	}

	class SpirvWriter::PreVisitor : public Ast::RecursiveVisitor
	{
		public:
			struct ExternalVar
			{
				struct Decoration
				{
					SpirvDecoration decoration;
					Nz::FixedVector<std::uint32_t, 2> decorationData;
				};

				Nz::FixedVector<Decoration, 3> decorations;
				SpirvVariable varData;
			};

			using BuiltinDecoration = tsl::ordered_map<std::uint32_t, SpirvBuiltIn>;
			using ConstantVariables = std::unordered_map<std::size_t /*constIndex*/, SpirvVariable /*variable*/>;
			using InterpolationDecoration = tsl::ordered_map<std::uint32_t, SpirvDecoration>;
			using LocationDecoration = tsl::ordered_map<std::uint32_t, std::uint32_t>;
			using ExtInstList = tsl::ordered_set<std::string>;
			using ExtVarContainer = tsl::ordered_map<std::size_t /*varIndex*/, ExternalVar>;
			using FunctionContainer = tsl::ordered_map<std::size_t, SpirvAstVisitor::FuncData>;
			using LocalContainer = tsl::ordered_set<Ast::ExpressionType>;
			using StructContainer = std::vector<Ast::StructDescription*>;

			PreVisitor(const SpirvWriter& writer, SpirvConstantCache& constantCache) :
			m_constantCache(constantCache),
			m_writer(writer)
			{
				m_constantCache.SetStructCallback([this](std::size_t structIndex) -> const Ast::StructDescription&
				{
					assert(structIndex < declaredStructs.size());
					return *declaredStructs[structIndex];
				});

				spirvCapabilities.insert(SpirvCapability::Shader);
			}

			void Visit(Ast::AccessIndexExpression& node) override
			{
				RecursiveVisitor::Visit(node);

				m_constantCache.Register(*m_constantCache.BuildType(node.cachedExpressionType.value()));
			}

			void Visit(Ast::BinaryExpression& node) override
			{
				RecursiveVisitor::Visit(node);

				m_constantCache.Register(*m_constantCache.BuildType(node.cachedExpressionType.value()));
			}

			void Visit(Ast::CallFunctionExpression& node) override
			{
				RecursiveVisitor::Visit(node);

				assert(m_funcIndex);
				auto it = funcs.find(*m_funcIndex);
				assert(it != funcs.end());
				auto& func = it.value();

				auto& funcCall = func.funcCalls.emplace_back();
				funcCall.firstVarIndex = func.variables.size();

				for (const auto& parameter : node.parameters)
				{
					auto& var = func.variables.emplace_back();
					var.sourceLocation = parameter.expr->sourceLocation;
					var.typeId = m_constantCache.Register(*m_constantCache.BuildPointerType(*GetExpressionType(*parameter.expr), SpirvStorageClass::Function));
				}
			}

			void Visit(Ast::CastExpression& node) override
			{
				RecursiveVisitor::Visit(node);

				m_constantCache.Register(*m_constantCache.BuildType(node.cachedExpressionType.value()));
			}

			void Visit(Ast::ConditionalExpression& /*node*/) override
			{
				throw std::runtime_error("unexpected conditional expression, did you forget to sanitize the shader?");
			}

			void Visit(Ast::ConditionalStatement& /*node*/) override
			{
				throw std::runtime_error("unexpected conditional expression, did you forget to sanitize the shader?");
			}

			void Visit(Ast::ConstantArrayValueExpression& node) override
			{
				m_constantCache.Register(*m_constantCache.BuildArrayConstant(node.values));

				RecursiveVisitor::Visit(node);
			}

			void Visit(Ast::ConstantValueExpression& node) override
			{
				m_constantCache.Register(*m_constantCache.BuildConstant(node.value));

				RecursiveVisitor::Visit(node);
			}

			void Visit(Ast::DeclareConstStatement& node) override
			{
				assert(node.constIndex);
				assert(constantVariables.find(*node.constIndex) == constantVariables.end());

				SpirvStorageClass storageClass = SpirvStorageClass::Private;

				SpirvConstantCache::TypePtr typePtr = m_constantCache.BuildType(*GetExpressionType(*node.expression));

				SpirvConstantCache::Variable constantVariable;
				constantVariable.debugName = node.name;
				constantVariable.storageClass = storageClass;
				constantVariable.type = m_constantCache.BuildPointerType(typePtr, constantVariable.storageClass);

				switch (node.expression->GetType())
				{
					case Ast::NodeType::ConstantValueExpression:
					{
						auto& constExpr = static_cast<Ast::ConstantValueExpression&>(*node.expression);
						constantVariable.initializer = m_constantCache.BuildConstant(constExpr.value);
						break;
					}

					case Ast::NodeType::ConstantArrayValueExpression:
					{
						auto& constExpr = static_cast<Ast::ConstantArrayValueExpression&>(*node.expression);
						constantVariable.initializer = m_constantCache.BuildArrayConstant(constExpr.values);
						break;
					}

					default:
						throw std::runtime_error("unexpected non-constant expression, is shader sanitized?");
				}

				std::uint32_t pointerTypeId = m_constantCache.Register(*constantVariable.type);
				std::uint32_t variableId = m_constantCache.Register(std::move(constantVariable));
				
				constantVariables.emplace(*node.constIndex, SpirvVariable{ variableId, pointerTypeId, std::move(typePtr), storageClass});
			}

			void Visit(Ast::DeclareExternalStatement& node) override
			{
				for (auto& extVar : node.externalVars)
				{
					assert(extVar.varIndex);
					ExternalVar& extVarData = extVars[*extVar.varIndex];

					SpirvConstantCache::Variable variable;
					variable.debugName = (!node.name.empty()) ? fmt::format("{}_{}", node.name, extVar.name) : extVar.name;

					const Ast::ExpressionType& extVarType = extVar.type.GetResultingValue();

					SpirvConstantCache::TypePtr typePtr;
					if (Ast::IsStorageType(extVarType) || Ast::IsUniformType(extVarType) || Ast::IsPushConstantType(extVarType))
					{
						if (Ast::IsStorageType(extVarType))
						{
							auto& storageType = std::get<Ast::StorageType>(extVarType);

							if (m_writer.IsVersionGreaterOrEqual(1, 3))
								variable.storageClass = SpirvStorageClass::StorageBuffer;
							else
								variable.storageClass = SpirvStorageClass::Uniform;

							typePtr = m_constantCache.BuildType(storageType);
							if (storageType.accessPolicy != AccessPolicy::ReadWrite)
								extVarData.decorations.push_back(ExternalVar::Decoration{ (storageType.accessPolicy == AccessPolicy::WriteOnly) ? SpirvDecoration::NonReadable : SpirvDecoration::NonWritable, {} });
						}
						else if (Ast::IsUniformType(extVarType))
						{
							variable.storageClass = SpirvStorageClass::Uniform;

							typePtr = m_constantCache.BuildType(std::get<Ast::UniformType>(extVarType));
						}
						else
						{
							variable.storageClass = SpirvStorageClass::PushConstant;

							typePtr = m_constantCache.BuildType(std::get<Ast::PushConstantType>(extVarType));
						}

						variable.type = m_constantCache.BuildPointerType(typePtr, variable.storageClass);
					}
					else if (Ast::IsSamplerType(extVarType) || Ast::IsArrayType(extVarType) || Ast::IsTextureType(extVarType))
					{
						variable.storageClass = SpirvStorageClass::UniformConstant;
						variable.type = m_constantCache.BuildPointerType(extVarType, variable.storageClass);
					}
					else
						throw std::runtime_error("unsupported type used in external block (SPIR-V doesn't allow primitive types as uniforms)");

					assert(Ast::IsPushConstantType(extVarType) || extVar.bindingIndex.IsResultingValue());

					extVarData.varData.typePtr = (typePtr) ? typePtr : m_constantCache.BuildType(extVarType, variable.storageClass);
					extVarData.varData.typeId = m_constantCache.Register(*extVarData.varData.typePtr);
					extVarData.varData.storageClass = variable.storageClass;
					extVarData.varData.pointerId = m_constantCache.Register(std::move(variable));

					if (!Ast::IsPushConstantType(extVarType))
					{
						extVarData.decorations.push_back(ExternalVar::Decoration{ SpirvDecoration::Binding, { extVar.bindingIndex.GetResultingValue() } });
						extVarData.decorations.push_back(ExternalVar::Decoration{ SpirvDecoration::DescriptorSet, { (extVar.bindingSet.HasValue()) ? extVar.bindingSet.GetResultingValue() : 0 } });
					}
				}
			}

			void Visit(Ast::DeclareFunctionStatement& node) override
			{
				std::optional<ShaderStageType> entryPointType;
				if (node.entryStage.HasValue())
					entryPointType = node.entryStage.GetResultingValue();

				assert(node.funcIndex);
				std::size_t funcIndex = *node.funcIndex;

				auto& funcData = funcs[funcIndex];
				funcData.name = node.name;
				funcData.funcIndex = funcIndex;

				if (!entryPointType)
				{
					std::vector<Ast::ExpressionType> parameterTypes;
					parameterTypes.reserve(node.parameters.size());
					for (auto& parameter : node.parameters)
						parameterTypes.push_back(parameter.type.GetResultingValue());

					SpirvConstantCache::TypePtr returnTypePtr;
					if (node.returnType.HasValue())
					{
						const auto& returnType = node.returnType.GetResultingValue();
						returnTypePtr = m_constantCache.BuildType(returnType);
					}
					else
						returnTypePtr = m_constantCache.BuildType(Ast::NoType{});

					funcData.returnTypeId = m_constantCache.Register(*returnTypePtr);

					std::vector<SpirvConstantCache::TypePtr> parameterTypePtr;
					parameterTypePtr.reserve(node.parameters.size());

					for (auto& parameter : node.parameters)
					{
						const auto& parameterType = parameter.type.GetResultingValue();

						bool isUniformConstant = Ast::IsExternalPointerType(parameterType);
						SpirvStorageClass storageClass = (isUniformConstant) ? SpirvStorageClass::UniformConstant : SpirvStorageClass::Function;

						SpirvConstantCache::TypePtr typePtr = m_constantCache.BuildType(parameterType, storageClass);
						SpirvConstantCache::TypePtr& pointerTypePtr = parameterTypePtr.emplace_back(m_constantCache.BuildPointerType(typePtr, storageClass));

						auto& funcParam = funcData.parameters.emplace_back();
						funcParam.pointerTypeId = m_constantCache.Register(*pointerTypePtr);
						funcParam.typeId = m_constantCache.Register(*typePtr);
						funcParam.typePtr = std::move(typePtr);
					}

					funcData.funcTypeId = m_constantCache.Register(*m_constantCache.BuildFunctionType(std::move(returnTypePtr), std::move(parameterTypePtr)));
				}
				else
				{
					using EntryPoint = SpirvAstVisitor::EntryPoint;

					std::vector<EntryPoint::ExecutionMode> executionModes;
					switch (*entryPointType)
					{
						case ShaderStageType::Compute:
						{
							if (node.workgroupSize.HasValue())
							{
								Vector3u32 workgroupSize = node.workgroupSize.GetResultingValue();
								executionModes.push_back({ SpirvExecutionMode::LocalSize, { workgroupSize.x(), workgroupSize.y(), workgroupSize.z()}});
							}
							break;
						}

						case ShaderStageType::Fragment:
						{
							executionModes.push_back({ SpirvExecutionMode::OriginUpperLeft, {} });
							if (node.earlyFragmentTests.HasValue() && node.earlyFragmentTests.GetResultingValue())
								executionModes.push_back({ SpirvExecutionMode::EarlyFragmentTests, {} });

							if (node.depthWrite.HasValue())
							{
								executionModes.push_back({ SpirvExecutionMode::DepthReplacing, {} });

								switch (node.depthWrite.GetResultingValue())
								{
									case Ast::DepthWriteMode::Replace:   break;
									case Ast::DepthWriteMode::Greater:   executionModes.push_back({ SpirvExecutionMode::DepthGreater, {} }); break;
									case Ast::DepthWriteMode::Less:      executionModes.push_back({ SpirvExecutionMode::DepthLess, {} }); break;
									case Ast::DepthWriteMode::Unchanged: executionModes.push_back({ SpirvExecutionMode::DepthUnchanged, {} }); break;
								}
							}

							break;
						}

						case ShaderStageType::Vertex:
							break;
					}

					funcData.returnTypeId = m_constantCache.Register(*m_constantCache.BuildType(Ast::NoType{}));
					funcData.funcTypeId = m_constantCache.Register(*m_constantCache.BuildFunctionType(Ast::NoType{}, {}));

					std::optional<EntryPoint::InputStruct> inputStruct;
					std::vector<EntryPoint::Input> inputs;
					if (!node.parameters.empty())
					{
						assert(node.parameters.size() == 1);
						auto& parameter = node.parameters.front();
						const auto& parameterType = parameter.type.GetResultingValue();

						assert(std::holds_alternative<Ast::StructType>(parameterType));

						std::size_t structIndex = std::get<Ast::StructType>(parameterType).structIndex;
						const Ast::StructDescription* structDesc = declaredStructs[structIndex];

						std::size_t memberIndex = 0;
						for (const auto& member : structDesc->members)
						{
							if (member.cond.HasValue() && !member.cond.GetResultingValue())
								continue;

							if (std::uint32_t varId = HandleEntryInOutType(*entryPointType, funcIndex, member, SpirvStorageClass::Input); varId != 0)
							{
								inputs.push_back({
									m_constantCache.Register(*m_constantCache.BuildConstant(std::int32_t(memberIndex))),
									m_constantCache.Register(*m_constantCache.BuildPointerType(member.type.GetResultingValue(), SpirvStorageClass::Function)),
									varId
								});
							}

							memberIndex++;
						}

						SpirvConstantCache::TypePtr typePtr = m_constantCache.BuildType(parameter.type.GetResultingValue());

						inputStruct = EntryPoint::InputStruct{
							m_constantCache.Register(*m_constantCache.BuildPointerType(parameterType, SpirvStorageClass::Function)),
							m_constantCache.Register(*typePtr),
							typePtr
						};
					}

					std::optional<std::uint32_t> outputStructId;
					std::vector<EntryPoint::Output> outputs;
					if (node.returnType.HasValue() && !IsNoType(node.returnType.GetResultingValue()))
					{
						const Ast::ExpressionType& returnType = node.returnType.GetResultingValue();

						assert(std::holds_alternative<Ast::StructType>(returnType));

						std::size_t structIndex = std::get<Ast::StructType>(returnType).structIndex;
						const Ast::StructDescription* structDesc = declaredStructs[structIndex];

						std::size_t memberIndex = 0;
						for (const auto& member : structDesc->members)
						{
							if (member.cond.HasValue() && !member.cond.GetResultingValue())
								continue;

							if (std::uint32_t varId = HandleEntryInOutType(*entryPointType, funcIndex, member, SpirvStorageClass::Output); varId != 0)
							{
								outputs.push_back({
									std::int32_t(memberIndex),
									m_constantCache.Register(*m_constantCache.BuildType(member.type.GetResultingValue())),
									varId
								});
							}

							memberIndex++;
						}

						outputStructId = m_constantCache.Register(*m_constantCache.BuildType(returnType));
					}

					funcData.entryPointData = EntryPoint{
						*entryPointType,
						inputStruct,
						outputStructId,
						std::move(executionModes),
						std::move(inputs),
						std::move(outputs)
					};
				}

				m_funcIndex = funcIndex;
				RecursiveVisitor::Visit(node);
				m_funcIndex.reset();
			}

			void Visit(Ast::DeclareStructStatement& node) override
			{
				RecursiveVisitor::Visit(node);

				assert(node.structIndex);
				std::size_t structIndex = *node.structIndex;
				if (structIndex >= declaredStructs.size())
					declaredStructs.resize(structIndex + 1);

				declaredStructs[structIndex] = &node.description;
			}

			void Visit(Ast::DeclareVariableStatement& node) override
			{
				RecursiveVisitor::Visit(node);

				assert(m_funcIndex);
				auto it = funcs.find(*m_funcIndex);
				assert(it != funcs.end());
				auto& func = it.value();

				assert(node.varIndex);
				func.varIndexToVarId[*node.varIndex] = func.variables.size();

				auto& var = func.variables.emplace_back();
				var.sourceLocation = node.sourceLocation;
				var.typeId = m_constantCache.Register(*m_constantCache.BuildPointerType(node.varType.GetResultingValue(), SpirvStorageClass::Function));
			}

			void Visit(Ast::IdentifierExpression& node) override
			{
				m_constantCache.Register(*m_constantCache.BuildType(node.cachedExpressionType.value()));

				RecursiveVisitor::Visit(node);
			}

			void Visit(Ast::IntrinsicExpression& node) override
			{
				RecursiveVisitor::Visit(node);

				auto it = SpirvGenData::s_intrinsicData.find(node.intrinsic);
				if (it == SpirvGenData::s_intrinsicData.end())
					throw std::runtime_error("unknown intrinsic value " + std::to_string(Nz::UnderlyingCast(node.intrinsic)));

				std::visit([&](auto&& arg)
				{
					using namespace SpirvGenData;

					using T = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<T, SpirvGlslStd450Op> || std::is_same_v<T, SpirvGlslStd450Selector>)
						extInsts.emplace("GLSL.std.450");
					else if constexpr (std::is_same_v<T, SpirvOp> || std::is_same_v<T, SpirvCodeGenerator>)
					{
						/* Part of SPIR-V core */
					}
					else
						static_assert(Nz::AlwaysFalse<T>(), "non-exhaustive visitor");
				}, it->second.op);

				m_constantCache.Register(*m_constantCache.BuildType(node.cachedExpressionType.value()));
			}

			void Visit(Ast::SwizzleExpression& node) override
			{
				RecursiveVisitor::Visit(node);

				for (std::size_t i = 0; i < node.componentCount; ++i)
				{
					std::int32_t indexCount = Nz::SafeCast<std::int32_t>(node.components[i]);
					m_constantCache.Register(*m_constantCache.BuildConstant(indexCount));
				}

				m_constantCache.Register(*m_constantCache.BuildType(node.cachedExpressionType.value()));
			}

			void Visit(Ast::UnaryExpression& node) override
			{
				RecursiveVisitor::Visit(node);

				m_constantCache.Register(*m_constantCache.BuildType(node.cachedExpressionType.value()));
			}

			std::uint32_t HandleEntryInOutType(ShaderStageType entryPointType, std::size_t funcIndex, const Ast::StructDescription::StructMember& member, SpirvStorageClass storageClass)
			{
				NAZARA_USE_ANONYMOUS_NAMESPACE

				if (member.builtin.HasValue())
				{
					auto builtinIt = LangData::s_builtinData.find(member.builtin.GetResultingValue());
					assert(builtinIt != LangData::s_builtinData.end());

					const LangData::BuiltinData& builtinData = builtinIt->second;

					if ((builtinData.compatibleStages & entryPointType) == 0)
						return 0;

					auto spvIt = SpirvGenData::s_builtinMapping.find(member.builtin.GetResultingValue());
					if (spvIt == SpirvGenData::s_builtinMapping.end())
						throw std::runtime_error("unknown builtin value " + std::to_string(Nz::UnderlyingCast(member.builtin.GetResultingValue())));

					const SpirvGenData::SpirvBuiltin& spirvBuiltin = spvIt->second;
					if (!m_writer.IsVersionGreaterOrEqual(spirvBuiltin.requiredVersion.majorVersion, spirvBuiltin.requiredVersion.minorVersion))
						throw std::runtime_error(fmt::format("using builtin {} requires SPIR-V {}.{}", builtinData.identifier, spirvBuiltin.requiredVersion.majorVersion, spirvBuiltin.requiredVersion.minorVersion));
					
					spirvCapabilities.insert(spirvBuiltin.capability);

					SpirvBuiltIn builtinDecoration = spirvBuiltin.decoration;

					SpirvConstantCache::Variable variable;
					variable.debugName = builtinData.identifier;
					variable.funcId = funcIndex;
					variable.storageClass = storageClass;
					variable.type = m_constantCache.BuildPointerType(member.type.GetResultingValue(), storageClass);

					std::uint32_t varId = m_constantCache.Register(variable);
					builtinDecorations[varId] = builtinDecoration;

					return varId;
				}
				else if (member.locationIndex.HasValue())
				{
					SpirvConstantCache::Variable variable;
					variable.debugName = (!member.originalName.empty()) ? member.originalName : member.name;
					variable.funcId = funcIndex;
					variable.storageClass = storageClass;
					variable.type = m_constantCache.BuildPointerType(member.type.GetResultingValue(), storageClass);

					std::uint32_t varId = m_constantCache.Register(variable);
					locationDecorations[varId] = member.locationIndex.GetResultingValue();

					if (member.interp.HasValue())
					{
						Ast::InterpolationQualifier interpolation = member.interp.GetResultingValue();
						if (interpolation != Ast::InterpolationQualifier::Smooth)
						{
							auto spvIt = SpirvGenData::s_interpolationMapping.find(member.interp.GetResultingValue());
							if (spvIt == SpirvGenData::s_interpolationMapping.end())
								throw std::runtime_error("unknown interpolation value " + std::to_string(Nz::UnderlyingCast(member.interp.GetResultingValue())));

							interpolationDecorations[varId] = spvIt->second.interpolationDecoration;
						}
					}

					return varId;
				}

				return 0;
			}

			BuiltinDecoration builtinDecorations;
			ConstantVariables constantVariables;
			ExtInstList extInsts;
			ExtVarContainer extVars;
			FunctionContainer funcs;
			InterpolationDecoration interpolationDecorations;
			LocationDecoration locationDecorations;
			StructContainer declaredStructs;
			tsl::ordered_set<SpirvCapability> spirvCapabilities;

		private:
			SpirvConstantCache& m_constantCache;
			const SpirvWriter& m_writer;
			std::optional<std::size_t> m_funcIndex;
	};

	struct SpirvWriter::State
	{
		State(SpirvWriter& writer) :
		constantTypeCache(writer, nextResultId)
		{
		}

		struct Func
		{
			const Ast::DeclareFunctionStatement* statement = nullptr;
			std::uint32_t typeId;
			std::uint32_t id;
		};

		tsl::ordered_map<std::size_t, SpirvAstVisitor::FuncData> funcs;
		tsl::ordered_map<std::string, std::uint32_t> extensionInstructionSet;
		std::unordered_map<std::shared_ptr<const std::string>, std::uint32_t> sourceFiles;
		std::vector<std::uint32_t> resultIds;
		std::uint32_t nextResultId = 1;
		SourceLocation sourceLocation;
		SpirvConstantCache constantTypeCache; //< init after nextVarIndex
		PreVisitor* previsitor;

		// Output
		SpirvSection header;
		SpirvSection constants;
		SpirvSection debugInfo;
		SpirvSection annotations;
		SpirvSection instructions;
	};

	SpirvWriter::SpirvWriter() :
	m_currentState(nullptr)
	{
	}

	std::vector<std::uint32_t> SpirvWriter::Generate(const Ast::Module& module, const States& states)
	{
		Ast::ModulePtr sanitizedModule;
		const Ast::Module* targetModule;
		if (!states.sanitized)
		{
			Ast::SanitizeVisitor::Options options = GetSanitizeOptions();
			options.optionValues = states.optionValues;
			options.moduleResolver = states.shaderModuleResolver;

			sanitizedModule = Ast::Sanitize(module, options);
			targetModule = sanitizedModule.get();
		}
		else
			targetModule = &module;

		if (states.optimize)
		{
			sanitizedModule = Ast::PropagateConstants(*targetModule);
			
			Ast::DependencyCheckerVisitor::Config dependencyConfig;
			dependencyConfig.usedShaderStages = ShaderStageType_All;

			sanitizedModule = Ast::EliminateUnusedPass(*sanitizedModule, dependencyConfig);

			targetModule = sanitizedModule.get();
		}

		// Previsitor

		m_context.states = &states;

		State state(*this);
		m_currentState = &state;
		NAZARA_DEFER({ m_currentState = nullptr; });

		// Register all extended instruction sets
		PreVisitor previsitor(*this, state.constantTypeCache);
		
		for (Ast::ModuleFeature feature : targetModule->metadata->enabledFeatures)
		{
			switch (feature)
			{
				case Ast::ModuleFeature::Float64:
					previsitor.spirvCapabilities.insert(SpirvCapability::Float64);
					break;

				case Ast::ModuleFeature::PrimitiveExternals:
					break; //< Don't trigger an error here, wait until it's actually used (just in case it's in disabled code)

				case Ast::ModuleFeature::Texture1D:
					previsitor.spirvCapabilities.insert(SpirvCapability::Sampled1D);
					break;
			}
		}

		for (const auto& importedModule : targetModule->importedModules)
			importedModule.module->rootNode->Visit(previsitor);

		targetModule->rootNode->Visit(previsitor);

		m_currentState->previsitor = &previsitor;

		for (const std::string& extInst : previsitor.extInsts)
			state.extensionInstructionSet[extInst] = AllocateResultId();

		// Assign function ID (required for forward declaration)
		for (auto it = previsitor.funcs.begin(); it != previsitor.funcs.end(); ++it)
		{
			auto& func = it.value();
			func.funcId = AllocateResultId();
			state.funcs.emplace(it.key(), std::move(func));
		}
		previsitor.funcs.clear(); //< since we moved every value, prevent further usage

		if (states.debugLevel >= DebugLevel::Regular)
		{
			auto RegisterSourceFile = [this, &states](const Ast::Module& module)
			{
				const SourceLocation& rootLocation = module.rootNode->sourceLocation;

				std::string source;
				std::uint32_t fileId = 0;
				if (rootLocation.file)
				{
					fileId = m_currentState->constantTypeCache.Register(*rootLocation.file);
					m_currentState->sourceFiles.emplace(rootLocation.file, fileId);

					if (states.debugLevel >= DebugLevel::Full)
					{
						std::ifstream file(Nz::Utf8Path(*rootLocation.file));
						if (file)
						{
							std::string line;
							while (std::getline(file, line))
							{
								source += line;
								source += '\n';
							}
						}
					}
				}

				m_currentState->constantTypeCache.RegisterSource(SpirvSourceLanguage::NZSL, module.metadata->shaderLangVersion, fileId, source);

				if (!module.metadata->moduleName.empty())
					m_currentState->constantTypeCache.RegisterSourceExtension("ModuleName: " + module.metadata->moduleName);

				if (!module.metadata->author.empty())
					m_currentState->constantTypeCache.RegisterSourceExtension("Author: " + module.metadata->author);

				if (!module.metadata->description.empty())
					m_currentState->constantTypeCache.RegisterSourceExtension("Description: " + module.metadata->description);

				if (!module.metadata->license.empty())
					m_currentState->constantTypeCache.RegisterSourceExtension("License: " + module.metadata->license);

				if (!module.metadata->enabledFeatures.empty())
				{
					std::string features;
					for (Ast::ModuleFeature feature : module.metadata->enabledFeatures)
					{
						if (!features.empty())
							features += ",";
					
						features += Parser::ToString(feature);
					}

					m_currentState->constantTypeCache.RegisterSourceExtension("Features: " + features);
				}
			};

			RegisterSourceFile(module);

			for (const auto& importedModule : targetModule->importedModules)
				RegisterSourceFile(*importedModule.module);
		}
		else if (states.debugLevel >= DebugLevel::Minimal)
			m_currentState->constantTypeCache.RegisterSource(SpirvSourceLanguage::NZSL, module.metadata->shaderLangVersion);

		auto funcDataRetriever = [&](std::size_t funcIndex) -> SpirvAstVisitor::FuncData&
		{
			auto it = m_currentState->funcs.find(funcIndex);
			if NAZARA_UNLIKELY(it == m_currentState->funcs.end())
				throw std::runtime_error("internal error");

			return it.value();
		};

		SpirvAstVisitor visitor(*this, state.instructions, funcDataRetriever);
		for (const auto& importedModule : targetModule->importedModules)
			importedModule.module->rootNode->Visit(visitor);

		targetModule->rootNode->Visit(visitor);

		AppendHeader();

		for (const auto& extVarPair : previsitor.extVars)
		{
			const auto& extVar = extVarPair.second;
			for (const auto& varDec : extVar.decorations)
			{
				state.annotations.AppendVariadic(SpirvOp::OpDecorate, [&](auto&& append)
				{
					append(extVar.varData.pointerId);
					append(varDec.decoration);
					for (std::uint32_t extraData : varDec.decorationData)
						append(extraData);
				});
			}
		}

		for (auto&& [varId, builtin] : previsitor.builtinDecorations)
			state.annotations.Append(SpirvOp::OpDecorate, varId, SpirvDecoration::BuiltIn, builtin);

		for (auto&& [varId, location] : previsitor.locationDecorations)
			state.annotations.Append(SpirvOp::OpDecorate, varId, SpirvDecoration::Location, location);

		for (auto&& [varId, interp] : previsitor.interpolationDecorations)
			state.annotations.Append(SpirvOp::OpDecorate, varId, interp);

		m_currentState->constantTypeCache.Write(m_currentState->annotations, m_currentState->constants, m_currentState->debugInfo, states.debugLevel);

		if (m_context.states->debugLevel >= DebugLevel::Minimal)
		{
			for (auto&& [funcIndex, func] : m_currentState->funcs)
				m_currentState->debugInfo.Append(SpirvOp::OpName, func.funcId, func.name);
		}

		std::vector<std::uint32_t> ret;
		MergeSections(ret, state.header);
		MergeSections(ret, state.debugInfo);
		MergeSections(ret, state.annotations);
		MergeSections(ret, state.constants);
		MergeSections(ret, state.instructions);

		return ret;
	}

	const SpirvVariable& SpirvWriter::GetConstantVariable(std::size_t constIndex) const
	{
		return Nz::Retrieve(m_currentState->previsitor->constantVariables, constIndex);
	}

	bool SpirvWriter::IsVersionGreaterOrEqual(std::uint32_t spvMajor, std::uint32_t spvMinor) const
	{
		if (m_environment.spvMajorVersion > spvMajor)
			return true;
		else if (m_environment.spvMajorVersion == spvMajor)
			return m_environment.spvMinorVersion >= spvMinor;
		else
			return false;
	}
	
	void SpirvWriter::SetEnv(Environment environment)
	{
		m_environment = std::move(environment);
	}

	std::pair<std::uint32_t, std::uint32_t> SpirvWriter::GetMaximumSupportedVersion(std::uint32_t vkMajorVersion, std::uint32_t vkMinorVersion)
	{
		assert(vkMinorVersion < 10);
		std::uint32_t vkVersion = vkMajorVersion * 100 + vkMinorVersion * 10;

		if (vkVersion >= 130)
			return { 1, 6 };
		else if (vkVersion >= 120)
			return { 1, 5 };
		else if (vkVersion >= 110)
			return { 1, 3 };
		else
			return { 1, 0 };
	}

	Ast::SanitizeVisitor::Options SpirvWriter::GetSanitizeOptions()
	{
		Ast::SanitizeVisitor::Options options;
		options.reduceLoopsToWhile = true;
		options.removeAliases = true;
		options.removeCompoundAssignments = true;
		options.removeConstArraySize = true;
		options.removeMatrixBinaryAddSub = true;
		options.removeMatrixCast = true;
		options.removeOptionDeclaration = true;
		options.removeSingleConstDeclaration = true;
		options.splitWrappedArrayAssignation = true;
		options.splitMultipleBranches = true;
		options.splitWrappedStructAssignation = true;
		options.useIdentifierAccessesForStructs = false;

		return options;
	}

	std::uint32_t SpirvWriter::AllocateResultId()
	{
		return m_currentState->nextResultId++;
	}

	void SpirvWriter::AppendHeader()
	{
		constexpr std::uint32_t VendorId = 39; //< NZSLc has been registered!

		m_currentState->header.AppendRaw(SpirvMagicNumber); //< SPIR-V magic number

		std::uint32_t version = MakeSpirvVersion(m_environment.spvMajorVersion, m_environment.spvMinorVersion);

		m_currentState->header.AppendRaw(version); //< SPIR-V version number
		m_currentState->header.AppendRaw(VendorId); //< Generator identifier

		m_currentState->header.AppendRaw(m_currentState->nextResultId); //< Bound (ID count)
		m_currentState->header.AppendRaw(0); //< Instruction schema (required to be 0 for now)

		for (SpirvCapability capability : m_currentState->previsitor->spirvCapabilities)
			m_currentState->header.Append(SpirvOp::OpCapability, capability);

		for (const auto& [extInst, resultId] : m_currentState->extensionInstructionSet)
			m_currentState->header.Append(SpirvOp::OpExtInstImport, resultId, extInst);

		m_currentState->header.Append(SpirvOp::OpMemoryModel, SpirvAddressingModel::Logical, SpirvMemoryModel::GLSL450);

		for (auto&& [funcIndex, func] : m_currentState->funcs)
		{
			if (func.entryPointData)
			{
				auto& entryPointData = func.entryPointData.value();

				SpirvExecutionModel execModel;

				switch (entryPointData.stageType)
				{
					case ShaderStageType::Compute:
						execModel = SpirvExecutionModel::GLCompute;
						break;

					case ShaderStageType::Fragment:
						execModel = SpirvExecutionModel::Fragment;
						break;

					case ShaderStageType::Vertex:
						execModel = SpirvExecutionModel::Vertex;
						break;

					default:
						throw std::runtime_error("not yet implemented");
				}

				auto& funcData = func;
				m_currentState->header.AppendVariadic(SpirvOp::OpEntryPoint, [&](const auto& appender)
				{
					appender(execModel);
					appender(funcData.funcId);
					appender(funcData.name);

					for (const auto& input : entryPointData.inputs)
						appender(input.varId);

					for (const auto& output : entryPointData.outputs)
						appender(output.varId);
				});
			}
		}

		// Write execution modes
		for (auto&& [funcIndex, funcData] : m_currentState->funcs)
		{
			if (funcData.entryPointData)
			{
				for (const auto& executionMode : funcData.entryPointData->executionModes)
				{
					// Lambda can't capture bindings until C++20
					const auto& func = funcData;
					const auto& execMode = executionMode;

					m_currentState->header.AppendVariadic(SpirvOp::OpExecutionMode, [&](const auto& appender)
					{
						appender(func.funcId);
						appender(execMode.mode);

						for (std::uint32_t litteral : execMode.params)
							appender(litteral);
					});
				}
			}
		}
	}

	SpirvConstantCache::TypePtr SpirvWriter::BuildType(const Ast::ExpressionType& type)
	{
		return m_currentState->constantTypeCache.BuildType(type);
	}

	SpirvConstantCache::TypePtr SpirvWriter::BuildFunctionType(const Ast::DeclareFunctionStatement& functionNode)
	{
		std::vector<Ast::ExpressionType> parameterTypes;
		parameterTypes.reserve(functionNode.parameters.size());

		for (const auto& parameter : functionNode.parameters)
			parameterTypes.push_back(parameter.type.GetResultingValue());

		if (functionNode.returnType.HasValue())
			return m_currentState->constantTypeCache.BuildFunctionType(functionNode.returnType.GetResultingValue(), parameterTypes);
		else
			return m_currentState->constantTypeCache.BuildFunctionType(Ast::NoType{}, parameterTypes);
	}
	
	std::uint32_t SpirvWriter::GetArrayConstantId(const Ast::ConstantArrayValue& values) const
	{
		return m_currentState->constantTypeCache.GetId(*m_currentState->constantTypeCache.BuildArrayConstant(values));
	}

	std::uint32_t SpirvWriter::GetSingleConstantId(const Ast::ConstantSingleValue& value) const
	{
		return m_currentState->constantTypeCache.GetId(*m_currentState->constantTypeCache.BuildConstant(value));
	}

	std::uint32_t SpirvWriter::GetExtendedInstructionSet(const std::string& instructionSetName) const
	{
		auto it = m_currentState->extensionInstructionSet.find(instructionSetName);
		assert(it != m_currentState->extensionInstructionSet.end());

		return it->second;
	}

	const SpirvVariable& SpirvWriter::GetExtVar(std::size_t extVarIndex) const
	{
		auto it = m_currentState->previsitor->extVars.find(extVarIndex);
		assert(it != m_currentState->previsitor->extVars.end());

		return it->second.varData;
	}

	std::uint32_t SpirvWriter::GetFunctionTypeId(const Ast::DeclareFunctionStatement& functionNode)
	{
		return m_currentState->constantTypeCache.GetId({ *BuildFunctionType(functionNode) });
	}

	std::uint32_t SpirvWriter::GetPointerTypeId(const SpirvConstantCache::TypePtr& typePtr, SpirvStorageClass storageClass) const
	{
		return m_currentState->constantTypeCache.GetId(*m_currentState->constantTypeCache.BuildPointerType(typePtr, storageClass));
	}

	std::uint32_t SpirvWriter::GetPointerTypeId(const Ast::ExpressionType& type, SpirvStorageClass storageClass) const
	{
		return m_currentState->constantTypeCache.GetId(*m_currentState->constantTypeCache.BuildPointerType(type, storageClass));
	}

	std::uint32_t SpirvWriter::GetSourceFileId(const std::shared_ptr<const std::string>& filepathPtr)
	{
		auto it = m_currentState->sourceFiles.find(filepathPtr);
		if (it == m_currentState->sourceFiles.end())
			throw std::runtime_error("unknown source filepath");

		return it->second;
	}

	std::uint32_t SpirvWriter::GetTypeId(const SpirvConstantCache::Type& type) const
	{
		return m_currentState->constantTypeCache.GetId(type);
	}

	std::uint32_t SpirvWriter::GetTypeId(const Ast::ExpressionType& type) const
	{
		return m_currentState->constantTypeCache.GetId(*m_currentState->constantTypeCache.BuildType(type));
	}

	bool SpirvWriter::HasDebugInfo(DebugLevel debugInfo) const
	{
		return m_context.states->debugLevel >= debugInfo;
	}

	std::uint32_t SpirvWriter::RegisterArrayConstant(const Ast::ConstantArrayValue& value)
	{
		return m_currentState->constantTypeCache.Register(*m_currentState->constantTypeCache.BuildArrayConstant(value));
	}

	std::uint32_t SpirvWriter::RegisterFunctionType(const Ast::DeclareFunctionStatement& functionNode)
	{
		return m_currentState->constantTypeCache.Register({ *BuildFunctionType(functionNode) });
	}

	std::uint32_t SpirvWriter::RegisterPointerType(const SpirvConstantCache::TypePtr& typePtr, SpirvStorageClass storageClass)
	{
		return m_currentState->constantTypeCache.Register(*m_currentState->constantTypeCache.BuildPointerType(typePtr, storageClass));
	}

	std::uint32_t SpirvWriter::RegisterPointerType(Ast::ExpressionType type, SpirvStorageClass storageClass)
	{
		return m_currentState->constantTypeCache.Register(*m_currentState->constantTypeCache.BuildPointerType(type, storageClass));
	}

	std::uint32_t SpirvWriter::RegisterSingleConstant(const Ast::ConstantSingleValue& value)
	{
		return m_currentState->constantTypeCache.Register(*m_currentState->constantTypeCache.BuildConstant(value));
	}

	std::uint32_t SpirvWriter::RegisterType(Ast::ExpressionType type)
	{
		assert(m_currentState);
		return m_currentState->constantTypeCache.Register(*m_currentState->constantTypeCache.BuildType(type));
	}

	void SpirvWriter::MergeSections(std::vector<std::uint32_t>& output, const SpirvSection& from)
	{
		const std::vector<std::uint32_t>& bytecode = from.GetBytecode();

		std::size_t prevSize = output.size();
		output.resize(prevSize + bytecode.size());
		std::copy(bytecode.begin(), bytecode.end(), output.begin() + prevSize);
	}
}
