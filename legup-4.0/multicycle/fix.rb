#Use this Ruby script if you stil have some warnings after you used "modifySdcFile.rb"
#This script change source and destination registers to correct one
#Basicly this script takes much time more than other scripts

#How to use
#ruby fix.rb [Verilog file] [sdc file]


#read each files from command line
verilogFile = ARGV[0] #Verilog file in the benchmark that you want to apply multi-cycle
sdcFile = ARGV[1] #SDC file in benchmark that you want to apply multi cycle

#declare each variable
flag = 0

allStatement = Array.new
fromRegList = Array.new
toRegList = Array.new
fromSubList = Array.new
toSubList = Array.new

fromChange = Hash.new
toChange = Hash.new

#get source register and destination register
File.open(sdcFile , "r"){|infile|
	while inputLine = infile.gets
		allStatement.push(inputLine)
		if inputLine =~ /set_multicycle_path/
			#get source register
			inputLine =~ /-from.*\{\*(.*)\*\}.*-to/
			fromReg = $+
			fromRegList.push(fromReg)
			
			#get destination register
			inputLine =~ /-to.*\{\*(.*)\*\}/
			toReg = $+
			toRegList.push(toReg)
		end
	end
}

fromRegList.uniq!
toRegList.uniq!

#to make hash {(original source register) => (right register)}
#this case should happen when resource sharing is used
fromRegList.each{|fromReg|
	File.open(verilogFile , "r"){|infile|
		while inputLine = infile.gets
			if inputLine =~ /#{fromReg}\s=\s(.*_reg);/
				fromChange.store(fromReg,$+)
				fromSubList.push(fromReg)
				break
			end
		end
	}
}

#to make sence the register exist or not in Verilog file 
#case of not should happen when one register is used as input of phi instruction
#and argument of the other function (not sure)
toRegList.each{|toReg|	
	File.open(verilogFile , "r"){|infile|
		while inputLine = infile.gets
			if inputLine =~ /#{toReg}/
				flag = 1
				break
			end
		end
	}
	#"flag == 0" mean that target register could NOT be found in Verilog file
	if flag == 0
		toSubList.push(toReg)
	end
	flag = 0
}

#to make hash {(original desitination register) => (right register)}
toSubList.each{|toReg|
	tmpReg = toReg.sub("_reg","")
	File.open(verilogFile , "r"){|infile|
		while inputLine = infile.gets
			if inputLine =~ /=\s#{tmpReg};/
				inputLine =~ /([a-zA-Z0-9_]+)\s<*=/
				dstReg = $+
				toChange.store(toReg,dstReg)
			end
		end
	}
}

toRegList.each{|toReg|
	File.open(verilogFile , "r"){|infile|
		while inputLine = infile.gets
			if inputLine =~ /#{toReg}\s=\s(.*_reg);/
				toChange.store(toReg,$+)
				toSubList.push(toReg)
				break
			end
		end
	}
}

#change source register
fromSubList.each{|fromReg|
	allStatement.each{|statement|
		if statement =~ /#{fromReg}/
			statement.gsub!(/#{fromReg}/,fromChange[fromReg])
		end
	}
}

#change destination register
toSubList.each{|toReg|
	allStatement.each{|statement|
		if statement =~ /-to.*#{toReg}/
			if toChange[toReg]
				statement.gsub!(/#{toReg}/,toChange[toReg])
			end
		end
	}
}

#output the information for sdc file
File.open(sdcFile , "w"){|outfile|
	allStatement.each{|statement|
		outfile.puts(statement)
	}
}

