// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

// this file was automatically generated and should not be edited

#pragma once

#ifndef NZSL_SPIRV_SPIRVDATA_HPP
#define NZSL_SPIRV_SPIRVDATA_HPP

#include <NazaraUtils/Flags.hpp>
#include <NZSL/Config.hpp>
#include <string_view>
#include <utility>

namespace nzsl
{
	constexpr std::uint32_t MakeSpirvVersion(std::uint32_t major, std::uint32_t minor)
	{
		return (major << 16) | (minor << 8);
	}

	constexpr std::uint32_t SpirvMagicNumber = 0x07230203;
	constexpr std::uint32_t SpirvMajorVersion = 1;
	constexpr std::uint32_t SpirvMinorVersion = 6;
	constexpr std::uint32_t SpirvRevision = 4;
	constexpr std::uint32_t SpirvVersion = MakeSpirvVersion(SpirvMajorVersion, SpirvMinorVersion);

	enum class SpirvOp
	{
		OpNop = 0,
		OpUndef = 1,
		OpSourceContinued = 2,
		OpSource = 3,
		OpSourceExtension = 4,
		OpName = 5,
		OpMemberName = 6,
		OpString = 7,
		OpLine = 8,
		OpExtension = 10,
		OpExtInstImport = 11,
		OpExtInst = 12,
		OpMemoryModel = 14,
		OpEntryPoint = 15,
		OpExecutionMode = 16,
		OpCapability = 17,
		OpTypeVoid = 19,
		OpTypeBool = 20,
		OpTypeInt = 21,
		OpTypeFloat = 22,
		OpTypeVector = 23,
		OpTypeMatrix = 24,
		OpTypeImage = 25,
		OpTypeSampler = 26,
		OpTypeSampledImage = 27,
		OpTypeArray = 28,
		OpTypeRuntimeArray = 29,
		OpTypeStruct = 30,
		OpTypeOpaque = 31,
		OpTypePointer = 32,
		OpTypeFunction = 33,
		OpTypeEvent = 34,
		OpTypeDeviceEvent = 35,
		OpTypeReserveId = 36,
		OpTypeQueue = 37,
		OpTypePipe = 38,
		OpTypeForwardPointer = 39,
		OpConstantTrue = 41,
		OpConstantFalse = 42,
		OpConstant = 43,
		OpConstantComposite = 44,
		OpConstantSampler = 45,
		OpConstantNull = 46,
		OpSpecConstantTrue = 48,
		OpSpecConstantFalse = 49,
		OpSpecConstant = 50,
		OpSpecConstantComposite = 51,
		OpSpecConstantOp = 52,
		OpFunction = 54,
		OpFunctionParameter = 55,
		OpFunctionEnd = 56,
		OpFunctionCall = 57,
		OpVariable = 59,
		OpImageTexelPointer = 60,
		OpLoad = 61,
		OpStore = 62,
		OpCopyMemory = 63,
		OpCopyMemorySized = 64,
		OpAccessChain = 65,
		OpInBoundsAccessChain = 66,
		OpPtrAccessChain = 67,
		OpArrayLength = 68,
		OpGenericPtrMemSemantics = 69,
		OpInBoundsPtrAccessChain = 70,
		OpDecorate = 71,
		OpMemberDecorate = 72,
		OpDecorationGroup = 73,
		OpGroupDecorate = 74,
		OpGroupMemberDecorate = 75,
		OpVectorExtractDynamic = 77,
		OpVectorInsertDynamic = 78,
		OpVectorShuffle = 79,
		OpCompositeConstruct = 80,
		OpCompositeExtract = 81,
		OpCompositeInsert = 82,
		OpCopyObject = 83,
		OpTranspose = 84,
		OpSampledImage = 86,
		OpImageSampleImplicitLod = 87,
		OpImageSampleExplicitLod = 88,
		OpImageSampleDrefImplicitLod = 89,
		OpImageSampleDrefExplicitLod = 90,
		OpImageSampleProjImplicitLod = 91,
		OpImageSampleProjExplicitLod = 92,
		OpImageSampleProjDrefImplicitLod = 93,
		OpImageSampleProjDrefExplicitLod = 94,
		OpImageFetch = 95,
		OpImageGather = 96,
		OpImageDrefGather = 97,
		OpImageRead = 98,
		OpImageWrite = 99,
		OpImage = 100,
		OpImageQueryFormat = 101,
		OpImageQueryOrder = 102,
		OpImageQuerySizeLod = 103,
		OpImageQuerySize = 104,
		OpImageQueryLod = 105,
		OpImageQueryLevels = 106,
		OpImageQuerySamples = 107,
		OpConvertFToU = 109,
		OpConvertFToS = 110,
		OpConvertSToF = 111,
		OpConvertUToF = 112,
		OpUConvert = 113,
		OpSConvert = 114,
		OpFConvert = 115,
		OpQuantizeToF16 = 116,
		OpConvertPtrToU = 117,
		OpSatConvertSToU = 118,
		OpSatConvertUToS = 119,
		OpConvertUToPtr = 120,
		OpPtrCastToGeneric = 121,
		OpGenericCastToPtr = 122,
		OpGenericCastToPtrExplicit = 123,
		OpBitcast = 124,
		OpSNegate = 126,
		OpFNegate = 127,
		OpIAdd = 128,
		OpFAdd = 129,
		OpISub = 130,
		OpFSub = 131,
		OpIMul = 132,
		OpFMul = 133,
		OpUDiv = 134,
		OpSDiv = 135,
		OpFDiv = 136,
		OpUMod = 137,
		OpSRem = 138,
		OpSMod = 139,
		OpFRem = 140,
		OpFMod = 141,
		OpVectorTimesScalar = 142,
		OpMatrixTimesScalar = 143,
		OpVectorTimesMatrix = 144,
		OpMatrixTimesVector = 145,
		OpMatrixTimesMatrix = 146,
		OpOuterProduct = 147,
		OpDot = 148,
		OpIAddCarry = 149,
		OpISubBorrow = 150,
		OpUMulExtended = 151,
		OpSMulExtended = 152,
		OpAny = 154,
		OpAll = 155,
		OpIsNan = 156,
		OpIsInf = 157,
		OpIsFinite = 158,
		OpIsNormal = 159,
		OpSignBitSet = 160,
		OpLessOrGreater = 161,
		OpOrdered = 162,
		OpUnordered = 163,
		OpLogicalEqual = 164,
		OpLogicalNotEqual = 165,
		OpLogicalOr = 166,
		OpLogicalAnd = 167,
		OpLogicalNot = 168,
		OpSelect = 169,
		OpIEqual = 170,
		OpINotEqual = 171,
		OpUGreaterThan = 172,
		OpSGreaterThan = 173,
		OpUGreaterThanEqual = 174,
		OpSGreaterThanEqual = 175,
		OpULessThan = 176,
		OpSLessThan = 177,
		OpULessThanEqual = 178,
		OpSLessThanEqual = 179,
		OpFOrdEqual = 180,
		OpFUnordEqual = 181,
		OpFOrdNotEqual = 182,
		OpFUnordNotEqual = 183,
		OpFOrdLessThan = 184,
		OpFUnordLessThan = 185,
		OpFOrdGreaterThan = 186,
		OpFUnordGreaterThan = 187,
		OpFOrdLessThanEqual = 188,
		OpFUnordLessThanEqual = 189,
		OpFOrdGreaterThanEqual = 190,
		OpFUnordGreaterThanEqual = 191,
		OpShiftRightLogical = 194,
		OpShiftRightArithmetic = 195,
		OpShiftLeftLogical = 196,
		OpBitwiseOr = 197,
		OpBitwiseXor = 198,
		OpBitwiseAnd = 199,
		OpNot = 200,
		OpBitFieldInsert = 201,
		OpBitFieldSExtract = 202,
		OpBitFieldUExtract = 203,
		OpBitReverse = 204,
		OpBitCount = 205,
		OpDPdx = 207,
		OpDPdy = 208,
		OpFwidth = 209,
		OpDPdxFine = 210,
		OpDPdyFine = 211,
		OpFwidthFine = 212,
		OpDPdxCoarse = 213,
		OpDPdyCoarse = 214,
		OpFwidthCoarse = 215,
		OpEmitVertex = 218,
		OpEndPrimitive = 219,
		OpEmitStreamVertex = 220,
		OpEndStreamPrimitive = 221,
		OpControlBarrier = 224,
		OpMemoryBarrier = 225,
		OpAtomicLoad = 227,
		OpAtomicStore = 228,
		OpAtomicExchange = 229,
		OpAtomicCompareExchange = 230,
		OpAtomicCompareExchangeWeak = 231,
		OpAtomicIIncrement = 232,
		OpAtomicIDecrement = 233,
		OpAtomicIAdd = 234,
		OpAtomicISub = 235,
		OpAtomicSMin = 236,
		OpAtomicUMin = 237,
		OpAtomicSMax = 238,
		OpAtomicUMax = 239,
		OpAtomicAnd = 240,
		OpAtomicOr = 241,
		OpAtomicXor = 242,
		OpPhi = 245,
		OpLoopMerge = 246,
		OpSelectionMerge = 247,
		OpLabel = 248,
		OpBranch = 249,
		OpBranchConditional = 250,
		OpSwitch = 251,
		OpKill = 252,
		OpReturn = 253,
		OpReturnValue = 254,
		OpUnreachable = 255,
		OpLifetimeStart = 256,
		OpLifetimeStop = 257,
		OpGroupAsyncCopy = 259,
		OpGroupWaitEvents = 260,
		OpGroupAll = 261,
		OpGroupAny = 262,
		OpGroupBroadcast = 263,
		OpGroupIAdd = 264,
		OpGroupFAdd = 265,
		OpGroupFMin = 266,
		OpGroupUMin = 267,
		OpGroupSMin = 268,
		OpGroupFMax = 269,
		OpGroupUMax = 270,
		OpGroupSMax = 271,
		OpReadPipe = 274,
		OpWritePipe = 275,
		OpReservedReadPipe = 276,
		OpReservedWritePipe = 277,
		OpReserveReadPipePackets = 278,
		OpReserveWritePipePackets = 279,
		OpCommitReadPipe = 280,
		OpCommitWritePipe = 281,
		OpIsValidReserveId = 282,
		OpGetNumPipePackets = 283,
		OpGetMaxPipePackets = 284,
		OpGroupReserveReadPipePackets = 285,
		OpGroupReserveWritePipePackets = 286,
		OpGroupCommitReadPipe = 287,
		OpGroupCommitWritePipe = 288,
		OpEnqueueMarker = 291,
		OpEnqueueKernel = 292,
		OpGetKernelNDrangeSubGroupCount = 293,
		OpGetKernelNDrangeMaxSubGroupSize = 294,
		OpGetKernelWorkGroupSize = 295,
		OpGetKernelPreferredWorkGroupSizeMultiple = 296,
		OpRetainEvent = 297,
		OpReleaseEvent = 298,
		OpCreateUserEvent = 299,
		OpIsValidEvent = 300,
		OpSetUserEventStatus = 301,
		OpCaptureEventProfilingInfo = 302,
		OpGetDefaultQueue = 303,
		OpBuildNDRange = 304,
		OpImageSparseSampleImplicitLod = 305,
		OpImageSparseSampleExplicitLod = 306,
		OpImageSparseSampleDrefImplicitLod = 307,
		OpImageSparseSampleDrefExplicitLod = 308,
		OpImageSparseSampleProjImplicitLod = 309,
		OpImageSparseSampleProjExplicitLod = 310,
		OpImageSparseSampleProjDrefImplicitLod = 311,
		OpImageSparseSampleProjDrefExplicitLod = 312,
		OpImageSparseFetch = 313,
		OpImageSparseGather = 314,
		OpImageSparseDrefGather = 315,
		OpImageSparseTexelsResident = 316,
		OpNoLine = 317,
		OpAtomicFlagTestAndSet = 318,
		OpAtomicFlagClear = 319,
		OpImageSparseRead = 320,
		OpSizeOf = 321,
		OpTypePipeStorage = 322,
		OpConstantPipeStorage = 323,
		OpCreatePipeFromPipeStorage = 324,
		OpGetKernelLocalSizeForSubgroupCount = 325,
		OpGetKernelMaxNumSubgroups = 326,
		OpTypeNamedBarrier = 327,
		OpNamedBarrierInitialize = 328,
		OpMemoryNamedBarrier = 329,
		OpModuleProcessed = 330,
		OpExecutionModeId = 331,
		OpDecorateId = 332,
		OpGroupNonUniformElect = 333,
		OpGroupNonUniformAll = 334,
		OpGroupNonUniformAny = 335,
		OpGroupNonUniformAllEqual = 336,
		OpGroupNonUniformBroadcast = 337,
		OpGroupNonUniformBroadcastFirst = 338,
		OpGroupNonUniformBallot = 339,
		OpGroupNonUniformInverseBallot = 340,
		OpGroupNonUniformBallotBitExtract = 341,
		OpGroupNonUniformBallotBitCount = 342,
		OpGroupNonUniformBallotFindLSB = 343,
		OpGroupNonUniformBallotFindMSB = 344,
		OpGroupNonUniformShuffle = 345,
		OpGroupNonUniformShuffleXor = 346,
		OpGroupNonUniformShuffleUp = 347,
		OpGroupNonUniformShuffleDown = 348,
		OpGroupNonUniformIAdd = 349,
		OpGroupNonUniformFAdd = 350,
		OpGroupNonUniformIMul = 351,
		OpGroupNonUniformFMul = 352,
		OpGroupNonUniformSMin = 353,
		OpGroupNonUniformUMin = 354,
		OpGroupNonUniformFMin = 355,
		OpGroupNonUniformSMax = 356,
		OpGroupNonUniformUMax = 357,
		OpGroupNonUniformFMax = 358,
		OpGroupNonUniformBitwiseAnd = 359,
		OpGroupNonUniformBitwiseOr = 360,
		OpGroupNonUniformBitwiseXor = 361,
		OpGroupNonUniformLogicalAnd = 362,
		OpGroupNonUniformLogicalOr = 363,
		OpGroupNonUniformLogicalXor = 364,
		OpGroupNonUniformQuadBroadcast = 365,
		OpGroupNonUniformQuadSwap = 366,
		OpCopyLogical = 400,
		OpPtrEqual = 401,
		OpPtrNotEqual = 402,
		OpPtrDiff = 403,
		OpColorAttachmentReadEXT = 4160,
		OpDepthAttachmentReadEXT = 4161,
		OpStencilAttachmentReadEXT = 4162,
		OpTerminateInvocation = 4416,
		OpTypeUntypedPointerKHR = 4417,
		OpUntypedVariableKHR = 4418,
		OpUntypedAccessChainKHR = 4419,
		OpUntypedInBoundsAccessChainKHR = 4420,
		OpSubgroupBallotKHR = 4421,
		OpSubgroupFirstInvocationKHR = 4422,
		OpUntypedPtrAccessChainKHR = 4423,
		OpUntypedInBoundsPtrAccessChainKHR = 4424,
		OpUntypedArrayLengthKHR = 4425,
		OpUntypedPrefetchKHR = 4426,
		OpSubgroupAllKHR = 4428,
		OpSubgroupAnyKHR = 4429,
		OpSubgroupAllEqualKHR = 4430,
		OpGroupNonUniformRotateKHR = 4431,
		OpSubgroupReadInvocationKHR = 4432,
		OpExtInstWithForwardRefsKHR = 4433,
		OpTraceRayKHR = 4445,
		OpExecuteCallableKHR = 4446,
		OpConvertUToAccelerationStructureKHR = 4447,
		OpIgnoreIntersectionKHR = 4448,
		OpTerminateRayKHR = 4449,
		OpSDot = 4450,
		OpUDot = 4451,
		OpSUDot = 4452,
		OpSDotAccSat = 4453,
		OpUDotAccSat = 4454,
		OpSUDotAccSat = 4455,
		OpTypeCooperativeMatrixKHR = 4456,
		OpCooperativeMatrixLoadKHR = 4457,
		OpCooperativeMatrixStoreKHR = 4458,
		OpCooperativeMatrixMulAddKHR = 4459,
		OpCooperativeMatrixLengthKHR = 4460,
		OpConstantCompositeReplicateEXT = 4461,
		OpSpecConstantCompositeReplicateEXT = 4462,
		OpCompositeConstructReplicateEXT = 4463,
		OpTypeRayQueryKHR = 4472,
		OpRayQueryInitializeKHR = 4473,
		OpRayQueryTerminateKHR = 4474,
		OpRayQueryGenerateIntersectionKHR = 4475,
		OpRayQueryConfirmIntersectionKHR = 4476,
		OpRayQueryProceedKHR = 4477,
		OpRayQueryGetIntersectionTypeKHR = 4479,
		OpImageSampleWeightedQCOM = 4480,
		OpImageBoxFilterQCOM = 4481,
		OpImageBlockMatchSSDQCOM = 4482,
		OpImageBlockMatchSADQCOM = 4483,
		OpImageBlockMatchWindowSSDQCOM = 4500,
		OpImageBlockMatchWindowSADQCOM = 4501,
		OpImageBlockMatchGatherSSDQCOM = 4502,
		OpImageBlockMatchGatherSADQCOM = 4503,
		OpGroupIAddNonUniformAMD = 5000,
		OpGroupFAddNonUniformAMD = 5001,
		OpGroupFMinNonUniformAMD = 5002,
		OpGroupUMinNonUniformAMD = 5003,
		OpGroupSMinNonUniformAMD = 5004,
		OpGroupFMaxNonUniformAMD = 5005,
		OpGroupUMaxNonUniformAMD = 5006,
		OpGroupSMaxNonUniformAMD = 5007,
		OpFragmentMaskFetchAMD = 5011,
		OpFragmentFetchAMD = 5012,
		OpReadClockKHR = 5056,
		OpAllocateNodePayloadsAMDX = 5074,
		OpEnqueueNodePayloadsAMDX = 5075,
		OpTypeNodePayloadArrayAMDX = 5076,
		OpFinishWritingNodePayloadAMDX = 5078,
		OpNodePayloadArrayLengthAMDX = 5090,
		OpIsNodePayloadValidAMDX = 5101,
		OpConstantStringAMDX = 5103,
		OpSpecConstantStringAMDX = 5104,
		OpGroupNonUniformQuadAllKHR = 5110,
		OpGroupNonUniformQuadAnyKHR = 5111,
		OpHitObjectRecordHitMotionNV = 5249,
		OpHitObjectRecordHitWithIndexMotionNV = 5250,
		OpHitObjectRecordMissMotionNV = 5251,
		OpHitObjectGetWorldToObjectNV = 5252,
		OpHitObjectGetObjectToWorldNV = 5253,
		OpHitObjectGetObjectRayDirectionNV = 5254,
		OpHitObjectGetObjectRayOriginNV = 5255,
		OpHitObjectTraceRayMotionNV = 5256,
		OpHitObjectGetShaderRecordBufferHandleNV = 5257,
		OpHitObjectGetShaderBindingTableRecordIndexNV = 5258,
		OpHitObjectRecordEmptyNV = 5259,
		OpHitObjectTraceRayNV = 5260,
		OpHitObjectRecordHitNV = 5261,
		OpHitObjectRecordHitWithIndexNV = 5262,
		OpHitObjectRecordMissNV = 5263,
		OpHitObjectExecuteShaderNV = 5264,
		OpHitObjectGetCurrentTimeNV = 5265,
		OpHitObjectGetAttributesNV = 5266,
		OpHitObjectGetHitKindNV = 5267,
		OpHitObjectGetPrimitiveIndexNV = 5268,
		OpHitObjectGetGeometryIndexNV = 5269,
		OpHitObjectGetInstanceIdNV = 5270,
		OpHitObjectGetInstanceCustomIndexNV = 5271,
		OpHitObjectGetWorldRayDirectionNV = 5272,
		OpHitObjectGetWorldRayOriginNV = 5273,
		OpHitObjectGetRayTMaxNV = 5274,
		OpHitObjectGetRayTMinNV = 5275,
		OpHitObjectIsEmptyNV = 5276,
		OpHitObjectIsHitNV = 5277,
		OpHitObjectIsMissNV = 5278,
		OpReorderThreadWithHitObjectNV = 5279,
		OpReorderThreadWithHintNV = 5280,
		OpTypeHitObjectNV = 5281,
		OpImageSampleFootprintNV = 5283,
		OpCooperativeMatrixConvertNV = 5293,
		OpEmitMeshTasksEXT = 5294,
		OpSetMeshOutputsEXT = 5295,
		OpGroupNonUniformPartitionNV = 5296,
		OpWritePackedPrimitiveIndices4x8NV = 5299,
		OpFetchMicroTriangleVertexPositionNV = 5300,
		OpFetchMicroTriangleVertexBarycentricNV = 5301,
		OpReportIntersectionKHR = 5334,
		OpIgnoreIntersectionNV = 5335,
		OpTerminateRayNV = 5336,
		OpTraceNV = 5337,
		OpTraceMotionNV = 5338,
		OpTraceRayMotionNV = 5339,
		OpRayQueryGetIntersectionTriangleVertexPositionsKHR = 5340,
		OpTypeAccelerationStructureKHR = 5341,
		OpExecuteCallableNV = 5344,
		OpTypeCooperativeMatrixNV = 5358,
		OpCooperativeMatrixLoadNV = 5359,
		OpCooperativeMatrixStoreNV = 5360,
		OpCooperativeMatrixMulAddNV = 5361,
		OpCooperativeMatrixLengthNV = 5362,
		OpBeginInvocationInterlockEXT = 5364,
		OpEndInvocationInterlockEXT = 5365,
		OpCooperativeMatrixReduceNV = 5366,
		OpCooperativeMatrixLoadTensorNV = 5367,
		OpCooperativeMatrixStoreTensorNV = 5368,
		OpCooperativeMatrixPerElementOpNV = 5369,
		OpTypeTensorLayoutNV = 5370,
		OpTypeTensorViewNV = 5371,
		OpCreateTensorLayoutNV = 5372,
		OpTensorLayoutSetDimensionNV = 5373,
		OpTensorLayoutSetStrideNV = 5374,
		OpTensorLayoutSliceNV = 5375,
		OpTensorLayoutSetClampValueNV = 5376,
		OpCreateTensorViewNV = 5377,
		OpTensorViewSetDimensionNV = 5378,
		OpTensorViewSetStrideNV = 5379,
		OpDemoteToHelperInvocation = 5380,
		OpIsHelperInvocationEXT = 5381,
		OpTensorViewSetClipNV = 5382,
		OpTensorLayoutSetBlockSizeNV = 5384,
		OpCooperativeMatrixTransposeNV = 5390,
		OpConvertUToImageNV = 5391,
		OpConvertUToSamplerNV = 5392,
		OpConvertImageToUNV = 5393,
		OpConvertSamplerToUNV = 5394,
		OpConvertUToSampledImageNV = 5395,
		OpConvertSampledImageToUNV = 5396,
		OpSamplerImageAddressingModeNV = 5397,
		OpRawAccessChainNV = 5398,
		OpSubgroupShuffleINTEL = 5571,
		OpSubgroupShuffleDownINTEL = 5572,
		OpSubgroupShuffleUpINTEL = 5573,
		OpSubgroupShuffleXorINTEL = 5574,
		OpSubgroupBlockReadINTEL = 5575,
		OpSubgroupBlockWriteINTEL = 5576,
		OpSubgroupImageBlockReadINTEL = 5577,
		OpSubgroupImageBlockWriteINTEL = 5578,
		OpSubgroupImageMediaBlockReadINTEL = 5580,
		OpSubgroupImageMediaBlockWriteINTEL = 5581,
		OpUCountLeadingZerosINTEL = 5585,
		OpUCountTrailingZerosINTEL = 5586,
		OpAbsISubINTEL = 5587,
		OpAbsUSubINTEL = 5588,
		OpIAddSatINTEL = 5589,
		OpUAddSatINTEL = 5590,
		OpIAverageINTEL = 5591,
		OpUAverageINTEL = 5592,
		OpIAverageRoundedINTEL = 5593,
		OpUAverageRoundedINTEL = 5594,
		OpISubSatINTEL = 5595,
		OpUSubSatINTEL = 5596,
		OpIMul32x16INTEL = 5597,
		OpUMul32x16INTEL = 5598,
		OpConstantFunctionPointerINTEL = 5600,
		OpFunctionPointerCallINTEL = 5601,
		OpAsmTargetINTEL = 5609,
		OpAsmINTEL = 5610,
		OpAsmCallINTEL = 5611,
		OpAtomicFMinEXT = 5614,
		OpAtomicFMaxEXT = 5615,
		OpAssumeTrueKHR = 5630,
		OpExpectKHR = 5631,
		OpDecorateString = 5632,
		OpMemberDecorateString = 5633,
		OpVmeImageINTEL = 5699,
		OpTypeVmeImageINTEL = 5700,
		OpTypeAvcImePayloadINTEL = 5701,
		OpTypeAvcRefPayloadINTEL = 5702,
		OpTypeAvcSicPayloadINTEL = 5703,
		OpTypeAvcMcePayloadINTEL = 5704,
		OpTypeAvcMceResultINTEL = 5705,
		OpTypeAvcImeResultINTEL = 5706,
		OpTypeAvcImeResultSingleReferenceStreamoutINTEL = 5707,
		OpTypeAvcImeResultDualReferenceStreamoutINTEL = 5708,
		OpTypeAvcImeSingleReferenceStreaminINTEL = 5709,
		OpTypeAvcImeDualReferenceStreaminINTEL = 5710,
		OpTypeAvcRefResultINTEL = 5711,
		OpTypeAvcSicResultINTEL = 5712,
		OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL = 5713,
		OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL = 5714,
		OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL = 5715,
		OpSubgroupAvcMceSetInterShapePenaltyINTEL = 5716,
		OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL = 5717,
		OpSubgroupAvcMceSetInterDirectionPenaltyINTEL = 5718,
		OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL = 5719,
		OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL = 5720,
		OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL = 5721,
		OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL = 5722,
		OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL = 5723,
		OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL = 5724,
		OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL = 5725,
		OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL = 5726,
		OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL = 5727,
		OpSubgroupAvcMceSetAcOnlyHaarINTEL = 5728,
		OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL = 5729,
		OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL = 5730,
		OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL = 5731,
		OpSubgroupAvcMceConvertToImePayloadINTEL = 5732,
		OpSubgroupAvcMceConvertToImeResultINTEL = 5733,
		OpSubgroupAvcMceConvertToRefPayloadINTEL = 5734,
		OpSubgroupAvcMceConvertToRefResultINTEL = 5735,
		OpSubgroupAvcMceConvertToSicPayloadINTEL = 5736,
		OpSubgroupAvcMceConvertToSicResultINTEL = 5737,
		OpSubgroupAvcMceGetMotionVectorsINTEL = 5738,
		OpSubgroupAvcMceGetInterDistortionsINTEL = 5739,
		OpSubgroupAvcMceGetBestInterDistortionsINTEL = 5740,
		OpSubgroupAvcMceGetInterMajorShapeINTEL = 5741,
		OpSubgroupAvcMceGetInterMinorShapeINTEL = 5742,
		OpSubgroupAvcMceGetInterDirectionsINTEL = 5743,
		OpSubgroupAvcMceGetInterMotionVectorCountINTEL = 5744,
		OpSubgroupAvcMceGetInterReferenceIdsINTEL = 5745,
		OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL = 5746,
		OpSubgroupAvcImeInitializeINTEL = 5747,
		OpSubgroupAvcImeSetSingleReferenceINTEL = 5748,
		OpSubgroupAvcImeSetDualReferenceINTEL = 5749,
		OpSubgroupAvcImeRefWindowSizeINTEL = 5750,
		OpSubgroupAvcImeAdjustRefOffsetINTEL = 5751,
		OpSubgroupAvcImeConvertToMcePayloadINTEL = 5752,
		OpSubgroupAvcImeSetMaxMotionVectorCountINTEL = 5753,
		OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL = 5754,
		OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL = 5755,
		OpSubgroupAvcImeSetWeightedSadINTEL = 5756,
		OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL = 5757,
		OpSubgroupAvcImeEvaluateWithDualReferenceINTEL = 5758,
		OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL = 5759,
		OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL = 5760,
		OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL = 5761,
		OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL = 5762,
		OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL = 5763,
		OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL = 5764,
		OpSubgroupAvcImeConvertToMceResultINTEL = 5765,
		OpSubgroupAvcImeGetSingleReferenceStreaminINTEL = 5766,
		OpSubgroupAvcImeGetDualReferenceStreaminINTEL = 5767,
		OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL = 5768,
		OpSubgroupAvcImeStripDualReferenceStreamoutINTEL = 5769,
		OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL = 5770,
		OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL = 5771,
		OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL = 5772,
		OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL = 5773,
		OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL = 5774,
		OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL = 5775,
		OpSubgroupAvcImeGetBorderReachedINTEL = 5776,
		OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL = 5777,
		OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL = 5778,
		OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL = 5779,
		OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL = 5780,
		OpSubgroupAvcFmeInitializeINTEL = 5781,
		OpSubgroupAvcBmeInitializeINTEL = 5782,
		OpSubgroupAvcRefConvertToMcePayloadINTEL = 5783,
		OpSubgroupAvcRefSetBidirectionalMixDisableINTEL = 5784,
		OpSubgroupAvcRefSetBilinearFilterEnableINTEL = 5785,
		OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL = 5786,
		OpSubgroupAvcRefEvaluateWithDualReferenceINTEL = 5787,
		OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL = 5788,
		OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL = 5789,
		OpSubgroupAvcRefConvertToMceResultINTEL = 5790,
		OpSubgroupAvcSicInitializeINTEL = 5791,
		OpSubgroupAvcSicConfigureSkcINTEL = 5792,
		OpSubgroupAvcSicConfigureIpeLumaINTEL = 5793,
		OpSubgroupAvcSicConfigureIpeLumaChromaINTEL = 5794,
		OpSubgroupAvcSicGetMotionVectorMaskINTEL = 5795,
		OpSubgroupAvcSicConvertToMcePayloadINTEL = 5796,
		OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL = 5797,
		OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL = 5798,
		OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL = 5799,
		OpSubgroupAvcSicSetBilinearFilterEnableINTEL = 5800,
		OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL = 5801,
		OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL = 5802,
		OpSubgroupAvcSicEvaluateIpeINTEL = 5803,
		OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL = 5804,
		OpSubgroupAvcSicEvaluateWithDualReferenceINTEL = 5805,
		OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL = 5806,
		OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL = 5807,
		OpSubgroupAvcSicConvertToMceResultINTEL = 5808,
		OpSubgroupAvcSicGetIpeLumaShapeINTEL = 5809,
		OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL = 5810,
		OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL = 5811,
		OpSubgroupAvcSicGetPackedIpeLumaModesINTEL = 5812,
		OpSubgroupAvcSicGetIpeChromaModeINTEL = 5813,
		OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL = 5814,
		OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL = 5815,
		OpSubgroupAvcSicGetInterRawSadsINTEL = 5816,
		OpVariableLengthArrayINTEL = 5818,
		OpSaveMemoryINTEL = 5819,
		OpRestoreMemoryINTEL = 5820,
		OpArbitraryFloatSinCosPiINTEL = 5840,
		OpArbitraryFloatCastINTEL = 5841,
		OpArbitraryFloatCastFromIntINTEL = 5842,
		OpArbitraryFloatCastToIntINTEL = 5843,
		OpArbitraryFloatAddINTEL = 5846,
		OpArbitraryFloatSubINTEL = 5847,
		OpArbitraryFloatMulINTEL = 5848,
		OpArbitraryFloatDivINTEL = 5849,
		OpArbitraryFloatGTINTEL = 5850,
		OpArbitraryFloatGEINTEL = 5851,
		OpArbitraryFloatLTINTEL = 5852,
		OpArbitraryFloatLEINTEL = 5853,
		OpArbitraryFloatEQINTEL = 5854,
		OpArbitraryFloatRecipINTEL = 5855,
		OpArbitraryFloatRSqrtINTEL = 5856,
		OpArbitraryFloatCbrtINTEL = 5857,
		OpArbitraryFloatHypotINTEL = 5858,
		OpArbitraryFloatSqrtINTEL = 5859,
		OpArbitraryFloatLogINTEL = 5860,
		OpArbitraryFloatLog2INTEL = 5861,
		OpArbitraryFloatLog10INTEL = 5862,
		OpArbitraryFloatLog1pINTEL = 5863,
		OpArbitraryFloatExpINTEL = 5864,
		OpArbitraryFloatExp2INTEL = 5865,
		OpArbitraryFloatExp10INTEL = 5866,
		OpArbitraryFloatExpm1INTEL = 5867,
		OpArbitraryFloatSinINTEL = 5868,
		OpArbitraryFloatCosINTEL = 5869,
		OpArbitraryFloatSinCosINTEL = 5870,
		OpArbitraryFloatSinPiINTEL = 5871,
		OpArbitraryFloatCosPiINTEL = 5872,
		OpArbitraryFloatASinINTEL = 5873,
		OpArbitraryFloatASinPiINTEL = 5874,
		OpArbitraryFloatACosINTEL = 5875,
		OpArbitraryFloatACosPiINTEL = 5876,
		OpArbitraryFloatATanINTEL = 5877,
		OpArbitraryFloatATanPiINTEL = 5878,
		OpArbitraryFloatATan2INTEL = 5879,
		OpArbitraryFloatPowINTEL = 5880,
		OpArbitraryFloatPowRINTEL = 5881,
		OpArbitraryFloatPowNINTEL = 5882,
		OpLoopControlINTEL = 5887,
		OpAliasDomainDeclINTEL = 5911,
		OpAliasScopeDeclINTEL = 5912,
		OpAliasScopeListDeclINTEL = 5913,
		OpFixedSqrtINTEL = 5923,
		OpFixedRecipINTEL = 5924,
		OpFixedRsqrtINTEL = 5925,
		OpFixedSinINTEL = 5926,
		OpFixedCosINTEL = 5927,
		OpFixedSinCosINTEL = 5928,
		OpFixedSinPiINTEL = 5929,
		OpFixedCosPiINTEL = 5930,
		OpFixedSinCosPiINTEL = 5931,
		OpFixedLogINTEL = 5932,
		OpFixedExpINTEL = 5933,
		OpPtrCastToCrossWorkgroupINTEL = 5934,
		OpCrossWorkgroupCastToPtrINTEL = 5938,
		OpReadPipeBlockingINTEL = 5946,
		OpWritePipeBlockingINTEL = 5947,
		OpFPGARegINTEL = 5949,
		OpRayQueryGetRayTMinKHR = 6016,
		OpRayQueryGetRayFlagsKHR = 6017,
		OpRayQueryGetIntersectionTKHR = 6018,
		OpRayQueryGetIntersectionInstanceCustomIndexKHR = 6019,
		OpRayQueryGetIntersectionInstanceIdKHR = 6020,
		OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR = 6021,
		OpRayQueryGetIntersectionGeometryIndexKHR = 6022,
		OpRayQueryGetIntersectionPrimitiveIndexKHR = 6023,
		OpRayQueryGetIntersectionBarycentricsKHR = 6024,
		OpRayQueryGetIntersectionFrontFaceKHR = 6025,
		OpRayQueryGetIntersectionCandidateAABBOpaqueKHR = 6026,
		OpRayQueryGetIntersectionObjectRayDirectionKHR = 6027,
		OpRayQueryGetIntersectionObjectRayOriginKHR = 6028,
		OpRayQueryGetWorldRayDirectionKHR = 6029,
		OpRayQueryGetWorldRayOriginKHR = 6030,
		OpRayQueryGetIntersectionObjectToWorldKHR = 6031,
		OpRayQueryGetIntersectionWorldToObjectKHR = 6032,
		OpAtomicFAddEXT = 6035,
		OpTypeBufferSurfaceINTEL = 6086,
		OpTypeStructContinuedINTEL = 6090,
		OpConstantCompositeContinuedINTEL = 6091,
		OpSpecConstantCompositeContinuedINTEL = 6092,
		OpCompositeConstructContinuedINTEL = 6096,
		OpConvertFToBF16INTEL = 6116,
		OpConvertBF16ToFINTEL = 6117,
		OpControlBarrierArriveINTEL = 6142,
		OpControlBarrierWaitINTEL = 6143,
		OpArithmeticFenceEXT = 6145,
		OpSubgroupBlockPrefetchINTEL = 6221,
		OpGroupIMulKHR = 6401,
		OpGroupFMulKHR = 6402,
		OpGroupBitwiseAndKHR = 6403,
		OpGroupBitwiseOrKHR = 6404,
		OpGroupBitwiseXorKHR = 6405,
		OpGroupLogicalAndKHR = 6406,
		OpGroupLogicalOrKHR = 6407,
		OpGroupLogicalXorKHR = 6408,
		OpMaskedGatherINTEL = 6428,
		OpMaskedScatterINTEL = 6429,
	};

