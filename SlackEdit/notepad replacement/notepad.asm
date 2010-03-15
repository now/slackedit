.386
.model flat,stdcall
option casemap :none
include \PROGRAMS\CODING~1\compil~1\masm32\include\windows.inc
include \PROGRAMS\CODING~1\compil~1\masm32\include\user32.inc
include \PROGRAMS\CODING~1\compil~1\masm32\include\kernel32.inc
include \PROGRAMS\CODING~1\compil~1\masm32\include\advapi32.inc
include \PROGRAMS\CODING~1\compil~1\masm32\include\shell32.inc
includelib \PROGRAMS\CODING~1\compil~1\masm32\lib\user32.lib
includelib \PROGRAMS\CODING~1\compil~1\masm32\lib\kernel32.lib
includelib \PROGRAMS\CODING~1\compil~1\masm32\lib\advapi32.lib
includelib \PROGRAMS\CODING~1\compil~1\masm32\lib\shell32.lib

.data
  szRootKey      db "Software\da.box Software Division\SlackEdit\Global Settings",0h
  szSubKey       db "Install Path",0h
  szMsg          db "Failed to run SlackEdit.",0Dh,"Check your Install Path in SlackEdit's Associations Conphiguration Tab.",0h
  szTitle        db "Notepad.exe: ShellExecute Failed!",0h
  szOperation    db "open",0h
  szPhileName    db "SlackEdit.exe",0h
  szCmdLine      db MAX_PATH dup(0)
  pszCmdLine     dd 0
  szInstallPath  db MAX_PATH dup(0),0h
  cchMax         dd MAX_PATH
  hKey           dd 0

.code
start:
  invoke GetCommandLine
  mov pszCmdLine, eax
  
  ;Start copying command line
  mov esi, pszCmdLine  ; value returned by GetCommandLine() is address
  mov ecx, 128         ; max command line length
  lea edi, szCmdLine    ; load address of buffer
    
lpStart:
  lodsb                ; load byte into al
  cmp al, 34           ; [ " ] character
  je SubLoop
  cmp al, 0            ; if zero terminator exit loop
  je PastIt

  stosb                ; write byte to buffer
  jmp lpStart

SubLoop:
  lodsb                ; load byte into al
  cmp al, 34           ; [ " ] character
  je LastLoop          ; we've stripped off philename

  cmp al, 0            ; if zero terminator exit loop
  je PastIt

  jmp SubLoop

LastLoop:
  lodsb
  cmp al, 0
  je PastIt
  stosb
  jmp LastLoop

PastIt:
  invoke RegOpenKey, HKEY_CURRENT_USER, addr szRootKey, addr hKey
  invoke RegQueryValueEx, hKey, addr szSubKey, NULL, NULL, addr szInstallPath, addr cchMax
  invoke ShellExecute, NULL, addr szOperation, addr szPhileName, addr szCmdLine, addr szInstallPath, SW_SHOWNORMAL
  .if eax < 32
    invoke ShellExecute, NULL, addr szOperation, addr szPhileName, addr szCmdLine, NULL, SW_SHOWNORMAL
    .if eax < 32
      invoke MessageBox, NULL, addr szMsg, addr szTitle, MB_OK + MB_ICONEXCLAMATION
    .endif
  .endif

  invoke ExitProcess, 0
end start
