"""Convert the AI review (review.json) into an Excel workbook and a Markdown
summary. Works the same for Codex and Claude, on GitHub Actions and GitLab CI.

Usage: python review_to_excel.py <review.json> <output.xlsx> [summary.md]

Context (repo, branch, commit range, ...) is read from the REVIEW_* environment
variables exported by ci/run_review.sh, so this script has no CI-specific code.
On GitHub the summary is also appended to the job's Step Summary page.
"""
import json
import os
import sys
from collections import Counter
from datetime import datetime, timezone

from openpyxl import Workbook
from openpyxl.styles import Alignment, Font, PatternFill
from openpyxl.utils import get_column_letter

SEVERITIES = ["Critical", "High", "Medium", "Low"]
SEVERITY_FILL = {
    "Critical": "C00000",
    "High": "E26B0A",
    "Medium": "FFC000",
    "Low": "4472C4",
}
WHITE_TEXT = {"Critical", "High", "Low"}
FINDING_FIELDS = ["severity", "category", "title", "file", "line",
                  "problem", "impact", "recommendation", "confidence",
                  "reference", "validation_steps"]


def full_scope():
    return env("REVIEW_SCOPE") == "full"


def finding_id(i):
    return f"INST-{i:03d}"


def reviewed_range():
    head = env("REVIEW_HEAD_SHA")[:7]
    if full_scope():
        return f"Entire code base at {head}"
    return f'{env("REVIEW_BASE_SHA")[:7] or "(root)"}..{head}'
HEADER_FILL = PatternFill("solid", fgColor="1F4E78")
HEADER_FONT = Font(bold=True, color="FFFFFF")
WRAP = Alignment(wrap_text=True, vertical="top")


def env(name, default=""):
    return os.environ.get(name) or default


def as_text(value):
    """Plain text for a cell; tolerate lists/objects a model may return."""
    if value is None:
        return ""
    if isinstance(value, (list, tuple)):
        return "\n".join(as_text(v) for v in value)
    if isinstance(value, dict):
        return json.dumps(value, ensure_ascii=False)
    return str(value)


def as_list(value):
    if value is None:
        return []
    if isinstance(value, list):
        return [as_text(v) for v in value]
    return [as_text(value)]


def normalise(review):
    """Fill in missing fields so a slightly off-schema reply never crashes the report."""
    if not isinstance(review, dict):
        raise ValueError("top-level JSON value is not an object")
    findings = []
    for raw in review.get("findings") or []:
        if not isinstance(raw, dict):
            continue
        f = {k: raw.get(k) for k in FINDING_FIELDS}
        for k in FINDING_FIELDS:
            if k != "line":
                f[k] = as_text(f[k])
        f["severity"] = f["severity"].strip().capitalize() or "Low"
        line = f["line"]
        if isinstance(line, str) and line.strip().isdigit():
            line = int(line.strip())
        f["line"] = line if isinstance(line, int) else None
        findings.append(f)
    return {
        "verdict": as_text(review.get("verdict")),
        "summary": as_text(review.get("summary")),
        "files_reviewed": as_list(review.get("files_reviewed")),
        "findings": findings,
        "positive_notes": as_list(review.get("positive_notes")),
    }


def style_header(ws, row=1):
    for cell in ws[row]:
        cell.fill = HEADER_FILL
        cell.font = HEADER_FONT
        cell.alignment = Alignment(vertical="center", wrap_text=True)