	enum class SpirvGlslStd450Op
	{
		Round = 1,
		RoundEven = 2,
		Trunc = 3,
		FAbs = 4,
		SAbs = 5,
		FSign = 6,
		SSign = 7,
		Floor = 8,
		Ceil = 9,
		Fract = 10,
		Radians = 11,
		Degrees = 12,
		Sin = 13,
		Cos = 14,
		Tan = 15,
		Asin = 16,
		Acos = 17,
		Atan = 18,
		Sinh = 19,
		Cosh = 20,
		Tanh = 21,
		Asinh = 22,
		Acosh = 23,
		Atanh = 24,
		Atan2 = 25,
		Pow = 26,
		Exp = 27,
		Log = 28,
		Exp2 = 29,
		Log2 = 30,
		Sqrt = 31,
		InverseSqrt = 32,
		Determinant = 33,
		MatrixInverse = 34,
		Modf = 35,
		ModfStruct = 36,
		FMin = 37,
		UMin = 38,
		SMin = 39,
		FMax = 40,
		UMax = 41,
		SMax = 42,
		FClamp = 43,
		UClamp = 44,
		SClamp = 45,
		FMix = 46,
		IMix = 47,
		Step = 48,
		SmoothStep = 49,
		Fma = 50,
		Frexp = 51,
		FrexpStruct = 52,
		Ldexp = 53,
		PackSnorm4x8 = 54,
		PackUnorm4x8 = 55,
		PackSnorm2x16 = 56,
		PackUnorm2x16 = 57,
		PackHalf2x16 = 58,
		PackDouble2x32 = 59,
		UnpackSnorm2x16 = 60,
		UnpackUnorm2x16 = 61,
		UnpackHalf2x16 = 62,
		UnpackSnorm4x8 = 63,
		UnpackUnorm4x8 = 64,
		UnpackDouble2x32 = 65,
		Length = 66,
		Distance = 67,
		Cross = 68,
		Normalize = 69,
		FaceForward = 70,
		Reflect = 71,
		Refract = 72,
		FindILsb = 73,
		FindSMsb = 74,
		FindUMsb = 75,
		InterpolateAtCentroid = 76,
		InterpolateAtSample = 77,
		InterpolateAtOffset = 78,
		NMin = 79,
		NMax = 80,
		NClamp = 81,
	};

