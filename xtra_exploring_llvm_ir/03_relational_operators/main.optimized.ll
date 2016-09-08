; ModuleID = 'main.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readnone uwtable
define zeroext i1 @_Z7greaterii(i32 %a, i32 %b) #0 {
  %1 = icmp sgt i32 %a, %b
  ret i1 %1
}

; Function Attrs: norecurse nounwind readnone uwtable
define zeroext i1 @_Z4lessii(i32 %a, i32 %b) #0 {
  %1 = icmp slt i32 %a, %b
  ret i1 %1
}

; Function Attrs: norecurse nounwind readnone uwtable
define zeroext i1 @_Z16greater_or_equalii(i32 %a, i32 %b) #0 {
  %1 = icmp sge i32 %a, %b
  ret i1 %1
}

; Function Attrs: norecurse nounwind readnone uwtable
define zeroext i1 @_Z13less_or_equalii(i32 %a, i32 %b) #0 {
  %1 = icmp sle i32 %a, %b
  ret i1 %1
}

; Function Attrs: norecurse nounwind readnone uwtable
define zeroext i1 @_Z5equalii(i32 %a, i32 %b) #0 {
  %1 = icmp eq i32 %a, %b
  ret i1 %1
}

; Function Attrs: norecurse nounwind readnone uwtable
define zeroext i1 @_Z9not_equalii(i32 %a, i32 %b) #0 {
  %1 = icmp ne i32 %a, %b
  ret i1 %1
}

attributes #0 = { norecurse nounwind readnone uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.1 (tags/RELEASE_381/final)"}
