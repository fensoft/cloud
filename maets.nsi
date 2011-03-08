!define SIZE "5000"
!define APPNAME "Cloud"

Name "${APPNAME}"
OutFile "cloud-install.exe"
InstallDir c:\Cloud
RequestExecutionLevel admin

!include WinMessages.nsh
!include FileFunc.nsh
!insertmacro GetDrives
!insertmacro DriveSpace
Function CustomCreate
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Settings' 'NumFields' '6'
 
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 1' 'Type' 'Label'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 1' 'Left' '5'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 1' 'Top' '5'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 1' 'Right' '-6'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 1' 'Bottom' '17'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 1' 'Text' \
         'Select Installation drive:'
 
         StrCpy $R2 0
         StrCpy $R0 ''
         ${GetDrives} "HDD" GetDrivesCallBack
 
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'Type' 'DropList'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'Left' '30'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'Top' '26'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'Right' '-31'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'Bottom' '100'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'Flags' 'Notify'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'State' '$R1'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'ListItems' '$R0'
 
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 3' 'Type' 'Label'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 3' 'Left' '5'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 3' 'Top' '109'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 3' 'Right' '59'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 3' 'Bottom' '119'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 3' 'Text' \
         'Space required:'
 
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 4' 'Type' 'Label'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 4' 'Left' '60'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 4' 'Top' '109'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 4' 'Right' '-5'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 4' 'Bottom' '119'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 4' 'Text' \
         '${SIZE} Mb'
 
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 5' 'Type' 'Label'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 5' 'Left' '5'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 5' 'Top' '120'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 5' 'Right' '59'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 5' 'Bottom' '130'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 5' 'Text' \
         'Space available:'
 
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 6' 'Type' 'Label'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 6' 'Left' '60'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 6' 'Top' '120'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 6' 'Right' '-5'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 6' 'Bottom' '130'
         WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 6' 'Text' \
         '$R3 Mb'
 
         push $0
         InstallOptions::Dialog '$PLUGINSDIR\custom.ini'
         pop $0
         pop $0
FunctionEnd
 
Function CustomLeave
        ReadIniStr $0 '$PLUGINSDIR\custom.ini' 'Settings' 'State'
        StrCmp $0 '2' 0 next
        ReadIniStr $0 '$PLUGINSDIR\custom.ini' 'Field 2' 'State'
        StrCpy $0 $0 3
        ${DriveSpace} "$0" "/D=F /S=M" $R3
        WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 6' 'Text' \
        '$R3 Mb'
        ReadIniStr $0 '$PLUGINSDIR\custom.ini' 'Field 6' 'HWND'
        SendMessage $0 ${WM_SETTEXT} 0 'STR:$R3 Mb'
        Abort
 
     next:
        ReadIniStr $0 '$PLUGINSDIR\custom.ini' 'Field 2' 'State'
        StrCpy '$INSTDIR' '$0'
FunctionEnd
 
Function GetDrivesCallBack
         ${DriveSpace} "$9" "/D=F /S=M" $R4
         IntCmp $R4 '${SIZE}' end end def
      def:
         StrCmp $R2 '0' 0 next
         StrCpy $R3 '$R4'
         StrCpy $R1 '$9${APPNAME}'
         IntOp $R2 $R2 + 1
      next:
         StrCpy $R0 '$R0$9${APPNAME}|'
      end:
	 Push $0
FunctionEnd
 
Function .onInit
         InitPluginsDir
         GetTempFileName $0
         Rename $0 '$PLUGINSDIR\custom.ini'
FunctionEnd

;--------------------------------

; Pages

;Page directory
Page Custom CustomCreate CustomLeave
Page instfiles

;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  File Cloud.exe
  File P:\Dev\QtSDK\mingw\bin\libgcc_s_dw2-1.dll
  File P:\Dev\QtSDK\mingw\bin\mingwm10.dll
  File P:\Dev\QtSDK\Desktop\Qt\4.7.2\mingw\lib\QtCore4.dll
  File P:\Dev\QtSDK\Desktop\Qt\4.7.2\mingw\lib\QtGui4.dll
  File P:\Dev\QtSDK\Desktop\Qt\4.7.2\mingw\lib\QtNetwork4.dll
  File zlib1.dll
  File config.ini
  SetOutPath $INSTDIR\tools
  File tools\*
  SetOutPath $INSTDIR\www
  File www\index.html
  SetOutPath $INSTDIR\utorrent
  File utorrent\utorrent.exe
  File utorrent\settings.dat.orig
  File utorrent\resume.dat.orig
  File utorrent\utorrent.lng
  File utorrent\webui.zip
  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\Cloud.lnk" "$INSTDIR\Cloud.exe"
  SetRegView 64
  WriteRegStr HKCU "Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers" "$INSTDIR\Steam-LAN.exe" "RUNASADMIN"
  Delete $INSTDIR\utorrent_configured
  
SectionEnd ; end the section
