# Microsoft Developer Studio Project File - Name="stormgate" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=stormgate - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "stormgate.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "stormgate.mak" CFG="stormgate - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "stormgate - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "stormgate - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "stormgate - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../obj/Stormgate/Release"
# PROP Intermediate_Dir "../obj/Stormgate/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "RUN_AS_WIN32SERVICE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "stormgate - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../obj/Stormgate/Debug"
# PROP Intermediate_Dir "../obj/Stormgate/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "RUN_AS_WIN32SERVICE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib WSOCK32.LIB /nologo /subsystem:console /debug /machine:I386 /out:"../area/stormgate.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "stormgate - Win32 Release"
# Name "stormgate - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\act_clan.c
# End Source File
# Begin Source File

SOURCE=.\act_comm.c
# End Source File
# Begin Source File

SOURCE=.\act_info.c
# End Source File
# Begin Source File

SOURCE=.\act_info2.c
# End Source File
# Begin Source File

SOURCE=.\act_move.c
# End Source File
# Begin Source File

SOURCE=.\act_multi.c
# End Source File
# Begin Source File

SOURCE=.\act_obj.c
# End Source File
# Begin Source File

SOURCE=.\act_obj2.c
# End Source File
# Begin Source File

SOURCE=.\act_race.c
# End Source File
# Begin Source File

SOURCE=.\act_wiz.c
# End Source File
# Begin Source File

SOURCE=.\act_wiz2.c
# End Source File
# Begin Source File

SOURCE=.\act_wiz3.c
# End Source File
# Begin Source File

SOURCE=.\arena.c
# End Source File
# Begin Source File

SOURCE=.\bit.c
# End Source File
# Begin Source File

SOURCE=.\cls_asn.c
# End Source File
# Begin Source File

SOURCE=.\cls_bar.c
# End Source File
# Begin Source File

SOURCE=.\cls_mag.c
# End Source File
# Begin Source File

SOURCE=.\cls_nec.c
# End Source File
# Begin Source File

SOURCE=.\cls_none.c
# End Source File
# Begin Source File

SOURCE=.\cls_pal.c
# End Source File
# Begin Source File

SOURCE=.\cls_psi.c
# End Source File
# Begin Source File

SOURCE=.\cls_rng.c
# End Source File
# Begin Source File

SOURCE=.\cls_shm.c
# End Source File
# Begin Source File

SOURCE=.\cls_thf.c
# End Source File
# Begin Source File

SOURCE=.\cls_vam.c
# End Source File
# Begin Source File

SOURCE=.\comm.c
# End Source File
# Begin Source File

SOURCE=.\const.c
# End Source File
# Begin Source File

SOURCE=.\crafting.c
# End Source File
# Begin Source File

SOURCE=.\crypt.c
# End Source File
# Begin Source File

SOURCE=.\db.c
# End Source File
# Begin Source File

SOURCE=.\drunk.c
# End Source File
# Begin Source File

SOURCE=.\economy.c
# End Source File
# Begin Source File

SOURCE=.\fight.c
# End Source File
# Begin Source File

SOURCE=.\fight2.c
# End Source File
# Begin Source File

SOURCE=.\games.c
# End Source File
# Begin Source File

SOURCE=.\garble.c
# End Source File
# Begin Source File

SOURCE=.\gr_magic.c
# End Source File
# Begin Source File

SOURCE=.\handler.c
# End Source File
# Begin Source File

SOURCE=.\healer.c
# End Source File
# Begin Source File

SOURCE=.\hunt.c
# End Source File
# Begin Source File

SOURCE=.\id.c
# End Source File
# Begin Source File

SOURCE=.\interp.c
# End Source File
# Begin Source File

SOURCE=.\language.c
# End Source File
# Begin Source File

SOURCE=.\magic.c
# End Source File
# Begin Source File

SOURCE=.\magic2.c
# End Source File
# Begin Source File

SOURCE=.\magic3.c
# End Source File
# Begin Source File

SOURCE=.\magic4.c
# End Source File
# Begin Source File

SOURCE=.\marriage.c
# End Source File
# Begin Source File

SOURCE=.\memory.c
# End Source File
# Begin Source File

SOURCE=.\mmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\mob_commands.c
# End Source File
# Begin Source File

SOURCE=.\mob_prog.c
# End Source File
# Begin Source File

SOURCE=.\olc.c
# End Source File
# Begin Source File

SOURCE=.\olc_act.c
# End Source File
# Begin Source File

SOURCE=.\olc_save.c
# End Source File
# Begin Source File

SOURCE=.\ore_prog.c
# End Source File
# Begin Source File

SOURCE=.\quest.c
# End Source File
# Begin Source File

SOURCE=.\rel_quest.c
# End Source File
# Begin Source File

SOURCE=.\religion.c
# End Source File
# Begin Source File

SOURCE=.\save.c
# End Source File
# Begin Source File

SOURCE=.\scan.c
# End Source File
# Begin Source File

SOURCE=.\skill_table.c
# End Source File
# Begin Source File

SOURCE=.\skills.c
# End Source File
# Begin Source File

SOURCE=".\social-edit.c"
# End Source File
# Begin Source File

SOURCE=.\special.c
# End Source File
# Begin Source File

SOURCE=.\sqldb.c
# End Source File
# Begin Source File

SOURCE=.\ssm.c
# End Source File
# Begin Source File

SOURCE=.\string.c
# End Source File
# Begin Source File

SOURCE=.\track.c
# End Source File
# Begin Source File

SOURCE=.\update.c
# End Source File
# Begin Source File

SOURCE=.\weapons.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\colors.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\crypt.h
# End Source File
# Begin Source File

SOURCE=.\merc.h
# End Source File
# Begin Source File

SOURCE=.\mmgr.h
# End Source File
# Begin Source File

SOURCE=.\nommgr.h
# End Source File
# Begin Source File

SOURCE=.\olc.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
