"""
Convert lesson markdown files to high-quality PDF.
Uses markdown2 + pygments for styled HTML, then Edge headless for PDF rendering.
"""
import os
import re
import glob
import subprocess
import tempfile

import markdown2
import latex2mathml.converter
from pygments.formatters import HtmlFormatter

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
PDF_DIR = os.path.join(BASE_DIR, "pdf")
EDGE = r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"

CODE_CSS = HtmlFormatter(style="monokai").get_style_defs(".codehilite")

TEMPLATE = """\
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="utf-8">
<style>
  @page {{
    size: A4;
    margin: 20mm 18mm 20mm 18mm;
  }}
  body {{
    font-family: "Microsoft YaHei", "PingFang SC", "Noto Sans SC", sans-serif;
    font-size: 14px;
    line-height: 1.8;
    color: #1a1a1a;
    max-width: 100%;
    word-wrap: break-word;
    overflow-wrap: break-word;
  }}
  h1 {{
    font-size: 26px;
    color: #1a1a1a;
    border-bottom: 2px solid #3b82f6;
    padding-bottom: 8px;
    margin-top: 30px;
  }}
  h2 {{
    font-size: 21px;
    color: #1e3a5f;
    border-bottom: 1px solid #d1d5db;
    padding-bottom: 6px;
    margin-top: 28px;
  }}
  h3 {{
    font-size: 17px;
    color: #374151;
    margin-top: 22px;
  }}
  h4 {{
    font-size: 15px;
    color: #4b5563;
    margin-top: 18px;
  }}
  p {{
    margin: 10px 0;
  }}
  blockquote {{
    border-left: 4px solid #3b82f6;
    margin: 14px 0;
    padding: 8px 16px;
    background: #eff6ff;
    color: #1e40af;
    border-radius: 0 6px 6px 0;
  }}
  blockquote p {{
    margin: 4px 0;
  }}
  code {{
    font-family: "Cascadia Code", "Consolas", "Courier New", monospace;
    background: #f1f5f9;
    color: #be185d;
    padding: 2px 5px;
    border-radius: 4px;
    font-size: 13px;
  }}
  pre {{
    background: #282a36;
    color: #f8f8f2;
    padding: 16px;
    border-radius: 8px;
    overflow-x: auto;
    font-size: 13px;
    line-height: 1.6;
    margin: 14px 0;
    page-break-inside: avoid;
  }}
  pre code {{
    background: none;
    color: inherit;
    padding: 0;
    font-size: 13px;
  }}
  table {{
    border-collapse: collapse;
    width: 100%;
    margin: 14px 0;
    font-size: 13px;
    page-break-inside: avoid;
  }}
  th {{
    background: #1e3a5f;
    color: #fff;
    padding: 10px 12px;
    text-align: left;
    font-weight: 600;
  }}
  td {{
    padding: 8px 12px;
    border: 1px solid #e5e7eb;
  }}
  tr:nth-child(even) {{
    background: #f9fafb;
  }}
  tr:hover {{
    background: #eff6ff;
  }}
  img {{
    max-width: 100%;
    height: auto;
    border-radius: 6px;
    margin: 12px 0;
  }}
  hr {{
    border: none;
    border-top: 1px solid #d1d5db;
    margin: 20px 0;
  }}
  a {{
    color: #2563eb;
    text-decoration: none;
  }}
  ul, ol {{
    padding-left: 24px;
  }}
  li {{
    margin: 4px 0;
  }}
  strong {{
    color: #1e3a5f;
  }}
  /* Math (MathML) styling */
  math {{
    font-size: 1.1em;
  }}
  math[display="block"] {{
    display: block;
    text-align: center;
    margin: 16px 0;
    font-size: 1.2em;
  }}
  .math-block {{
    text-align: center;
    margin: 16px 0;
    overflow-x: auto;
  }}
  /* Pygments syntax highlighting */
  {code_css}
  .codehilite pre {{
    background: #272822;
    margin: 0;
  }}
</style>
</head>
<body>
{content}
</body>
</html>
"""


def convert_latex_to_mathml(text):
    """Convert $...$ and $$...$$ LaTeX math to MathML before markdown processing."""
    # First protect code blocks from math conversion
    code_blocks = []
    def save_code(m):
        code_blocks.append(m.group(0))
        return f"\x00CODEBLOCK{len(code_blocks)-1}\x00"
    text = re.sub(r'```[\s\S]*?```', save_code, text)
    text = re.sub(r'`[^`\n]+`', save_code, text)

    # Convert block math $$...$$ first
    def convert_block(m):
        latex = m.group(1).strip()
        try:
            mathml = latex2mathml.converter.convert(latex)
            mathml = mathml.replace('display="inline"', 'display="block"')
            return f'\n<div class="math-block">{mathml}</div>\n'
        except Exception:
            return m.group(0)
    text = re.sub(r'\$\$\s*([\s\S]*?)\s*\$\$', convert_block, text)

    # Convert inline math $...$
    def convert_inline(m):
        latex = m.group(1).strip()
        try:
            return latex2mathml.converter.convert(latex)
        except Exception:
            return m.group(0)
    text = re.sub(r'(?<!\$)\$(?!\$)(.+?)(?<!\$)\$(?!\$)', convert_inline, text)

    # Restore code blocks
    for i, block in enumerate(code_blocks):
        text = text.replace(f'\x00CODEBLOCK{i}\x00', block)
    return text


