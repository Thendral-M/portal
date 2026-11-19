# Task: Code Review of a Pushed Commit

## Role
Act as a senior reviewer for the PC Portal application (Electron + Vue 3 frontend, local Express server, WiX Burn installer). Review only the changes in `changes.diff`, using the surrounding source files for context.

## How to review
1. Read `changes.diff` to see which files and lines changed.
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

## Report format

Your final answer must be a single JSON object matching the provided output schema. It is converted into an Excel workbook, so keep each field plain text (no Markdown tables).

- `verdict`: `Approve` (no issues), `Approve with comments` (only Low/Medium issues), or `Changes requested` (any High/Critical issue).
- `summary`: 2–4 sentences on what the change does and the overall assessment.
- `files_reviewed`: repository-relative paths of the changed files you reviewed.
- `findings`: one entry per issue, most severe first.
  - `severity`: Critical / High / Medium / Low.
  - `category`: Correctness / Security / Dependencies / Reliability / Maintainability.
  - `title`: short one-line label.
  - `file`: repository-relative path; `line`: line number in the new version of the file, or null if not applicable.
  - `problem`: what is wrong. `impact`: what can happen, with a concrete scenario. `recommendation`: the specific fix.
  - `confidence`: Confirmed (verified in code) / Likely / Uncertain.
- `positive_notes`: anything done well, briefly; empty array if none.

If there are no findings, return an empty `findings` array and say so in `summary`.
