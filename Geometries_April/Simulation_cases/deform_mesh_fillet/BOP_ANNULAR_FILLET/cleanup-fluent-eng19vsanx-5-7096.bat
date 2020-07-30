echo off
set LOCALHOST=%COMPUTERNAME%
set KILL_CMD="C:\PROGRA~1\ANSYSI~1\v191\fluent/ntbin/win64/winkill.exe"

"C:\PROGRA~1\ANSYSI~1\v191\fluent\ntbin\win64\tell.exe" eng19vsanx-5 58686 CLEANUP_EXITING
if /i "%LOCALHOST%"=="eng19vsanx-5" (%KILL_CMD% 288) 
if /i "%LOCALHOST%"=="eng19vsanx-5" (%KILL_CMD% 7096) 
if /i "%LOCALHOST%"=="eng19vsanx-5" (%KILL_CMD% 11104)
del "C:\Users\dbarre5\Downloads\BOP_ANNULAR_FILLET-20190327T223638Z-001\BOP_ANNULAR_FILLET\cleanup-fluent-eng19vsanx-5-7096.bat"