def md_to_html(md_path):
    """Convert markdown to rich HTML with syntax highlighting."""
    with open(md_path, "r", encoding="utf-8") as f:
        text = f.read()

    # Convert LaTeX math to MathML before markdown processing
    text = convert_latex_to_mathml(text)

    extras = [
        "fenced-code-blocks",
        "tables",
        "code-friendly",
        "cuddled-lists",
        "header-ids",
        "break-on-newline",
    ]
    html = markdown2.markdown(text, extras=extras)

    # Inline images as base64 data URIs for reliable rendering
    md_dir = os.path.dirname(md_path)
    import base64
    import mimetypes
    def inline_img(m):
        src = m.group(1)
        if not os.path.isabs(src) and not src.startswith(("http", "data:")):
            abs_src = os.path.join(md_dir, src)
            if os.path.exists(abs_src):
                mime = mimetypes.guess_type(abs_src)[0] or "image/png"
                with open(abs_src, "rb") as img_f:
                    b64 = base64.b64encode(img_f.read()).decode()
                data_uri = f"data:{mime};base64,{b64}"
                return m.group(0).replace(src, data_uri)
        return m.group(0)
    html = re.sub(r'src="([^"]+)"', inline_img, html)
    return html


def html_to_pdf_edge(html_path, pdf_path):
    """Use Edge headless to print HTML to PDF."""
    import shutil
    import time

    tmp_pdf = os.path.join("C:\\Temp", "edge_output.pdf")
    os.makedirs("C:\\Temp", exist_ok=True)
    if os.path.exists(tmp_pdf):
        os.remove(tmp_pdf)

    # Edge needs file:// URL for the HTML
    html_url = "file:///" + html_path.replace("\\", "/")
    cmd = [
        EDGE,
        "--headless",
        "--disable-gpu",
        "--run-all-compositor-stages-before-draw",
        "--print-to-pdf=" + tmp_pdf,
        "--print-to-pdf-no-header",
        "--no-pdf-header-footer",
        html_url,
    ]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)

    # Wait for Edge to finish writing
    for _ in range(40):
        if os.path.exists(tmp_pdf) and os.path.getsize(tmp_pdf) > 0:
            time.sleep(1)
            break
        time.sleep(0.5)

    if not os.path.exists(tmp_pdf) or os.path.getsize(tmp_pdf) < 1000:
        raise RuntimeError(
            f"Edge failed (size={os.path.getsize(tmp_pdf) if os.path.exists(tmp_pdf) else 0}): "
            f"{result.stderr.strip() or result.stdout.strip()}"
        )
    shutil.move(tmp_pdf, pdf_path)


def convert(md_path, pdf_path):
    html_body = md_to_html(md_path)
    full_html = TEMPLATE.format(content=html_body, code_css=CODE_CSS)

    # Write temp HTML with simple ASCII path to avoid Unicode path issues
    os.makedirs("C:\\Temp", exist_ok=True)
    html_path = "C:\\Temp\\md_convert_temp.html"
    with open(html_path, "w", encoding="utf-8") as f:
        f.write(full_html)

    try:
        html_to_pdf_edge(html_path, pdf_path)
    finally:
        try:
            os.remove(html_path)
        except OSError:
            pass


def main():
    os.makedirs(PDF_DIR, exist_ok=True)
    dirs = sorted(glob.glob(os.path.join(BASE_DIR, "[0-9]*")))
    count = 0
    for d in dirs:
        if not os.path.isdir(d):
            continue
        dirname = os.path.basename(d)
        md_file = os.path.join(d, dirname + ".md")
        if not os.path.exists(md_file):
            print(f"  SKIP: {md_file} not found")
            continue
        pdf_file = os.path.join(PDF_DIR, dirname + ".pdf")
        print(f"  Converting: {dirname}.md -> {dirname}.pdf ...", end=" ", flush=True)
        try:
            convert(md_file, pdf_file)
            size_kb = os.path.getsize(pdf_file) / 1024
            print(f"OK ({size_kb:.0f} KB)")
            count += 1
        except Exception as e:
            print(f"FAIL: {e}")
    print(f"\nDone! {count} PDFs generated in {PDF_DIR}")


if __name__ == "__main__":
    main()
