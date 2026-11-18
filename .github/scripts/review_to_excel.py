"""Convert Codex's JSON review (review.json) into an Excel workbook and a
Markdown summary shown on the GitHub Actions run page.

Usage: python review_to_excel.py <review.json> <output.xlsx>
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
    "High": "FF6B6B",
    "Medium": "FFD966",
    "Low": "A9D08E",
}
HEADER_FILL = PatternFill("solid", fgColor="1F4E78")
HEADER_FONT = Font(bold=True, color="FFFFFF")
WRAP = Alignment(wrap_text=True, vertical="top")


def env(name, default=""):
    return os.environ.get(name, default)


def style_header(ws, row=1):
    for cell in ws[row]:
        cell.fill = HEADER_FILL
        cell.font = HEADER_FONT
        cell.alignment = Alignment(vertical="center", wrap_text=True)


def build_workbook(review, out_path):
    order = {s: i for i, s in enumerate(SEVERITIES)}
    findings = sorted(review.get("findings", []), key=lambda f: order.get(f["severity"], 99))
    counts = Counter(f["severity"] for f in findings)
    repo = env("GITHUB_REPOSITORY")
    run_url = f'{env("GITHUB_SERVER_URL", "https://github.com")}/{repo}/actions/runs/{env("GITHUB_RUN_ID")}'

    wb = Workbook()

    # --- Summary sheet ---
    ws = wb.active
    ws.title = "Summary"
    rows = [
        ("Repository", repo),
        ("Branch", env("GITHUB_REF_NAME")),
        ("Commit range", f'{env("BASE_SHA")[:7]}..{env("HEAD_SHA")[:7]}'),
        ("Pushed by", env("GITHUB_ACTOR")),
        ("Reviewed at (UTC)", datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M")),
        ("Model", env("REVIEW_MODEL")),
        ("Workflow run", run_url),
        ("Verdict", review.get("verdict", "")),
        ("Total findings", len(findings)),
    ]
    rows += [(f"  {s}", counts.get(s, 0)) for s in SEVERITIES]
    rows += [
        ("Summary", review.get("summary", "")),
        ("Files reviewed", "\n".join(review.get("files_reviewed", []))),
        ("Positive notes", "\n".join(f"- {n}" for n in review.get("positive_notes", []))),
    ]
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
    headers = ["#", "Severity", "Category", "Title", "File", "Line",
               "Problem", "Impact", "Recommendation", "Confidence"]
    widths = [5, 11, 15, 35, 40, 7, 60, 50, 60, 12]
    wf.append(headers)
    style_header(wf)
    for i, f in enumerate(findings, 1):
        wf.append([i, f["severity"], f["category"], f["title"], f["file"], f["line"],
                   f["problem"], f["impact"], f["recommendation"], f["confidence"]])
        sev_cell = wf.cell(row=i + 1, column=2)
        color = SEVERITY_FILL.get(f["severity"])
        if color:
            sev_cell.fill = PatternFill("solid", fgColor=color)
            sev_cell.font = Font(bold=True, color="FFFFFF" if f["severity"] == "Critical" else "000000")
    for row in wf.iter_rows(min_row=2):
        for c in row:
            c.alignment = WRAP
    for idx, w in enumerate(widths, 1):
        wf.column_dimensions[get_column_letter(idx)].width = w
    wf.freeze_panes = "A2"
    if findings:
        wf.auto_filter.ref = wf.dimensions
    else:
        wf.append(["", "", "", "No findings - the reviewer found no issues in this change."])

    wb.save(out_path)
    return findings, counts


def write_step_summary(review, findings, counts, xlsx_name):
    lines = [
        "## Codex Code Review",
        "",
        f"**Verdict:** {review.get('verdict', '')}  ",
        f"**Commit range:** `{env('BASE_SHA')[:7]}..{env('HEAD_SHA')[:7]}`  ",
        f"**Findings:** {len(findings)} "
        + "(" + ", ".join(f"{s}: {counts.get(s, 0)}" for s in SEVERITIES) + ")",
        "",
        review.get("summary", ""),
        "",
    ]
    if findings:
        lines += ["| # | Severity | Category | Location | Title |", "|---|---|---|---|---|"]
        for i, f in enumerate(findings, 1):
            loc = f["file"] + (f":{f['line']}" if f.get("line") else "")
            title = f["title"].replace("|", "\\|")
            lines.append(f"| {i} | {f['severity']} | {f['category']} | `{loc}` | {title} |")
        lines.append("")
    lines.append(f"Download **{xlsx_name}** from the *Artifacts* section of this run.")
    text = "\n".join(lines) + "\n"

    summary_file = env("GITHUB_STEP_SUMMARY")
    if summary_file:
        with open(summary_file, "a", encoding="utf-8") as fh:
            fh.write(text)
    print(text)


def main():
    if len(sys.argv) != 3:
        sys.exit("Usage: review_to_excel.py <review.json> <output.xlsx>")
    src, out = sys.argv[1], sys.argv[2]
    try:
        with open(src, encoding="utf-8") as fh:
            review = json.load(fh)
    except (OSError, json.JSONDecodeError) as exc:
        print(f"::error::Code review did not produce valid JSON ({exc}). See the 'Run Codex Code Review' step log.")
        sys.exit(1)

    findings, counts = build_workbook(review, out)
    write_step_summary(review, findings, counts, os.path.basename(out))


if __name__ == "__main__":
    main()