def build_workbook(review, out_path):
    order = {s: i for i, s in enumerate(SEVERITIES)}
    findings = sorted(review["findings"], key=lambda f: order.get(f["severity"], 99))
    counts = Counter(f["severity"] for f in findings)

    wb = Workbook()

    # --- Summary sheet ---
    ws = wb.active
    ws.title = "Summary"
    rows = [
        ("Repository", env("REVIEW_REPO")),
        ("Branch", env("REVIEW_BRANCH")),
        ("Review scope", "Entire application and installer" if full_scope() else "Commits in this push"),
        ("Reviewed", reviewed_range()),
        ("Pushed by", env("REVIEW_ACTOR")),
        ("CI platform", env("REVIEW_PLATFORM")),
        ("Reviewed at (UTC)", datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M")),
        ("Reviewer", env("REVIEW_PROVIDER_LABEL")),
        ("Model", env("REVIEW_MODEL")),
        ("Pipeline run", env("REVIEW_RUN_URL")),
        ("Verdict", review["verdict"]),
        ("Total findings", len(findings)),
    ]
    rows += [(f"  {s}", counts.get(s, 0)) for s in SEVERITIES]
    rows += [
        ("Summary", review["summary"]),
        ("Files reviewed", "\n".join(review["files_reviewed"])),
        ("Clean checks / positive notes", "\n".join(f"- {n}" for n in review["positive_notes"])),
    ]
    note = env("REVIEW_NOTE")
    if note:
        rows.append(("Note", note))
    ws.append(("Field", "Value"))
    style_header(ws)
    for r in rows:
        ws.append(r)
    for row in ws.iter_rows(min_row=2):
        row[0].font = Font(bold=True)
        for c in row:
            c.alignment = WRAP
    ws.column_dimensions["A"].width = 22
    ws.column_dimensions["B"].width = 100

    # --- Findings sheet ---
    wf = wb.create_sheet("Findings")
    headers = ["Finding ID", "Severity", "Category", "Title", "File", "Line",
               "Description / Root cause", "Impact", "Recommended remediation",
               "Confidence", "CWE / CVE / Reference", "Validation steps"]
    widths = [11, 11, 15, 35, 40, 7, 60, 50, 60, 12, 45, 50]
    wf.append(headers)
    style_header(wf)
    for i, f in enumerate(findings, 1):
        wf.append([finding_id(i)] + [f[k] for k in FINDING_FIELDS])
        sev_cell = wf.cell(row=i + 1, column=2)
        color = SEVERITY_FILL.get(f["severity"])
        if color:
            sev_cell.fill = PatternFill("solid", fgColor=color)
            sev_cell.font = Font(bold=True, color="FFFFFF" if f["severity"] in WHITE_TEXT else "000000")
    for row in wf.iter_rows(min_row=2):
        for c in row:
            c.alignment = WRAP
    for idx, w in enumerate(widths, 1):
        wf.column_dimensions[get_column_letter(idx)].width = w
    wf.freeze_panes = "A2"
    if findings:
        wf.auto_filter.ref = wf.dimensions
    else:
        wf.append(["", "", "", "No findings - the reviewer found no issues."])

    wb.save(out_path)
    return findings, counts


def build_summary(review, findings, counts, xlsx_name):
    reviewer = env("REVIEW_PROVIDER_LABEL", "AI")
    lines = [
        f"## {reviewer} Code Review",
        "",
        f"**Verdict:** {review['verdict']}  ",
        f"**Reviewed:** `{reviewed_range()}`  ",
        f"**Model:** {env('REVIEW_MODEL')}  ",
        f"**Findings:** {len(findings)} "
        + "(" + ", ".join(f"{s}: {counts.get(s, 0)}" for s in SEVERITIES) + ")",
        "",
        review["summary"],
        "",
    ]
    if env("REVIEW_NOTE"):
        lines += [f"> {env('REVIEW_NOTE')}", ""]
    if findings:
        lines += ["| ID | Severity | Category | Location | Title |", "|---|---|---|---|---|"]
        for i, f in enumerate(findings, 1):
            loc = f["file"] + (f":{f['line']}" if f["line"] else "")
            title = f["title"].replace("|", "\\|").replace("\n", " ")
            lines.append(f"| {finding_id(i)} | {f['severity']} | {f['category']} | `{loc}` | {title} |")
        lines.append("")
    lines.append(f"Download **{xlsx_name}** from the job's artifacts.")
    return "\n".join(lines) + "\n"


def main():
    if len(sys.argv) not in (3, 4):
        sys.exit("Usage: review_to_excel.py <review.json> <output.xlsx> [summary.md]")
    src, out = sys.argv[1], sys.argv[2]
    summary_path = sys.argv[3] if len(sys.argv) == 4 else None
    try:
        with open(src, encoding="utf-8") as fh:
            review = normalise(json.load(fh))
    except (OSError, ValueError) as exc:
        # json.JSONDecodeError is a subclass of ValueError
        print(f"ERROR: The code review did not produce valid JSON ({exc}). "
              "See the reviewer output earlier in this job log.")
        sys.exit(1)

    findings, counts = build_workbook(review, out)
    text = build_summary(review, findings, counts, os.path.basename(out))

    if summary_path:
        with open(summary_path, "w", encoding="utf-8") as fh:
            fh.write(text)
    step_summary = env("GITHUB_STEP_SUMMARY")
    if step_summary:
        with open(step_summary, "a", encoding="utf-8") as fh:
            fh.write(text)
    print(text)


if __name__ == "__main__":
    main()
