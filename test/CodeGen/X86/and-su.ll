; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i686-- | FileCheck %s

; Don't duplicate the load.

define fastcc i32 @foo(i32* %p) nounwind {
; CHECK-LABEL: foo:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movl (%ecx), %eax
; CHECK-NEXT:    andl $10, %eax
; CHECK-NEXT:    je .LBB0_2
; CHECK-NEXT:  # %bb.1: # %bb63
; CHECK-NEXT:    retl
; CHECK-NEXT:  .LBB0_2: # %bb76
; CHECK-NEXT:    xorl %eax, %eax
; CHECK-NEXT:    retl
	%t0 = load i32, i32* %p
	%t2 = and i32 %t0, 10
	%t3 = icmp ne i32 %t2, 0
	br i1 %t3, label %bb63, label %bb76
bb63:
	ret i32 %t2
bb76:
	ret i32 0
}

define fastcc double @bar(i32 %hash, double %x, double %y) nounwind {
; CHECK-LABEL: bar:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    pushl %ebp
; CHECK-NEXT:    movl %esp, %ebp
; CHECK-NEXT:    andl $-8, %esp
; CHECK-NEXT:    fldl 16(%ebp)
; CHECK-NEXT:    fldl 8(%ebp)
; CHECK-NEXT:    movl %ecx, %eax
; CHECK-NEXT:    andl $15, %eax
; CHECK-NEXT:    cmpl $8, %eax
; CHECK-NEXT:    jb .LBB1_2
; CHECK-NEXT:  # %bb.1: # %bb10
; CHECK-NEXT:    testb $1, %cl
; CHECK-NEXT:    je .LBB1_3
; CHECK-NEXT:  .LBB1_2: # %bb11
; CHECK-NEXT:    fchs
; CHECK-NEXT:  .LBB1_3: # %bb13
; CHECK-NEXT:    testb $2, %cl
; CHECK-NEXT:    je .LBB1_5
; CHECK-NEXT:  # %bb.4: # %bb14
; CHECK-NEXT:    fxch %st(1)
; CHECK-NEXT:    fchs
; CHECK-NEXT:    fxch %st(1)
; CHECK-NEXT:  .LBB1_5: # %bb16
; CHECK-NEXT:    faddp %st(1)
; CHECK-NEXT:    movl %ebp, %esp
; CHECK-NEXT:    popl %ebp
; CHECK-NEXT:    retl
entry:
  %0 = and i32 %hash, 15
  %1 = icmp ult i32 %0, 8
  br i1 %1, label %bb11, label %bb10

bb10:
  %2 = and i32 %hash, 1
  %3 = icmp eq i32 %2, 0
  br i1 %3, label %bb13, label %bb11

bb11:
  %4 = fsub double -0.000000e+00, %x
  br label %bb13

bb13:
  %iftmp.9.0 = phi double [ %4, %bb11 ], [ %x, %bb10 ]
  %5 = and i32 %hash, 2
  %6 = icmp eq i32 %5, 0
  br i1 %6, label %bb16, label %bb14

bb14:
  %7 = fsub double -0.000000e+00, %y
  br label %bb16

bb16:
  %iftmp.10.0 = phi double [ %7, %bb14 ], [ %y, %bb13 ]
  %8 = fadd double %iftmp.9.0, %iftmp.10.0
  ret double %8
}