	enum class SpirvOperandKind
	{
		ImageOperands,
		FPFastMathMode,
		SelectionControl,
		LoopControl,
		FunctionControl,
		MemorySemantics,
		MemoryAccess,
		KernelProfilingInfo,
		RayFlags,
		FragmentShadingRate,
		RawAccessChainOperands,
		SourceLanguage,
		ExecutionModel,
		AddressingModel,
		MemoryModel,
		ExecutionMode,
		StorageClass,
		Dim,
		SamplerAddressingMode,
		SamplerFilterMode,
		ImageFormat,
		ImageChannelOrder,
		ImageChannelDataType,
		FPRoundingMode,
		FPDenormMode,
		QuantizationModes,
		FPOperationMode,
		OverflowModes,
		LinkageType,
		AccessQualifier,
		HostAccessQualifier,
		FunctionParameterAttribute,
		Decoration,
		BuiltIn,
		Scope,
		GroupOperation,
		KernelEnqueueFlags,
		Capability,
		RayQueryIntersection,
		RayQueryCommittedIntersectionType,
		RayQueryCandidateIntersectionType,
		PackedVectorFormat,
		CooperativeMatrixOperands,
		CooperativeMatrixLayout,
		CooperativeMatrixUse,
		CooperativeMatrixReduce,
		TensorClampMode,
		TensorAddressingOperands,
		InitializationModeQualifier,
		LoadCacheControl,
		StoreCacheControl,
		NamedMaximumNumberOfRegisters,
		FPEncoding,
		IdResultType,
		IdResult,
		IdMemorySemantics,
		IdScope,
		IdRef,
		LiteralInteger,
		LiteralString,
		LiteralFloat,
		LiteralContextDependentNumber,
		LiteralExtInstInteger,
		LiteralSpecConstantOpInteger,
		PairLiteralIntegerIdRef,
		PairIdRefLiteralInteger,
		PairIdRefIdRef,
	};

