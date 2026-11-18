# Task: Security Review of the Entire PC Portal and Installer

## Role and objective

Act as a senior security reviewer with expertise to Perform a comprehensive, evidence-based security vulnerability assessment of the entire PC Portal Electron application and its complete Windows bundled installer, including the WiX Burn Bootstrapper BA, its bootstrapper engine, bundled MSI packages, prerequisite packages, installation chain, upgrade and repair mechanisms, and uninstall behavior.

Review the PC Portal application and installer source for security weaknesses and produce an evidence-based Excel report.

## 1. Review Scope

- First, inspect the repository structure and identify all relevant application, installer, build, configuration, and dependency files. Review the actual implementation rather than assuming the project structure.

## Execution instructions
- Complete the review without asking questions or showing intermediate results.
- Do not include secrets in the report. If a secret is exposed, record only its file path and the fact of exposure.
- Do not modify, create, or delete any file in the repository. The only permitted write is the final workbook in the report folder named in the run instruction (outside the repository), plus the generator script in a temporary folder.

## A. PC Portal Electron Application

Review the following areas:
- Application vulnerabilities: XSS, command injection, path traversal, arbitrary file access, unsafe deserialization, insecure external links, and renderer-to-main privilege escalation.
- Local Express server: src/express_server.js, localhost endpoints, binding configuration, request validation, and access to privileged operations.
- Electron security: src/background.js, src/ipcMainHandler.js, BrowserWindow settings, preload scripts, IPC handlers
- Secrets and sensitive data: Private keys, hardcoded credentials, encryption keys, API tokens, Firebase configuration, certificates, logs, URLs, and electron-store.
- Network and backend APIs: HTTP/HTTPS, API response validation, timeouts, redirects, and sensitive-data exposure.
- Vue frontend: Vue 3 with @vue/compat, Vuex, Vue Router, Vuetify, BootstrapVue, Bootstrap, HTML rendering, and third-party libraries.
- Windows integration: child_process, PowerShell or shell commands, process manipulation, file-system access, temporary files, permissions, and native modules.
- Dependencies and build security: package.json, lockfiles, vulnerable dependencies, unsafe package versions, build scripts, update mechanisms, and application build process and 32-bit / 64-bit architecture review for security (Discover which builds actually exist. Do not assume both architectures ).
- Reliability-related security risks: Unsafe error handling, sensitive logging, resource exhaustion.
- Security Research: Refer to the official Electron security guidance, Electron and Chromium security releases, relevant CVEs

## B. WiX Burn Bootstrapper BA Bundled Installer
Review the complete bundled installer including MSI.
Discover the actual installer architecture and inspect relevant WiX source files (.wxs), Burn bundle definitions, Bootstrapper Application (BA) implementation, build scripts, and available installer artifacts.

## Review:
## Scope
- Review all files under installer/ except those listed under Exclusions.
- Review Burn bundle and bootstrapper application: pc-portal-installer-BA/, including pc-portal-installer-BA/cocoro-office_Setup.wxs and its .wixproj files.
- Review WiX source, configuration, project, theme, and localization files.
- Review MSI authoring files: cocoro-office.wxs, Custom*.wxs, other non-excluded .wxs files at the installer root, and pc-portal-installer.wixproj.
- Review repository-root file _build_and_make_msi.bat , _sign_build_and_make_msi.bat
- Do not log any signing-related issues from _build_and_make_msi.bat.

## Exclusions

Do not review, cite, or report findings on the following items.

1.  **Excluded folders, including all subfolders and files:**
    -   `installer/CA_source/`
    -   `installer/guard/`
    -   `installer/InstallModules/`
2.  **Unused files:**
    -   `installer/cocoro-office_Setup.wxs` (the installer-root copy
        only; `installer/pc-portal-installer-BA/cocoro-office_Setup.wxs`
        remains in scope)
    -   `installer/import.bat`
    -   `installer/log.bat`
    -   `installer/makemsi.bat`
    -   `installer/makesetup.bat`
    -   `installer/NAS_Tool_files.wxs`
    -   `installer/PC_Agent_files.wxs`
    -   `installer/Portal_files.wxs`
    -   `installer/TSVerification_files.wxs`

Do not read `CA_source/` to determine what those binaries do.

## Review process

Consider:  
- Malicious or substituted third-party/prebuilt binaries. 
- Installer command-line, property, registry, environment-variable, or filesystem inputs controlled by an unprivileged user. 
- Administrative mistakes that security checks should detect.

Use these scenarios to assess realistic exploitability and severity, not to limit the review. Report other relevant security weaknesses too.

### 1. Discovery

