// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

// no header guards

#if !defined(NZSL_SHADERLANG_ERROR) && (!defined(NZSL_SHADERLANG_LEXER_ERROR) || !defined(NZSL_SHADERLANG_PARSER_ERROR) || !defined(NZSL_SHADERLANG_COMPILER_ERROR) || !defined(NZSL_SHADERLANG_AST_ERROR))
#error You must define NZSL_SHADERLANG_ERROR or NZSL_SHADERLANG_LEXER_ERROR/NZSL_SHADERLANG_PARSER_ERROR/NZSL_SHADERLANG_COMPILER_ERROR before including this file
#endif

#ifndef NZSL_SHADERLANG_AST_ERROR
#define NZSL_SHADERLANG_AST_ERROR(...) NZSL_SHADERLANG_ERROR(A, __VA_ARGS__)
#endif

#ifndef NZSL_SHADERLANG_COMPILER_ERROR
#define NZSL_SHADERLANG_COMPILER_ERROR(...) NZSL_SHADERLANG_ERROR(C, __VA_ARGS__)
#endif

#ifndef NZSL_SHADERLANG_LEXER_ERROR
#define NZSL_SHADERLANG_LEXER_ERROR(...) NZSL_SHADERLANG_ERROR(L, __VA_ARGS__)
#endif

#ifndef NZSL_SHADERLANG_PARSER_ERROR
#define NZSL_SHADERLANG_PARSER_ERROR(...) NZSL_SHADERLANG_ERROR(P, __VA_ARGS__)
#endif

// Lexer errors
NZSL_SHADERLANG_LEXER_ERROR(BadNumber, "bad number")
NZSL_SHADERLANG_LEXER_ERROR(NumberOutOfRange, "number is out of range")
NZSL_SHADERLANG_LEXER_ERROR(UnfinishedString, "unfinished string")
NZSL_SHADERLANG_LEXER_ERROR(UnrecognizedChar, "unrecognized character")
NZSL_SHADERLANG_LEXER_ERROR(UnrecognizedToken, "unrecognized token")

// Parser errors
NZSL_SHADERLANG_PARSER_ERROR(AttributeExpectString, "attribute {} requires a string parameter", Ast::AttributeType)
NZSL_SHADERLANG_PARSER_ERROR(AttributeInvalidParameter, "invalid parameter {} for attribute {}", std::string, Ast::AttributeType)
NZSL_SHADERLANG_PARSER_ERROR(AttributeMissingParameter, "attribute {} requires a parameter", Ast::AttributeType)
NZSL_SHADERLANG_PARSER_ERROR(AttributeMultipleUnique, "attribute {} can only be present once", Ast::AttributeType)
NZSL_SHADERLANG_PARSER_ERROR(AttributeParameterIdentifier, "attribute {} parameter can only be an identifier", Ast::AttributeType)
NZSL_SHADERLANG_PARSER_ERROR(ExpectedToken, "expected token {}, got {}", TokenType, TokenType)
NZSL_SHADERLANG_PARSER_ERROR(DuplicateIdentifier, "duplicate identifier")
NZSL_SHADERLANG_PARSER_ERROR(DuplicateModule, "duplicate module")
NZSL_SHADERLANG_PARSER_ERROR(InvalidVersion, "\"{}\" is not a valid version", std::string)
NZSL_SHADERLANG_PARSER_ERROR(MissingAttribute, "missing attribute {}", Ast::AttributeType)
NZSL_SHADERLANG_PARSER_ERROR(ModuleFeatureMultipleUnique, "module feature {} has already been specified", Ast::ModuleFeature)
NZSL_SHADERLANG_PARSER_ERROR(ReservedKeyword, "reserved keyword")
NZSL_SHADERLANG_PARSER_ERROR(UnknownAttribute, "unknown attribute")
NZSL_SHADERLANG_PARSER_ERROR(UnknownType, "unknown type")
NZSL_SHADERLANG_PARSER_ERROR(UnexpectedAttribute, "unexpected attribute {}", Ast::AttributeType)
NZSL_SHADERLANG_PARSER_ERROR(UnexpectedEndOfFile, "unexpected end of file")
NZSL_SHADERLANG_PARSER_ERROR(UnexpectedToken, "unexpected token {}", TokenType)

