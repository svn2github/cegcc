rshd - Remote Shell Daemon for Windows NT version 1.6

Written by Silviu C. Marghescu (http://www.cs.umd.edu/~silviu)
Copyright (C) 1996  Silviu C. Marghescu, Emaginet, Inc.
All Rights Reserved.

Password functionality added by Ashley M. Hopkins (http://www.csee.usf.edu/~amhopki2)

rshd is free software; you can redistribute it in its entirety
in any form you like.  If you find any bugs, feel free to send me an
email at silviu@emaginet.com.  If you have added new features to rshd,
please send me all the source code modifications, including the version
of rshd that you are based on.  Your additions may benefit other users.


Disclaimer
==========

rshd is distributed hoping that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.

Good data processing procedure dictates that any program be
thoroughly tested with non-critical data before relying on it.
The user must assume the entire risk of using the program.
THE AUTHOR SHALL NOT BE HELD LIABLE FOR ANY KIND OF DAMAGES OR CLAIMS THAT
DIRECTLY OR INDIRECTLY RESULT FROM USING THIS SOFTWARE.


Description
===========

rshd is a multithreaded daemon service that listens for connections on port
514 (tcp port for the shell/cmd protocol), runs commands passed by clients and sends
back the results.  It was my experience that the rshd service included in the
Windows NT Resource Kit does not fully follow the BSD specification for the rsh protocol;
it works fine with the rsh client in NT, but other clients fail to connect.
This implementation of rshd tries to get as close as possible to the BSD specs
(http://www.bsdi.com).

As of version 1.5, rshd comes with RCP server support, thanks to Gary Doss
(gdoss@rpspo2.atlantaga.ncr.com); any problem/question concerning the rcp
part of rshd would be better answered by him.

Important note: rshd was designed and implemented to be convenient and reliable,
rather than tightly secure.  A client trying to connect to rshd will have to pass
a security clearance process, but rshd is probably far from a secure service.
If security is of major concern across your network, you should be very careful
when using this service.  My target for rshd was a closed network, or a network
guarded by a firewall.


Requirements
============

o An Intel processor based machine running Microsoft Windows NT and TCP/IP.
o Window Socket DLL installed properly (version 1.1 or higher).


Installation
============

This package contains the following files:
	readme.txt - this file
	rshd.exe - the rshd daemon executable
The source distribution also contains:
	rshd.cpp - the C++ source file (actually mostly C, but I prefer to define
		variables when I really need them; plus, I like the // comments)
	rshd_rcp.cpp - the C++ source file for the RCP service
	service.cpp, service.h - the service routines (many thanks to Craig Link, 
		Microsoft for including the Service project in the samples)
	rshd.ide - the project file for Borland C++ 5.0 users
	rshd.mak - the project file for Microsoft Visual C++ 2.0


Running rshd
============

In this new version, rshd runs as a service.  You also have the option of running rshd
as a command line application (see "-d" later on).  In order to get the service up and 
running, you will have to complete the following steps:
1. install the service:
	rhsd -install
2. start the service:
	net start rshd
That's it!  Stopping the service is as easy as starting it:
	net stop rshd
Should you decide rshd is not the way to go:
	rshd -remove
Starting/stopping the service can also be done through the "Services" Control Panel
applet; just look for the RSHD Daemon service.

Note that if the applications you are running require user credentials, you should 
run the service under the corresponding user account.

Command line options:
-install installs the rshd service
-remove  removes the rshd service

The following command line options have been inherited from the previous, interactive
versions.  I don't know if they'll be useful if you decide to run rshd as a service.
I haven't figured out yet how to run the service with command line options, therefore
the '-r' is set by default.

-d	enables debugging messages and allows you to run rshd as a command line process. Good 
	for those days when nothing works...
-1	no stdout redirection. By default, rshd will redirect the output of your
	command into a temporary file and send the result back thru the client
	socket.  If however you are not interested in the output, or the command
	is already redirected, this option will prevent stdout redirection.
	Note that the option is global, meaning it will disable redirection
	regardless of the commands you're passing...
-2	no stderr redirection.  Same as '-1', but for stderr.  At this point it
	should be noted that under the BSD rshd specification, the client can pass
	an auxillary tcp port number that the daemon can use to send the stderr
	output back.  The rshd will connect to that port if provided and send
	back the stderr, unless this option is given.  If no alternative stderr port
	is provided, rshd will use the main socket for both stdout and stderr.
-4  4DOS command shell.  Different shells and different operating systems have
    different ways of redirecting output, especially for the standard error stream.
    rshd was tested in the following configurations: CMD.EXE and 4NT.EXE on
    Windows NT; COMMAND.COM and 4DOS.COM on Windows 95.  If you're running 4DOS
    on Windows 95, make sure you set the '-4' command parameter, otherwise the
    stderr redirection will fail.
-s	stronger security enabled.  By default, when the client credentials can't
	be checked, rshd assumes it to be friendly and runs the command.  If that
	creates security concernes, this option will accept the connection to a client
	only if everything checks out.
-r	no .rhosts checking.  Per BSD rshd specification, rshd loads the
	<windir>\.rhosts file and builds a list of trusted hosts.  
	Any further connections will be accepted only from a host in the
	list.  '-r' disables this checking.  Note that this is a major security
	issue: if your network is not closed or guarded by a firewall, anybody
	can connect thru the rsh protocol and run commands on your machines.
	Use this option only if you know exactly who is running what across your
	network!
-p	password option enabled.  User will be prompted to enter a password after start of 
	the daemon.  To enable rsh commands to execute on the daemon with this enabled user 
	must enter password from the command line in the rsh command between the hostname 	and the command. (rsh hostname password command) 
	The password option does not affect the rcp command.
-v	displays the rshd version.
-h  help screen.

RCP usage:
    Valid rcp requests are in the form:
         rcp -t [-d] [-r] [-p] target 
         rcp -f [r] [-p] target
    NOTE:  The -p option is being ignored since there is not a good
           correlation between UNIX and NT when it comes to file
           permissions and ownership.

I have tested rshd with the following rsh clients: WinRSH (both 16 and 32 bit
versions; this was the client I needed to use and didn't work with the Resource
Kit implementation of rshd); NT client of rsh; SunOS client of rsh.  The main
difference between WinRSH and the other rsh clients is that WinRSH does not open
a stderr additional port; rshd will send both the stdout and stderr output thru
the main socket.


Security considerations
=======================

As stated above, security was not the main issue while implementing rshd.  The
daemon still tries to authenticate the remote user/host, but the authentication
process is not fully implemented and should not be considered secure.
In this version, only host authentication is implemented. If not disabled (see
the '-r' switch), an .rhosts mechanism is used: the remote host name is searched
in the .rhosts file; if it is not found, the connection is refused.
Sometimes, rshd does not have enough information to decide whether the connection
is secure or not (e.g. cannot determine the remote host name); in this cases, by
default, the connection is accepted, unless the '-s' switch is on.  The '-s' switch
will enable a tighter security: only fully recognized clients will be able to connect.

The password functionality added by Ashley M. Hopkins enables another layer of security by requiring that the user enter a password.  This option can be used in conjunction with the .rhosts file or with the .rhosts checking disabled (-r).

To allow compatibility across NT/95 platforms, the required path for the .rhosts file
(if you decide to use the feature) is: <windir>\.rhosts, where <windir> is your
Windows root directory as reported by the WINDIR environment variable.


Rebuilding rshd
===============

You probably have the sources already...  I've built rshd with both Visual C++
and Borland C++; the .ide file is the Borland project and the .mak is the one
you need for Visual C++.  Make sure you define the appropriate macro (first lines
in rshd.cpp define either VISUALCPP or BORLANDCPP).


Known problems
==============

Some rsh clients open an additional connection for the stderr output.  There is a 
known problem/feature in Microsoft's implementation of TCP/IP that causes closed
connections to linger on for 2 maximum segment lives (4 minutes).  Within the timeout 
period, the local port is unusable.  For this reason, rshd has a mechanism for port 
resolution that tries to assign local ports in a round-robin fashion.  
It is not a clean solution, but it works for the time being (there is still a problem
if rshd is restarted, since it begins assigning ports from 1023; if those ports are
taken by TIME_WAIT connections, they'll be unusable).  A way of reducing the timeout
period to less than 4 minutes is described in Microsoft's Knowledge Base article Q149532:

*****************************************************************************************
A new registry has been added to TCP/IP to allow the TIME-WAIT state to be configurable. 
This will allow TCP/IP users to free closed connection resources more quickly. 

The new registry entry is:

   TcpTimedWaitDelay
      Key: Tcpip\Parameters
      Value Type: REG_DWORD - Time in seconds
      Valid Range: 30-300 (decimal)
      Default: 0xF0 (240 decimal)
*****************************************************************************************


Also, very long command lines (e.g. more than 1024 characters) can cause rshd to 
crash.  Still working on it.

For complex commands (e.g. "comm1 | comm2"), to achieve correct redirection, the whole
command needs to be enclosed in parenthesis (e.g. (comm1 | comm2) ).  However, that creates
problems on some machines (errors have been reported on Windows 95, running under command.com).
Things go smoothly though if you have 4NT or 4DOS (use the '-4' flag then).  



Whether you have good or bad comments, improvements and suggestions, I would like
to hear from you at silviu@emaginet.com.


Bug fixes, improvements
=======================

Gary Doss (gdoss@rpspo2.atlantaga.ncr.com) has had a major contribution to rshd, 
with the RCP support and some pretty good bug fixes.

Barbara Dawkins (bdawkins@rpspo3.atlantaga.ncr.com) has improved Gary's RCP support
and fixed a couple of bugs.


Ashley Hopkins (amhopki2@csee.usf.edu) has added the password option to the rshd to improve security of this daemon.  Additionally a bug has been fixed to allow the transfer of executable files.