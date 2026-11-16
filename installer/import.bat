Heat dir .\InstallModules\cocoro-office-win32-ia32 -dr Portal -cg PortalGroup -nologo -sw 5150 -gg -g1 -sfrag -srd -var "var.PortalDir" -out Portal_files.wxs
Heat dir .\InstallModules\NAS_Tool -dr NAS_Tool -cg NAS_ToolGroup -nologo -sw 5150 -gg -g1 -sfrag -srd -var "var.NAS_ToolDir" -out NAS_Tool_files.wxs
Heat dir .\InstallModules\PC_Agent -dr PC_Agent -cg PC_AgentGroup -nologo -sw 5150 -gg -g1 -sfrag -srd -var "var.PC_AgentDir" -out PC_Agent_files.wxs
rem Heat dir .\InstallModules\TSVerification -dr TSVerification -cg TSVerificationGroup -gg -g1 -sfrag -srd -var "var.TSVerificationDir" -out TSVerification_files.wxs
