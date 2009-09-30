; Based on several NSIS examples, including http://nsis.sourceforge.net/Run_an_application_shortcut_after_an_install
!include "MUI2.nsh"

; Some defines
!define PRODUCT_NAME "MP3 Diags"
!define PRODUCT_DIR "MP3Diags"
!define PRODUCT_UNINSTALL "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_DIR}"

; The name of the installer
Name "${PRODUCT_NAME}"

; The file to write
OutFile "MP3DiagsSetup.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\${PRODUCT_DIR}"
;InstallDirRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MP3Diags" "UninstallString"
InstallDirRegKey HKLM "${PRODUCT_UNINSTALL}" "UninstallString"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

SetCompressor /SOLID lzma

 Var StartMenuFolder

!define MUI_ABORTWARNING

;--------------------------------
; Pages

;Page license
;Page directory
;Page components
;Page instfiles

;--------------------------------
; Display License
;LicenseData "gplv2.txt"


  !insertmacro MUI_PAGE_LICENSE "gplv2.txt"
;  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY

  ;Start Menu Folder Page Configuration
;  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
;  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Modern UI Test"
;  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

  !insertmacro MUI_PAGE_INSTFILES

    !define MUI_FINISHPAGE_NOAUTOCLOSE
    !define MUI_FINISHPAGE_RUN
    !define MUI_FINISHPAGE_RUN_CHECKED
    !define MUI_FINISHPAGE_RUN_TEXT "Run ${PRODUCT_NAME}"
    !define MUI_FINISHPAGE_RUN_FUNCTION "LaunchLink"
  !insertmacro MUI_PAGE_FINISH



  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES






!insertmacro MUI_LANGUAGE "English"

Section "Main Application" !Required ;No components page, name is not important
  SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Put file there
  File Mp3DiagsWindows.exe
  File favicon.ico

  File boost.txt
  File boost_serialization-mgw34-mt-1_39.dll
  File changelog.txt
  File gplv2.txt
  File gplv3.txt
  File lgpl-2.1.txt
  File lgplv3.txt
  File mingwm10.dll
  File QtCore4.dll
  File QtGui4.dll
  File QtNetwork4.dll
  File QtSvg4.dll
  File QtXml4.dll
  File zlib.txt
  File zlib1.dll

  SetOutPath $INSTDIR\iconengines
  File iconengines\qsvgicon4.dll
  SetOutPath $INSTDIR\imageformats
  File imageformats\qsvg4.dll
  File imageformats\qjpeg4.dll
  File imageformats\qgif4.dll


  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\MP3 Diags.lnk" "$INSTDIR\Mp3DiagsWindows.exe" \
      "" "$INSTDIR\favicon.ico" 0 SW_SHOWNORMAL
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END


  ; Tell the compiler to write an uninstaller and to look for a "Uninstall" section
  WriteUninstaller $INSTDIR\Uninstall.exe

  WriteRegStr HKLM "${PRODUCT_UNINSTALL}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr HKLM "${PRODUCT_UNINSTALL}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
  ;WriteRegStr HKLM "${PRODUCT_UNINSTALL}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"

SectionEnd ; end the section


; The uninstall section
Section "un.Uninstall"

  Delete $INSTDIR\Uninstall.exe
  Delete $INSTDIR\Mp3DiagsWindows.exe
  Delete $INSTDIR\favicon.ico

  Delete $INSTDIR\boost.txt
  Delete $INSTDIR\boost_serialization-mgw34-mt-1_39.dll
  Delete $INSTDIR\changelog.txt
  Delete $INSTDIR\gplv2.txt
  Delete $INSTDIR\gplv3.txt
  Delete $INSTDIR\lgpl-2.1.txt
  Delete $INSTDIR\lgplv3.txt
  Delete $INSTDIR\mingwm10.dll
  Delete $INSTDIR\QtCore4.dll
  Delete $INSTDIR\QtGui4.dll
  Delete $INSTDIR\QtNetwork4.dll
  Delete $INSTDIR\QtSvg4.dll
  Delete $INSTDIR\QtXml4.dll
  Delete $INSTDIR\zlib.txt
  Delete $INSTDIR\zlib1.dll

  Delete $INSTDIR\iconengines\qsvgicon4.dll
  Delete $INSTDIR\imageformats\qsvg4.dll
  Delete $INSTDIR\imageformats\qjpeg4.dll
  Delete $INSTDIR\imageformats\qgif4.dll

  RMDir $INSTDIR\iconengines
  RMDir $INSTDIR\imageformats
  RMDir $INSTDIR

  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

  Delete "$SMPROGRAMS\$StartMenuFolder\MP3 Diags.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  DeleteRegKey HKLM "${PRODUCT_UNINSTALL}"

SectionEnd

Function LaunchLink
;  ExecShell "" "$SMPROGRAMS\$StartMenuFolder\MP3 Diags.lnk"
  ExecShell "" "$INSTDIR\Mp3DiagsWindows.exe"
FunctionEnd
