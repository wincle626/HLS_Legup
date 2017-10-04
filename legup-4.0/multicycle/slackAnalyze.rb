#This script allow to analyze slack of all path from "scheduling.legup.rpt"
#This script make log files for each functions

#How to use
#ruby slackAnalyze_bottom.rb


#declare each variable			
functionList = Array.new
outFileList = Array.new

phiRegList = Hash.new
allStatement = Hash.new
inputStateList = Hash.new
outputStateList = Hash.new
checkRegList = Hash.new
needRegList = Hash.new
allData = Hash.new


#separate "scheduling.legup.rpt" into each functions
File.open("scheduling.legup.rpt","r"){|infile|
	while inputLine = infile.gets
		if inputLine =~ /Start\sFunction:\s(\w+)/
			functionName = $+
			functionList.push(functionName)
			allStatement.store(functionName,[])
			allStatement[functionName].push(inputLine)
		elsif allStatement.key?(functionName)
			allStatement[functionName].push(inputLine)
		end
	end
}

#make result file of each functions
functionList.each{|fnc|
	outFile = "scheduling.analyze." + fnc + ".log"
	flag = 0
	File.open(outFile,"w"){|outfile|
		allStatement[fnc].each{|st|
			if st =~ /Basic Block:/
				if flag == 0
					outfile.puts("state: LEGUP_0")
					flag = 1
				end
			end
			outfile.puts(st)
		}
	}
}