// Compiler errors
NZSL_SHADERLANG_COMPILER_ERROR(AliasUnexpectedType, "for now, only aliases, functions and structs can be aliased (got {})", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(ArrayLength, "array length must a strictly positive integer, got {}", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(ArrayLengthRequired, "array length is required in this context")
NZSL_SHADERLANG_COMPILER_ERROR(AssignTemporary, "temporary values cannot be assigned")
NZSL_SHADERLANG_COMPILER_ERROR(AttributeUnexpectedExpression, "unexpected expression for this type")
NZSL_SHADERLANG_COMPILER_ERROR(AttributeUnexpectedType, "unexpected attribute type")
NZSL_SHADERLANG_COMPILER_ERROR(BinaryIncompatibleTypes, "incompatibles types ({} and {})", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(BinaryUnsupported, "{} type ({}) does not support this binary operation", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(BuiltinUnexpectedType, "builtin {} expected type {}, got type {}", Ast::BuiltinEntry, std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(BuiltinUnsupportedStage, "builtin {} is not available in {} stage", Ast::BuiltinEntry, ShaderStageType)
NZSL_SHADERLANG_COMPILER_ERROR(BranchOutsideOfFunction, "non-const branching statements can only exist inside a function")
NZSL_SHADERLANG_COMPILER_ERROR(CastComponentMismatch, "component count ({}) doesn't match required component count ({})", std::uint32_t, std::uint32_t)
NZSL_SHADERLANG_COMPILER_ERROR(CastIncompatibleBaseTypes, "incompatibles base types (expected {}, got {})", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(CastIncompatibleTypes, "incompatibles types ({} and {})", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(CastMatrixExpectedVectorOrScalar, "expected vector or scalar type, got {}", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(CastMatrixVectorComponentMismatch, "vector component count ({}) doesn't match target matrix row count ({})", std::uint32_t, std::uint32_t)
NZSL_SHADERLANG_COMPILER_ERROR(CircularImport, "circular import detected on {}", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(ConditionExpectedBool, "expected boolean for condition, got {}", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(ConstMissingExpression, "const variables must have an expression")
NZSL_SHADERLANG_COMPILER_ERROR(ConstantExpectedValue, "expected a value")
NZSL_SHADERLANG_COMPILER_ERROR(ConstantExpressionRequired, "a constant expression is required in this context")
NZSL_SHADERLANG_COMPILER_ERROR(DepthWriteAttribute, "only fragment entry-points can have the depth_write attribute")
NZSL_SHADERLANG_COMPILER_ERROR(DiscardEarlyFragmentTests, "discard is not compatible with early fragment tests")
NZSL_SHADERLANG_COMPILER_ERROR(DiscardOutsideOfFunction, "discard can only be used inside a function")
NZSL_SHADERLANG_COMPILER_ERROR(EarlyFragmentTestsAttribute, "only functions with entry(frag) attribute can have the early_fragments_tests attribute")
NZSL_SHADERLANG_COMPILER_ERROR(EntryFunctionParameter, "entry functions can either take one struct parameter or no parameter")
NZSL_SHADERLANG_COMPILER_ERROR(EntryPointAlreadyDefined, "the {} entry type has been defined multiple times", ShaderStageType)
NZSL_SHADERLANG_COMPILER_ERROR(ExpectedFunction, "expected function expression")
NZSL_SHADERLANG_COMPILER_ERROR(ExpectedIntrinsicFunction, "expected intrinsic function expression")
NZSL_SHADERLANG_COMPILER_ERROR(ExpectedPartialType, "only partial types can be specialized, got {}", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(ExtAlreadyDeclared, "external variable {} is already declared", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(ExtBindingAlreadyUsed, "binding (set={}, binding={}) is already in use", std::uint32_t, std::uint32_t)
NZSL_SHADERLANG_COMPILER_ERROR(ExtMissingBindingIndex, "external variable requires a binding index")
NZSL_SHADERLANG_COMPILER_ERROR(ExtTypeNotAllowed, "external variable {} has unauthorized type ({}): only storage buffers, samplers and uniform buffers (and primitives, vectors and matrices if primitive external feature is enabled) are allowed in external blocks", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(ForEachUnsupportedType, "for-each statements can only be called on array types, got {}", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(ForFromTypeExpectIntegerType, "numerical for from expression must be an integer or unsigned integer, got {}", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(ForStepUnmatchingType, "numerical for step expression type ({}) must match from expression type ({})", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(ForToUnmatchingType, "numerical for to expression type ({}) must match from expression type ({})", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(FullTypeExpected, "expected a full type, got {}", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(FunctionCallExpectedFunction, "expected function expression")
NZSL_SHADERLANG_COMPILER_ERROR(FunctionCallOutsideOfFunction, "function calls must happen inside a function")
NZSL_SHADERLANG_COMPILER_ERROR(FunctionCallUnexpectedEntryFunction, "{} is an entry function which cannot be called by the program", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(FunctionCallUnmatchingParameterCount, "function {} expects {} parameter(s), but got {}", std::string, std::uint32_t, std::uint32_t)
NZSL_SHADERLANG_COMPILER_ERROR(FunctionCallUnmatchingParameterType, "function {} parameter #{} type mismatch (expected {}, got {})", std::string, std::uint32_t, std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(FunctionDeclarationInsideFunction, "a function cannot be defined inside another function")
NZSL_SHADERLANG_COMPILER_ERROR(IdentifierAlreadyUsed, "identifier {} is already used", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(ImportIdentifierAlreadyPresent, "{} identifier was already imported", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(ImportMultipleWildcard, "only one wildcard can be present in an import directive")
NZSL_SHADERLANG_COMPILER_ERROR(ImportWildcardRename, "wildcard cannot be renamed")
NZSL_SHADERLANG_COMPILER_ERROR(IndexRequiresIntegerIndices, "index access requires integer indices (got {})", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(IndexStructRequiresInt32Indices, "struct indexing requires constant i32 indices (got {})", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(IndexUnexpectedType, "unexpected type: only arrays, structs, vectors and matrices can be indexed (got {})", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(IntegralDivisionByZero, "integral division by zero in expression ({} / {})", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(IntegralModuloByZero, "integral modulo by zero in expression ({} % {})", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(IntrinsicExpectedFloat, "expected scalar or vector floating-points")
NZSL_SHADERLANG_COMPILER_ERROR(IntrinsicExpectedParameterCount, "expected {} parameter(s)", std::uint32_t)
NZSL_SHADERLANG_COMPILER_ERROR(IntrinsicExpectedType, "expected type {1} for parameter #{0}, got {2}", std::uint32_t, std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(IntrinsicUnexpectedBoolean, "boolean parameters are not allowed")
NZSL_SHADERLANG_COMPILER_ERROR(IntrinsicUnmatchingParameterType, "all types must match")
NZSL_SHADERLANG_COMPILER_ERROR(InvalidCast, "invalid cast to type {}", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(InvalidScalarSwizzle, "invalid swizzle for scalar")
NZSL_SHADERLANG_COMPILER_ERROR(InvalidStageDependency, "this is only valid in the {} stage but this functions gets called in the {} stage", ShaderStageType, ShaderStageType)
NZSL_SHADERLANG_COMPILER_ERROR(InvalidSwizzle, "invalid swizzle {}", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(LoopControlOutsideOfLoop, "loop control instruction {} found outside of loop", std::string_view)
NZSL_SHADERLANG_COMPILER_ERROR(MissingOptionValue, "option {} requires a value (no default value set)", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(ModuleCompilationFailed, "module {} compilation failed: {}", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(ModuleFeatureMismatch, "module {} requires feature {}", std::string, Ast::ModuleFeature)
NZSL_SHADERLANG_COMPILER_ERROR(ModuleNotFound, "module {} not found", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(NoModuleResolver, "import statement found but no module resolver has been set (and partial sanitization is not enabled)")
NZSL_SHADERLANG_COMPILER_ERROR(OptionDeclarationInsideFunction, "options must be declared outside of functions")
NZSL_SHADERLANG_COMPILER_ERROR(PartialTypeExpect, "expected a {} type at #{}", std::string, std::uint32_t)
NZSL_SHADERLANG_COMPILER_ERROR(PartialTypeTooFewParameters, "parameter count mismatch (expected at least {}, got {})", std::uint32_t, std::uint32_t)
NZSL_SHADERLANG_COMPILER_ERROR(PartialTypeTooManyParameters, "parameter count mismatch (expected at most {}, got {})", std::uint32_t, std::uint32_t)
NZSL_SHADERLANG_COMPILER_ERROR(SamplerUnexpectedType, "for now only f32 samplers are supported (got {})", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(StructDeclarationInsideFunction, "structs must be declared outside of functions")
NZSL_SHADERLANG_COMPILER_ERROR(StructExpected, "struct type expected, got {}", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(StructFieldBuiltinLocation, "a struct field cannot have both builtin and location attributes")
NZSL_SHADERLANG_COMPILER_ERROR(StructFieldMultiple, "multiple {} active struct field found, only one can be active at a time", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(StructLayoutInnerMismatch, "inner struct layout mismatch, struct is declared with {} but field has layout {}", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(StructLayoutTypeNotAllowed, "{} type is not allowed in {} layout", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(SwizzleUnexpectedType, "expression type ({}) does not support swizzling", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(UnaryUnsupported, "type ({}) does not support this unary operation", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(UnexpectedAccessedType, "unexpected type (only struct and vectors can be indexed with identifiers)")
NZSL_SHADERLANG_COMPILER_ERROR(UnknownField, "unknown field {}", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(UnknownIdentifier, "unknown identifier {}", std::string)
NZSL_SHADERLANG_COMPILER_ERROR(UnknownMethod, "type {} has no method {}", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(UnmatchingTypes, "left expression type ({}) doesn't match right expression type ({})", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(VarDeclarationMissingTypeAndValue, "variable must either have a type or an initial value")
NZSL_SHADERLANG_COMPILER_ERROR(VarDeclarationOutsideOfFunction, "global variables outside of external blocks are forbidden")
NZSL_SHADERLANG_COMPILER_ERROR(VarDeclarationTypeUnmatching, "initial expression type ({}) doesn't match specified type ({})", std::string, std::string)
NZSL_SHADERLANG_COMPILER_ERROR(WhileUnrollNotSupported, "unroll(always) is not yet supported on while, use a for loop")

// AST errors
NZSL_SHADERLANG_AST_ERROR(AlreadyUsedIndex, "index {} is already used", std::size_t)
NZSL_SHADERLANG_AST_ERROR(AttributeRequiresValue, "index {} is already used", std::size_t)
NZSL_SHADERLANG_AST_ERROR(AlreadyUsedIndexPreregister, "cannot preregister used index {} as its already used", std::size_t)
NZSL_SHADERLANG_AST_ERROR(EmptyIdentifier, "identifier cannot be empty")
NZSL_SHADERLANG_AST_ERROR(EmptyImport, "no identifiers found in import statement")
NZSL_SHADERLANG_AST_ERROR(Internal, "internal error: {}", std::string)
NZSL_SHADERLANG_AST_ERROR(InvalidConstantIndex, "invalid constant index #{}", std::size_t)
NZSL_SHADERLANG_AST_ERROR(InvalidMethodIndex, "invalid method index #{} for type {}", std::size_t, std::string)
NZSL_SHADERLANG_AST_ERROR(InvalidIndex, "invalid index {}", std::size_t)
NZSL_SHADERLANG_AST_ERROR(MissingExpression, "a mandatory expression is missing")
NZSL_SHADERLANG_AST_ERROR(MissingStatement, "a mandatory statement is missing")
NZSL_SHADERLANG_AST_ERROR(NoIdentifier, "at least one identifier is required")
NZSL_SHADERLANG_AST_ERROR(NoIndex, "at least one index is required")
NZSL_SHADERLANG_AST_ERROR(UnexpectedIdentifier, "unexpected identifier of type {}", std::string)

#undef NZSL_SHADERLANG_ERROR
#undef NZSL_SHADERLANG_AST_ERROR
#undef NZSL_SHADERLANG_COMPILER_ERROR
#undef NZSL_SHADERLANG_LEXER_ERROR
#undef NZSL_SHADERLANG_PARSER_ERROR
