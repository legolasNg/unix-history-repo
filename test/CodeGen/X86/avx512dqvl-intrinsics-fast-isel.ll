; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i686-unknown-unknown -mattr=+avx512dq,+avx512vl | FileCheck %s --check-prefixes=CHECK,X86
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx512dq,+avx512vl | FileCheck %s --check-prefixes=CHECK,X64

; NOTE: This should use IR equivalent to what is generated by clang/test/CodeGen/avx512vldq-builtins.c

define <2 x double> @test_mm_cvtepi64_pd(<2 x i64> %__A) {
; CHECK-LABEL: test_mm_cvtepi64_pd:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vcvtqq2pd %xmm0, %xmm0
; CHECK-NEXT:    ret{{[l|q]}}
entry:
  %conv.i = sitofp <2 x i64> %__A to <2 x double>
  ret <2 x double> %conv.i
}

define <2 x double> @test_mm_mask_cvtepi64_pd(<2 x double> %__W, i8 zeroext %__U, <2 x i64> %__A) {
; X86-LABEL: test_mm_mask_cvtepi64_pd:
; X86:       # %bb.0: # %entry
; X86-NEXT:    kmovb {{[0-9]+}}(%esp), %k1
; X86-NEXT:    vcvtqq2pd %xmm1, %xmm0 {%k1}
; X86-NEXT:    retl
;
; X64-LABEL: test_mm_mask_cvtepi64_pd:
; X64:       # %bb.0: # %entry
; X64-NEXT:    kmovw %edi, %k1
; X64-NEXT:    vcvtqq2pd %xmm1, %xmm0 {%k1}
; X64-NEXT:    retq
entry:
  %conv.i.i = sitofp <2 x i64> %__A to <2 x double>
  %0 = bitcast i8 %__U to <8 x i1>
  %extract.i = shufflevector <8 x i1> %0, <8 x i1> undef, <2 x i32> <i32 0, i32 1>
  %1 = select <2 x i1> %extract.i, <2 x double> %conv.i.i, <2 x double> %__W
  ret <2 x double> %1
}

define <2 x double> @test_mm_maskz_cvtepi64_pd(i8 zeroext %__U, <2 x i64> %__A) {
; X86-LABEL: test_mm_maskz_cvtepi64_pd:
; X86:       # %bb.0: # %entry
; X86-NEXT:    kmovb {{[0-9]+}}(%esp), %k1
; X86-NEXT:    vcvtqq2pd %xmm0, %xmm0 {%k1} {z}
; X86-NEXT:    retl
;
; X64-LABEL: test_mm_maskz_cvtepi64_pd:
; X64:       # %bb.0: # %entry
; X64-NEXT:    kmovw %edi, %k1
; X64-NEXT:    vcvtqq2pd %xmm0, %xmm0 {%k1} {z}
; X64-NEXT:    retq
entry:
  %conv.i.i = sitofp <2 x i64> %__A to <2 x double>
  %0 = bitcast i8 %__U to <8 x i1>
  %extract.i = shufflevector <8 x i1> %0, <8 x i1> undef, <2 x i32> <i32 0, i32 1>
  %1 = select <2 x i1> %extract.i, <2 x double> %conv.i.i, <2 x double> zeroinitializer
  ret <2 x double> %1
}

define <4 x double> @test_mm256_cvtepi64_pd(<4 x i64> %__A) {
; CHECK-LABEL: test_mm256_cvtepi64_pd:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vcvtqq2pd %ymm0, %ymm0
; CHECK-NEXT:    ret{{[l|q]}}
entry:
  %conv.i = sitofp <4 x i64> %__A to <4 x double>
  ret <4 x double> %conv.i
}

