# Task: Code Review of a Pushed Commit

## Role
Act as a senior reviewer for the PC Portal application (Electron + Vue 3 frontend, local Express server, WiX Burn installer). Review only the changes in `changes.diff`, using the surrounding source files for context.

## How to review
1. Read `changes.diff` to see which files and lines changed..
2. Open the changed files, and the code they call or are called by, so you judge the change in context rather than from the diff alone.
3. Report problems introduced or exposed by this change. Do not report issues in untouched code unless the change makes them reachable or worse.
4. Verify each finding against the actual code. Do not guess; if you are unsure, say so and explain what would confirm it.

## What to look for
- **Correctness:** logic errors, wrong conditions, unhandled errors or rejected promises, race conditions, null/undefined access, broken edge cases.
- **Security:**
  - Electron: `nodeIntegration`, `contextIsolation`, `sandbox`, `webSecurity`, preload exposure, IPC handlers that trust renderer input, `shell.openExternal` with unvalidated URLs.
  - Express server: binding address (should be localhost only), missing input validation, privileged operations reachable over HTTP.
  - Command/path injection via `child_process`, PowerShell, or file paths built from input.
  - XSS through `v-html`, `innerHTML`, or unsanitised data.
  - Hardcoded secrets, keys, tokens or credentials; sensitive data in logs.
  - Installer: elevated actions, insecure file permissions, unsafe temp-file use.
- **Dependencies:** newly added or upgraded packages with known vulnerabilities or unpinned versions.
- **Reliability:** resource leaks, missing timeouts on network calls, unbounded loops or memory growth.
- **Maintainability:** duplicated code, dead code, misleading names — only when it is likely to cause bugs.

## Rules
- Do not modify, create or delete any file.
- Do not include secret values in the report. If a secret is exposed, give only the file path and line.
- Do not ask questions; complete the review in one pass.

## Report format (Markdown)

```
# Code Review Report

**Commit range:** <base>..<head>
**Files changed:** <count>

## Summary
<2–4 sentences: what the change does and the overall verdict>

## Findings

| # | Severity | Category | File:Line | Issue |
|---|----------|----------|-----------|-------|

### 1. <Short title>
- **Severity:** Critical / High / Medium / Low
- **Location:** `path/to/file.js:42`
- **Problem:** <what is wrong>
- **Impact:** <what can happen, with a concrete scenario>
- **Fix:** <specific recommendation, with a code snippet if helpful>

## Positive notes
<anything done well, briefly>
```

If there are no findings, say so clearly in the Summary and leave the Findings table empty.