	enum class SpirvImageOperands
	{
		None = 0x0000,
		Bias = 0x0001,
		Lod = 0x0002,
		Grad = 0x0004,
		ConstOffset = 0x0008,
		Offset = 0x0010,
		ConstOffsets = 0x0020,
		Sample = 0x0040,
		MinLod = 0x0080,
		MakeTexelAvailable = 0x0100,
		MakeTexelVisible = 0x0200,
		NonPrivateTexel = 0x0400,
		VolatileTexel = 0x0800,
		SignExtend = 0x1000,
		ZeroExtend = 0x2000,
		Nontemporal = 0x4000,
		Offsets = 0x10000,
	};

	enum class SpirvFPFastMathMode
	{
		None = 0x0000,
		NotNaN = 0x0001,
		NotInf = 0x0002,
		NSZ = 0x0004,
		AllowRecip = 0x0008,
		Fast = 0x0010,
		AllowContract = 0x10000,
		AllowReassoc = 0x20000,
		AllowTransform = 0x40000,
	};

	enum class SpirvSelectionControl
	{
		None = 0x0000,
		Flatten = 0x0001,
		DontFlatten = 0x0002,
	};

	enum class SpirvLoopControl
	{
		None = 0x0000,
		Unroll = 0x0001,
		DontUnroll = 0x0002,
		DependencyInfinite = 0x0004,
		DependencyLength = 0x0008,
		MinIterations = 0x0010,
		MaxIterations = 0x0020,
		IterationMultiple = 0x0040,
		PeelCount = 0x0080,
		PartialCount = 0x0100,
		InitiationIntervalINTEL = 0x10000,
		MaxConcurrencyINTEL = 0x20000,
		DependencyArrayINTEL = 0x40000,
		PipelineEnableINTEL = 0x80000,
		LoopCoalesceINTEL = 0x100000,
		MaxInterleavingINTEL = 0x200000,
		SpeculatedIterationsINTEL = 0x400000,
		NoFusionINTEL = 0x800000,
		LoopCountINTEL = 0x1000000,
		MaxReinvocationDelayINTEL = 0x2000000,
	};

	enum class SpirvFunctionControl
	{
		None = 0x0000,
		Inline = 0x0001,
		DontInline = 0x0002,
		Pure = 0x0004,
		Const = 0x0008,
		OptNoneEXT = 0x10000,
	};

	enum class SpirvMemorySemantics
	{
		Relaxed = 0x0000,
		Acquire = 0x0002,
		Release = 0x0004,
		AcquireRelease = 0x0008,
		SequentiallyConsistent = 0x0010,
		UniformMemory = 0x0040,
		SubgroupMemory = 0x0080,
		WorkgroupMemory = 0x0100,
		CrossWorkgroupMemory = 0x0200,
		AtomicCounterMemory = 0x0400,
		ImageMemory = 0x0800,
		OutputMemory = 0x1000,
		MakeAvailable = 0x2000,
		MakeVisible = 0x4000,
		Volatile = 0x8000,
	};

	enum class SpirvMemoryAccess
	{
		None = 0x0000,
		Volatile = 0x0001,
		Aligned = 0x0002,
		Nontemporal = 0x0004,
		MakePointerAvailable = 0x0008,
		MakePointerVisible = 0x0010,
		NonPrivatePointer = 0x0020,
		AliasScopeINTELMask = 0x10000,
		NoAliasINTELMask = 0x20000,
	};

	enum class SpirvKernelProfilingInfo
	{
		None = 0x0000,
		CmdExecTime = 0x0001,
	};

	enum class SpirvRayFlags
	{
		NoneKHR = 0x0000,
		OpaqueKHR = 0x0001,
		NoOpaqueKHR = 0x0002,
		TerminateOnFirstHitKHR = 0x0004,
		SkipClosestHitShaderKHR = 0x0008,
		CullBackFacingTrianglesKHR = 0x0010,
		CullFrontFacingTrianglesKHR = 0x0020,
		CullOpaqueKHR = 0x0040,
		CullNoOpaqueKHR = 0x0080,
		SkipTrianglesKHR = 0x0100,
		SkipAABBsKHR = 0x0200,
		ForceOpacityMicromap2StateEXT = 0x0400,
	};