define <4 x double> @test_mm256_mask_cvtepi64_pd(<4 x double> %__W, i8 zeroext %__U, <4 x i64> %__A) {
; X86-LABEL: test_mm256_mask_cvtepi64_pd:
; X86:       # %bb.0: # %entry
; X86-NEXT:    kmovb {{[0-9]+}}(%esp), %k1
; X86-NEXT:    vcvtqq2pd %ymm1, %ymm0 {%k1}
; X86-NEXT:    retl
;
; X64-LABEL: test_mm256_mask_cvtepi64_pd:
; X64:       # %bb.0: # %entry
; X64-NEXT:    kmovw %edi, %k1
; X64-NEXT:    vcvtqq2pd %ymm1, %ymm0 {%k1}
; X64-NEXT:    retq
entry:
  %conv.i.i = sitofp <4 x i64> %__A to <4 x double>
  %0 = bitcast i8 %__U to <8 x i1>
  %extract.i = shufflevector <8 x i1> %0, <8 x i1> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %1 = select <4 x i1> %extract.i, <4 x double> %conv.i.i, <4 x double> %__W
  ret <4 x double> %1
}

define <4 x double> @test_mm256_maskz_cvtepi64_pd(i8 zeroext %__U, <4 x i64> %__A) {
; X86-LABEL: test_mm256_maskz_cvtepi64_pd:
; X86:       # %bb.0: # %entry
; X86-NEXT:    kmovb {{[0-9]+}}(%esp), %k1
; X86-NEXT:    vcvtqq2pd %ymm0, %ymm0 {%k1} {z}
; X86-NEXT:    retl
;
; X64-LABEL: test_mm256_maskz_cvtepi64_pd:
; X64:       # %bb.0: # %entry
; X64-NEXT:    kmovw %edi, %k1
; X64-NEXT:    vcvtqq2pd %ymm0, %ymm0 {%k1} {z}
; X64-NEXT:    retq
entry:
  %conv.i.i = sitofp <4 x i64> %__A to <4 x double>
  %0 = bitcast i8 %__U to <8 x i1>
  %extract.i = shufflevector <8 x i1> %0, <8 x i1> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %1 = select <4 x i1> %extract.i, <4 x double> %conv.i.i, <4 x double> zeroinitializer
  ret <4 x double> %1
}

define <2 x double> @test_mm_cvtepu64_pd(<2 x i64> %__A) {
; CHECK-LABEL: test_mm_cvtepu64_pd:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vcvtuqq2pd %xmm0, %xmm0
; CHECK-NEXT:    ret{{[l|q]}}
entry:
  %conv.i = uitofp <2 x i64> %__A to <2 x double>
  ret <2 x double> %conv.i
}

define <2 x double> @test_mm_mask_cvtepu64_pd(<2 x double> %__W, i8 zeroext %__U, <2 x i64> %__A) {
; X86-LABEL: test_mm_mask_cvtepu64_pd:
; X86:       # %bb.0: # %entry
; X86-NEXT:    kmovb {{[0-9]+}}(%esp), %k1
; X86-NEXT:    vcvtuqq2pd %xmm1, %xmm0 {%k1}
; X86-NEXT:    retl
;
; X64-LABEL: test_mm_mask_cvtepu64_pd:
; X64:       # %bb.0: # %entry
; X64-NEXT:    kmovw %edi, %k1
; X64-NEXT:    vcvtuqq2pd %xmm1, %xmm0 {%k1}
; X64-NEXT:    retq
entry:
  %conv.i.i = uitofp <2 x i64> %__A to <2 x double>
  %0 = bitcast i8 %__U to <8 x i1>
  %extract.i = shufflevector <8 x i1> %0, <8 x i1> undef, <2 x i32> <i32 0, i32 1>
  %1 = select <2 x i1> %extract.i, <2 x double> %conv.i.i, <2 x double> %__W
  ret <2 x double> %1
}

define <2 x double> @test_mm_maskz_cvtepu64_pd(i8 zeroext %__U, <2 x i64> %__A) {
; X86-LABEL: test_mm_maskz_cvtepu64_pd:
; X86:       # %bb.0: # %entry
; X86-NEXT:    kmovb {{[0-9]+}}(%esp), %k1
; X86-NEXT:    vcvtuqq2pd %xmm0, %xmm0 {%k1} {z}
; X86-NEXT:    retl
;
; X64-LABEL: test_mm_maskz_cvtepu64_pd:
; X64:       # %bb.0: # %entry
; X64-NEXT:    kmovw %edi, %k1
; X64-NEXT:    vcvtuqq2pd %xmm0, %xmm0 {%k1} {z}
; X64-NEXT:    retq
entry:
  %conv.i.i = uitofp <2 x i64> %__A to <2 x double>
  %0 = bitcast i8 %__U to <8 x i1>
  %extract.i = shufflevector <8 x i1> %0, <8 x i1> undef, <2 x i32> <i32 0, i32 1>
  %1 = select <2 x i1> %extract.i, <2 x double> %conv.i.i, <2 x double> zeroinitializer
  ret <2 x double> %1
}

