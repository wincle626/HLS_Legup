set fp [open "trace.txt" w]

puts $fp [expr {$now / 20000}]

for {set t [expr wide(40000)]} {$t < $now} {incr t 20000} {
	
	set module [examine -u -time $t "top_inst/dbg_active_instance"]
	set state [examine -u -time $t "top_inst/dbg_current_state"]
	
	puts $fp "State $t $module $state"
}

close $fp