# Microsoft Developer Studio Project File - Name="SlackEdit" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=SlackEdit - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SlackEdit.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SlackEdit.mak" CFG="SlackEdit - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SlackEdit - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "SlackEdit - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SlackEdit - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Output/Release"
# PROP Intermediate_Dir "Output/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Ox /Ot /Og /I "../SlackEdit" /I "../pcp_controls" /I "../pcp_edit" /I "../pcp_generic" /I "../pcp_paint" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_TCL_STUBS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib comctl32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib qdcm.lib ../pcp_controls/release/pcp_controls.lib ../pcp_generic/release/pcp_generic.lib ../pcp_edit/release/pcp_edit.lib tclstub83.lib msvcrt.lib libcmt.lib /nologo /subsystem:windows /profile /machine:I386 /nodefaultlib /opt:nowin98
# SUBTRACT LINK32 /verbose
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=tclsh83 ./scripts/m_copy_dlls.tcl release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "SlackEdit - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Output/Debug"
# PROP Intermediate_Dir "Output/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /Gm /GX /ZI /Od /I "../SlackEdit" /I "../pcp_controls" /I "../pcp_edit" /I "../pcp_generic" /I "../pcp_paint" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_TCL_STUBS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib ctl3d32.lib winspool.lib comdlg32.lib comctl32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib qdcm.lib msvcrtd.lib libcmtd.lib ../pcp_controls/debug/pcp_controls.lib ../pcp_generic/debug/pcp_generic.lib ../pcp_edit/debug/pcp_edit.lib tclstub83.lib /nologo /subsystem:windows /profile /debug /machine:I386 /nodefaultlib
# SUBTRACT LINK32 /verbose
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Updating Build Info and Copying DLLs
PostBuild_Cmds=tclsh83 ./scripts/m_auto_update.tcl	tclsh83 ./scripts/m_copy_dlls.tcl debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "SlackEdit - Win32 Release"
# Name "SlackEdit - Win32 Debug"
# Begin Group "main"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\controls\qdcm.h
# End Source File
# Begin Source File

SOURCE=.\src\slack_main.c
# End Source File
# Begin Source File

SOURCE=.\src\slack_main.h
# End Source File
# Begin Source File

SOURCE=.\slack_picdef.h
# End Source File
# End Group
# Begin Group "controls"

# PROP Default_Filter ""
# Begin Group "clipboard_monitor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\controls\clipboard_monitor.c
# End Source File
# Begin Source File

SOURCE=.\src\controls\clipboard_monitor.h
# End Source File
# End Group
# End Group
# Begin Group "dialogs"

# PROP Default_Filter ""
# Begin Group "about"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\dialogs\dlg_about.c
# End Source File
# Begin Source File

SOURCE=.\src\dialogs\dlg_about.h
# End Source File
# End Group
# Begin Group "advanced open"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\dialogs\dlg_advancedopen.c
# End Source File
# Begin Source File

SOURCE=.\src\dialogs\dlg_advancedopen.h
# End Source File
# End Group
# Begin Group "favorites"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\dialogs\dlg_favorites.c
# End Source File
# Begin Source File

SOURCE=.\src\dialogs\dlg_favorites.h
# End Source File
# End Group
# Begin Group "find"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\dialogs\dlg_find.c
# End Source File
# Begin Source File

SOURCE=.\src\dialogs\dlg_find.h
# End Source File
# End Group
# Begin Group "print"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\dialogs\dlg_print.c
# End Source File
# Begin Source File

SOURCE=.\src\dialogs\dlg_print.h
# End Source File
# End Group
# Begin Group "windowlist"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\dialogs\dlg_windowlist.c
# End Source File
# Begin Source File

SOURCE=.\src\dialogs\dlg_windowlist.h
# End Source File
# End Group
# End Group
# Begin Group "file_handling"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\file_handling\file_interface.c
# End Source File
# Begin Source File

SOURCE=.\src\file_handling\file_interface.h
# End Source File
# End Group
# Begin Group "mdi"

# PROP Default_Filter ""
# Begin Group "child_edit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\mdi\mdi_child_edit.c
# End Source File
# Begin Source File

SOURCE=.\src\mdi\mdi_child_edit.h
# End Source File
# End Group
# Begin Group "child_hex"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\mdi\mdi_child_hex.c
# End Source File
# Begin Source File

SOURCE=.\src\mdi\mdi_child_hex.h
# End Source File
# End Group
# Begin Group "child_generic"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\mdi\mdi_child_generic.c
# End Source File
# Begin Source File

SOURCE=.\src\mdi\mdi_child_generic.h
# End Source File
# End Group
# Begin Group "tcl_commandline"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\mdi\child_tclcommandline.c
# End Source File
# Begin Source File

SOURCE=.\src\mdi\child_tclcommandline.h
# End Source File
# End Group
# Begin Group "client"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\mdi\mdi_client.c
# End Source File
# Begin Source File

SOURCE=.\src\mdi\mdi_client.h
# End Source File
# End Group
# End Group
# Begin Group "parsing"

# PROP Default_Filter ""
# Begin Group "ecp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\parsing\parse_ecp.c
# End Source File
# Begin Source File

SOURCE=.\src\parsing\parse_ecp.h
# End Source File
# End Group
# Begin Group "projects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\parsing\parse_project.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\src\parsing\parse_project.h
# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Group
# Begin Group "rebar"

# PROP Default_Filter ""
# Begin Group "find_combo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\rebar\bar_findcombo.c
# End Source File
# Begin Source File

