#---------------------------------------------------------------
# Remote Modelsim access demo
# HABT08
#
# Source this script before opening up a terminal
#---------------------------------------------------------------


#restart -f
#onbreak {resume}
							 
# Error handler
proc bgerror {args} {
  global listen_socket
  catch {close $listen_socket}
  echo "bgerror invoked, args were: $args"
  stop
}

proc rx_command {} {
  global listen_socket com_sock

  if {[eof $com_sock]} {
    echo "Telnet client disconnected"; 
    catch {close $listen_socket}
    stop
  } else {
    # Read Telnet command
    set cmd [gets $com_sock]; 
    flush $com_sock	
	echo "Received from Telnet client: $cmd"
	if {[scan $cmd "%c"]==27} {
		echo "Escape received, terminating link....."
    	catch {close $listen_socket}
		catch {close $com_sock}
    	stop
	} else { 
		# Execute received command
		#set res [eval transcribe $cmd]
		set res [eval $cmd]
		puts -nonewline $com_sock "$res\nVSIM>"
		flush $com_sock
		
		#if {[catch {eval $cmd} results]} { 
        	#echo "Remote error: $results" 
		#	puts $com_sock "$results"; 
	    	#flush $com_sock
	    #}
	    # Transmit command prompt terminal 
		#puts -nonewline $com_sock "VSIM>"; 		
		#set res [eval $cmd]
		#puts -nonewline $com_sock "$res\nVSIM>"
		#puts -nonewline $com_sock "VSIM>"
	   	#flush $com_sock
	}
  }
  return
}

# Event handler for server 
proc accept {fd address port} {
  	global client_online com_sock

	set com_sock $fd
  	puts $fd "Connected to Modelsim Server on port $com_sock \n"; 
	flush $fd
	set client_online 1
  
	# create event when cmd is available from telnet
  	fileevent $fd readable "rx_command"

#  	flush $fd
}

# open the listen socket on port 2000
set serverPort 2000
set listen_socket [socket -server accept $serverPort]

# put server in non-blocking mode
fconfigure $listen_socket -blocking 0

# send server startup notice to stdout
echo "Simple Server running...\n"
echo "Listening on port $serverPort"; 
#flush stdout

# optional, uncomment if you wish to see the socket info
echo "socket configuration:\n[fconfigure $listen_socket]"; 
#flush stdout

# wait for telnet session to become active
vwait client_online
echo "Start remote terminal and issue commands, ESC terminates the link";