define <4 x double> @test_mm256_cvtepu64_pd(<4 x i64> %__A) {
; CHECK-LABEL: test_mm256_cvtepu64_pd:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vcvtuqq2pd %ymm0, %ymm0
; CHECK-NEXT:    ret{{[l|q]}}
entry:
  %conv.i = uitofp <4 x i64> %__A to <4 x double>
  ret <4 x double> %conv.i
}

define <4 x double> @test_mm256_mask_cvtepu64_pd(<4 x double> %__W, i8 zeroext %__U, <4 x i64> %__A) {
; X86-LABEL: test_mm256_mask_cvtepu64_pd:
; X86:       # %bb.0: # %entry
; X86-NEXT:    kmovb {{[0-9]+}}(%esp), %k1
; X86-NEXT:    vcvtuqq2pd %ymm1, %ymm0 {%k1}
; X86-NEXT:    retl
;
; X64-LABEL: test_mm256_mask_cvtepu64_pd:
; X64:       # %bb.0: # %entry
; X64-NEXT:    kmovw %edi, %k1
; X64-NEXT:    vcvtuqq2pd %ymm1, %ymm0 {%k1}
; X64-NEXT:    retq
entry:
  %conv.i.i = uitofp <4 x i64> %__A to <4 x double>
  %0 = bitcast i8 %__U to <8 x i1>
  %extract.i = shufflevector <8 x i1> %0, <8 x i1> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %1 = select <4 x i1> %extract.i, <4 x double> %conv.i.i, <4 x double> %__W
  ret <4 x double> %1
}

define <4 x double> @test_mm256_maskz_cvtepu64_pd(i8 zeroext %__U, <4 x i64> %__A) {
; X86-LABEL: test_mm256_maskz_cvtepu64_pd:
; X86:       # %bb.0: # %entry
; X86-NEXT:    kmovb {{[0-9]+}}(%esp), %k1
; X86-NEXT:    vcvtuqq2pd %ymm0, %ymm0 {%k1} {z}
; X86-NEXT:    retl
;
; X64-LABEL: test_mm256_maskz_cvtepu64_pd:
; X64:       # %bb.0: # %entry
; X64-NEXT:    kmovw %edi, %k1
; X64-NEXT:    vcvtuqq2pd %ymm0, %ymm0 {%k1} {z}
; X64-NEXT:    retq
entry:
  %conv.i.i = uitofp <4 x i64> %__A to <4 x double>
  %0 = bitcast i8 %__U to <8 x i1>
  %extract.i = shufflevector <8 x i1> %0, <8 x i1> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %1 = select <4 x i1> %extract.i, <4 x double> %conv.i.i, <4 x double> zeroinitializer
  ret <4 x double> %1
}

define zeroext i8 @test_mm_mask_fpclass_pd_mask(i8 zeroext %__U, <2 x double> %__A) {
; X86-LABEL: test_mm_mask_fpclass_pd_mask:
; X86:       # %bb.0: # %entry
; X86-NEXT:    kmovb {{[0-9]+}}(%esp), %k1
; X86-NEXT:    vfpclasspd $2, %xmm0, %k0 {%k1}
; X86-NEXT:    kmovw %k0, %eax
; X86-NEXT:    # kill: def $al killed $al killed $eax
; X86-NEXT:    retl
;
; X64-LABEL: test_mm_mask_fpclass_pd_mask:
; X64:       # %bb.0: # %entry
; X64-NEXT:    kmovw %edi, %k1
; X64-NEXT:    vfpclasspd $2, %xmm0, %k0 {%k1}
; X64-NEXT:    kmovw %k0, %eax
; X64-NEXT:    # kill: def $al killed $al killed $eax
; X64-NEXT:    retq
entry:
  %0 = tail call <2 x i1> @llvm.x86.avx512.fpclass.pd.128(<2 x double> %__A, i32 2)
  %1 = bitcast i8 %__U to <8 x i1>
  %extract = shufflevector <8 x i1> %1, <8 x i1> undef, <2 x i32> <i32 0, i32 1>
  %2 = and <2 x i1> %0, %extract
  %3 = shufflevector <2 x i1> %2, <2 x i1> zeroinitializer, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 2, i32 3, i32 2, i32 3>
  %4 = bitcast <8 x i1> %3 to i8
  ret i8 %4
}

