#!/usr/bin/env python3
"""Compare 1+ experiment tags' results (perf/results/<tag>/*.json) --
prints a text table, renders paper-ready bar charts per metric, and (unless
--no-html) writes a single report.html that embeds all of it for viewing in
a browser -- that's the "visually view/compare" entry point; the text
table/PNGs on their own work fine too if that's all you want.

Usage:
    python3 report.py <tag1> <tag2> ...
    python3 report.py --all               # every tag under perf/results/
    python3 report.py --all --open        # ...and open report.html when done

A single tag works too (just renders one bar per circuit per chart) --
useful for eyeballing one run's shape even with nothing to compare it against.

See perf.py for the one-shot "run the harness, then report" entry point.
"""

import argparse
import glob
import html
import json
import os
import webbrowser

import matplotlib.pyplot as plt
import numpy as np

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
RESULTS_DIR = os.path.join(SCRIPT_DIR, "results")

# (dotted path into the per-case JSON record, display label)
METRICS = [
    ("runtime.ns_per_sample", "ns per sample"),
    ("runtime.cycles_per_sample", "cycles per sample"),
    ("runtime.ipc", "IPC"),
    ("runtime.l1d_load_requests_per_sample", "L1D load requests per sample"),
    ("runtime.l1d_store_requests_per_sample", "L1D store requests per sample"),
    ("codegen.temp_count_total", "temp vars (register pressure)"),
    ("codegen.estimated_worst_case_ops_per_sample", "estimated ops per sample"),
]


def get_path(record, dotted_path):
    value = record
    for part in dotted_path.split("."):
        if value is None:
            return None
        value = value.get(part)
    return value


def load_records(tags):
    records = []
    for tag in tags:
        tag_dir = os.path.join(RESULTS_DIR, tag)
        for json_path in sorted(glob.glob(os.path.join(tag_dir, "*.json"))):
            with open(json_path) as f:
                records.append(json.load(f))
    return records


def pivot(records, metric_path):
    """circuit -> {experiment_tag -> value}, preserving first-seen order."""
    table = {}
    circuits = []
    experiments = []
    for r in records:
        value = get_path(r, metric_path)
        if value is None:
            continue
        circuit = r["circuit"]
        experiment = r["experiment_tag"]
        if circuit not in table:
            table[circuit] = {}
            circuits.append(circuit)
        if experiment not in experiments:
            experiments.append(experiment)
        table[circuit][experiment] = value
    return table, circuits, experiments


def print_table(table, circuits, experiments, label):
    if not table:
        print(f"  (no data for {label})")
        return
    print(f"\n=== {label} ===")
    col_width = 16
    header = f"{'circuit':<20}" + "".join(f"{e:>{col_width}}" for e in experiments)
    print(header)
    for circuit in circuits:
        row = f"{circuit:<20}"
        for e in experiments:
            v = table[circuit].get(e)
            row += f"{v:>{col_width}.4g}" if v is not None else f"{'--':>{col_width}}"
        print(row)


def plot_metric(table, circuits, experiments, label, out_path):
    if not table:
        return

    fig, ax = plt.subplots(figsize=(8, 6))
    x = np.arange(len(circuits))
    width = 0.8 / len(experiments)

    for i, experiment in enumerate(experiments):
        values = [table[c].get(experiment, 0) for c in circuits]
        ax.bar(x + i * width, values, width, label=experiment)

    ax.set_ylabel(label)
    ax.set_title(label)
    ax.set_xticks(x + width * (len(experiments) - 1) / 2)
    ax.set_xticklabels(circuits, rotation=20, ha="right")
    ax.legend(title="experiment")
    plt.tight_layout()
    plt.savefig(out_path)
    plt.close(fig)
    print(f"  wrote {out_path}")


def html_table(table, circuits, experiments):
    esc = html.escape
    head = "<tr><th>circuit</th>" + "".join(f"<th>{esc(e)}</th>" for e in experiments) + "</tr>"
    rows = []
    for circuit in circuits:
        cells = f"<td>{esc(circuit)}</td>"
        for e in experiments:
            v = table[circuit].get(e)
            cells += f"<td>{v:.4g}</td>" if v is not None else "<td class='missing'>--</td>"
        rows.append(f"<tr>{cells}</tr>")
    return f"<table>{head}{''.join(rows)}</table>"


HTML_STYLE = """
body { font-family: -apple-system, sans-serif; margin: 2rem; color: #222; }
h1 { font-size: 1.4rem; }
h2 { font-size: 1.1rem; margin-top: 2.5rem; border-bottom: 1px solid #ccc; padding-bottom: 0.3rem; }
table { border-collapse: collapse; margin: 0.75rem 0 1.25rem; }
th, td { border: 1px solid #ccc; padding: 0.3rem 0.7rem; text-align: right; font-variant-numeric: tabular-nums; }
th:first-child, td:first-child { text-align: left; }
td.missing { color: #999; }
img { max-width: 700px; display: block; }
.meta { color: #666; font-size: 0.9rem; }
"""


def build_html(tags, sections, out_path):
    """`sections` is a list of (label, html_table_str, chart_filename_or_None)."""
    body_parts = [f"<h2>{html.escape(label)}</h2>{table_html}"
                  + (f"<img src='{html.escape(chart_name)}'>" if chart_name else "")
                  for label, table_html, chart_name in sections]
    doc = f"""<!doctype html>
<html><head><meta charset="utf-8"><title>perf report</title><style>{HTML_STYLE}</style></head>
<body>
<h1>perf report</h1>
<p class="meta">Experiment tags: {html.escape(', '.join(tags))}</p>
{''.join(body_parts)}
</body></html>
"""
    with open(out_path, "w") as f:
        f.write(doc)
    return out_path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("tags", nargs="*", help="experiment tags to compare")
    parser.add_argument("--all", action="store_true", help="use every tag under perf/results/")
    parser.add_argument("--out-dir", default=os.path.join(SCRIPT_DIR, "results"),
                         help="where to write comparison charts + report.html")
    parser.add_argument("--no-html", action="store_true", help="skip writing report.html")
    parser.add_argument("--open", action="store_true", help="open report.html in a browser when done")
    args = parser.parse_args()

    if args.all:
        tags = sorted(d for d in os.listdir(RESULTS_DIR)
                       if os.path.isdir(os.path.join(RESULTS_DIR, d)))
    else:
        tags = args.tags

    if not tags:
        parser.error("pass at least one experiment tag, or --all")

    print(f"Comparing experiment tags: {', '.join(tags)}")
    records = load_records(tags)
    if not records:
        print("No result JSON files found for the given tags.")
        return

    sections = []
    for metric_path, label in METRICS:
        table, circuits, experiments = pivot(records, metric_path)
        print_table(table, circuits, experiments, label)
        chart_name = metric_path.replace(".", "_") + ".png"
        plot_metric(table, circuits, experiments, label, os.path.join(args.out_dir, chart_name))
        if table:
            sections.append((label, html_table(table, circuits, experiments), chart_name))

    if not args.no_html:
        report_path = build_html(tags, sections, os.path.join(args.out_dir, "report.html"))
        print(f"\nWrote {report_path}")
        if args.open:
            webbrowser.open(f"file://{report_path}")


if __name__ == "__main__":
    main()
