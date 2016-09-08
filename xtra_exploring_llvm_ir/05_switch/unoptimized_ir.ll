; ModuleID = 'main.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @example(i32 %x) #0 {
  %1 = alloca i32, align 4
  %y = alloca i32, align 4
  store i32 %x, i32* %1, align 4
  %2 = load i32, i32* %1, align 4
  switch i32 %2, label %6 [
    i32 1, label %3
    i32 2, label %4
    i32 3, label %5
  ]

; <label>:3                                       ; preds = %0
  store i32 1, i32* %y, align 4
  br label %7

; <label>:4                                       ; preds = %0
  store i32 2, i32* %y, align 4
  br label %7

; <label>:5                                       ; preds = %0
  store i32 3, i32* %y, align 4
  br label %7

; <label>:6                                       ; preds = %0
  store i32 0, i32* %y, align 4
  br label %7

; <label>:7                                       ; preds = %6, %5, %4, %3
  %8 = load i32, i32* %y, align 4
  ret i32 %8
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.1 (tags/RELEASE_381/final)"}