	enum class SpirvFragmentShadingRate
	{
		Vertical2Pixels = 0x0001,
		Vertical4Pixels = 0x0002,
		Horizontal2Pixels = 0x0004,
		Horizontal4Pixels = 0x0008,
	};

	enum class SpirvRawAccessChainOperands
	{
		None = 0x0000,
		RobustnessPerComponentNV = 0x0001,
		RobustnessPerElementNV = 0x0002,
	};

	enum class SpirvSourceLanguage
	{
		Unknown = 0,
		ESSL = 1,
		GLSL = 2,
		OpenCL_C = 3,
		OpenCL_CPP = 4,
		HLSL = 5,
		CPP_for_OpenCL = 6,
		SYCL = 7,
		HERO_C = 8,
		NZSL = 9,
		WGSL = 10,
		Slang = 11,
		Zig = 12,
	};

	enum class SpirvExecutionModel
	{
		Vertex = 0,
		TessellationControl = 1,
		TessellationEvaluation = 2,
		Geometry = 3,
		Fragment = 4,
		GLCompute = 5,
		Kernel = 6,
		TaskNV = 5267,
		MeshNV = 5268,
		RayGenerationKHR = 5313,
		IntersectionKHR = 5314,
		AnyHitKHR = 5315,
		ClosestHitKHR = 5316,
		MissKHR = 5317,
		CallableKHR = 5318,
		TaskEXT = 5364,
		MeshEXT = 5365,
	};

	enum class SpirvAddressingModel
	{
		Logical = 0,
		Physical32 = 1,
		Physical64 = 2,
		PhysicalStorageBuffer64 = 5348,
	};

	enum class SpirvMemoryModel
	{
		Simple = 0,
		GLSL450 = 1,
		OpenCL = 2,
		Vulkan = 3,
	};

	enum class SpirvExecutionMode
	{
		Invocations = 0,
		SpacingEqual = 1,
		SpacingFractionalEven = 2,
		SpacingFractionalOdd = 3,
		VertexOrderCw = 4,
		VertexOrderCcw = 5,
		PixelCenterInteger = 6,
		OriginUpperLeft = 7,
		OriginLowerLeft = 8,
		EarlyFragmentTests = 9,
		PointMode = 10,
		Xfb = 11,
		DepthReplacing = 12,
		DepthGreater = 14,
		DepthLess = 15,
		DepthUnchanged = 16,
		LocalSize = 17,
		LocalSizeHint = 18,
		InputPoints = 19,
		InputLines = 20,
		InputLinesAdjacency = 21,
		Triangles = 22,
		InputTrianglesAdjacency = 23,
		Quads = 24,
		Isolines = 25,
		OutputVertices = 26,
		OutputPoints = 27,
		OutputLineStrip = 28,
		OutputTriangleStrip = 29,
		VecTypeHint = 30,
		ContractionOff = 31,
		Initializer = 33,
		Finalizer = 34,
		SubgroupSize = 35,
		SubgroupsPerWorkgroup = 36,
		SubgroupsPerWorkgroupId = 37,
		LocalSizeId = 38,
		LocalSizeHintId = 39,
		NonCoherentColorAttachmentReadEXT = 4169,
		NonCoherentDepthAttachmentReadEXT = 4170,
		NonCoherentStencilAttachmentReadEXT = 4171,
		SubgroupUniformControlFlowKHR = 4421,
		PostDepthCoverage = 4446,
		DenormPreserve = 4459,
		DenormFlushToZero = 4460,
		SignedZeroInfNanPreserve = 4461,
		RoundingModeRTE = 4462,
		RoundingModeRTZ = 4463,
		EarlyAndLateFragmentTestsAMD = 5017,
		StencilRefReplacingEXT = 5027,
		CoalescingAMDX = 5069,
		IsApiEntryAMDX = 5070,
		MaxNodeRecursionAMDX = 5071,
		StaticNumWorkgroupsAMDX = 5072,
		ShaderIndexAMDX = 5073,
		MaxNumWorkgroupsAMDX = 5077,
		StencilRefUnchangedFrontAMD = 5079,
		StencilRefGreaterFrontAMD = 5080,
		StencilRefLessFrontAMD = 5081,
		StencilRefUnchangedBackAMD = 5082,
		StencilRefGreaterBackAMD = 5083,
		StencilRefLessBackAMD = 5084,
		QuadDerivativesKHR = 5088,
		RequireFullQuadsKHR = 5089,
		SharesInputWithAMDX = 5102,
		OutputLinesEXT = 5269,
		OutputPrimitivesEXT = 5270,
		DerivativeGroupQuadsKHR = 5289,
		DerivativeGroupLinearKHR = 5290,
		OutputTrianglesEXT = 5298,
		PixelInterlockOrderedEXT = 5366,
		PixelInterlockUnorderedEXT = 5367,
		SampleInterlockOrderedEXT = 5368,
		SampleInterlockUnorderedEXT = 5369,
		ShadingRateInterlockOrderedEXT = 5370,
		ShadingRateInterlockUnorderedEXT = 5371,
		SharedLocalMemorySizeINTEL = 5618,
		RoundingModeRTPINTEL = 5620,
		RoundingModeRTNINTEL = 5621,
		FloatingPointModeALTINTEL = 5622,
		FloatingPointModeIEEEINTEL = 5623,
		MaxWorkgroupSizeINTEL = 5893,
		MaxWorkDimINTEL = 5894,
		NoGlobalOffsetINTEL = 5895,
		NumSIMDWorkitemsINTEL = 5896,
		SchedulerTargetFmaxMhzINTEL = 5903,
		MaximallyReconvergesKHR = 6023,
		FPFastMathDefault = 6028,
		StreamingInterfaceINTEL = 6154,
		RegisterMapInterfaceINTEL = 6160,
		NamedBarrierCountINTEL = 6417,
		MaximumRegistersINTEL = 6461,
		MaximumRegistersIdINTEL = 6462,
		NamedMaximumRegistersINTEL = 6463,
	};

	enum class SpirvStorageClass
	{
		UniformConstant = 0,
		Input = 1,
		Uniform = 2,
		Output = 3,
		Workgroup = 4,
		CrossWorkgroup = 5,
		Private = 6,
		Function = 7,
		Generic = 8,
		PushConstant = 9,
		AtomicCounter = 10,
		Image = 11,
		StorageBuffer = 12,
		TileImageEXT = 4172,
		NodePayloadAMDX = 5068,
		CallableDataKHR = 5328,
		IncomingCallableDataKHR = 5329,
		RayPayloadKHR = 5338,
		HitAttributeKHR = 5339,
		IncomingRayPayloadKHR = 5342,
		ShaderRecordBufferKHR = 5343,
		PhysicalStorageBuffer = 5349,
		HitObjectAttributeNV = 5385,
		TaskPayloadWorkgroupEXT = 5402,
		CodeSectionINTEL = 5605,
		DeviceOnlyINTEL = 5936,
		HostOnlyINTEL = 5937,
	};

	enum class SpirvDim
	{
		Dim1D = 0,
		Dim2D = 1,
		Dim3D = 2,
		Cube = 3,
		Rect = 4,
		Buffer = 5,
		SubpassData = 6,
		TileImageDataEXT = 4173,
	};

	enum class SpirvSamplerAddressingMode
	{
		None = 0,
		ClampToEdge = 1,
		Clamp = 2,
		Repeat = 3,
		RepeatMirrored = 4,
	};

	enum class SpirvSamplerFilterMode
	{
		Nearest = 0,
		Linear = 1,
	};

	enum class SpirvImageFormat
	{
		Unknown = 0,
		Rgba32f = 1,
		Rgba16f = 2,
		R32f = 3,
		Rgba8 = 4,
		Rgba8Snorm = 5,
		Rg32f = 6,
		Rg16f = 7,
		R11fG11fB10f = 8,
		R16f = 9,
		Rgba16 = 10,
		Rgb10A2 = 11,
		Rg16 = 12,
		Rg8 = 13,
		R16 = 14,
		R8 = 15,
		Rgba16Snorm = 16,
		Rg16Snorm = 17,
		Rg8Snorm = 18,
		R16Snorm = 19,
		R8Snorm = 20,
		Rgba32i = 21,
		Rgba16i = 22,
		Rgba8i = 23,
		R32i = 24,
		Rg32i = 25,
		Rg16i = 26,
		Rg8i = 27,
		R16i = 28,
		R8i = 29,
		Rgba32ui = 30,
		Rgba16ui = 31,
		Rgba8ui = 32,
		R32ui = 33,
		Rgb10a2ui = 34,
		Rg32ui = 35,
		Rg16ui = 36,
		Rg8ui = 37,
		R16ui = 38,
		R8ui = 39,
		R64ui = 40,
		R64i = 41,
	};

	enum class SpirvImageChannelOrder
	{
		R = 0,
		A = 1,
		RG = 2,
		RA = 3,
		RGB = 4,
		RGBA = 5,
		BGRA = 6,
		ARGB = 7,
		Intensity = 8,
		Luminance = 9,
		Rx = 10,
		RGx = 11,
		RGBx = 12,
		Depth = 13,
		DepthStencil = 14,
		sRGB = 15,
		sRGBx = 16,
		sRGBA = 17,
		sBGRA = 18,
		ABGR = 19,
	};

	enum class SpirvImageChannelDataType
	{
		SnormInt8 = 0,
		SnormInt16 = 1,
		UnormInt8 = 2,
		UnormInt16 = 3,
		UnormShort565 = 4,
		UnormShort555 = 5,
		UnormInt101010 = 6,
		SignedInt8 = 7,
		SignedInt16 = 8,
		SignedInt32 = 9,
		UnsignedInt8 = 10,
		UnsignedInt16 = 11,
		UnsignedInt32 = 12,
		HalfFloat = 13,
		Float = 14,
		UnormInt24 = 15,
		UnormInt101010_2 = 16,
		UnsignedIntRaw10EXT = 19,
		UnsignedIntRaw12EXT = 20,
		UnormInt2_101010EXT = 21,
	};

	enum class SpirvFPRoundingMode
	{
		RTE = 0,
		RTZ = 1,
		RTP = 2,
		RTN = 3,
	};

	enum class SpirvFPDenormMode
	{
		Preserve = 0,
		FlushToZero = 1,
	};

	enum class SpirvQuantizationModes
	{
		TRN = 0,
		TRN_ZERO = 1,
		RND = 2,
		RND_ZERO = 3,
		RND_INF = 4,
		RND_MIN_INF = 5,
		RND_CONV = 6,
		RND_CONV_ODD = 7,
	};

	enum class SpirvFPOperationMode
	{
		IEEE = 0,
		ALT = 1,
	};

	enum class SpirvOverflowModes
	{
		WRAP = 0,
		SAT = 1,
		SAT_ZERO = 2,
		SAT_SYM = 3,
	};

	enum class SpirvLinkageType
	{
		Export = 0,
		Import = 1,
		LinkOnceODR = 2,
	};

	enum class SpirvAccessQualifier
	{
		ReadOnly = 0,
		WriteOnly = 1,
		ReadWrite = 2,
	};

	enum class SpirvHostAccessQualifier
	{
		NoneINTEL = 0,
		ReadINTEL = 1,
		WriteINTEL = 2,
		ReadWriteINTEL = 3,
	};

