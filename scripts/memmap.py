
"""

Description 
    Will generate some files to aid in writing MemMap jobs. Files created are:
    
      For linker file:
        - linkscript_bss_diab.ldf
        - linkscript_data_diab.ldf
        - linkscript_data_rom_diab.ldf
      
      For MemMap file:
        - MemMap_part1.h
        - MemMap_part2.h
    
      Include the MemMap files into the normal MemMap.h file.
      Include the linker file parts into the linker file.    
 
      It will only generate sections that are related to the RTE, e.g.
      - <PREFIX>_START_SEC_VAR_CLEARED_UNSPECIFIED
      - <PREFIX>_START_SEC_VAR_INIT_UNSPECIFIED
      - <PREFIX>_START_SEC_VAR_CALIB_UNSPECIFIED 
 
Usage:
    python ../../../core/scripts/memmap.py config/mpc5516it/Rte/MemMap/

Limitations:
    The obvious way to do find "mapping" between the SWC and a partition would 
    be to read the model... but short of time.
    
    - STOP sections are generated but contains nothing
"""


import optparse
import re
import os

mapping = {
	'ApplicationModeManagerType':'TrustedPartition',
	'ApplicationModeUserType':'TrustedPartition',
	'CalibrationComponentType':'TrustedPartition',
	'ComplexDeviceDriverComponentType':'TrustedPartition',
	'Dem':'TrustedPartition',
	'EcuAbstractionComponentType':'TrustedPartition',
	'ExclusiveAreaComponentType':'TrustedPartition',
	'ExclusiveAreaMultiComponentType':'TrustedPartition',
	'ExclusiveAreaRelayType':'TrustedPartition',
	'MonitorComponentType':'TrustedPartition',
	'MultiComponentType2':'UntrustedPartion1',
	'MultiComponentType':'UntrustedPartion1',
	'ParameterComponentType':'TrustedPartition',
	'PimComponentType':'TrustedPartition',
	'ReaderComponentType':'TrustedPartition',
	'Rte':'TrustedPartition',
	'SensorComponentType':'TrustedPartition',
	'SingleReaderComponentType':'UntrustedPartion2',
	'SingleWriterComponentType':'UntrustedPartion2',
	'SlaveCoreComponentType':'SlaveCoreParition',
	'UntrustedComponentType1':'UntrustedPartion1',
	'UntrustedComponentType2':'UntrustedPartion2',
	'UntrustedParameterComponentType':'UntrustedPartion1',
	'WriterComponentType':'TrustedPartition',
}

taskMapping = {
   'UntrustedPartion1':['AsynchronousServers1','UntrustedTask1'],
   'UntrustedPartion2':['UntrustedTask2','AsynchronousServers2','SingleWriterComponent_ServerPort1_enumOp'],
   'TrustedPartition':['BasicTask2','BasicTask5','BasicTask7','BasicTask9','EventsBasicTask','EventsExtendedTask','ExtendedTask1','ExtendedTask2','TestMaster'],
   'SlaveCoreParition':['SlaveCoreTask']
}


class Emitter_MemMap:

	def __init__(self):
		self.file_def = open("MemMap_part1.h",'w')
		self.file = open("MemMap_part2.h",'w')

	def __del__(self):
		self.file.close()
		self.file_def.close()

	def addPartition(self,partition):
		self.file_def.write("#define " + partition.upper() + "_VAR_INIT_UNSPECIFIED __attribute__ ((section (\"." + partition.lower() + "_data\")))\n")
		self.file_def.write("#define " + partition.upper() + "_VAR_CLEARED_UNSPECIFIED __attribute__ ((section (\"." + partition.lower() + "_bss\")))\n")

	def add(self, swc, partition ):
		self.file.write("#ifdef %s_START_SEC_VAR_CLEARED_UNSPECIFIED\n" % (swc))
		self.file.write("%s_VAR_CLEARED_UNSPECIFIED\n" % (partition))
		self.file.write("#endif\n\n")

		self.file.write("#ifdef %s_START_SEC_VAR_INIT_UNSPECIFIED\n" % (swc))
		self.file.write("%s_VAR_INIT_UNSPECIFIED\n" % (partition))
		self.file.write("#endif\n\n")
		self.file.write("")

		self.file.write("#ifdef %s_START_SEC_VAR_CALIB_UNSPECIFIED\n" % (swc))
		self.file.write("%s_VAR_CALIB_UNSPECIFIED\n" % (partition))
		self.file.write("#endif\n\n")

		self.file.write("#ifdef %s_STOP_SEC_VAR_CLEARED_UNSPECIFIED\n" % (swc))
		self.file.write("#endif\n\n")

		self.file.write("#ifdef %s_STOP_SEC_VAR_INIT_UNSPECIFIED\n" % (swc))
		self.file.write("#endif\n\n")
		self.file.write("")

		self.file.write("#ifdef %s_STOP_SEC_VAR_CALIB_UNSPECIFIED\n" % (swc))
		self.file.write("#endif\n\n")


class Emitter_Linker:

	def __init__(self):
		self.file_bss_diab = open("linkscript_bss_diab.ldf",'w')
		self.file_data_diab = open("linkscript_data_diab.ldf",'w')
		self.file_data_rom_diab = open("linkscript_data_rom_diab.ldf",'w')

	def __del__(self):
		self.file_bss_diab.close()
		self.file_data_diab.close()
		self.file_data_rom_diab.close()

	def add(self, partition, tasks ):
		
		for x in tasks:
			self.file_data_diab.write("__OS_START_SEC_data_" + x + " = .;\n")
			self.file_bss_diab.write("__OS_START_SEC_bss_" + x + " = .;\n")
		
		self.file_data_diab.write("." + partition.lower() + "_data (DATA) LOAD(__" + partition.upper() + "_ROM) : {}\n")
		self.file_bss_diab.write("." + partition.lower() + "_bss (BSS)  : {}\n")
		
		self.file_data_rom_diab.write("__" + partition.upper() + "_ROM = .;\n")
		self.file_data_rom_diab.write(".=.+SIZEOF(." + partition.lower() + "_data);\n" )
		
		for x in tasks:
			self.file_data_diab.write("__OS_STOP_SEC_data_" + x + " = .;\n")
			self.file_bss_diab.write("__OS_STOP_SEC_bss_" + x + " = .;\n")


if __name__ == "__main__":
	parser = optparse.OptionParser()
	(options, args) = parser.parse_args()

	if len(args) != 1:
		parser.error("Need path to RTE MemMap files")

	# Grab all swc's
	memmap = Emitter_MemMap()
	linker_diab = Emitter_Linker()

	apa=os.listdir(args[0]);
	for file in apa:
		m = re.search('^(.*)_MemMap.h',file)
		swc = m.group(1)
#		print swc
		partition = mapping[swc];
		memmap.add(swc, partition.upper())
		

	for partition in taskMapping:
		linker_diab.add(partition, taskMapping[partition])
		memmap.addPartition(partition)

