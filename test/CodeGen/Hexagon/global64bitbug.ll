; RUN: llc -march=hexagon < %s
; REQUIRES: asserts

target triple = "hexagon-unknown-linux-gnu"

; Make sure we can emit globals whose size is not a multiple of 64bit.
; We used to assert here.
@switch.table = private unnamed_addr constant [6 x i928] [i928 744282853678701455922507579277316643178128753343813693743423064681488139394677769633078380312040969226121498541966016087590661425559764997, i928 744282853678701455922507579277316643178128753343813693743423064681488139394677769633078380312040969226121498541966016087590661425559764997, i928 744282853678701455922507579277316643178128753343813693743423064681488139394677769633078380312040969226121498541966016087590661425559764997, i928 744282853678701455922507579277316643178128753343813693743423064681488139394677769633078378850539331895218580338281183371307641769627222021, i928 744282853678701455922507579277316643178128753343813693743423064681488139394677769633078377389037694564315662134596350655024622113694679045, i928 744282853678701455922507579277316643178128753343813693743423064681488139394677769633078377389037694564315662134596350655024622113694679045]

