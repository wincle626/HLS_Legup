#Use this file to modify sdc file after you used "slackAnalyze_bottom.rb"

#How to use
#ruby modifySdcFile.rb [sdc file]

#read sdc file from command line
outFile = ARGV[0] #SDC file in benchmark directory that you want to apply multi cycle

File.open("scheduling.analyze.all.log","r"){|infile|
	File.open(outFile,"w"){|outfile|
		#this 5 lines is written in the original sdc file made by LegUp
		outfile.puts("create_clock -period 2 -name clk [get_ports clk]")
		outfile.puts("create_clock -period 1 -name clk2x [get_ports clk2x]")
		outfile.puts("create_clock -period 2 -name OSC_50_BANK2 [get_ports OSC_50_BANK2]")
		outfile.puts("derive_pll_clocks")
		outfile.puts("derive_clock_uncertainty")
		outfile.puts
		#apply false path for memory controller
		outfile.puts("#set_false_path -from [get_registers {*memory_controller*}] -to [get_registers {*}]")
		outfile.puts("#set_false_path -from [get_registers {*}] -to [get_registers {*memory_controller*}]")
		outfile.puts
		#apply false path for the path has state register
		outfile.puts("#set_false_path -from [get_registers {*cur_state*}] -to [get_registers {*}]")
		outfile.puts("#set_false_path -from [get_registers {*}] -to [get_registers {*cur_state*}]")
		outfile.puts
		
		outfile.puts("#set_false_path -from [get_registers {*ram*}] -to [get_registers {*}]")
		outfile.puts("#set_false_path -from [get_registers {*}] -to [get_registers {*ram*}]")
		outfile.puts
		
		outfile.puts("#set_false_path -from [get_registers {*DFF*}] -to [get_registers {*}]")
		outfile.puts("#set_false_path -from [get_registers {*}] -to [get_registers {*DFF*}]")
		outfile.puts
		
		outfile.puts("#set_false_path -from [get_registers {*arg*}] -to [get_registers {*}]")
		outfile.puts("#set_false_path -from [get_registers {*}] -to [get_registers {*arg*}]")
		outfile.puts
		
		#apply false path for the path from phi_temp to anything
		#do NOT apply for the path from anything to phi_temp
		outfile.puts("#set_false_path -from [get_registers {*_phi_temp*}] -to [get_registers {*}]")
		outfile.puts("#set_false_path -from [get_registers {*}] -to [get_registers {*_phi_temp*}]")
		outfile.puts
		
		#read the "scheduling.analyze.all.log" made by slack analyze
		while inputLine = infile.gets
			if inputLine =~ /function:\s(\w+)/
				functionName = $+
			else
				inputLine =~ /src:%(.*)\sdst:/
				src = $+
				
				inputLine =~ /dst:%(.*)\ssrcBB/
				dst = $+
				
				inputLine =~ /srcBB:(.*)\sdstBB/
				srcBB = $+
				
				inputLine =~ /dstBB:(.*)\sslack/
				dstBB = $+
				inputLine =~ /slack:([0-9]+)/
				slack = $+
				
				tmpReg = src.gsub(".","_")
				tmpReg.gsub!("-","_")
				
				fromVerilogName = functionName+"_"+srcBB+"_"+tmpReg+"_reg"
				
				tmpReg = dst.gsub(".","_")
				tmpReg.gsub!("-","_")
				
				toVerilogName = functionName+"_"+dstBB+"_"+tmpReg+"_reg"
				
				#apply multi-cycle for the path has slack >= 2				
				outfile.puts("set_multicycle_path\s-from\s[get_registers\s{*"+fromVerilogName+"*}]\s-to\s[get_registers\s{*"+toVerilogName+"*}]\s-setup\s-end\s"+slack)
				outfile.puts("set_multicycle_path\s-from\s[get_registers\s{*"+fromVerilogName+"*}]\s-to\s[get_registers\s{*"+toVerilogName+"*}]\s-hold\s-end\s"+(slack.to_i - 1).to_s)
			end
		end
	}
}


