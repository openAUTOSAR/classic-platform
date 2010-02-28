comment {
  Rebol 3.0 Alpha version ONLY
  May be run with from msys:
   $ powershell -Command "r3-a96.exe scripts\extract_req.r > result.txt"
     OR
   $ cmd.exe /S /K "r3-a96.exe scripts\extract_req.r > result.txt && exit"
}

REBOL [
    Title:  "Requirements extractor"
    Date:   6-Jan-2010
    File:   %extract_req.r
    Author: "mahi"
    Version: 0.0.1
]

usage: does [
; do/args %req_extract.r ["diff" "OS" "req" "Class=1,Component=Code"]
  print "Usage: do/args %req_extract.r [ ^"<cmd>^" ^"[....]^" ]"
  print "       req.sh diff OS req ^"Class=1^""
  print "  <cmd> = (diff|ls) "
  print "    diff <module> (code|test) <filter>"
  print "    ls   <module> <filter>"
  quit
]

;=======================================================
; See http://www.rebol.com/r3/docs/functions/secure.html
secure [
    net quit
    file allow  ; Allow Read/write to files.
]

;=======================================================
right: func [s [string!] n [integer!]] [
  copy/part at s (length? s) - n + 1 n
]

;=======================================================

find-files: func [dir list /local files] [
    files: sort load dir

    ; Get files that match the above types:
    foreach file files [
        if find file-types suffix? file [
            append list dir/:file
        ]
    ]
    ; Search sub-directories:
    foreach file files [
        if find file "/" [find-files dir/:file list]
    ]
]

;=======================================================

req-xml: func[
  filename [file!]
  filter [string!]
  "Filters separated with a ,"
][
  ; print filename
  ; Get a good rebol3 xml parser..
  if not any-function? :load-xml [
  	do http://www.ross-gill.com/r/r3xml.r
  ]

  filter-split: parse/all filter ","
  doc: load-xml/dom filename
  body: doc/get-by-tag <Req>
  list: []
  foreach j filter-split [print j]

  print "go........"
  ; Go over all req tags.
  foreach iter body [
    ; Look for any matching filter
    fail: false
    foreach i filter-split [
      val: parse i "="
      if/else find (iter/get to-tag val/1) val/2 [
      ] [
        fail: true
        break
      ]
    ]
    if fail == false [ append list iter/get #id ]
  ]
  return list
]

comment {
req-xml: func[
  filename [file!]
  filter [string!]
  "Filters separated with a ,"
][
  ; print filename
  ; Get a good rebol3 xml parser..
  if not any-function? :load-xml [
  	do http://www.ross-gill.com/r/r3xml.r
  ]

  filter-split: parse/all filter "="
  doc: load-xml/dom filename
  body: doc/get-by-tag <Req>
  list: []
  print ["Filter:" filter-split/1 filter-split/2]
  foreach iter body [
    foreach filter-entry filter-split [
      if/else find (iter/get to-tag filter-split/1) filter-split/2 [
        append list iter/get #id
      ] [ break ]
    ]
  ]
  return list
]
}

;=======================================================
req-source: func[
  dir-list [block!]
][
  file-types: [%.c %.h]
  file-list: []
  req-list: []
  text: string!
;  text2: string!

  ; Grab the file-list...
  foreach dir dir-list [
    print dir
    foreach file load dir [
      if find file-types suffix? file [append file-list dir/:file]
    ]
  ]

  rule:[
    any [
      thru "@req " copy text to [ newline | "*/"]
      (append req-list first parse text none)
;      (text2: first parse text none)
;      (append req-list text2)
;      (probe text2)
    ]
  ]

  clear req-list
  foreach file file-list [
    ;clear text
    text: ""
    content: read/string file
;    probe file
    parse content rule
;    print text
    ;append req-list first parse text none
    ;parse content [ any [thru "@req" begin: to [ newline | "*/" ] fin: (append req-list trim copy/part at begin 0 ((index? fin) - (index? begin)) )]]
  ]
  return req-list
]

;=======================================================

; Simplest to keep all requirement files in one directory:
; e.g req-os.xml, req-can.xml

; Rebol strange argument passing...
if/else (found? system/options/args) [
  ; Used from real command line
  args: system/options/args
] [
  if/else(found? system/script/args) [
    ; Used when running from within rebol, e.g. do/args...
    args: system/script/args
  ] [ usage ]
]

foreach a args [print ["arg:" a]]
cmd: pick args 1

print length? args
; Check args
switch cmd [
  "diff" [ if (length? args) != 4 [usage]]
  "ls"   [ if (length? args) != 4 [usage]]
]

path-info: [
	["OS"  %req-os.xml [%../system/kernel/  %../system/kernel/include/ %../include/ ] "Class=1,2,3,4"]
	["CAN" %req-can.xml [%../arch/ppc/mpc55xx/drivers] ]
]

print length? path-info
;foreach a path-info/1 [print a]
;print length? path-info/1
;print path-info/1/1
;print dir? path-info/1/3
;print path-info/1/2
;print exists? path-info/1/2

list-xml: req-xml path-info/1/2 pick args 4 ;"Class=1"
list-source: req-source path-info/1/3

print length? list-xml
print length? list-source

print ["Requirements for module:" (pick args 2) "with filter:" pick args 4 ]
probe sort list-xml
print newline

print "Requirements for from source"
probe sort list-source
print newline

print "Requirements diff"
probe exclude list-xml list-source
print newline

comment {
  none
}






