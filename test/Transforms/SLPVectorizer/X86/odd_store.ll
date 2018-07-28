; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -basicaa -slp-vectorizer -dce -S -mtriple=x86_64-apple-macosx10.8.0 -mcpu=corei7-avx | FileCheck %s

;int foo(char * restrict A, float * restrict B, float T) {
;  A[0] = (T * B[10] + 4.0);
;  A[1] = (T * B[11] + 5.0);
;  A[2] = (T * B[12] + 6.0);
;}

define i32 @foo(i8* noalias nocapture %A, float* noalias nocapture %B, float %T) {
; CHECK-LABEL: @foo(
; CHECK-NEXT:    [[TMP1:%.*]] = getelementptr inbounds float, float* [[B:%.*]], i64 10
; CHECK-NEXT:    [[TMP2:%.*]] = load float, float* [[TMP1]], align 4
; CHECK-NEXT:    [[TMP3:%.*]] = fmul float [[TMP2]], [[T:%.*]]
; CHECK-NEXT:    [[TMP4:%.*]] = fpext float [[TMP3]] to double
; CHECK-NEXT:    [[TMP5:%.*]] = fadd double [[TMP4]], 4.000000e+00
; CHECK-NEXT:    [[TMP6:%.*]] = fptosi double [[TMP5]] to i8
; CHECK-NEXT:    store i8 [[TMP6]], i8* [[A:%.*]], align 1
; CHECK-NEXT:    [[TMP7:%.*]] = getelementptr inbounds float, float* [[B]], i64 11
; CHECK-NEXT:    [[TMP8:%.*]] = load float, float* [[TMP7]], align 4
; CHECK-NEXT:    [[TMP9:%.*]] = fmul float [[TMP8]], [[T]]
; CHECK-NEXT:    [[TMP10:%.*]] = fpext float [[TMP9]] to double
; CHECK-NEXT:    [[TMP11:%.*]] = fadd double [[TMP10]], 5.000000e+00
; CHECK-NEXT:    [[TMP12:%.*]] = fptosi double [[TMP11]] to i8
; CHECK-NEXT:    [[TMP13:%.*]] = getelementptr inbounds i8, i8* [[A]], i64 1
; CHECK-NEXT:    store i8 [[TMP12]], i8* [[TMP13]], align 1
; CHECK-NEXT:    [[TMP14:%.*]] = getelementptr inbounds float, float* [[B]], i64 12
; CHECK-NEXT:    [[TMP15:%.*]] = load float, float* [[TMP14]], align 4
; CHECK-NEXT:    [[TMP16:%.*]] = fmul float [[TMP15]], [[T]]
; CHECK-NEXT:    [[TMP17:%.*]] = fpext float [[TMP16]] to double
; CHECK-NEXT:    [[TMP18:%.*]] = fadd double [[TMP17]], 6.000000e+00
; CHECK-NEXT:    [[TMP19:%.*]] = fptosi double [[TMP18]] to i8
; CHECK-NEXT:    [[TMP20:%.*]] = getelementptr inbounds i8, i8* [[A]], i64 2
; CHECK-NEXT:    store i8 [[TMP19]], i8* [[TMP20]], align 1
; CHECK-NEXT:    ret i32 undef
;
  %1 = getelementptr inbounds float, float* %B, i64 10
  %2 = load float, float* %1, align 4
  %3 = fmul float %2, %T
  %4 = fpext float %3 to double
  %5 = fadd double %4, 4.000000e+00
  %6 = fptosi double %5 to i8
  store i8 %6, i8* %A, align 1
  %7 = getelementptr inbounds float, float* %B, i64 11
  %8 = load float, float* %7, align 4
  %9 = fmul float %8, %T
  %10 = fpext float %9 to double
  %11 = fadd double %10, 5.000000e+00
  %12 = fptosi double %11 to i8
  %13 = getelementptr inbounds i8, i8* %A, i64 1
  store i8 %12, i8* %13, align 1
  %14 = getelementptr inbounds float, float* %B, i64 12
  %15 = load float, float* %14, align 4
  %16 = fmul float %15, %T
  %17 = fpext float %16 to double
  %18 = fadd double %17, 6.000000e+00
  %19 = fptosi double %18 to i8
  %20 = getelementptr inbounds i8, i8* %A, i64 2
  store i8 %19, i8* %20, align 1
  ret i32 undef
}