Inspect the actual files and establish: 
- **WiX versions:** WiX SDK and package versions for each project; identify bundle/MSI mismatches. 
- **Build and signing:** authoritative build paths, stale or divergent paths, signing targets and arguments, and artifacts that may not be signed.  
- **MSI and bundle files:** all MSI and bundle files, trace properties, registry searches, bundle variables and filesystem paths that influence commands or security decisions. Determine whether an unprivileged user can control them.  
- **Review gaps:** list missing source, binaries, configuration, or build context that limits conclusions.

### 2. External research

Use current, authoritative sources. Research relevant WiX/FireGiant documentation and security advisories, including: 
- Burn payload extraction, working/cache folders, payload integrity, and bundle signing. 
- Check all WiX CVE (Common Vulnerabilities and Exposures) whether the relevant feature is actually used before treating an advisory as applicable. 
- WiX, FireGiant's security releases. 
- All WiX issues and the pull request that implemented the related hardening.

Put full URLs for sources that support a finding in that finding's `CVE / Reference` field. Distinguish a documented vulnerability from a similarity or an unverified possibility.

**Burn payload locking:** Determine: 
- Which WiX version first introduced read-only locking of extracted Burn payloads, based on reliable sources. 

Treat this as a priority investigation. Do not claim a mitigation is present or effective without evidence.

### 3. Security analysis

Read all accessible in-scope source and configuration files. Review installation, upgrade, repair, and uninstall behavior. The following checks are mandatory, but not exhaustive:

1.  WiX bundle/MSI version mismatches and their practical consequences.
2.  Burn payload locking and extraction security.
3.  Signing and integrity of every artifact executed with elevated privileges across relevant build paths.
4.  Paths from user-controlled inputs to elevated execution, including registry searches, public MSI properties, environment variables.
5.  Elevated deferred actions and appropriate rollback behavior.
6.  Suppressed build warnings, ICE checks, or validation; security-relevant source paths excluded from automated analysis.
7.  Build and packaging scripts' handling of exit codes and failed steps.
8.  Version-control coverage and whether the review can be reproduced from a clean checkout.

Continue beyond this list when evidence suggests other security-relevant defects.


## C. Do not report the following findings, or similar ones, as issues:

1. Remote Code Exec - Plaintext HTTP content is loaded into windows that have full Node.js access - reason: Non-remote NAS services ( Full text file search, fax search, contract management, transaction document manament) which uses http:// link to launch NAS-hosted web apps loaded over HTTP into Electron windows. These windows load remote NAS pages, and those page sources are not in this repository.
2. Insecure Config - Node integration on and context isolation off in every application window - reason: Disabling nodeIntegration and contextIsolation:true is not a configuration-only change. The current application architecture relies heavily on direct Node.js API access (fs, path, etc.) from the renderer process
3. Insecure Config - Browser security is switched off in six windows - reason: Changing webSecurity from false to true is not straightforward, several application features rely on loading HTTP-based resources like services, thumbnail, icons(http://localhost:) which called with the url of http://  will not work. Similarly, disabling nodeIntegration is not a configuration-only change.
4. Weak Crypto - AES key hardcoded as sixteen 0xFF bytes - Reason: This 0xFF code is required when upgrading the application from market version to this current version for successful auto login purpose. migration fallback is required to avoid disrupting existing auto-login users after the update
5. Hardcoded Secret - Backend API keys committed to source and shipped in the installer  ( log_apikey ) - Reason : It is a Backend API key file.
6. Credential Exposure - Login ID and password are placed in a window URL query string ( handler_new_window ) - never called.
7. Hardcoded Key - Firebase API keys and VAPID keys embedded in the renderer - Reason: A Firebase Web API key is a project identifier, not a secret. Google publishes it in every web app. Extracting it from the bundle grants no data access on its own — so this is not a credential leak.
8. node_modules/: third-party code and Tests: test/ and wdio.conf.js
9. Exclude the ‘/shared’ route ({ path: '/shared', component: SharedFolderView }) and its component, child components, referance places and related files from the review.


## Excel deliverable

Create: `<report folder from the run instruction>/Entire_PC_Portal_Application_and_Installer_Security_Review_<YYYY-MM-DD>.xlsx`

Use Python and `openpyxl`. Keep the generator script outside the repository. Do not alter source files.

Create the below sheet:

### Findings

### Findings:
- Finding ID (`INST-001`, sequential)
- Title
- Category
- Severity and confidence
- Affected file and exact line number or configuration element
- Description
- Vulnerable behavior and root cause
- Impact
- Relevant CWE or authoritative reference, where applicable
- Recommended remediation
- Validation Steps

Formatting: bold frozen header, autofilter, wrapped text, readable column widths, and severity colors (Critical `#C00000`, High `#E26B0A`, Medium `#FFC000`, Low `#4472C4`).

Use this sheet to record completed clean checks as well as findings.

After the workbook is successfully written, reply with only its absolute path. If the workbook cannot be created, report the specific blocker instead of claiming success.
