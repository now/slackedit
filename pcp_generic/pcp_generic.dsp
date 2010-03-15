# Microsoft Developer Studio Project File - Name="pcp_generic" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=pcp_generic - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pcp_generic.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pcp_generic.mak" CFG="pcp_generic - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pcp_generic - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pcp_generic - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pcp_generic - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PCP_GENERIC_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PCP_GENERIC_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /dll /machine:I386 /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "pcp_generic - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PCP_GENERIC_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PCP_GENERIC_EXPORTS" /YX /FD /GZ /c
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
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /profile /debug /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Updating Version Info
PostBuild_Cmds=tclsh83 ../SlackEdit/scripts/m_auto_update.tcl
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "pcp_generic - Win32 Release"
# Name "pcp_generic - Win32 Debug"
# Begin Group "pcp_combobox"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_combobox.c
# End Source File
# Begin Source File

SOURCE=.\pcp_combobox.h
# End Source File
# End Group
# Begin Group "pcp_dpa"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_dpa.c
# End Source File
# Begin Source File

SOURCE=.\pcp_dpa.h
# End Source File
# End Group
# Begin Group "pcp_editctrl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_editctrl.c
# End Source File
# Begin Source File

SOURCE=.\pcp_editctrl.h
# End Source File
# End Group
# Begin Group "pcp_listbox"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_listbox.c
# End Source File
# Begin Source File

SOURCE=.\pcp_listbox.h
# End Source File
# End Group
# Begin Group "pcp_mem"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_mem.c
# End Source File
# Begin Source File

SOURCE=.\pcp_mem.h
# End Source File
# End Group
# Begin Group "pcp_rect"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_rect.c
# End Source File
# Begin Source File

SOURCE=.\pcp_rect.h
# End Source File
# End Group
# Begin Group "pcp_registry"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_registry.c
# End Source File
# Begin Source File

SOURCE=.\pcp_registry.h
# End Source File
# End Group
# Begin Group "pcp_string"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_string.c
# End Source File
# Begin Source File

SOURCE=.\pcp_string.h
# End Source File
# End Group
# Begin Group "pcp_headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_definitions.h
# End Source File
# Begin Source File

SOURCE=.\pcp_includes.h
# End Source File
# End Group
# Begin Group "pcp_generic"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_generic.c
# End Source File
# Begin Source File

SOURCE=.\pcp_generic.h
# End Source File
# Begin Source File

SOURCE=.\pcp_generic.rc
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\version_info.h
# End Source File
# End Group
# Begin Group "pcp_window"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_window.c
# End Source File
# Begin Source File

SOURCE=.\pcp_window.h
# End Source File
# End Group
# Begin Group "pcp_spin"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_spin.c
# End Source File
# Begin Source File

SOURCE=.\pcp_spin.h
# End Source File
# End Group
# Begin Group "pcp_mru"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_mru.c
# End Source File
# Begin Source File

SOURCE=.\pcp_mru.h
# End Source File
# End Group
# Begin Group "pcp_point"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_point.c
# End Source File
# Begin Source File

SOURCE=.\pcp_point.h
# End Source File
# End Group
# Begin Group "pcp_debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_debug.c
# End Source File
# Begin Source File

SOURCE=.\pcp_debug.h
# End Source File
# End Group
# Begin Group "pcp_linkedlist"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_linkedlist.c
# End Source File
# Begin Source File

SOURCE=.\pcp_linkedlist.h
# End Source File
# End Group
# Begin Group "pcp_browse"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_browse.c
# End Source File
# Begin Source File

SOURCE=.\pcp_browse.h
# End Source File
# End Group
# Begin Group "pcp_systray"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_systray.c
# End Source File
# Begin Source File

SOURCE=.\pcp_systray.h
# End Source File
# End Group
# Begin Group "pcp_regexp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_regexp.c
# End Source File
# Begin Source File

SOURCE=.\pcp_regexp.h
# End Source File
# End Group
# Begin Group "pcp_clipboard"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_clipboard.c
# End Source File
# Begin Source File

SOURCE=.\pcp_clipboard.h
# End Source File
# End Group
# Begin Group "pcp_filenotify"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_filenotify.c
# End Source File
# Begin Source File

SOURCE=.\pcp_filenotify.h
# End Source File
# End Group
# Begin Group "pcp_path"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_path.c
# End Source File
# Begin Source File

SOURCE=.\pcp_path.h
# End Source File
# End Group
# Begin Group "pcp_toolbar"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_toolbar.c
# End Source File
# Begin Source File

SOURCE=.\pcp_toolbar.h
# End Source File
# End Group
# Begin Group "pcp_rebar"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_rebar.c
# End Source File
# Begin Source File

SOURCE=.\pcp_rebar.h
# End Source File
# End Group
# Begin Group "pcp_mdi"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_mdi.c
# End Source File
# Begin Source File

SOURCE=.\pcp_mdi.h
# End Source File
# End Group
# Begin Group "pcp_listview"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_listview.c
# End Source File
# Begin Source File

SOURCE=.\pcp_listview.h
# End Source File
# End Group
# Begin Group "pcp_math"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_math.c
# End Source File
# Begin Source File

SOURCE=.\pcp_math.h
# End Source File
# End Group
# Begin Group "pcp_hashtable"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_hashtable.c
# End Source File
# Begin Source File

SOURCE=.\pcp_hashtable.h
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
# Begin Group "pcp_timer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_timer.c
# End Source File
# Begin Source File

SOURCE=.\pcp_timer.h
# End Source File
# End Group
# End Target
# End Project
