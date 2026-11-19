# PC Portal

## AI code review on push

Every push to `main`, `develop` or `feature/*` is reviewed by an AI model. The result is an
Excel report (plus `review.json` and `summary.md`) attached to the pipeline run.

| | GitHub Actions | GitLab CI |
|---|---|---|
| Pipeline file | `.github/workflows/code-review.yml` | `.gitlab-ci.yml` |
| Where to set keys | Settings > Secrets and variables > Actions | Settings > CI/CD > Variables |
| Where the report is | Run page > *Artifacts*, summary on the run page | Job page > *Job artifacts* > Download, summary in job log |

Both run the same script, `ci/run_review.sh`, with the same prompt (`prompts/code_review_prompt.md`)
and output schema (`prompts/review_schema.json`).

### Choosing the reviewer

Set `REVIEW_PROVIDER` to `codex` or `claude`. Leave it empty to pick automatically from whichever
key is configured (Codex first if both are).

**Codex (Azure OpenAI)**

| Name | Type | Value |
|---|---|---|
| `AZURE_OPENAI_API_KEY` | secret | Azure portal > OpenAI resource > Keys and Endpoint > KEY 1 |
| `AZURE_OPENAI_ENDPOINT` | secret or variable | e.g. `https://my-resource.openai.azure.com` (the full Foundry URL also works) |
| `AZURE_OPENAI_DEPLOYMENT` | variable, optional | deployment name, default `gpt-5.1-codex` |

**Claude (Anthropic API)**

| Name | Type | Value |
|---|---|---|
| `ANTHROPIC_API_KEY` | secret | console.anthropic.com > API keys |
| `CLAUDE_MODEL` | variable, optional | default `sonnet` (latest Sonnet); e.g. `opus` |
| `CLAUDE_MAX_BUDGET_USD` | variable, optional | cost cap per review, default `5` |

Claude runs with read-only tools (Read, Grep, Glob), confined to the repository, with no shell,
no web access and no MCP servers.

**Optional (both)**

| Name | Default | Meaning |
|---|---|---|
| `FAIL_ON_CHANGES_REQUESTED` | `false` | `true` fails the pipeline when the verdict is *Changes requested* |
| `MAX_DIFF_BYTES` | `300000` | larger diffs are truncated (noted in the report) |
| `REVIEW_BASE_BRANCHES` | `develop main master` | branches a new branch may start from |

GitLab: tick **Masked** for the keys. Untick **Protected** unless every reviewed branch
(including `feature/*`) is protected, otherwise the job won't see the keys on those branches.
GitLab's `.github/` folder is unused and can stay; GitHub ignores `.gitlab-ci.yml`.

### What gets reviewed

- Normal push: the commits added by that push.
- First push of a new branch, or a force-push: all commits since the branch left its closest
  parent branch (`develop` or `main`).
- Lock files, `dist/`, `node_modules/`, `*.min.js` and `*.map` are skipped. If nothing
  reviewable changed, the model is not called.

CLI versions are pinned at the top of `ci/run_review.sh` (`CODEX_VERSION`,
`CLAUDE_CODE_VERSION`); update them deliberately.
