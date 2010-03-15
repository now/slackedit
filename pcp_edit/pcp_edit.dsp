# Microsoft Developer Studio Project File - Name="pcp_edit" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=pcp_edit - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pcp_edit.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pcp_edit.mak" CFG="pcp_edit - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pcp_edit - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pcp_edit - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pcp_edit - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PCP_EDIT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../pcp_generic" /I "../pcp_paint" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PCP_EDIT_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib ../pcp_generic/release/pcp_generic.lib /nologo /dll /machine:I386 /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "pcp_edit - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PCP_EDIT_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../pcp_generic" /I "../pcp_paint" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PCP_EDIT_EXPORTS" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib ../pcp_generic/debug/pcp_generic.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Updating Version Info
PostBuild_Cmds=tclsh83 ../SlackEdit/scripts/m_auto_update.tcl
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "pcp_edit - Win32 Release"
# Name "pcp_edit - Win32 Debug"
# Begin Group "pcp_edit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_edit.c
# End Source File
# Begin Source File

SOURCE=.\pcp_edit.h
# End Source File
# End Group
# Begin Group "pcp_edit_common"

# PROP Default_Filter ""
# Begin Group "buffer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_textedit_buffer.c
# End Source File
# Begin Source File

SOURCE=.\pcp_textedit_buffer.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\pcp_edit_common.c
# End Source File
# Begin Source File

SOURCE=.\pcp_edit_common.h
# End Source File
# End Group
# Begin Group "pcp_textedit"

# PROP Default_Filter ""
# Begin Group "interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_textedit_interface.c
# End Source File
# Begin Source File

SOURCE=.\pcp_textedit_interface.h
# End Source File
# End Group
# Begin Group "move"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_textedit_move.c
# End Source File
# Begin Source File

SOURCE=.\pcp_textedit_move.h
# End Source File
# End Group
# Begin Group "view"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_textedit_view.c
# End Source File
# Begin Source File

SOURCE=.\pcp_textedit_view.h
# End Source File
# End Group
# Begin Group "syntax"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_textedit_syntax.c
# End Source File
# Begin Source File

SOURCE=.\pcp_textedit_syntax.h
# End Source File
# End Group
# Begin Group "internal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcp_textedit_internal.c
# End Source File
# Begin Source File

SOURCE=.\pcp_textedit_internal.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\pcp_textedit.c
# End Source File
# Begin Source File

SOURCE=.\pcp_textedit.h
# End Source File
# End Group
# Begin Group "pcp_hexedit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\controls\pcp_hexedit.c
# End Source File
# Begin Source File

SOURCE=.\src\controls\pcp_hexedit.h
# End Source File
# End Group
# Begin Group "pcp_edit_dll"

# PROP Default_Filter ""
# Begin Group "resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\resources\mg_cur.cur
# End Source File
# Begin Source File

SOURCE=.\resources\mg_icons2.bmp
# End Source File
# Begin Source File

SOURCE=.\pcp_edit.rc
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\pcp_edit_dll.c
# End Source File
# Begin Source File

SOURCE=.\pcp_edit_dll.h
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
# Begin Source File

SOURCE=.\version_info.h
# End Source File
# End Group
# End Target
# End Project
