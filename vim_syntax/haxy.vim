if exists("b:current_syntax")
    finish
endif

let b:current_syntax="haxy"

syn keyword haxyKeyword  def var if else elif while import return class
syn match   haxyFunction "\w\+\s*("me=e-1
syn match   haxyClass    "\w\+\s*{"me=e-1

syn match   haxyNumber   "\d\+"
syn match   haxyNumber   "-\d\+"
syn match   haxyNumber   "\d\+.\d\+"
syn match   haxyNumber   "-\d\+.\d\+"
syn region  haxyString   start='"' end='"' contained

syn match   haxyDot         "\."
syn match   haxyReference   "@\w\+"

hi link haxyKeyword Keyword
hi link haxyFunction Function
hi link haxyNumber Number
hi link haxyString String
hi link haxyClass  Type
hi link haxyDot    Macro
hi link haxyReference Macro
