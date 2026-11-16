candle cocoro-office.wxs CustomUI_InstallDir.wxs CustomLicenseAgreementDlg.wxs CustomInstallDirDlg.wxs Portal_files.wxs NAS_Tool_files.wxs PC_Agent_files.wxs TSVerification_files.wxs -ext WixUIExtension -dPortalDir=.\InstallModules\cocoro-office-win32-ia32 -dNAS_ToolDir=.\InstallModules\NAS_Tool -dPC_AgentDir=.\InstallModules\PC_Agent -dTSVerificationDir=.\InstallModules\TSVerification
ECHO ERRORLEVEL=%ERRORLEVEL%
if not %ERRORLEVEL% equ 0 goto end
light cocoro-office.wixobj CustomUI_InstallDir.wixobj CustomLicenseAgreementDlg.wixobj CustomInstallDirDlg.wixobj Portal_files.wixobj NAS_Tool_files.wixobj PC_Agent_files.wixobj TSVerification_files.wixobj -out cocoro-office.msi -ext WixUIExtension -ext WixNetFxExtension -loc WixUI_Alt_ja-jp.wxl -cultures:ja-jp
:end