#analze scheduling result each functions
functionList.each{|function|
	
	#declare each variable
	finish = 0
	i = 0
	
	curState = String.new
	preState = String.new
	endState = String.new
	
	dstReg = String.new
	srcReg = String.new
	
	tmpRegList = Array.new
	srcRegList = Array.new
	dstRegList = Array.new
	check = Array.new
	regList = Array.new
	makeLogFile = Array.new
	makeSdcInfo = Array.new
	
	usedStateInfo = Hash.new
	endStateInfo = Hash.new
	chainDstInfo = Hash.new
	instList = Hash.new
	instInfo = Hash.new
	lastStateList = Hash.new
	
	fileName = "scheduling.analyze." + function + ".log"
	phiRegList.store(function , [])
	
	File.open(fileName,"r"){|infile|
		while inputLine = infile.gets
			#get current state name
			if inputLine =~ /state:\s([A-Za-z0-9_]+)/
				preState = Marshal.load(Marshal.dump(curState))
				curState = $+
				
				#function call is in previous state
				if curState =~ /function_call_([0-9]+)/
					curState = preState.sub(/[0-9]+$/ , $+)
				end
			#get the information of registers
			elsif inputLine =~ /=/
				#get end state
				inputLine =~ /endState:\s([A-Za-z0-9_]+)/
				endState = $+
				#the case of function call is same as above
				if endState =~ /function_call_([0-9]+)/
					endState = preState.sub(/[0-9]+$/ , $+)
				end
				
				#get the all registers from current input line
				tmpRegList.concat(inputLine.scan(/(%[A-Za-z0-9\.\_\-]+)/))
				tmpRegList.flatten!
				
				#most left register, like A = B+C, is the output
				dstReg = tmpRegList.shift
				dstRegList.push(dstReg)
				
				usedStateInfo.store(dstReg,curState)
				
				#case of phi
				#mark registers is used in phi instruction to avoid infinite loop
				if inputLine =~ /phi/
					tmpRegList.clear
					tmpRegList.concat(inputLine.scan(/\[\s(%[A-Za-z0-9\.\_\-]+),/))
					tmpRegList.flatten!
					
					phiRegList[function].concat(tmpRegList)
					
					tmpRegList.each{|reg|
						tmpRegList[i] = tmpRegList[i] + "_phi"
						i = i + 1
					}
					i = 0
				end
				
				endStateInfo.store(dstReg,endState)
				chainDstInfo.store(dstReg,[])
				chainDstInfo[dstReg].concat(tmpRegList)
				
				tmpRegList.clear
			end
		end
	}
	
	srcRegList = Marshal.load(Marshal.dump(dstRegList))
	dstRegList.reverse!
	
	dstRegList.each{|reg|
	
		while true
			chainDstInfo[reg].each{|chain|
				chain =~ /^(%[A-Za-z0-9\.\_\-]+)/
				tmpChain = $+
				if usedStateInfo[reg] == endStateInfo[tmpChain]
					chainDstInfo[tmpChain].each{|each|
						chainDstInfo[reg].push(each+chain)
					}
					if !(chainDstInfo[tmpChain].empty?)
						chainDstInfo[reg].delete(chain)
						finish = 1
					end
				end
			}
			if finish == 0
				break
			end
			finish = 0
		end
	}
	
	dstRegList.each{|reg|
		chainDstInfo[reg].each{|chain|
			if chain =~ /phi/
				chainDstInfo[reg][i] = chainDstInfo[reg][i].sub(/_phi/ , "")
			end
			i = i + 1
		}
		i = 0
	}
	
	File.open(fileName,"r"){|infile|
		curState = "LEGUP_0"
		while inputLine = infile.gets
			if inputLine =~ /(%.*)\s=/
				dstReg = $+
				inputLine =~ /=\s(\w+)\s/
				
				instList.store(dstReg,$+)
			elsif inputLine =~ /state:\s([A-Za-z0-9_]+)/
				preState = Marshal.load(Marshal.dump(curState))
				curState = $+
				if curState =~ /function_call_([0-9]+)/
					curState = preState.sub(/[0-9]+$/ , $+)
				end
				
				curState =~ /([0-9]+$)/
				number = $+
				tmpState = preState.sub(/[0-9]+$/,number)
				preState =~ /([0-9]+$)/
				stateNum = $+
				
				if tmpState != curState
					tmpState.sub!(/#{number}$/,"")
					lastStateList.store(tmpState,stateNum)
				end
			end
		end
	}
	
	dstRegList.each{|reg|
		instInfo.store(reg,{})
		
		chainDstInfo[reg].each{|chain|
			regList.concat(chain.scan(/(%[A-Za-z0-9\.\_\-]+)/))
			regList.flatten!
			
			tmpChain = Marshal.load(Marshal.dump(chain))
			
			regList.each{|list|
				if instList[list]
					tmpChain.sub!(list,instList[list] + ".")
				else
					tmpChain.sub!(list, "prim.")
				end
			}
			
			tmpChain = tmpChain + instList[reg]
			tmpChain.sub!(/^\w+./,"")
			instInfo[reg].store(chain,tmpChain)
			
			regList.clear
		}
	}
	
	srcRegList.each{|src|
		endState = endStateInfo[src]
		dstRegList.each{|dst|
			chainDstInfo[dst].each{|chain|
				chain =~ /^(%[A-Za-z0-9\.\_\-]+)/
				fromReg = $+
				if src == fromReg
					if chain =~ /^%[A-Za-z0-9\.\_\-]+(%[A-Za-z0-9\.\_\-]+)/
						nxtReg = $+
					else
						nxtReg = Marshal.load(Marshal.dump(dst))
					end
					
					toReg = Marshal.load(Marshal.dump(dst))
					
					curState = usedStateInfo[nxtReg]
					curState =~ /([0-9]+$)/
					toNum = $+
					toBB = curState.sub(/([0-9]+$)/,"")
					
					toBB =~ /BB_(.*)/
					dstBasicBlock = $+
					dstBasicBlock.sub!(/(_)$/,"")
					
					endState =~ /([0-9]+$)/
					fromNum = ($+)
					fromBB = endState.sub(/([0-9]+$)/,"")
					
					curState = usedStateInfo[src]
					curState =~ /BB_(.*)/
					srcBasicBlock = $+
					srcBasicBlock.sub!(/(_[0-9]+)$/,"")
					
					if toBB != fromBB
						toNum = lastStateList[fromBB]
					end
					
					slack = (toNum.to_i - fromNum.to_i).to_s
					
					inst = instInfo[dst][chain]
					path = chain + dst
					
					makeLogFile.push([fromReg,toReg,path,inst,slack])
#					if inst =~ /load/
#					elsif inst =~ /phi/
##					if inst =~ /icmp/
##						makeSdcInfo.push([fromReg,toReg,srcBasicBlock,dstBasicBlock,slack])
#p inst
#					if inst =~ /.?sh./
						makeSdcInfo.push([fromReg,toReg,srcBasicBlock,dstBasicBlock,slack])
#p inst
					#end
				end
			}
		}
	}
	
	makeSdcInfo.uniq!
	allData.store(function,makeSdcInfo)
	
	File.open(fileName,"w"){|outfile|
		allStatement[function].each{|st|
			outfile.puts(st)
		}
		outfile.puts
		
		makeLogFile.each{|data|
			outfile.puts("source:\s" + data[0])
			outfile.puts("destination:\s" + data[1])
			outfile.puts("path:\s" + data[2])
			outfile.puts("instruction:\s" + data[3])
			outfile.puts("slack:\s" + data[4])
			outfile.puts
		}
		outfile.puts("--------------------------------------------------------------------------------")
	}
	
}

File.open("scheduling.legup.rpt","r"){|infile|
	
	curState = String.new
	preState = String.new
	endState = String.new
	
	regList = Array.new
	
	while inputLine = infile.gets
		if inputLine =~ /Start\sFunction:\s(\w+)/
			functionName = $+
			inputStateList.store(functionName,{})
			outputStateList.store(functionName,{})
			checkRegList.store(functionName,[])
		elsif inputLine =~ /state:\s([A-Za-z0-9_]+)/
			preState = Marshal.load(Marshal.dump(curState))
			curState = $+
			
			if curState =~ /function_call_([0-9]+)/
				curState = preState.sub(/[0-9]+$/ , $+)
			end
		elsif inputLine =~ /Basic\sBlock/
		elsif inputLine =~ /%/
			regList.concat(inputLine.scan(/(%[A-Za-z0-9\.\_\-]+)/))
			regList.flatten!
			
			
			if inputLine =~ /=/
				output = regList.shift
				checkRegList[functionName].push(output)
				inputLine =~ /endState:\s([A-Za-z0-9_]+)/
				endState = $+
				if endState =~ /function_call_([0-9]+)/
					endState = preState.sub(/[0-9]+$/ , $+)
				end
				
				outputStateList[functionName].store(output,endState)
				
			end
			
			regList.each{|reg|
				if !(inputStateList[functionName].key?(reg))
					inputStateList[functionName].store(reg,[])
				end
				
				inputStateList[functionName][reg].push(curState)
				inputStateList[functionName][reg].uniq!
			}
			regList.clear
		end
	end
}

functionList.each{|function|
	needRegList.store(function,[])
	
	checkRegList[function].each{|reg|
		if inputStateList[function].key?(reg)
			inputStateList[function][reg].each{|input|
				if input != outputStateList[function][reg]
					needRegList[function].push(reg)
					break
				end
			}
		end
	}
	needRegList[function].concat(phiRegList[function])
	needRegList[function].uniq!
}

File.open("scheduling.analyze.all.log","w"){|outfile|
	functionList.each{|function|
		outfile.puts("function:\s" + function)
		allData[function].each{|data|
			if needRegList[function].include?(data[0])
				if needRegList[function].include?(data[1])
					if data[4].to_i >= 2
						outfile.puts("src:"+data[0]+"\sdst:"+data[1]+"\ssrcBB:"+data[2]+"\sdstBB:"+data[3]+"\sslack:"+data[4])
					end
				end
			end
		}
	}
}

