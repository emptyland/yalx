// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#ifndef YAX_BACKEND_X64_INSTRUCTION_CODES_X64_H_
#define YAX_BACKEND_X64_INSTRUCTION_CODES_X64_H_

namespace yalx::backend {

// X64-specific opcodes that specify which assembly sequence to emit.
// Most opcodes specify a single instruction.
#define X64_ARCH_OPCODE_LIST(V)             \
    V(X64Add)                               \
    V(X64Add8)                              \
    V(X64Add16)                             \
    V(X64Add32)                             \
    V(X64And)                               \
    V(X64And32)                             \
    V(X64Cmp)                               \
    V(X64Cmp32)                             \
    V(X64Cmp16)                             \
    V(X64Cmp8)                              \
    V(X64Test)                              \
    V(X64Test32)                            \
    V(X64Test16)                            \
    V(X64Test8)                             \
    V(X64Or)                                \
    V(X64Or32)                              \
    V(X64Xor)                               \
    V(X64Xor32)                             \
    V(X64Sub)                               \
    V(X64Sub8)                              \
    V(X64Sub16)                             \
    V(X64Sub32)                             \
    V(X64Imul)                              \
    V(X64Imul32)                            \
    V(X64ImulHigh32)                        \
    V(X64UmulHigh32)                        \
    V(X64Idiv)                              \
    V(X64Idiv32)                            \
    V(X64Ja)                                \
    V(X64Jae)                               \
    V(X64Jb)                                \
    V(X64Jbe)                               \
    V(X64Jc)                                \
    V(X64Je)                                \
    V(X64Jz)                                \
    V(X64Jg)                                \
    V(X64Jge)                               \
    V(X64Jl)                                \
    V(X64Jle)                               \
    V(X64Jna)                               \
    V(X64Jnae)                              \
    V(X64Jnb)                               \
    V(X64Jnbe)                              \
    V(X64Jnc)                               \
    V(X64Jne)                               \
    V(X64Jng)                               \
    V(X64Jnge)                              \
    V(X64Jnl)                               \
    V(X64Jnle)                              \
    V(X64Jno)                               \
    V(X64Jnp)                               \
    V(X64Jns)                               \
    V(X64Jnz)                               \
    V(X64Jo)                                \
    V(X64Jp)                                \
    V(X64Jpe)                               \
    V(X64Jpo)                               \
    V(X64Js)                                \
    V(X64Seta)                              \
    V(X64Setae)                             \
    V(X64Setb)                              \
    V(X64Setbe)                             \
    V(X64Setc)                              \
    V(X64Sete)                              \
    V(X64Setz)                              \
    V(X64Setg)                              \
    V(X64Setge)                             \
    V(X64Setl)                              \
    V(X64Setle)                             \
    V(X64Setna)                             \
    V(X64Setnae)                            \
    V(X64Setnb)                             \
    V(X64Setnbe)                            \
    V(X64Setnc)                             \
    V(X64Setne)                             \
    V(X64Setng)                             \
    V(X64Setnge)                            \
    V(X64Setnl)                             \
    V(X64Setnle)                            \
    V(X64Setno)                             \
    V(X64Setnp)                             \
    V(X64Setns)                             \
    V(X64Setnz)                             \
    V(X64Seto)                              \
    V(X64Setp)                              \
    V(X64Setpe)                             \
    V(X64Setpo)                             \
    V(X64Sets)                              \
    V(X64Udiv)                              \
    V(X64Udiv32)                            \
    V(X64Not)                               \
    V(X64Not32)                             \
    V(X64Neg)                               \
    V(X64Neg32)                             \
    V(X64Shl)                               \
    V(X64Shl32)                             \
    V(X64Shr)                               \
    V(X64Shr32)                             \
    V(X64Sar)                               \
    V(X64Sar32)                             \
    V(X64Rol)                               \
    V(X64Rol32)                             \
    V(X64Ror)                               \
    V(X64Ror32)                             \
    V(X64Lzcnt)                             \
    V(X64Lzcnt32)                           \
    V(X64Tzcnt)                             \
    V(X64Tzcnt32)                           \
    V(X64Pop)                               \
    V(X64Popcnt)                            \
    V(X64Popcnt32)                          \
    V(X64Bswap)                             \
    V(X64Bswap32)                           \
    V(X64MFence)                            \
    V(X64LFence)                            \
    V(SSEFloat32Cmp)                        \
    V(SSEFloat32Add)                        \
    V(SSEFloat32Sub)                        \
    V(SSEFloat32Mul)                        \
    V(SSEFloat32Div)                        \
    V(SSEFloat32Abs)                        \
    V(SSEFloat32Neg)                        \
    V(SSEFloat32Sqrt)                       \
    V(SSEFloat32ToFloat64)                  \
    V(SSEFloat32ToInt32)                    \
    V(SSEFloat32ToUint32)                   \
    V(SSEFloat32Round)                      \
    V(SSEFloat64Cmp)                        \
    V(SSEFloat64Add)                        \
    V(SSEFloat64Sub)                        \
    V(SSEFloat64Mul)                        \
    V(SSEFloat64Div)                        \
    V(SSEFloat64Mod)                        \
    V(SSEFloat64Abs)                        \
    V(SSEFloat64Neg)                        \
    V(SSEFloat64Sqrt)                       \
    V(SSEFloat64Round)                      \
    V(SSEFloat32Max)                        \
    V(SSEFloat64Max)                        \
    V(SSEFloat32Min)                        \
    V(SSEFloat64Min)                        \
    V(SSEFloat64ToFloat32)                  \
    V(SSEFloat64ToInt32)                    \
    V(SSEFloat64ToUint32)                   \
    V(SSEFloat32ToInt64)                    \
    V(SSEFloat64ToInt64)                    \
    V(SSEFloat32ToUint64)                   \
    V(SSEFloat64ToUint64)                   \
    V(SSEInt32ToFloat64)                    \
    V(SSEInt32ToFloat32)                    \
    V(SSEInt64ToFloat32)                    \
    V(SSEInt64ToFloat64)                    \
    V(SSEUint64ToFloat32)                   \
    V(SSEUint64ToFloat64)                   \
    V(SSEUint32ToFloat64)                   \
    V(SSEUint32ToFloat32)                   \
    V(SSEFloat64ExtractLowWord32)           \
    V(SSEFloat64ExtractHighWord32)          \
    V(SSEFloat64InsertLowWord32)            \
    V(SSEFloat64InsertHighWord32)           \
    V(SSEFloat64LoadLowWord32)              \
    V(SSEFloat64SilenceNaN)                 \
    V(AVXFloat32Cmp)                        \
    V(AVXFloat32Add)                        \
    V(AVXFloat32Sub)                        \
    V(AVXFloat32Mul)                        \
    V(AVXFloat32Div)                        \
    V(AVXFloat64Cmp)                        \
    V(AVXFloat64Add)                        \
    V(AVXFloat64Sub)                        \
    V(AVXFloat64Mul)                        \
    V(AVXFloat64Div)                        \
    V(AVXFloat64Abs)                        \
    V(AVXFloat64Neg)                        \
    V(AVXFloat32Abs)                        \
    V(AVXFloat32Neg)                        \
    V(X64Movsxbw)                           \
    V(X64Movzxbw)                           \
    V(X64Movsxbl)                           \
    V(X64Movzxbl)                           \
    V(X64Movsxbq)                           \
    V(X64Movzxbq)                           \
    V(X64Movb)                              \
    V(X64Movsxwb)                           \
    V(X64Movzxwb)                           \
    V(X64Movsxwl)                           \
    V(X64Movzxwl)                           \
    V(X64Movsxwq)                           \
    V(X64Movzxwq)                           \
    V(X64Movw)                              \
    V(X64Movl)                              \
    V(X64Movsxlq)                           \
    V(X64MovqDecompressTaggedSigned)        \
    V(X64MovqDecompressTaggedPointer)       \
    V(X64MovqDecompressAnyTagged)           \
    V(X64MovqCompressTagged)                \
    V(X64Movq)                              \
    V(X64Movsd)                             \
    V(X64Movss)                             \
    V(X64Movdqu)                            \
    V(X64BitcastFI)                         \
    V(X64BitcastDL)                         \
    V(X64BitcastIF)                         \
    V(X64BitcastLD)                         \
    V(X64Lea32)                             \
    V(X64Lea)                               \
    V(X64Dec32)                             \
    V(X64Inc32)                             \
    V(X64Push)                              \
    V(X64Poke)                              \
    V(X64Peek)                              \
    V(X64F64x2Splat)                        \
    V(X64F64x2ExtractLane)                  \
    V(X64F64x2Abs)                          \
    V(X64F64x2Neg)                          \
    V(X64F64x2Sqrt)                         \
    V(X64F64x2Add)                          \
    V(X64F64x2Sub)                          \
    V(X64F64x2Mul)                          \
    V(X64F64x2Div)                          \
    V(X64F64x2Min)                          \
    V(X64F64x2Max)                          \
    V(X64F64x2Eq)                           \
    V(X64F64x2Ne)                           \
    V(X64F64x2Lt)                           \
    V(X64F64x2Le)                           \
    V(X64F64x2Qfma)                         \
    V(X64F64x2Qfms)                         \
    V(X64F64x2Pmin)                         \
    V(X64F64x2Pmax)                         \
    V(X64F64x2Round)                        \
    V(X64F64x2ConvertLowI32x4S)             \
    V(X64F64x2ConvertLowI32x4U)             \
    V(X64F64x2PromoteLowF32x4)              \
    V(X64F32x4Splat)                        \
    V(X64F32x4ExtractLane)                  \
    V(X64F32x4ReplaceLane)                  \
    V(X64F32x4SConvertI32x4)                \
    V(X64F32x4UConvertI32x4)                \
    V(X64F32x4Abs)                          \
    V(X64F32x4Neg)                          \
    V(X64F32x4Sqrt)                         \
    V(X64F32x4RecipApprox)                  \
    V(X64F32x4RecipSqrtApprox)              \
    V(X64F32x4Add)                          \
    V(X64F32x4Sub)                          \
    V(X64F32x4Mul)                          \
    V(X64F32x4Div)                          \
    V(X64F32x4Min)                          \
    V(X64F32x4Max)                          \
    V(X64F32x4Eq)                           \
    V(X64F32x4Ne)                           \
    V(X64F32x4Lt)                           \
    V(X64F32x4Le)                           \
    V(X64F32x4Qfma)                         \
    V(X64F32x4Qfms)                         \
    V(X64F32x4Pmin)                         \
    V(X64F32x4Pmax)                         \
    V(X64F32x4Round)                        \
    V(X64F32x4DemoteF64x2Zero)              \
    V(X64I64x2Splat)                        \
    V(X64I64x2ExtractLane)                  \
    V(X64I64x2Abs)                          \
    V(X64I64x2Neg)                          \
    V(X64I64x2BitMask)                      \
    V(X64I64x2Shl)                          \
    V(X64I64x2ShrS)                         \
    V(X64I64x2Add)                          \
    V(X64I64x2Sub)                          \
    V(X64I64x2Mul)                          \
    V(X64I64x2Eq)                           \
    V(X64I64x2GtS)                          \
    V(X64I64x2GeS)                          \
    V(X64I64x2Ne)                           \
    V(X64I64x2ShrU)                         \
    V(X64I64x2ExtMulLowI32x4S)              \
    V(X64I64x2ExtMulHighI32x4S)             \
    V(X64I64x2ExtMulLowI32x4U)              \
    V(X64I64x2ExtMulHighI32x4U)             \
    V(X64I64x2SConvertI32x4Low)             \
    V(X64I64x2SConvertI32x4High)            \
    V(X64I64x2UConvertI32x4Low)             \
    V(X64I64x2UConvertI32x4High)            \
    V(X64I32x4Splat)                        \
    V(X64I32x4ExtractLane)                  \
    V(X64I32x4SConvertF32x4)                \
    V(X64I32x4SConvertI16x8Low)             \
    V(X64I32x4SConvertI16x8High)            \
    V(X64I32x4Neg)                          \
    V(X64I32x4Shl)                          \
    V(X64I32x4ShrS)                         \
    V(X64I32x4Add)                          \
    V(X64I32x4Sub)                          \
    V(X64I32x4Mul)                          \
    V(X64I32x4MinS)                         \
    V(X64I32x4MaxS)                         \
    V(X64I32x4Eq)                           \
    V(X64I32x4Ne)                           \
    V(X64I32x4GtS)                          \
    V(X64I32x4GeS)                          \
    V(X64I32x4UConvertF32x4)                \
    V(X64I32x4UConvertI16x8Low)             \
    V(X64I32x4UConvertI16x8High)            \
    V(X64I32x4ShrU)                         \
    V(X64I32x4MinU)                         \
    V(X64I32x4MaxU)                         \
    V(X64I32x4GtU)                          \
    V(X64I32x4GeU)                          \
    V(X64I32x4Abs)                          \
    V(X64I32x4BitMask)                      \
    V(X64I32x4DotI16x8S)                    \
    V(X64I32x4ExtMulLowI16x8S)              \
    V(X64I32x4ExtMulHighI16x8S)             \
    V(X64I32x4ExtMulLowI16x8U)              \
    V(X64I32x4ExtMulHighI16x8U)             \
    V(X64I32x4ExtAddPairwiseI16x8S)         \
    V(X64I32x4ExtAddPairwiseI16x8U)         \
    V(X64I32x4TruncSatF64x2SZero)           \
    V(X64I32x4TruncSatF64x2UZero)           \
    V(X64I16x8Splat)                        \
    V(X64I16x8ExtractLaneS)                 \
    V(X64I16x8SConvertI8x16Low)             \
    V(X64I16x8SConvertI8x16High)            \
    V(X64I16x8Neg)                          \
    V(X64I16x8Shl)                          \
    V(X64I16x8ShrS)                         \
    V(X64I16x8SConvertI32x4)                \
    V(X64I16x8Add)                          \
    V(X64I16x8AddSatS)                      \
    V(X64I16x8Sub)                          \
    V(X64I16x8SubSatS)                      \
    V(X64I16x8Mul)                          \
    V(X64I16x8MinS)                         \
    V(X64I16x8MaxS)                         \
    V(X64I16x8Eq)                           \
    V(X64I16x8Ne)                           \
    V(X64I16x8GtS)                          \
    V(X64I16x8GeS)                          \
    V(X64I16x8UConvertI8x16Low)             \
    V(X64I16x8UConvertI8x16High)            \
    V(X64I16x8ShrU)                         \
    V(X64I16x8UConvertI32x4)                \
    V(X64I16x8AddSatU)                      \
    V(X64I16x8SubSatU)                      \
    V(X64I16x8MinU)                         \
    V(X64I16x8MaxU)                         \
    V(X64I16x8GtU)                          \
    V(X64I16x8GeU)                          \
    V(X64I16x8RoundingAverageU)             \
    V(X64I16x8Abs)                          \
    V(X64I16x8BitMask)                      \
    V(X64I16x8ExtMulLowI8x16S)              \
    V(X64I16x8ExtMulHighI8x16S)             \
    V(X64I16x8ExtMulLowI8x16U)              \
    V(X64I16x8ExtMulHighI8x16U)             \
    V(X64I16x8ExtAddPairwiseI8x16S)         \
    V(X64I16x8ExtAddPairwiseI8x16U)         \
    V(X64I16x8Q15MulRSatS)                  \
    V(X64I8x16Splat)                        \
    V(X64I8x16ExtractLaneS)                 \
    V(X64Pinsrb)                            \
    V(X64Pinsrw)                            \
    V(X64Pinsrd)                            \
    V(X64Pinsrq)                            \
    V(X64Pextrb)                            \
    V(X64Pextrw)                            \
    V(X64I8x16SConvertI16x8)                \
    V(X64I8x16Neg)                          \
    V(X64I8x16Shl)                          \
    V(X64I8x16ShrS)                         \
    V(X64I8x16Add)                          \
    V(X64I8x16AddSatS)                      \
    V(X64I8x16Sub)                          \
    V(X64I8x16SubSatS)                      \
    V(X64I8x16MinS)                         \
    V(X64I8x16MaxS)                         \
    V(X64I8x16Eq)                           \
    V(X64I8x16Ne)                           \
    V(X64I8x16GtS)                          \
    V(X64I8x16GeS)                          \
    V(X64I8x16UConvertI16x8)                \
    V(X64I8x16AddSatU)                      \
    V(X64I8x16SubSatU)                      \
    V(X64I8x16ShrU)                         \
    V(X64I8x16MinU)                         \
    V(X64I8x16MaxU)                         \
    V(X64I8x16GtU)                          \
    V(X64I8x16GeU)                          \
    V(X64I8x16RoundingAverageU)             \
    V(X64I8x16Abs)                          \
    V(X64I8x16BitMask)                      \
    V(X64S128Const)                         \
    V(X64S128Zero)                          \
    V(X64S128AllOnes)                       \
    V(X64S128Not)                           \
    V(X64S128And)                           \
    V(X64S128Or)                            \
    V(X64S128Xor)                           \
    V(X64S128Select)                        \
    V(X64S128AndNot)                        \
    V(X64I8x16Swizzle)                      \
    V(X64I8x16Shuffle)                      \
    V(X64I8x16Popcnt)                       \
    V(X64S128Load8Splat)                    \
    V(X64S128Load16Splat)                   \
    V(X64S128Load32Splat)                   \
    V(X64S128Load64Splat)                   \
    V(X64S128Load8x8S)                      \
    V(X64S128Load8x8U)                      \
    V(X64S128Load16x4S)                     \
    V(X64S128Load16x4U)                     \
    V(X64S128Load32x2S)                     \
    V(X64S128Load32x2U)                     \
    V(X64S128Store32Lane)                   \
    V(X64S128Store64Lane)                   \
    V(X64Shufps)                            \
    V(X64S32x4Rotate)                       \
    V(X64S32x4Swizzle)                      \
    V(X64S32x4Shuffle)                      \
    V(X64S16x8Blend)                        \
    V(X64S16x8HalfShuffle1)                 \
    V(X64S16x8HalfShuffle2)                 \
    V(X64S8x16Alignr)                       \
    V(X64S16x8Dup)                          \
    V(X64S8x16Dup)                          \
    V(X64S16x8UnzipHigh)                    \
    V(X64S16x8UnzipLow)                     \
    V(X64S8x16UnzipHigh)                    \
    V(X64S8x16UnzipLow)                     \
    V(X64S64x2UnpackHigh)                   \
    V(X64S32x4UnpackHigh)                   \
    V(X64S16x8UnpackHigh)                   \
    V(X64S8x16UnpackHigh)                   \
    V(X64S64x2UnpackLow)                    \
    V(X64S32x4UnpackLow)                    \
    V(X64S16x8UnpackLow)                    \
    V(X64S8x16UnpackLow)                    \
    V(X64S8x16TransposeLow)                 \
    V(X64S8x16TransposeHigh)                \
    V(X64S8x8Reverse)                       \
    V(X64S8x4Reverse)                       \
    V(X64S8x2Reverse)                       \
    V(X64V128AnyTrue)                       \
    V(X64I64x2AllTrue)                      \
    V(X64I32x4AllTrue)                      \
    V(X64I16x8AllTrue)                      \
    V(X64I8x16AllTrue)                      \
    V(X64Word64AtomicAddUint8)              \
    V(X64Word64AtomicAddUint16)             \
    V(X64Word64AtomicAddUint32)             \
    V(X64Word64AtomicAddUint64)             \
    V(X64Word64AtomicSubUint8)              \
    V(X64Word64AtomicSubUint16)             \
    V(X64Word64AtomicSubUint32)             \
    V(X64Word64AtomicSubUint64)             \
    V(X64Word64AtomicAndUint8)              \
    V(X64Word64AtomicAndUint16)             \
    V(X64Word64AtomicAndUint32)             \
    V(X64Word64AtomicAndUint64)             \
    V(X64Word64AtomicOrUint8)               \
    V(X64Word64AtomicOrUint16)              \
    V(X64Word64AtomicOrUint32)              \
    V(X64Word64AtomicOrUint64)              \
    V(X64Word64AtomicXorUint8)              \
    V(X64Word64AtomicXorUint16)             \
    V(X64Word64AtomicXorUint32)             \
    V(X64Word64AtomicXorUint64)             \
    V(X64Word64AtomicExchangeUint8)         \
    V(X64Word64AtomicExchangeUint16)        \
    V(X64Word64AtomicExchangeUint32)        \
    V(X64Word64AtomicExchangeUint64)        \
    V(X64Word64AtomicCompareExchangeUint8)  \
    V(X64Word64AtomicCompareExchangeUint16) \
    V(X64Word64AtomicCompareExchangeUint32) \
    V(X64Word64AtomicCompareExchangeUint64)

// Addressing modes represent the "shape" of inputs to an instruction.
// Many instructions support multiple addressing modes. Addressing modes
// are encoded into the InstructionCode of the instruction and tell the
// code generator after register allocation which assembler method to call.
//
// We use the following local notation for addressing modes:
//
// M = memory operand
// R = base register
// N = index register * N for N in {1, 2, 4, 8}
// I = immediate displacement (32-bit signed integer)

#define X64_ADDRESSING_MODE_LIST(V)      \
    V(MR)   /* [%r1            ] */      \
    V(MRI)  /* [%r1         + K] */      \
    V(MR1)  /* [%r1 + %r2*1    ] */      \
    V(MR2)  /* [%r1 + %r2*2    ] */      \
    V(MR4)  /* [%r1 + %r2*4    ] */      \
    V(MR8)  /* [%r1 + %r2*8    ] */      \
    V(MR1I) /* [%r1 + %r2*1 + K] */      \
    V(MR2I) /* [%r1 + %r2*2 + K] */      \
    V(MR4I) /* [%r1 + %r2*3 + K] */      \
    V(MR8I) /* [%r1 + %r2*4 + K] */      \
    V(M1)   /* [      %r2*1    ] */      \
    V(M2)   /* [      %r2*2    ] */      \
    V(M4)   /* [      %r2*4    ] */      \
    V(M8)   /* [      %r2*8    ] */      \
    V(M1I)  /* [      %r2*1 + K] */      \
    V(M2I)  /* [      %r2*2 + K] */      \
    V(M4I)  /* [      %r2*4 + K] */      \
    V(M8I)  /* [      %r2*8 + K] */      \
    V(Root) /* [%root       + K] */

}  // namespace yalx

#endif  // YAX_BACKEND_X64_INSTRUCTION_CODES_X64_H_

