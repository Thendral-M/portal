# PC Portal

## AI code review on push

Every push to `main`, `develop` or `feature/*` is reviewed by an AI model (Codex or Claude).
The result is an Excel report, plus `review.json` and `summary.md`, attached to the pipeline run.

| | GitHub Actions | GitLab CI |
|---|---|---|
| Pipeline file | `.github/workflows/code-review.yml` | `.gitlab-ci.yml` |
| Where to set keys | Settings > Secrets and variables > Actions | Settings > CI/CD > Variables |
| Where the report is | Run page > *Artifacts* (summary also on the run page) | Job page > *Job artifacts* > Download (summary in the job log) |
| Job timeout | 90 minutes | 90 minutes |

Both pipelines run the same script, `ci/run_review.sh`, so the report is identical whichever
platform or reviewer is used.

### Files

| File | Purpose |
|---|---|
| `ci/run_review.sh` | Picks the reviewer and scope, builds the prompt, runs the model |
| `ci/review_to_excel.py` | Converts `review.json` into the Excel report and `summary.md` |
| `prompts/entire-application-review.md` | Review prompt used by both scopes: review areas, exclusions and accepted risks |
| `prompts/review_schema.json` | JSON schema both reviewers must return |

### Review scope

Set `REVIEW_SCOPE`. Both values use `prompts/entire-application-review.md`, so the same review
areas, exclusions (e.g. `installer/CA_source/`, `installer/guard/`, `installer/InstallModules/`,
unused installer files, `node_modules/`, tests) and "Do not report" rules apply.

| Value | What is reviewed |
|---|---|
| `full` (default) | The entire repository at the pushed commit: Electron app and the WiX Burn installer |
| `diff` | Only the files changed by the push (excluded files are skipped); the model reads related code for context but reports only issues introduced or exposed by the change |

**Diff scope details**

- Normal push: the commits added by that push.
- First push of a new branch, or a force-push: all commits since the branch left its closest
  parent branch (from `REVIEW_BASE_BRANCHES`, default `develop main master`).
- Lock files, `dist/`, `node_modules/`, `*.min.js` and `*.map` are skipped. If nothing
  reviewable changed, the model is not called and an "Approve" report is still produced.
- Diffs larger than `MAX_DIFF_BYTES` (default `300000`) are truncated; the report notes this
  and the model reads the remaining changed files directly.

### Choosing the reviewer

Set `REVIEW_PROVIDER` to `codex` or `claude`. Leave it empty to pick automatically from whichever
key is configured (Codex first if both are).

**Codex (Azure OpenAI)**

| Name | Type | Value |
|---|---|---|
| `AZURE_OPENAI_API_KEY` | secret | Azure portal > OpenAI resource > Keys and Endpoint > KEY 1 |
| `AZURE_OPENAI_ENDPOINT` | secret or variable | e.g. `https://my-resource.openai.azure.com` (the full Foundry URL also works) |
| `AZURE_OPENAI_DEPLOYMENT` | variable, optional | deployment name, default `gpt-5.1-codex-max` |

Codex runs in a read-only sandbox with no network access.

**Claude (Anthropic API)**

| Name | Type | Value |
|---|---|---|
| `ANTHROPIC_API_KEY` | secret | console.anthropic.com > API keys |
| `CLAUDE_MODEL` | variable, optional | default `sonnet` (latest Sonnet); e.g. `opus` |
| `CLAUDE_MAX_BUDGET_USD` | variable, optional | cost cap per review in USD; default `25` for `full`, `5` for `diff` |
| `REVIEW_WEB_RESEARCH` | variable, optional | `true` also allows WebSearch/WebFetch (CVE and advisory research); default `false` |

Claude runs with read-only tools (Read, Grep, Glob), confined to the repository, with no shell
and no MCP servers. Web access is off unless `REVIEW_WEB_RESEARCH=true`.

Each reviewer only receives its own API key.

**Optional (both)**

| Name | Default | Meaning |
|---|---|---|
| `REVIEW_SCOPE` | `full` | `full` or `diff` (see above) |
| `FAIL_ON_CHANGES_REQUESTED` | `false` | `true` fails the pipeline when the verdict is *Changes requested* |
| `MAX_DIFF_BYTES` | `300000` | `diff` scope only: larger diffs are truncated |
| `REVIEW_BASE_BRANCHES` | `develop main master` | `diff` scope only: branches a new branch may start from |

On GitHub, only the variables listed in the workflow's `env:` block reach the script.
`MAX_DIFF_BYTES` and `REVIEW_BASE_BRANCHES` are not in it, so add them there to change them.
On GitLab, every CI/CD variable is passed automatically.

GitLab: tick **Masked** for the keys. Untick **Protected** unless every reviewed branch
(including `feature/*`) is protected; otherwise the job won't see the keys on those branches.
Each platform ignores the other's pipeline file, so both can stay in the repository.

### Output (`review-output/`)

| File | Content |
|---|---|
| `Entire_PC_Portal_Application_and_Installer_Security_Review_<date>_<sha>.xlsx` | `full` scope report |
| `Code_Review_<branch>_<sha>.xlsx` | `diff` scope report |

Both reports have a *Summary* sheet and a *Findings* sheet, with findings numbered `INST-001`, `INST-002`, ...
| `review.json` | Raw review returned by the model |
| `summary.md` | Short Markdown summary (also on the GitHub run page) |
| `claude_result.json` | Claude only: full CLI output, including cost |

Verdict rules: *Approve* (no findings), *Approve with comments* (only Low/Medium),
*Changes requested* (any High/Critical). Artifacts are kept for 30 days on GitLab and per the
repository's retention setting on GitHub.

### Running locally

Requires Git, Bash, Node.js/npm (the CLI is installed with `npm install -g` if missing) and
Python 3 with `openpyxl`.

```bash
export ANTHROPIC_API_KEY=...        # or AZURE_OPENAI_API_KEY + AZURE_OPENAI_ENDPOINT
export REVIEW_SCOPE=diff            # optional; default full
bash ci/run_review.sh
```

Locally, `diff` scope reviews the last commit (`HEAD~1..HEAD`) unless `BASE_SHA` is set.

### CLI versions

CLI versions are pinned at the top of `ci/run_review.sh`: `CODEX_VERSION` (`0.170.0`) and
`CLAUDE_CODE_VERSION` (`2.1.328`). Update them deliberately after testing.
