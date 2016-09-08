; ModuleID = 'main.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@int_global = global i32 10, align 4
@double_global = global double 2.000000e+01, align 8

; Function Attrs: nounwind uwtable
define void @f(i32 %x) #0 {
  %1 = alloca i32, align 4
  store i32 %x, i32* %1, align 4
  %2 = load i32, i32* %1, align 4
  store i32 %2, i32* @int_global, align 4
  ret void
}

; Function Attrs: nounwind uwtable
define void @g(double %x) #0 {
  %1 = alloca double, align 8
  store double %x, double* %1, align 8
  %2 = load double, double* %1, align 8
  store double %2, double* @double_global, align 8
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.1 (tags/RELEASE_381/final)"}
