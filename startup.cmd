@echo off
rem
rem MurkMUD++ - A Windows compatible, C++ compatible Merc 2.2 Mud.
rem
rem author Jon A. Lambert
rem date 01/02/2007
rem version 1.5
rem remarks
rem  This source code copyright (C) 2005, 2006, 2007 by Jon A. Lambert
rem  All rights reserved.
rem
rem  Use governed by the MurkMUD++ public license found in license.murk++

setlocal enableextensions

rem Set the port number.
set port=4000
if not "%1"=="" set port=%1
if exist shutdown.txt erase shutdown.txt
if exist hotboot.$$$ erase hotboot.$$$

:loop
    set /a index=1000
    :loop2
        set logfile=%index%.log
	if not exist %logfile% goto start
	set /a index = %index% + 1
    goto loop2

    :start
    rem Record starting time
    >  %logfile% echo %DATE% %TIME%

    rem Run mud.
    echo Starting MURK on port %port% using log %logfile%
    >> %logfile% murk.exe %port% 2>&1 

    if not exist hotboot.$$$ goto chkshut
    erase hotboot.$$$
    goto done
    
    :chkshut
    if not exist shutdown.txt goto sleep
    erase shutdown.txt
    goto done

    :sleep
    ping 1.1.1.1 -n 15 -w 1000 >NUL
goto loop

:done
endlocal
