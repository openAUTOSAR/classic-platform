#!/bin/bash

if [[ $1 == '-h' || $1 == '-help' || $1 == '--h' || $1 == '--help' ]]; then
	echo ""
	echo "Usage: $0 [board1 board2 ...]"
	echo ""
	echo "       board1 board2 ... "
	echo "         Boards to build for (optional)"
	echo "         If no boards are given, all boards will be built."
	exit 0
fi

ARC_DIR=../
EXAMPLE_DIR=${ARC_DIR}/examples/
COMPILER_DIR=/opt/compilers/
OUTPUT=test_output.xml
BUILDOUTTMP=tmp_out.txt


function failedTest {
echo "<FailedTest id=\"${1}\"> \
  <Name>${2}</Name> \
  <Message>${3}</Message> \
</FailedTest>" >> $OUTPUT

#	echo "<Location>"
#	echo "<File>c:/ArcticStudio/workspace/arc-tests/ComTests/com_test.c</File>"
#	echo "<Line>444</Line>"
#	echo "</Location>"
#	echo "<Message>UPNotificationCalled == getNumberOfSignalsInPdu(IPduId) &amp;&amp; 0</Message>"
}

function successfulTest {
echo "<Test id=\"${1}\"> \
  <Name>${2}</Name> \
</Test>" >> $OUTPUT
}

function warningTest {
echo "<Test id=\"${1}\"> \
  <Name>${2}</Name> \
  <Warning>${3}</Warning>
</Test>" >> $OUTPUT
}

cl_args=$@

function inArguments {
	board=$1
	for cl_arg in $cl_args; do
		if [ "$cl_arg" == "$board" ]; then
			return 1
		fi
	done
	return 0
}

function shouldBuild {
	board=$1
	if [ -z "$cl_args" ]; then
		# No args; Build all
		return 1
	else
		# Check if told to build for board
		inArguments "$board"; ret=$?
		return $ret
	fi
}

echo "<?xml version=\"1.0\" encoding='utf8' standalone='yes' ?> \
<?xml-stylesheet type=\"text/xsl\" href=\"unittests.xsl\" ?> \
<TestRuns> \
<TestRun> \
<CompileExamples>" > $OUTPUT


LOGFILE=compile_examples.log
if [ -f $LOGFILE ]; then
	echo "Removing old log"
	rm -f $ $LOGFILE
fi

TEST_ID=1
for EXAMPLE in `ls $EXAMPLE_DIR`; do
	for BOARD in `ls ${EXAMPLE_DIR}${EXAMPLE}/config`; do
		shouldBuild "$BOARD"; build=$?
		if [ $build -eq 1 ]; then
			C=`grep $BOARD compilers.txt`
			COMPILER=${COMPILER_DIR}${C#$BOARD:}
	
			TEST_NAME=${EXAMPLE}-${BOARD}
	
			if [ -x "${COMPILER}gcc" ]; then
				BUILD_FILE=${EXAMPLE}_${BOARD}.elf 
				make -C ${ARC_DIR} \
					CROSS_COMPILE=$COMPILER \
					BOARDDIR=$BOARD \
					BDIR=examples/$EXAMPLE \
					build-exe-y=${BUILD_FILE} \
					clean all 2>&1 | tee -a $LOGFILE
	
				if [ -a ${ARC_DIR}/binaries/${BUILD_FILE} ]; then
					successfulTest $TEST_ID $TEST_NAME
				else
					BUILDOUT=`cat $BUILDOUTTMP`
					failedTest $TEST_ID $TEST_NAME "See console output for information. (compile_examples.log)"
				fi
			else
				warningTest $TEST_ID $TEST_NAME "No compiler found"
			fi
			let TEST_ID=TEST_ID+1
		fi
	done
done

echo "</CompileExamples> \
</TestRun> \
</TestRuns>" >> $OUTPUT