	enum class SpirvFunctionParameterAttribute
	{
		Zext = 0,
		Sext = 1,
		ByVal = 2,
		Sret = 3,
		NoAlias = 4,
		NoCapture = 5,
		NoWrite = 6,
		NoReadWrite = 7,
		RuntimeAlignedINTEL = 5940,
	};

	enum class SpirvDecoration
	{
		RelaxedPrecision = 0,
		SpecId = 1,
		Block = 2,
		BufferBlock = 3,
		RowMajor = 4,
		ColMajor = 5,
		ArrayStride = 6,
		MatrixStride = 7,
		GLSLShared = 8,
		GLSLPacked = 9,
		CPacked = 10,
		BuiltIn = 11,
		NoPerspective = 13,
		Flat = 14,
		Patch = 15,
		Centroid = 16,
		Sample = 17,
		Invariant = 18,
		Restrict = 19,
		Aliased = 20,
		Volatile = 21,
		Constant = 22,
		Coherent = 23,
		NonWritable = 24,
		NonReadable = 25,
		Uniform = 26,
		UniformId = 27,
		SaturatedConversion = 28,
		Stream = 29,
		Location = 30,
		Component = 31,
		Index = 32,
		Binding = 33,
		DescriptorSet = 34,
		Offset = 35,
		XfbBuffer = 36,
		XfbStride = 37,
		FuncParamAttr = 38,
		FPRoundingMode = 39,
		FPFastMathMode = 40,
		LinkageAttributes = 41,
		NoContraction = 42,
		InputAttachmentIndex = 43,
		Alignment = 44,
		MaxByteOffset = 45,
		AlignmentId = 46,
		MaxByteOffsetId = 47,
		NoSignedWrap = 4469,
		NoUnsignedWrap = 4470,
		WeightTextureQCOM = 4487,
		BlockMatchTextureQCOM = 4488,
		BlockMatchSamplerQCOM = 4499,
		ExplicitInterpAMD = 4999,
		NodeSharesPayloadLimitsWithAMDX = 5019,
		NodeMaxPayloadsAMDX = 5020,
		TrackFinishWritingAMDX = 5078,
		PayloadNodeNameAMDX = 5091,
		PayloadNodeBaseIndexAMDX = 5098,
		PayloadNodeSparseArrayAMDX = 5099,
		PayloadNodeArraySizeAMDX = 5100,
		PayloadDispatchIndirectAMDX = 5105,
		OverrideCoverageNV = 5248,
		PassthroughNV = 5250,
		ViewportRelativeNV = 5252,
		SecondaryViewportRelativeNV = 5256,
		PerPrimitiveEXT = 5271,
		PerViewNV = 5272,
		PerTaskNV = 5273,
		PerVertexKHR = 5285,
		NonUniform = 5300,
		RestrictPointer = 5355,
		AliasedPointer = 5356,
		HitObjectShaderRecordBufferNV = 5386,
		BindlessSamplerNV = 5398,
		BindlessImageNV = 5399,
		BoundSamplerNV = 5400,
		BoundImageNV = 5401,
		SIMTCallINTEL = 5599,
		ReferencedIndirectlyINTEL = 5602,
		ClobberINTEL = 5607,
		SideEffectsINTEL = 5608,
		VectorComputeVariableINTEL = 5624,
		FuncParamIOKindINTEL = 5625,
		VectorComputeFunctionINTEL = 5626,
		StackCallINTEL = 5627,
		GlobalVariableOffsetINTEL = 5628,
		CounterBuffer = 5634,
		UserSemantic = 5635,
		UserTypeGOOGLE = 5636,
		FunctionRoundingModeINTEL = 5822,
		FunctionDenormModeINTEL = 5823,
		RegisterINTEL = 5825,
		MemoryINTEL = 5826,
		NumbanksINTEL = 5827,
		BankwidthINTEL = 5828,
		MaxPrivateCopiesINTEL = 5829,
		SinglepumpINTEL = 5830,
		DoublepumpINTEL = 5831,
		MaxReplicatesINTEL = 5832,
		SimpleDualPortINTEL = 5833,
		MergeINTEL = 5834,
		BankBitsINTEL = 5835,
		ForcePow2DepthINTEL = 5836,
		StridesizeINTEL = 5883,
		WordsizeINTEL = 5884,
		TrueDualPortINTEL = 5885,
		BurstCoalesceINTEL = 5899,
		CacheSizeINTEL = 5900,
		DontStaticallyCoalesceINTEL = 5901,
		PrefetchINTEL = 5902,
		StallEnableINTEL = 5905,
		FuseLoopsInFunctionINTEL = 5907,
		MathOpDSPModeINTEL = 5909,
		AliasScopeINTEL = 5914,
		NoAliasINTEL = 5915,
		InitiationIntervalINTEL = 5917,
		MaxConcurrencyINTEL = 5918,
		PipelineEnableINTEL = 5919,
		BufferLocationINTEL = 5921,
		IOPipeStorageINTEL = 5944,
		FunctionFloatingPointModeINTEL = 6080,
		SingleElementVectorINTEL = 6085,
		VectorComputeCallableFunctionINTEL = 6087,
		MediaBlockIOINTEL = 6140,
		StallFreeINTEL = 6151,
		FPMaxErrorDecorationINTEL = 6170,
		LatencyControlLabelINTEL = 6172,
		LatencyControlConstraintINTEL = 6173,
		ConduitKernelArgumentINTEL = 6175,
		RegisterMapKernelArgumentINTEL = 6176,
		MMHostInterfaceAddressWidthINTEL = 6177,
		MMHostInterfaceDataWidthINTEL = 6178,
		MMHostInterfaceLatencyINTEL = 6179,
		MMHostInterfaceReadWriteModeINTEL = 6180,
		MMHostInterfaceMaxBurstINTEL = 6181,
		MMHostInterfaceWaitRequestINTEL = 6182,
		StableKernelArgumentINTEL = 6183,
		HostAccessINTEL = 6188,
		InitModeINTEL = 6190,
		ImplementInRegisterMapINTEL = 6191,
		CacheControlLoadINTEL = 6442,
		CacheControlStoreINTEL = 6443,
	};

	enum class SpirvBuiltIn
	{
		Position = 0,
		PointSize = 1,
		ClipDistance = 3,
		CullDistance = 4,
		VertexId = 5,
		InstanceId = 6,
		PrimitiveId = 7,
		InvocationId = 8,
		Layer = 9,
		ViewportIndex = 10,
		TessLevelOuter = 11,
		TessLevelInner = 12,
		TessCoord = 13,
		PatchVertices = 14,
		FragCoord = 15,
		PointCoord = 16,
		FrontFacing = 17,
		SampleId = 18,
		SamplePosition = 19,
		SampleMask = 20,
		FragDepth = 22,
		HelperInvocation = 23,
		NumWorkgroups = 24,
		WorkgroupSize = 25,
		WorkgroupId = 26,
		LocalInvocationId = 27,
		GlobalInvocationId = 28,
		LocalInvocationIndex = 29,
		WorkDim = 30,
		GlobalSize = 31,
		EnqueuedWorkgroupSize = 32,
		GlobalOffset = 33,
		GlobalLinearId = 34,
		SubgroupSize = 36,
		SubgroupMaxSize = 37,
		NumSubgroups = 38,
		NumEnqueuedSubgroups = 39,
		SubgroupId = 40,
		SubgroupLocalInvocationId = 41,
		VertexIndex = 42,
		InstanceIndex = 43,
		CoreIDARM = 4160,
		CoreCountARM = 4161,
		CoreMaxIDARM = 4162,
		WarpIDARM = 4163,
		WarpMaxIDARM = 4164,
		SubgroupEqMask = 4416,
		SubgroupGeMask = 4417,
		SubgroupGtMask = 4418,
		SubgroupLeMask = 4419,
		SubgroupLtMask = 4420,
		BaseVertex = 4424,
		BaseInstance = 4425,
		DrawIndex = 4426,
		PrimitiveShadingRateKHR = 4432,
		DeviceIndex = 4438,
		ViewIndex = 4440,
		ShadingRateKHR = 4444,
		BaryCoordNoPerspAMD = 4992,
		BaryCoordNoPerspCentroidAMD = 4993,
		BaryCoordNoPerspSampleAMD = 4994,
		BaryCoordSmoothAMD = 4995,
		BaryCoordSmoothCentroidAMD = 4996,
		BaryCoordSmoothSampleAMD = 4997,
		BaryCoordPullModelAMD = 4998,
		FragStencilRefEXT = 5014,
		RemainingRecursionLevelsAMDX = 5021,
		ShaderIndexAMDX = 5073,
		ViewportMaskNV = 5253,
		SecondaryPositionNV = 5257,
		SecondaryViewportMaskNV = 5258,
		PositionPerViewNV = 5261,
		ViewportMaskPerViewNV = 5262,
		FullyCoveredEXT = 5264,
		TaskCountNV = 5274,
		PrimitiveCountNV = 5275,
		PrimitiveIndicesNV = 5276,
		ClipDistancePerViewNV = 5277,
		CullDistancePerViewNV = 5278,
		LayerPerViewNV = 5279,
		MeshViewCountNV = 5280,
		MeshViewIndicesNV = 5281,
		BaryCoordKHR = 5286,
		BaryCoordNoPerspKHR = 5287,
		FragSizeEXT = 5292,
		FragInvocationCountEXT = 5293,
		PrimitivePointIndicesEXT = 5294,
		PrimitiveLineIndicesEXT = 5295,
		PrimitiveTriangleIndicesEXT = 5296,
		CullPrimitiveEXT = 5299,
		LaunchIdKHR = 5319,
		LaunchSizeKHR = 5320,
		WorldRayOriginKHR = 5321,
		WorldRayDirectionKHR = 5322,
		ObjectRayOriginKHR = 5323,
		ObjectRayDirectionKHR = 5324,
		RayTminKHR = 5325,
		RayTmaxKHR = 5326,
		InstanceCustomIndexKHR = 5327,
		ObjectToWorldKHR = 5330,
		WorldToObjectKHR = 5331,
		HitTNV = 5332,
		HitKindKHR = 5333,
		CurrentRayTimeNV = 5334,
		HitTriangleVertexPositionsKHR = 5335,
		HitMicroTriangleVertexPositionsNV = 5337,
		HitMicroTriangleVertexBarycentricsNV = 5344,
		IncomingRayFlagsKHR = 5351,
		RayGeometryIndexKHR = 5352,
		WarpsPerSMNV = 5374,
		SMCountNV = 5375,
		WarpIDNV = 5376,
		SMIDNV = 5377,
		HitKindFrontFacingMicroTriangleNV = 5405,
		HitKindBackFacingMicroTriangleNV = 5406,
		CullMaskKHR = 6021,
	};

	enum class SpirvScope
	{
		CrossDevice = 0,
		Device = 1,
		Workgroup = 2,
		Subgroup = 3,
		Invocation = 4,
		QueueFamily = 5,
		ShaderCallKHR = 6,
	};

	enum class SpirvGroupOperation
	{
		Reduce = 0,
		InclusiveScan = 1,
		ExclusiveScan = 2,
		ClusteredReduce = 3,
		PartitionedReduceNV = 6,
		PartitionedInclusiveScanNV = 7,
		PartitionedExclusiveScanNV = 8,
	};

	enum class SpirvKernelEnqueueFlags
	{
		NoWait = 0,
		WaitKernel = 1,
		WaitWorkGroup = 2,
	};

