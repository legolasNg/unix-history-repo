; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown | FileCheck %s --check-prefix=X64
; RUN: llc < %s -mtriple=i386-unknown | FileCheck %s --check-prefix=X86

define i128 @foo(i128 %t, i128 %u) {
; X64-LABEL: foo:
; X64:       # %bb.0:
; X64-NEXT:    movq %rdx, %r8
; X64-NEXT:    imulq %rdi, %rcx
; X64-NEXT:    movq %rdi, %rax
; X64-NEXT:    mulq %rdx
; X64-NEXT:    addq %rcx, %rdx
; X64-NEXT:    imulq %r8, %rsi
; X64-NEXT:    addq %rsi, %rdx
; X64-NEXT:    retq
;
; X86-LABEL: foo:
; X86:       # %bb.0:
; X86-NEXT:    pushl %ebp
; X86-NEXT:    .cfi_def_cfa_offset 8
; X86-NEXT:    pushl %ebx
; X86-NEXT:    .cfi_def_cfa_offset 12
; X86-NEXT:    pushl %edi
; X86-NEXT:    .cfi_def_cfa_offset 16
; X86-NEXT:    pushl %esi
; X86-NEXT:    .cfi_def_cfa_offset 20
; X86-NEXT:    subl $8, %esp
; X86-NEXT:    .cfi_def_cfa_offset 28
; X86-NEXT:    .cfi_offset %esi, -20
; X86-NEXT:    .cfi_offset %edi, -16
; X86-NEXT:    .cfi_offset %ebx, -12
; X86-NEXT:    .cfi_offset %ebp, -8
; X86-NEXT:    movl {{[0-9]+}}(%esp), %edx
; X86-NEXT:    movl {{[0-9]+}}(%esp), %edi
; X86-NEXT:    movl {{[0-9]+}}(%esp), %esi
; X86-NEXT:    imull %edx, %esi
; X86-NEXT:    movl %edi, %eax
; X86-NEXT:    mull %edx
; X86-NEXT:    movl %eax, %ebx
; X86-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X86-NEXT:    imull %edi, %ecx
; X86-NEXT:    addl %edx, %ecx
; X86-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-NEXT:    addl %esi, %ecx
; X86-NEXT:    movl %eax, %esi
; X86-NEXT:    imull {{[0-9]+}}(%esp), %esi
; X86-NEXT:    movl {{[0-9]+}}(%esp), %ebp
; X86-NEXT:    mull %ebp
; X86-NEXT:    addl %esi, %edx
; X86-NEXT:    movl {{[0-9]+}}(%esp), %edi
; X86-NEXT:    imull %ebp, %edi
; X86-NEXT:    addl %edx, %edi
; X86-NEXT:    addl %ebx, %eax
; X86-NEXT:    movl %eax, {{[0-9]+}}(%esp) # 4-byte Spill
; X86-NEXT:    adcl %ecx, %edi
; X86-NEXT:    movl %ebp, %eax
; X86-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X86-NEXT:    mull %ecx
; X86-NEXT:    movl %edx, %ebx
; X86-NEXT:    movl %eax, (%esp) # 4-byte Spill
; X86-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-NEXT:    mull %ecx
; X86-NEXT:    movl %edx, %esi
; X86-NEXT:    movl %eax, %ecx
; X86-NEXT:    addl %ebx, %ecx
; X86-NEXT:    adcl $0, %esi
; X86-NEXT:    movl %ebp, %eax
; X86-NEXT:    mull {{[0-9]+}}(%esp)
; X86-NEXT:    movl %edx, %ebx
; X86-NEXT:    movl %eax, %ebp
; X86-NEXT:    addl %ecx, %ebp
; X86-NEXT:    adcl %esi, %ebx
; X86-NEXT:    setb %cl
; X86-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-NEXT:    mull {{[0-9]+}}(%esp)
; X86-NEXT:    addl %ebx, %eax
; X86-NEXT:    movzbl %cl, %ecx
; X86-NEXT:    adcl %ecx, %edx
; X86-NEXT:    addl {{[0-9]+}}(%esp), %eax # 4-byte Folded Reload
; X86-NEXT:    adcl %edi, %edx
; X86-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X86-NEXT:    movl (%esp), %esi # 4-byte Reload
; X86-NEXT:    movl %esi, (%ecx)
; X86-NEXT:    movl %ebp, 4(%ecx)
; X86-NEXT:    movl %eax, 8(%ecx)
; X86-NEXT:    movl %edx, 12(%ecx)
; X86-NEXT:    movl %ecx, %eax
; X86-NEXT:    addl $8, %esp
; X86-NEXT:    .cfi_def_cfa_offset 20
; X86-NEXT:    popl %esi
; X86-NEXT:    .cfi_def_cfa_offset 16
; X86-NEXT:    popl %edi
; X86-NEXT:    .cfi_def_cfa_offset 12
; X86-NEXT:    popl %ebx
; X86-NEXT:    .cfi_def_cfa_offset 8
; X86-NEXT:    popl %ebp
; X86-NEXT:    .cfi_def_cfa_offset 4
; X86-NEXT:    retl $4
  %k = mul i128 %t, %u
  ret i128 %k
}
