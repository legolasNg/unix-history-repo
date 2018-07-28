//===- MipsLegalizerInfo.cpp ------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements the targeting of the Machinelegalizer class for Mips.
/// \todo This should be generated by TableGen.
//===----------------------------------------------------------------------===//

#include "MipsLegalizerInfo.h"
#include "MipsTargetMachine.h"

using namespace llvm;

MipsLegalizerInfo::MipsLegalizerInfo(const MipsSubtarget &ST) {
  using namespace TargetOpcode;

  const LLT s32 = LLT::scalar(32);
  const LLT p0 = LLT::pointer(0, 32);

  getActionDefinitionsBuilder(G_ADD).legalFor({s32});

  getActionDefinitionsBuilder({G_LOAD, G_STORE})
      .legalForCartesianProduct({p0, s32}, {p0});

  getActionDefinitionsBuilder(G_CONSTANT)
      .legalFor({s32});

  getActionDefinitionsBuilder(G_GEP)
      .legalFor({{p0, s32}});

  getActionDefinitionsBuilder(G_FRAME_INDEX)
      .legalFor({p0});

  computeTables();
  verify(*ST.getInstrInfo());
}