	enum class SpirvCapability
	{
		Matrix = 0,
		Shader = 1,
		Geometry = 2,
		Tessellation = 3,
		Addresses = 4,
		Linkage = 5,
		Kernel = 6,
		Vector16 = 7,
		Float16Buffer = 8,
		Float16 = 9,
		Float64 = 10,
		Int64 = 11,
		Int64Atomics = 12,
		ImageBasic = 13,
		ImageReadWrite = 14,
		ImageMipmap = 15,
		Pipes = 17,
		Groups = 18,
		DeviceEnqueue = 19,
		LiteralSampler = 20,
		AtomicStorage = 21,
		Int16 = 22,
		TessellationPointSize = 23,
		GeometryPointSize = 24,
		ImageGatherExtended = 25,
		StorageImageMultisample = 27,
		UniformBufferArrayDynamicIndexing = 28,
		SampledImageArrayDynamicIndexing = 29,
		StorageBufferArrayDynamicIndexing = 30,
		StorageImageArrayDynamicIndexing = 31,
		ClipDistance = 32,
		CullDistance = 33,
		ImageCubeArray = 34,
		SampleRateShading = 35,
		ImageRect = 36,
		SampledRect = 37,
		GenericPointer = 38,
		Int8 = 39,
		InputAttachment = 40,
		SparseResidency = 41,
		MinLod = 42,
		Sampled1D = 43,
		Image1D = 44,
		SampledCubeArray = 45,
		SampledBuffer = 46,
		ImageBuffer = 47,
		ImageMSArray = 48,
		StorageImageExtendedFormats = 49,
		ImageQuery = 50,
		DerivativeControl = 51,
		InterpolationFunction = 52,
		TransformFeedback = 53,
		GeometryStreams = 54,
		StorageImageReadWithoutFormat = 55,
		StorageImageWriteWithoutFormat = 56,
		MultiViewport = 57,
		SubgroupDispatch = 58,
		NamedBarrier = 59,
		PipeStorage = 60,
		GroupNonUniform = 61,
		GroupNonUniformVote = 62,
		GroupNonUniformArithmetic = 63,
		GroupNonUniformBallot = 64,
		GroupNonUniformShuffle = 65,
		GroupNonUniformShuffleRelative = 66,
		GroupNonUniformClustered = 67,
		GroupNonUniformQuad = 68,
		ShaderLayer = 69,
		ShaderViewportIndex = 70,
		UniformDecoration = 71,
		CoreBuiltinsARM = 4165,
		TileImageColorReadAccessEXT = 4166,
		TileImageDepthReadAccessEXT = 4167,
		TileImageStencilReadAccessEXT = 4168,
		CooperativeMatrixLayoutsARM = 4201,
		FragmentShadingRateKHR = 4422,
		SubgroupBallotKHR = 4423,
		DrawParameters = 4427,
		WorkgroupMemoryExplicitLayoutKHR = 4428,
		WorkgroupMemoryExplicitLayout8BitAccessKHR = 4429,
		WorkgroupMemoryExplicitLayout16BitAccessKHR = 4430,
		SubgroupVoteKHR = 4431,
		StorageBuffer16BitAccess = 4433,
		UniformAndStorageBuffer16BitAccess = 4434,
		StoragePushConstant16 = 4435,
		StorageInputOutput16 = 4436,
		DeviceGroup = 4437,
		MultiView = 4439,
		VariablePointersStorageBuffer = 4441,
		VariablePointers = 4442,
		AtomicStorageOps = 4445,
		SampleMaskPostDepthCoverage = 4447,
		StorageBuffer8BitAccess = 4448,
		UniformAndStorageBuffer8BitAccess = 4449,
		StoragePushConstant8 = 4450,
		DenormPreserve = 4464,
		DenormFlushToZero = 4465,
		SignedZeroInfNanPreserve = 4466,
		RoundingModeRTE = 4467,
		RoundingModeRTZ = 4468,
		RayQueryProvisionalKHR = 4471,
		RayQueryKHR = 4472,
		UntypedPointersKHR = 4473,
		RayTraversalPrimitiveCullingKHR = 4478,
		RayTracingKHR = 4479,
		TextureSampleWeightedQCOM = 4484,
		TextureBoxFilterQCOM = 4485,
		TextureBlockMatchQCOM = 4486,
		TextureBlockMatch2QCOM = 4498,
		Float16ImageAMD = 5008,
		ImageGatherBiasLodAMD = 5009,
		FragmentMaskAMD = 5010,
		StencilExportEXT = 5013,
		ImageReadWriteLodAMD = 5015,
		Int64ImageEXT = 5016,
		ShaderClockKHR = 5055,
		ShaderEnqueueAMDX = 5067,
		QuadControlKHR = 5087,
		SampleMaskOverrideCoverageNV = 5249,
		GeometryShaderPassthroughNV = 5251,
		ShaderViewportIndexLayerEXT = 5254,
		ShaderViewportMaskNV = 5255,
		ShaderStereoViewNV = 5259,
		PerViewAttributesNV = 5260,
		FragmentFullyCoveredEXT = 5265,
		MeshShadingNV = 5266,
		ImageFootprintNV = 5282,
		MeshShadingEXT = 5283,
		FragmentBarycentricKHR = 5284,
		ComputeDerivativeGroupQuadsKHR = 5288,
		FragmentDensityEXT = 5291,
		GroupNonUniformPartitionedNV = 5297,
		ShaderNonUniform = 5301,
		RuntimeDescriptorArray = 5302,
		InputAttachmentArrayDynamicIndexing = 5303,
		UniformTexelBufferArrayDynamicIndexing = 5304,
		StorageTexelBufferArrayDynamicIndexing = 5305,
		UniformBufferArrayNonUniformIndexing = 5306,
		SampledImageArrayNonUniformIndexing = 5307,
		StorageBufferArrayNonUniformIndexing = 5308,
		StorageImageArrayNonUniformIndexing = 5309,
		InputAttachmentArrayNonUniformIndexing = 5310,
		UniformTexelBufferArrayNonUniformIndexing = 5311,
		StorageTexelBufferArrayNonUniformIndexing = 5312,
		RayTracingPositionFetchKHR = 5336,
		RayTracingNV = 5340,
		RayTracingMotionBlurNV = 5341,
		VulkanMemoryModel = 5345,
		VulkanMemoryModelDeviceScope = 5346,
		PhysicalStorageBufferAddresses = 5347,
		ComputeDerivativeGroupLinearKHR = 5350,
		RayTracingProvisionalKHR = 5353,
		CooperativeMatrixNV = 5357,
		FragmentShaderSampleInterlockEXT = 5363,
		FragmentShaderShadingRateInterlockEXT = 5372,
		ShaderSMBuiltinsNV = 5373,
		FragmentShaderPixelInterlockEXT = 5378,
		DemoteToHelperInvocation = 5379,
		DisplacementMicromapNV = 5380,
		RayTracingOpacityMicromapEXT = 5381,
		ShaderInvocationReorderNV = 5383,
		BindlessTextureNV = 5390,
		RayQueryPositionFetchKHR = 5391,
		AtomicFloat16VectorNV = 5404,
		RayTracingDisplacementMicromapNV = 5409,
		RawAccessChainsNV = 5414,
		CooperativeMatrixReductionsNV = 5430,
		CooperativeMatrixConversionsNV = 5431,
		CooperativeMatrixPerElementOperationsNV = 5432,
		CooperativeMatrixTensorAddressingNV = 5433,
		CooperativeMatrixBlockLoadsNV = 5434,
		TensorAddressingNV = 5439,
		SubgroupShuffleINTEL = 5568,
		SubgroupBufferBlockIOINTEL = 5569,
		SubgroupImageBlockIOINTEL = 5570,
		SubgroupImageMediaBlockIOINTEL = 5579,
		RoundToInfinityINTEL = 5582,
		FloatingPointModeINTEL = 5583,
		IntegerFunctions2INTEL = 5584,
		FunctionPointersINTEL = 5603,
		IndirectReferencesINTEL = 5604,
		AsmINTEL = 5606,
		AtomicFloat32MinMaxEXT = 5612,
		AtomicFloat64MinMaxEXT = 5613,
		AtomicFloat16MinMaxEXT = 5616,
		VectorComputeINTEL = 5617,
		VectorAnyINTEL = 5619,
		ExpectAssumeKHR = 5629,
		SubgroupAvcMotionEstimationINTEL = 5696,
		SubgroupAvcMotionEstimationIntraINTEL = 5697,
		SubgroupAvcMotionEstimationChromaINTEL = 5698,
		VariableLengthArrayINTEL = 5817,
		FunctionFloatControlINTEL = 5821,
		FPGAMemoryAttributesINTEL = 5824,
		FPFastMathModeINTEL = 5837,
		ArbitraryPrecisionIntegersINTEL = 5844,
		ArbitraryPrecisionFloatingPointINTEL = 5845,
		UnstructuredLoopControlsINTEL = 5886,
		FPGALoopControlsINTEL = 5888,
		KernelAttributesINTEL = 5892,
		FPGAKernelAttributesINTEL = 5897,
		FPGAMemoryAccessesINTEL = 5898,
		FPGAClusterAttributesINTEL = 5904,
		LoopFuseINTEL = 5906,
		FPGADSPControlINTEL = 5908,
		MemoryAccessAliasingINTEL = 5910,
		FPGAInvocationPipeliningAttributesINTEL = 5916,
		FPGABufferLocationINTEL = 5920,
		ArbitraryPrecisionFixedPointINTEL = 5922,
		USMStorageClassesINTEL = 5935,
		RuntimeAlignedAttributeINTEL = 5939,
		IOPipesINTEL = 5943,
		BlockingPipesINTEL = 5945,
		FPGARegINTEL = 5948,
		DotProductInputAll = 6016,
		DotProductInput4x8Bit = 6017,
		DotProductInput4x8BitPacked = 6018,
		DotProduct = 6019,
		RayCullMaskKHR = 6020,
		CooperativeMatrixKHR = 6022,
		ReplicatedCompositesEXT = 6024,
		BitInstructions = 6025,
		GroupNonUniformRotateKHR = 6026,
		FloatControls2 = 6029,
		AtomicFloat32AddEXT = 6033,
		AtomicFloat64AddEXT = 6034,
		LongCompositesINTEL = 6089,
		OptNoneEXT = 6094,
		AtomicFloat16AddEXT = 6095,
		DebugInfoModuleINTEL = 6114,
		BFloat16ConversionINTEL = 6115,
		SplitBarrierINTEL = 6141,
		ArithmeticFenceEXT = 6144,
		FPGAClusterAttributesV2INTEL = 6150,
		FPGAKernelAttributesv2INTEL = 6161,
		FPMaxErrorINTEL = 6169,
		FPGALatencyControlINTEL = 6171,
		FPGAArgumentInterfacesINTEL = 6174,
		GlobalVariableHostAccessINTEL = 6187,
		GlobalVariableFPGADecorationsINTEL = 6189,
		SubgroupBufferPrefetchINTEL = 6220,
		GroupUniformArithmeticKHR = 6400,
		MaskedGatherScatterINTEL = 6427,
		CacheControlsINTEL = 6441,
		RegisterLimitsINTEL = 6460,
	};

	enum class SpirvRayQueryIntersection
	{
		RayQueryCandidateIntersectionKHR = 0,
		RayQueryCommittedIntersectionKHR = 1,
	};

