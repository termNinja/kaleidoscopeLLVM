; ModuleID = 'main.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @add(i32 %x, i32 %y) #0 {
  %1 = alloca i32, align 4 				; allocate i32 on stack
  %2 = alloca i32, align 4 				; allocate i32 on stack
  %z = alloca i32, align 4 				; allocate i32 on stack
  store i32 %x, i32* %1, align 4 		; store argument 1 on stack
  store i32 %y, i32* %2, align 4 		; store argument 2 on stack
  %3 = load i32, i32* %1, align 4 		; load argument 1 from stack
  %4 = load i32, i32* %2, align 4 		; load argument 2 from stack
  %5 = add nsw i32 %3, %4 				; add arg1, arg2
  store i32 %5, i32* %z, align 4 		; store result of add
  %6 = load i32, i32* %z, align 4 		; load result of add
  ret i32 %6 							; return result
}

; Function Attrs: nounwind uwtable
define i32 @sub(i32 %x, i32 %y) #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %z = alloca i32, align 4
  store i32 %x, i32* %1, align 4
  store i32 %y, i32* %2, align 4
  %3 = load i32, i32* %1, align 4
  %4 = load i32, i32* %2, align 4
  %5 = mul nsw i32 %3, %4
  store i32 %5, i32* %z, align 4
  %6 = load i32, i32* %z, align 4
  ret i32 %6
}

; Function Attrs: nounwind uwtable
define i32 @div(i32 %x, i32 %y) #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %z = alloca i32, align 4
  store i32 %x, i32* %1, align 4
  store i32 %y, i32* %2, align 4
  %3 = load i32, i32* %1, align 4
  %4 = load i32, i32* %2, align 4
  %5 = sdiv i32 %3, %4
  store i32 %5, i32* %z, align 4
  %6 = load i32, i32* %z, align 4
  ret i32 %6
}

; Function Attrs: nounwind uwtable
define i32 @mod(i32 %x, i32 %y) #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %z = alloca i32, align 4
  store i32 %x, i32* %1, align 4
  store i32 %y, i32* %2, align 4
  %3 = load i32, i32* %1, align 4
  %4 = load i32, i32* %2, align 4
  %5 = srem i32 %3, %4
  store i32 %5, i32* %z, align 4
  %6 = load i32, i32* %z, align 4
  ret i32 %6
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.1 (tags/RELEASE_381/final)"}
