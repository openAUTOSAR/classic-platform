#/bin/bash
#
# $ req (OS|xxx) (get|diff) ... [filter <xx>]
#     ls                : Print all requirements
#                       :
#     get <requirement> :
#     diff <1> <2>      : <> = req | code | test
#     filter <xx>       :
#
# Diff the requirements against the code
# $ req OS diff req code
#
# Diff the requirements against the code
# $ req OS diff req test
#
# Get the requiment for OS123
# $ req OS get OS123
#
# Get the requiment for OS123
# $ req OS get OS123 "<Class>=1"
# $ scripts/req.sh diff OS code "Class=1"

REBOL=/C/devtools/utils/r3-a96.exe

#case $2 in
#  get) echo "get";;
#  diff) echo "diff";;
#  *)  echo "default";
#esac

#echo $2
#echo $3
#echo $4


# exit

# req-diff req code "<Class>=2"
# req-ls req
# req-ls code "<Class>=1"
# req-ls test

SCRIPT_DIR=`dirname $0`
cmd.exe /S /K "$REBOL $SCRIPT_DIR\extract_req.r $1 $2 $3 \"$4\" > result.txt && exit"
#powershell -Command "$REBOL $SCRIPT_DIR\extract_req.r $1 $2 $3 \"$4\" > result.txt"
#powershell -Command $REBOL $SCRIPT_DIR/extract_req.r $1 $2 $3 \"$4\"
cat result.txt









