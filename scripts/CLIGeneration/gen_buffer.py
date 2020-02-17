# -*- coding: utf-8 -*-
import argparse
import json
import os
import subprocess

#ECLIPSE_BIN = "/c/Projects/ides/com.arccore.cli.product-win32.win32.x86_64-6.0.0/eclipsec.exe"

json_file_name = "generation_registry.json"

if  __name__ =='__main__':

	parser = argparse.ArgumentParser()
	parser.add_argument("-a", "--action", dest = "action", help="What to do: register/generate")
	parser.add_argument("-x", "--arxml_files", nargs='*', dest = "arxml_files", help="Arxml files to load")
	parser.add_argument("-m", "--module", dest = "module", help="Module to generate")
	parser.add_argument("-i", "--project_id", dest = "project_id", help="A unique string to identify the project.")
	parser.add_argument("-e", "--eclipse_executable", dest = "eclipse_executable", help = "Path to the generator executable (eclipsec/arcgen)")
	parser.add_argument("-v", "--verbosity", dest = "verbosity", help = "Verbosity level for the generator (DEBUG/INFO/WARN)")
	parser.add_argument("-c", "--ecuc", dest = "ecuc", help = "Ecuc Value Collection")
	parser.add_argument("-o", "--output", dest = "output", help = "Path to output dir")
	parser.add_argument("-d", "--data", dest = "data", help = "Workspace location (path)")
	parser.add_argument("-p", "--project", dest = "project", help = "Name of temporary project")
	parser.add_argument("-j", "--vm", dest = "vm", help = "Path to java VM (bin folder)")

	arguments = parser.parse_args()

	if( arguments.action == "register" ):

		if( os.path.isfile(json_file_name) ):
			json_input_file = open(json_file_name)
			json_data = json_input_file.readline()
			json_input_file.close()
			entrys_in = json.loads(json_data)

			if( arguments.project_id in entrys_in ):
				arxml_set = set(entrys_in[arguments.project_id]['x'])
				arxml_set.update(arguments.arxml_files)
				entrys_in[arguments.project_id]['x'] = list(arxml_set)

				module_set = set(entrys_in[arguments.project_id]['m'])
				module_set.update([arguments.module])
				entrys_in[arguments.project_id]['m'] = list(module_set)
			else:
				entrys_in[arguments.project_id] = { 'x' : arguments.arxml_files, 'm' : [arguments.module] }
			
		else:
			entrys_in = { arguments.project_id : { 'x' : arguments.arxml_files, 'm' : [arguments.module] } }

		json_file = open(json_file_name,"w")
		json_file.write(json.dumps(entrys_in))
		json_file.close()

	elif( arguments.action == "generate"):
		if( os.path.isfile(json_file_name) ):
			json_input_file = open(json_file_name)
			json_data = json_input_file.readline()
			entrys_in = json.loads(json_data)

			if( arguments.project_id in entrys_in ):
				arxml_files_comma = ",".join(entrys_in[arguments.project_id]['x'])
				modules_comma = ",".join(entrys_in[arguments.project_id]['m'])
				call = [
					arguments.eclipse_executable,
					"-application", "com.arccore.cli.generator",
					"-data", arguments.data,
					"-project", arguments.project,
					"-ecuc", arguments.ecuc,
					"-output", arguments.output,
					"-external", arxml_files_comma,
					"-modules", modules_comma
				]
				if( arguments.vm != None ):
					call.extend(["-vm", arguments.vm])
				print( " ".join(call) )
				subprocess.call(call)
			else:
				print("Project not registered")


		else:
			print("No modules registered for generation (nothing to be done for target)")

	else:
		print( "Unknown action" )