	enum class SpirvRayQueryCommittedIntersectionType
	{
		RayQueryCommittedIntersectionNoneKHR = 0,
		RayQueryCommittedIntersectionTriangleKHR = 1,
		RayQueryCommittedIntersectionGeneratedKHR = 2,
	};

	enum class SpirvRayQueryCandidateIntersectionType
	{
		RayQueryCandidateIntersectionTriangleKHR = 0,
		RayQueryCandidateIntersectionAABBKHR = 1,
	};

	enum class SpirvPackedVectorFormat
	{
		PackedVectorFormat4x8Bit = 0,
	};

	enum class SpirvCooperativeMatrixOperands
	{
		NoneKHR = 0x0000,
		MatrixASignedComponentsKHR = 0x0001,
		MatrixBSignedComponentsKHR = 0x0002,
		MatrixCSignedComponentsKHR = 0x0004,
		MatrixResultSignedComponentsKHR = 0x0008,
		SaturatingAccumulationKHR = 0x0010,
	};

	enum class SpirvCooperativeMatrixLayout
	{
		RowMajorKHR = 0,
		ColumnMajorKHR = 1,
		RowBlockedInterleavedARM = 4202,
		ColumnBlockedInterleavedARM = 4203,
	};

	enum class SpirvCooperativeMatrixUse
	{
		MatrixAKHR = 0,
		MatrixBKHR = 1,
		MatrixAccumulatorKHR = 2,
	};

	enum class SpirvCooperativeMatrixReduce
	{
		Row = 0x0001,
		Column = 0x0002,
		CooperativeMatrixReduce2x2 = 0x0004,
	};

	enum class SpirvTensorClampMode
	{
		Undefined = 0,
		Constant = 1,
		ClampToEdge = 2,
		Repeat = 3,
		RepeatMirrored = 4,
	};

	enum class SpirvTensorAddressingOperands
	{
		None = 0x0000,
		TensorView = 0x0001,
		DecodeFunc = 0x0002,
	};

	enum class SpirvInitializationModeQualifier
	{
		InitOnDeviceReprogramINTEL = 0,
		InitOnDeviceResetINTEL = 1,
	};

	enum class SpirvLoadCacheControl
	{
		UncachedINTEL = 0,
		CachedINTEL = 1,
		StreamingINTEL = 2,
		InvalidateAfterReadINTEL = 3,
		ConstCachedINTEL = 4,
	};

	enum class SpirvStoreCacheControl
	{
		UncachedINTEL = 0,
		WriteThroughINTEL = 1,
		WriteBackINTEL = 2,
		StreamingINTEL = 3,
	};

	enum class SpirvNamedMaximumNumberOfRegisters
	{
		AutoINTEL = 0,
	};

	enum class SpirvFPEncoding
	{
	};

	struct SpirvOperand
	{
		SpirvOperandKind kind;
		const char* name;
	};

	struct SpirvInstruction
	{
		SpirvOp op;
		const char* name;
		const SpirvOperand* operands;
		const SpirvOperand* resultOperand;
		std::size_t minOperandCount;
	};

	struct SpirvGlslStd450Instruction
	{
		SpirvGlslStd450Op op;
		const char* name;
		const SpirvOperand* operands;
		const SpirvOperand* resultOperand;
		std::size_t minOperandCount;
	};

	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvAccessQualifier kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvAddressingModel kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvBuiltIn kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvCapability kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvCooperativeMatrixLayout kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvCooperativeMatrixUse kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvDecoration kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvDim kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvExecutionMode kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvExecutionModel kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvFPDenormMode kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvFPEncoding kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvFPOperationMode kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvFPRoundingMode kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvFunctionParameterAttribute kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvGroupOperation kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvHostAccessQualifier kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvImageChannelDataType kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvImageChannelOrder kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvImageFormat kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvInitializationModeQualifier kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvKernelEnqueueFlags kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvLinkageType kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvLoadCacheControl kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvMemoryModel kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvNamedMaximumNumberOfRegisters kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvOverflowModes kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvPackedVectorFormat kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvQuantizationModes kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvRayQueryCandidateIntersectionType kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvRayQueryCommittedIntersectionType kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvRayQueryIntersection kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvSamplerAddressingMode kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvSamplerFilterMode kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvScope kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvSourceLanguage kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvStorageClass kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvStoreCacheControl kind);
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(SpirvTensorClampMode kind);

	NZSL_API const SpirvInstruction* GetSpirvInstruction(std::uint16_t op);
	NZSL_API const SpirvGlslStd450Instruction* GetSpirvGlslStd450Instruction(std::uint16_t op);

	NZSL_API std::string_view ToString(SpirvAccessQualifier value);
	NZSL_API std::string_view ToString(SpirvAddressingModel value);
	NZSL_API std::string_view ToString(SpirvBuiltIn value);
	NZSL_API std::string_view ToString(SpirvCapability value);
	NZSL_API std::string_view ToString(SpirvCooperativeMatrixLayout value);
	NZSL_API std::string_view ToString(SpirvCooperativeMatrixUse value);
	NZSL_API std::string_view ToString(SpirvDecoration value);
	NZSL_API std::string_view ToString(SpirvDim value);
	NZSL_API std::string_view ToString(SpirvExecutionMode value);
	NZSL_API std::string_view ToString(SpirvExecutionModel value);
	NZSL_API std::string_view ToString(SpirvFPDenormMode value);
	NZSL_API std::string_view ToString(SpirvFPEncoding value);
	NZSL_API std::string_view ToString(SpirvFPOperationMode value);
	NZSL_API std::string_view ToString(SpirvFPRoundingMode value);
	NZSL_API std::string_view ToString(SpirvFunctionParameterAttribute value);
	NZSL_API std::string_view ToString(SpirvGroupOperation value);
	NZSL_API std::string_view ToString(SpirvHostAccessQualifier value);
	NZSL_API std::string_view ToString(SpirvImageChannelDataType value);
	NZSL_API std::string_view ToString(SpirvImageChannelOrder value);
	NZSL_API std::string_view ToString(SpirvImageFormat value);
	NZSL_API std::string_view ToString(SpirvInitializationModeQualifier value);
	NZSL_API std::string_view ToString(SpirvKernelEnqueueFlags value);
	NZSL_API std::string_view ToString(SpirvLinkageType value);
	NZSL_API std::string_view ToString(SpirvLoadCacheControl value);
	NZSL_API std::string_view ToString(SpirvMemoryModel value);
	NZSL_API std::string_view ToString(SpirvNamedMaximumNumberOfRegisters value);
	NZSL_API std::string_view ToString(SpirvOverflowModes value);
	NZSL_API std::string_view ToString(SpirvPackedVectorFormat value);
	NZSL_API std::string_view ToString(SpirvQuantizationModes value);
	NZSL_API std::string_view ToString(SpirvRayQueryCandidateIntersectionType value);
	NZSL_API std::string_view ToString(SpirvRayQueryCommittedIntersectionType value);
	NZSL_API std::string_view ToString(SpirvRayQueryIntersection value);
	NZSL_API std::string_view ToString(SpirvSamplerAddressingMode value);
	NZSL_API std::string_view ToString(SpirvSamplerFilterMode value);
	NZSL_API std::string_view ToString(SpirvScope value);
	NZSL_API std::string_view ToString(SpirvSourceLanguage value);
	NZSL_API std::string_view ToString(SpirvStorageClass value);
	NZSL_API std::string_view ToString(SpirvStoreCacheControl value);
	NZSL_API std::string_view ToString(SpirvTensorClampMode value);
}

namespace Nz
{
	template<>
	struct EnumAsFlags<nzsl::SpirvImageOperands>
	{
		static constexpr nzsl::SpirvImageOperands max = nzsl::SpirvImageOperands::Offsets;

		static constexpr bool AutoFlag = false;
	};


	template<>
	struct EnumAsFlags<nzsl::SpirvFPFastMathMode>
	{
		static constexpr nzsl::SpirvFPFastMathMode max = nzsl::SpirvFPFastMathMode::AllowTransform;

		static constexpr bool AutoFlag = false;
	};


	template<>
	struct EnumAsFlags<nzsl::SpirvSelectionControl>
	{
		static constexpr nzsl::SpirvSelectionControl max = nzsl::SpirvSelectionControl::DontFlatten;

		static constexpr bool AutoFlag = false;
	};


	template<>
	struct EnumAsFlags<nzsl::SpirvLoopControl>
	{
		static constexpr nzsl::SpirvLoopControl max = nzsl::SpirvLoopControl::MaxReinvocationDelayINTEL;

		static constexpr bool AutoFlag = false;
	};


	template<>
	struct EnumAsFlags<nzsl::SpirvFunctionControl>
	{
		static constexpr nzsl::SpirvFunctionControl max = nzsl::SpirvFunctionControl::OptNoneEXT;

		static constexpr bool AutoFlag = false;
	};


	template<>
	struct EnumAsFlags<nzsl::SpirvMemorySemantics>
	{
		static constexpr nzsl::SpirvMemorySemantics max = nzsl::SpirvMemorySemantics::Volatile;

		static constexpr bool AutoFlag = false;
	};


	template<>
	struct EnumAsFlags<nzsl::SpirvMemoryAccess>
	{
		static constexpr nzsl::SpirvMemoryAccess max = nzsl::SpirvMemoryAccess::NoAliasINTELMask;

		static constexpr bool AutoFlag = false;
	};


	template<>
	struct EnumAsFlags<nzsl::SpirvKernelProfilingInfo>
	{
		static constexpr nzsl::SpirvKernelProfilingInfo max = nzsl::SpirvKernelProfilingInfo::CmdExecTime;

		static constexpr bool AutoFlag = false;
	};


	template<>
	struct EnumAsFlags<nzsl::SpirvRayFlags>
	{
		static constexpr nzsl::SpirvRayFlags max = nzsl::SpirvRayFlags::ForceOpacityMicromap2StateEXT;

		static constexpr bool AutoFlag = false;
	};


	template<>
	struct EnumAsFlags<nzsl::SpirvFragmentShadingRate>
	{
		static constexpr nzsl::SpirvFragmentShadingRate max = nzsl::SpirvFragmentShadingRate::Horizontal4Pixels;

		static constexpr bool AutoFlag = false;
	};


	template<>
	struct EnumAsFlags<nzsl::SpirvRawAccessChainOperands>
	{
		static constexpr nzsl::SpirvRawAccessChainOperands max = nzsl::SpirvRawAccessChainOperands::RobustnessPerElementNV;

		static constexpr bool AutoFlag = false;
	};


	template<>
	struct EnumAsFlags<nzsl::SpirvCooperativeMatrixOperands>
	{
		static constexpr nzsl::SpirvCooperativeMatrixOperands max = nzsl::SpirvCooperativeMatrixOperands::SaturatingAccumulationKHR;

		static constexpr bool AutoFlag = false;
	};


	template<>
	struct EnumAsFlags<nzsl::SpirvCooperativeMatrixReduce>
	{
		static constexpr nzsl::SpirvCooperativeMatrixReduce max = nzsl::SpirvCooperativeMatrixReduce::CooperativeMatrixReduce2x2;

		static constexpr bool AutoFlag = false;
	};


	template<>
	struct EnumAsFlags<nzsl::SpirvTensorAddressingOperands>
	{
		static constexpr nzsl::SpirvTensorAddressingOperands max = nzsl::SpirvTensorAddressingOperands::DecodeFunc;

		static constexpr bool AutoFlag = false;
	};


}

#endif // NZSL_SPIRV_SPIRVDATA_HPP
