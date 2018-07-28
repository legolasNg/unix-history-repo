; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i686-unknown -mcpu=pentium4 | FileCheck %s

%struct.Foo = type { i32, %struct.Bar }
%struct.Bar = type { i32, %struct.Buffer, i32 }
%struct.Buffer = type { i8*, i32 }

; This test checks that the load of store %2 is not dropped.
; 
define i32 @pr34088() local_unnamed_addr {
; CHECK-LABEL: pr34088:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    pushl %ebp
; CHECK-NEXT:    .cfi_def_cfa_offset 8
; CHECK-NEXT:    .cfi_offset %ebp, -8
; CHECK-NEXT:    movl %esp, %ebp
; CHECK-NEXT:    .cfi_def_cfa_register %ebp
; CHECK-NEXT:    andl $-16, %esp
; CHECK-NEXT:    subl $32, %esp
; CHECK-NEXT:    xorps %xmm0, %xmm0
; CHECK-NEXT:    movaps {{.*#+}} xmm1 = [205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205]
; CHECK-NEXT:    xorl %eax, %eax
; CHECK-NEXT:    movaps %xmm0, (%esp)
; CHECK-NEXT:    movsd {{.*#+}} xmm0 = mem[0],zero
; CHECK-NEXT:    movl $-842150451, {{[0-9]+}}(%esp) # imm = 0xCDCDCDCD
; CHECK-NEXT:    movaps %xmm1, (%esp)
; CHECK-NEXT:    movsd %xmm0, {{[0-9]+}}(%esp)
; CHECK-NEXT:    movl %ebp, %esp
; CHECK-NEXT:    popl %ebp
; CHECK-NEXT:    .cfi_def_cfa %esp, 4
; CHECK-NEXT:    retl
entry:
  %foo = alloca %struct.Foo, align 4
  %0 = bitcast %struct.Foo* %foo to i8*
  call void @llvm.memset.p0i8.i32(i8* align 4 nonnull %0, i8 0, i32 20, i1 false)
  %buffer1 = getelementptr inbounds %struct.Foo, %struct.Foo* %foo, i32 0, i32 1, i32 1
  %1 = bitcast %struct.Buffer* %buffer1 to i64*
  %2 = load i64, i64* %1, align 4
  call void @llvm.memset.p0i8.i32(i8* align 4 nonnull %0, i8 -51, i32 20, i1 false)
  store i64 %2, i64* %1, align 4
  ret i32 0
}

declare void @llvm.memset.p0i8.i32(i8* nocapture writeonly, i8, i32, i1)
