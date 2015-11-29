if exists("b:current_syntax")
    finish
endif

let b:current_syntax="haxy"

syn keyword haxyKeyword  def var if else elif while import return
syn match   haxyFunction "\w\+\s*("me=e-1

syn match   haxyNumber   "\d\+"
syn match   haxyNumber   "-\d\+"
syn match   haxyNumber   "\d\+.\d\+"
syn match   haxyNumber   "-\d\+.\d\+"
syn region  haxyString   start='"' end='"' contained

hi link haxyKeyword Keyword
hi link haxyFunction Function
hi link haxyNumber Number
hi link haxyString String