declare <2 x i1> @llvm.x86.avx512.fpclass.pd.128(<2 x double>, i32)

define zeroext i8 @test_mm_fpclass_pd_mask(<2 x double> %__A) {
; CHECK-LABEL: test_mm_fpclass_pd_mask:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfpclasspd $2, %xmm0, %k0
; CHECK-NEXT:    kmovw %k0, %eax
; CHECK-NEXT:    # kill: def $al killed $al killed $eax
; CHECK-NEXT:    ret{{[l|q]}}
entry:
  %0 = tail call <2 x i1> @llvm.x86.avx512.fpclass.pd.128(<2 x double> %__A, i32 2)
  %1 = shufflevector <2 x i1> %0, <2 x i1> zeroinitializer, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 2, i32 3, i32 2, i32 3>
  %2 = bitcast <8 x i1> %1 to i8
  ret i8 %2
}

define zeroext i8 @test_mm256_mask_fpclass_pd_mask(i8 zeroext %__U, <4 x double> %__A) {
; X86-LABEL: test_mm256_mask_fpclass_pd_mask:
; X86:       # %bb.0: # %entry
; X86-NEXT:    kmovb {{[0-9]+}}(%esp), %k1
; X86-NEXT:    vfpclasspd $2, %ymm0, %k0 {%k1}
; X86-NEXT:    kmovw %k0, %eax
; X86-NEXT:    # kill: def $al killed $al killed $eax
; X86-NEXT:    vzeroupper
; X86-NEXT:    retl
;
; X64-LABEL: test_mm256_mask_fpclass_pd_mask:
; X64:       # %bb.0: # %entry
; X64-NEXT:    kmovw %edi, %k1
; X64-NEXT:    vfpclasspd $2, %ymm0, %k0 {%k1}
; X64-NEXT:    kmovw %k0, %eax
; X64-NEXT:    # kill: def $al killed $al killed $eax
; X64-NEXT:    vzeroupper
; X64-NEXT:    retq
entry:
  %0 = tail call <4 x i1> @llvm.x86.avx512.fpclass.pd.256(<4 x double> %__A, i32 2)
  %1 = bitcast i8 %__U to <8 x i1>
  %extract = shufflevector <8 x i1> %1, <8 x i1> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %2 = and <4 x i1> %0, %extract
  %3 = shufflevector <4 x i1> %2, <4 x i1> zeroinitializer, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %4 = bitcast <8 x i1> %3 to i8
  ret i8 %4
}

declare <4 x i1> @llvm.x86.avx512.fpclass.pd.256(<4 x double>, i32)

define zeroext i8 @test_mm256_fpclass_pd_mask(<4 x double> %__A) {
; CHECK-LABEL: test_mm256_fpclass_pd_mask:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfpclasspd $2, %ymm0, %k0
; CHECK-NEXT:    kmovw %k0, %eax
; CHECK-NEXT:    # kill: def $al killed $al killed $eax
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    ret{{[l|q]}}
entry:
  %0 = tail call <4 x i1> @llvm.x86.avx512.fpclass.pd.256(<4 x double> %__A, i32 2)
  %1 = shufflevector <4 x i1> %0, <4 x i1> zeroinitializer, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %2 = bitcast <8 x i1> %1 to i8
  ret i8 %2
}

