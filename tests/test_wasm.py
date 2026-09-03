#!/usr/bin/env python3
"""Load web/index.html in a real browser, run a BASIC program, and check the
output against tests/smoke.expected.

Why a browser and not node: the page is the product. mbasic.js is built with
-s ENVIRONMENT='web' -s EXPORT_ES6=1, so it is an ES6 module that expects a
DOM, and the UI drives the interpreter through EM_JS bindings that touch
document. Loading the .wasm outside a page would test something nobody runs.

Why a server and not file://: an ES6 module fetched from a file:// origin is
blocked by CORS, and so is the .wasm alongside it. The page silently renders
empty, which looks like a broken build rather than a broken test setup.

smoke.expected is the output of the NATIVE interpreter for the same program
(../mbasicc/build/mbasicc tests/smoke.bas), so this asserts the wasm agrees
with the C++ build rather than merely that it printed something. That is the
distinction that matters: a toolchain change is most likely to show up in
float formatting or MBASIC's leading-space number convention, and a test that
only checked for "some output" would sail past both.

    make test          # or: python3 tests/test_wasm.py

Skips (exit 0) when playwright is not installed, so an unattended sweep on a
machine without it does not fail; run it by hand there.
"""
import functools
import http.server
import socketserver
import sys
import threading
from pathlib import Path

HERE = Path(__file__).resolve().parent
WEB = HERE.parent / "web"
LOAD_MS = 4000    # module fetch + instantiate
RUN_MS = 6000     # program execution


def main() -> int:
    try:
        from playwright.sync_api import sync_playwright
    except ImportError:
        print("SKIP: playwright is not installed "
              "(pip install playwright && playwright install chromium)")
        return 0

    if not (WEB / "mbasic.wasm").exists():
        print(f"FAIL: {WEB}/mbasic.wasm is missing; run `make` first")
        return 1

    program = (HERE / "smoke.bas").read_text()
    expected = (HERE / "smoke.expected").read_text().strip()

    handler = functools.partial(http.server.SimpleHTTPRequestHandler,
                                directory=str(WEB))

    class Quiet(handler.func):
        def log_message(self, *args):
            pass

    # Port 0, not a fixed one: the kernel picks a free port and hands back
    # which. A hardcoded port makes two runs in a row fail with EADDRINUSE,
    # because the listening socket lingers after serve_forever stops — and it
    # collides with anything else already on that port.
    class Server(socketserver.TCPServer):
        allow_reuse_address = True

    httpd = Server(("127.0.0.1", 0), functools.partial(Quiet, directory=str(WEB)))
    port = httpd.server_address[1]
    threading.Thread(target=httpd.serve_forever, daemon=True).start()

    errors: list[str] = []
    try:
        with sync_playwright() as p:
            browser = p.chromium.launch()
            page = browser.new_page()
            page.on("pageerror", lambda e: errors.append(str(e)))
            page.on("console",
                    lambda m: errors.append(f"console {m.type}: {m.text}")
                    if m.type == "error" else None)
            page.goto(f"http://127.0.0.1:{port}/index.html", wait_until="load")
            page.wait_for_timeout(LOAD_MS)

            # Everything printed before the program runs is the banner; the
            # program's own output is whatever the RUN appends after it.
            banner = page.inner_text("#output")
            page.fill("#editor", program)
            page.click("#btn-run")
            page.wait_for_timeout(RUN_MS)
            produced = page.inner_text("#output")[len(banner):]
            browser.close()
    finally:
        httpd.shutdown()
        httpd.server_close()   # release the listening socket, not just the loop

    if errors:
        print("FAIL: the page reported errors")
        for e in errors[:10]:
            print("   ", e)
        return 1

    # Compare the lines the program printed. The trailing "Ok" prompt and the
    # blank lines around it are the REPL's, not the program's.
    got = "\n".join(ln for ln in produced.strip().splitlines()
                    if ln.strip() and ln.strip() != "Ok")
    if got != expected:
        print("FAIL: wasm output does not match the native interpreter")
        print("--- expected ---")
        print(expected)
        print("--- got ---")
        print(got)
        return 1

    print("PASS: wasm output matches the native interpreter "
          f"({len(expected.splitlines())} lines)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