SOURCE=.\src\rebar\bar_findcombo.h
# End Source File
# End Group
# Begin Group "goto_line"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\rebar\bar_gotoline.c
# End Source File
# Begin Source File

SOURCE=.\src\rebar\bar_gotoline.h
# End Source File
# End Group
# Begin Group "rebar_main"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\rebar\bar_main.c
# End Source File
# Begin Source File

SOURCE=.\src\rebar\bar_main.h
# End Source File
# End Group
# Begin Group "toolbar"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\rebar\bar_toolbar.c
# End Source File
# Begin Source File

SOURCE=.\src\rebar\bar_toolbar.h
# End Source File
# End Group
# End Group
# Begin Group "settings"

# PROP Default_Filter ""
# Begin Group "font"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\settings\settings_font.c
# End Source File
# Begin Source File

SOURCE=.\src\settings\settings_font.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\settings\settings.c
# End Source File
# Begin Source File

SOURCE=.\src\settings\settings.h
# End Source File
# End Group
# Begin Group "subclasses"

# PROP Default_Filter ""
# Begin Group "pcp_edit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\subclasses\sub_pcp_edit.c
# End Source File
# Begin Source File

SOURCE=.\src\subclasses\sub_pcp_edit.h
# End Source File
# End Group
# Begin Group "pcp_hex"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\subclasses\sub_pcp_hex.c
# End Source File
# Begin Source File

SOURCE=.\src\subclasses\sub_pcp_hex.h
# End Source File
# End Group
# End Group
# Begin Group "tcl"

# PROP Default_Filter ""
# Begin Group "dialog"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\tcl\tcl_dialog.c
# End Source File
# Begin Source File

SOURCE=.\src\tcl\tcl_dialog.h
# End Source File
# End Group
# Begin Group "internal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\tcl\tcl_internal.c
# End Source File
# Begin Source File

SOURCE=.\src\tcl\tcl_internal.h
# End Source File
# End Group
# Begin Group "tcl_main"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\tcl\tcl_main.c
# End Source File
# Begin Source File

SOURCE=.\src\tcl\tcl_main.h
# End Source File
# End Group
# Begin Group "tcl_pcp_edit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\tcl\tcl_pcpedit.c
# End Source File
# Begin Source File

SOURCE=.\src\tcl\tcl_pcpedit.h
# End Source File
# End Group
# Begin Group "shell"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\tcl\tcl_shell.c
# End Source File
# Begin Source File

SOURCE=.\src\tcl\tcl_shell.h
# End Source File
# End Group
# Begin Group "window"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\tcl\tcl_window.c
# End Source File
# Begin Source File

SOURCE=.\src\tcl\tcl_window.h
# End Source File
# End Group
# Begin Group "hotkey"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\tcl\tcl_hotkey.c
# End Source File
# Begin Source File

SOURCE=.\src\tcl\tcl_hotkey.h
# End Source File
# End Group
# Begin Group "tcl_load"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\tcl\tcl_load.c
# End Source File
# Begin Source File

SOURCE=.\src\tcl\tcl_load.h
# End Source File
# End Group
# End Group
# Begin Group "windows"

# PROP Default_Filter ""
# Begin Group "clipbook"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\windows\wnd_clipbook.c
# End Source File
# Begin Source File

SOURCE=.\src\windows\wnd_clipbook.h
# End Source File
# End Group
# Begin Group "output"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\windows\wnd_output.c
# End Source File
# Begin Source File

SOURCE=.\src\windows\wnd_output.h
# End Source File
# End Group
# Begin Group "statusbar"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\windows\wnd_statusbar.c
# End Source File
# Begin Source File

SOURCE=.\src\windows\wnd_statusbar.h
# End Source File
# End Group
# Begin Group "windowbar"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\windows\wnd_windowbar.c
# End Source File
# Begin Source File

SOURCE=.\src\windows\wnd_windowbar.h
# End Source File
# End Group
# End Group
# Begin Group "resources"

# PROP Default_Filter ""
# Begin Group "icons"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\resources\cancel.ico
# End Source File
# Begin Source File

SOURCE=.\resources\clipbook.ico
# End Source File
# Begin Source File

SOURCE=.\resources\doc.ico
# End Source File
# Begin Source File

SOURCE=.\resources\main.ico
# End Source File
# End Group
# Begin Group "bitmaps"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\resources\menubmp8.bmp
# End Source File
# Begin Source File

SOURCE=.\resources\sled_title.bmp
# End Source File
# Begin Source File

SOURCE=.\resources\sysmenu.bmp
# End Source File
# End Group
# Begin Group "other"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\resources\slackedit.asc
# End Source File
# End Group
# Begin Source File

SOURCE=.\slack_resource.h
# End Source File
# Begin Source File

SOURCE=.\SlackEdit.rc
# End Source File
# End Group
# Begin Group "scripts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\scripts\m_auto_update.tcl
# End Source File
# Begin Source File

SOURCE=.\scripts\m_copy_dlls.tcl
# End Source File
# Begin Source File

SOURCE=.\scripts\m_utilitymacros.dsm
# End Source File
# End Group
# Begin Group "info"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\info\betatesters.txt
# End Source File
# Begin Source File

SOURCE=.\info\fixme.txt
# End Source File
# Begin Source File

SOURCE=.\info\history.txt
# End Source File
# Begin Source File

SOURCE=.\info\readme.txt
# End Source File
# Begin Source File

SOURCE=.\info\slackedit.tip
# End Source File
# Begin Source File

SOURCE=.\info\slacktcl.txt
# End Source File
# Begin Source File

SOURCE=.\info\todo.txt
# End Source File
# End Group
# End Target
# End Project
