# Microsoft Developer Studio Project File - Name="pcp_controls" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=pcp_controls - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pcp_controls.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pcp_controls.mak" CFG="pcp_controls - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pcp_controls - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pcp_controls - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pcp_controls - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PCP_CONTROLS_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Ox /Ot /Og /Oi /I "../pcp_generic" /I "../pcp_paint" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PCP_CONTROLS_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../pcp_generic/release/pcp_generic.lib /nologo /dll /machine:I386 /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "pcp_controls - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PCP_CONTROLS_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../pcp_generic" /I "../pcp_paint" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PCP_CONTROLS_EXPORTS" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../pcp_generic/debug/pcp_generic.lib ../SlackEdit/qdcm.lib /nologo /dll /profile /debug /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Updating Version Info
PostBuild_Cmds=tclsh83 ../SlackEdit/scripts/m_auto_update.tcl
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "pcp_controls - Win32 Release"
# Name "pcp_controls - Win32 Debug"
# Begin Group "pcp_menu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_menu.c
# End Source File
# Begin Source File

SOURCE=.\pcp_menu.h
# End Source File
# End Group
# Begin Group "pcp_urlctrl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_urlctrl.c
# End Source File
# Begin Source File

SOURCE=.\pcp_urlctrl.h
# End Source File
# End Group
# Begin Group "pcp_tip_of_da_day"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_tip_of_da_day.c
# End Source File
# Begin Source File

SOURCE=.\pcp_tip_of_da_day.h
# End Source File
# End Group
# Begin Group "pcp_controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_controls.c
# End Source File
# Begin Source File

SOURCE=.\pcp_controls.h
# End Source File
# Begin Source File

SOURCE=.\version_info.h
# End Source File
# End Group
# Begin Group "resources"

# PROP Default_Filter ""
# Begin Group "bitmaps"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\resources\203.bmp
# End Source File
# Begin Source File

SOURCE=.\resources\402.bmp
# End Source File
# Begin Source File

SOURCE=.\resources\litebulb.bmp
# End Source File
# Begin Source File

SOURCE=.\resources\mail.bmp
# End Source File
# End Group
# Begin Group "icons"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\resources\link.cur
# End Source File
# Begin Source File

SOURCE=.\resources\url.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\pcp_controls.rc
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "pcp_about_fade"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_about_fade.c
# End Source File
# Begin Source File

SOURCE=.\pcp_about_fade.h
# End Source File
# End Group
# Begin Group "pcp_statusbar"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_statusbar.c
# End Source File
# Begin Source File

SOURCE=.\pcp_statusbar.h
# End Source File
# End Group
# Begin Group "pcp_basebar"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_basebar.c
# End Source File
# Begin Source File

SOURCE=.\pcp_basebar.h
# End Source File
# End Group
# Begin Group "info"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\info\history.txt
# End Source File
# Begin Source File

SOURCE=.\info\todo.txt
# End Source File
# End Group
# End Target
# End Project