define zeroext i8 @test_mm_mask_fpclass_ps_mask(i8 zeroext %__U, <4 x float> %__A) {
; X86-LABEL: test_mm_mask_fpclass_ps_mask:
; X86:       # %bb.0: # %entry
; X86-NEXT:    kmovb {{[0-9]+}}(%esp), %k1
; X86-NEXT:    vfpclassps $2, %xmm0, %k0 {%k1}
; X86-NEXT:    kmovw %k0, %eax
; X86-NEXT:    # kill: def $al killed $al killed $eax
; X86-NEXT:    retl
;
; X64-LABEL: test_mm_mask_fpclass_ps_mask:
; X64:       # %bb.0: # %entry
; X64-NEXT:    kmovw %edi, %k1
; X64-NEXT:    vfpclassps $2, %xmm0, %k0 {%k1}
; X64-NEXT:    kmovw %k0, %eax
; X64-NEXT:    # kill: def $al killed $al killed $eax
; X64-NEXT:    retq
entry:
  %0 = tail call <4 x i1> @llvm.x86.avx512.fpclass.ps.128(<4 x float> %__A, i32 2)
  %1 = bitcast i8 %__U to <8 x i1>
  %extract = shufflevector <8 x i1> %1, <8 x i1> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %2 = and <4 x i1> %0, %extract
  %3 = shufflevector <4 x i1> %2, <4 x i1> zeroinitializer, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %4 = bitcast <8 x i1> %3 to i8
  ret i8 %4
}

declare <4 x i1> @llvm.x86.avx512.fpclass.ps.128(<4 x float>, i32)

define zeroext i8 @test_mm_fpclass_ps_mask(<4 x float> %__A) {
; CHECK-LABEL: test_mm_fpclass_ps_mask:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfpclassps $2, %xmm0, %k0
; CHECK-NEXT:    kmovw %k0, %eax
; CHECK-NEXT:    # kill: def $al killed $al killed $eax
; CHECK-NEXT:    ret{{[l|q]}}
entry:
  %0 = tail call <4 x i1> @llvm.x86.avx512.fpclass.ps.128(<4 x float> %__A, i32 2)
  %1 = shufflevector <4 x i1> %0, <4 x i1> zeroinitializer, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %2 = bitcast <8 x i1> %1 to i8
  ret i8 %2
}

define zeroext i8 @test_mm256_mask_fpclass_ps_mask(i8 zeroext %__U, <8 x float> %__A) {
; X86-LABEL: test_mm256_mask_fpclass_ps_mask:
; X86:       # %bb.0: # %entry
; X86-NEXT:    vfpclassps $2, %ymm0, %k0
; X86-NEXT:    kmovw %k0, %eax
; X86-NEXT:    andb {{[0-9]+}}(%esp), %al
; X86-NEXT:    # kill: def $al killed $al killed $eax
; X86-NEXT:    vzeroupper
; X86-NEXT:    retl
;
; X64-LABEL: test_mm256_mask_fpclass_ps_mask:
; X64:       # %bb.0: # %entry
; X64-NEXT:    vfpclassps $2, %ymm0, %k0
; X64-NEXT:    kmovw %k0, %eax
; X64-NEXT:    andb %dil, %al
; X64-NEXT:    # kill: def $al killed $al killed $eax
; X64-NEXT:    vzeroupper
; X64-NEXT:    retq
entry:
  %0 = tail call <8 x i1> @llvm.x86.avx512.fpclass.ps.256(<8 x float> %__A, i32 2)
  %1 = bitcast i8 %__U to <8 x i1>
  %2 = and <8 x i1> %0, %1
  %3 = bitcast <8 x i1> %2 to i8
  ret i8 %3
}

declare <8 x i1> @llvm.x86.avx512.fpclass.ps.256(<8 x float>, i32)

define zeroext i8 @test_mm256_fpclass_ps_mask(<8 x float> %__A) {
; CHECK-LABEL: test_mm256_fpclass_ps_mask:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfpclassps $2, %ymm0, %k0
; CHECK-NEXT:    kmovw %k0, %eax
; CHECK-NEXT:    # kill: def $al killed $al killed $eax
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    ret{{[l|q]}}
entry:
  %0 = tail call <8 x i1> @llvm.x86.avx512.fpclass.ps.256(<8 x float> %__A, i32 2)
  %1 = bitcast <8 x i1> %0 to i8
  ret i8 %1
}
