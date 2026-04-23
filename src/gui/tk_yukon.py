from __future__ import annotations

import json
import pathlib
import subprocess
import sys
import threading
import webbrowser
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

try:
    import tkinter as tk
    from tkinter import filedialog, messagebox

    TK_IMPORT_ERROR = None
except Exception as exc:  # pragma: no cover - this path depends on local GUI setup
    tk = None
    filedialog = None
    messagebox = None
    TK_IMPORT_ERROR = exc


class BackendBridge:
    def __init__(self, bridge_path: str | None = None) -> None:
        self.bridge_path = bridge_path or self._guess_bridge_path()
        self.process = subprocess.Popen(
            [self.bridge_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
        )
        self.state = self._read_state()

    def _guess_bridge_path(self) -> str:
        root = pathlib.Path(__file__).resolve().parents[2]
        candidates = [
            root / "cmake-build-debug" / "yukon_gui_bridge",
            root / "cmake-build-release" / "yukon_gui_bridge",
            root / "yukon_gui_bridge",
        ]

        for candidate in candidates:
            if candidate.exists():
                return str(candidate)

        raise FileNotFoundError(
            "Could not find yukon_gui_bridge. Build it first, then start the GUI again."
        )

    def _read_state(self) -> dict:
        if self.process.stdout is None:
            raise RuntimeError("Bridge stdout is not available.")

        state = {
            "phase": "STARTUP",
            "show_all": False,
            "last_command": "",
            "message": "",
            "deck_cards": [],
            "foundations": {},
            "tableau": {index: [] for index in range(7)},
        }

        while True:
            line = self.process.stdout.readline()
            if line == "":
                raise RuntimeError("Bridge closed unexpectedly.")

            line = line.rstrip("\n")
            if line == "BEGIN_STATE":
                break

        while True:
            line = self.process.stdout.readline()
            if line == "":
                raise RuntimeError("Bridge closed unexpectedly.")

            line = line.rstrip("\n")
            if line == "END_STATE":
                return state

            parts = line.split("\t")
            tag = parts[0]

            if tag == "PHASE":
                state["phase"] = parts[1]
            elif tag == "SHOW_ALL":
                state["show_all"] = parts[1] == "1"
            elif tag == "LAST_COMMAND":
                state["last_command"] = parts[1] if len(parts) > 1 else ""
            elif tag == "MESSAGE":
                state["message"] = parts[1] if len(parts) > 1 else ""
            elif tag == "DECK_CARD":
                state["deck_cards"].append(
                    {"index": int(parts[1]), "code": parts[2], "face_up": parts[3] == "1"}
                )
            elif tag == "FOUNDATION":
                state["foundations"][int(parts[1])] = {
                    "size": int(parts[2]),
                    "assigned": parts[3] == "1",
                    "suit": int(parts[4]),
                    "top_code": parts[5],
                }
            elif tag == "TABLEAU_CARD":
                state["tableau"][int(parts[1])].append(
                    {"row": int(parts[2]), "code": parts[3], "face_up": parts[4] == "1"}
                )

    def send_command(self, command: str) -> dict:
        if self.process.stdin is None:
            raise RuntimeError("Bridge stdin is not available.")

        self.process.stdin.write(command + "\n")
        self.process.stdin.flush()
        self.state = self._read_state()
        return self.state

    def close(self) -> None:
        if self.process.stdin is not None:
            try:
                self.process.stdin.write("__QUIT__\n")
                self.process.stdin.flush()
            except OSError:
                pass

        self.process.terminate()


if tk is not None:

    class YukonGui:
        STARTUP_COLUMN_CARD_COUNTS = [1, 6, 7, 8, 9, 10, 11]

        def __init__(self, root: tk.Tk, bridge: BackendBridge) -> None:
            self.root = root
            self.bridge = bridge
            self.root.title("Yukon Solitaire")
            self.root.geometry("1420x900")
            self.root.configure(bg="#1c4a2a")

            self.status_var = tk.StringVar()
            self.phase_var = tk.StringVar()
            self.last_command_var = tk.StringVar()
            self.command_var = tk.StringVar()
            self.last_popup_message = ""

            self._build_toolbar()
            self._build_content()
            self._build_status()
            self.refresh()

        def _build_toolbar(self) -> None:
            toolbar = tk.Frame(self.root, bg="#13351d", padx=10, pady=10)
            toolbar.pack(fill="x")

            buttons = [
                ("Load Default", lambda: self.run_command("LD")),
                ("Load File", self.load_file),
                ("Show Deck", lambda: self.run_command("SW")),
                ("Interleave", lambda: self.run_command("SI")),
                ("Random Shuffle", lambda: self.run_command("SR")),
                ("Save Deck", self.save_file),
                ("Start Game", lambda: self.run_command("P")),
                ("Quit Game", lambda: self.run_command("Q")),
                ("Refresh", lambda: self.refresh()),
            ]

            for label, callback in buttons:
                tk.Button(
                    toolbar,
                    text=label,
                    command=callback,
                    bg="#e8d8b8",
                    relief="raised",
                    padx=10,
                ).pack(side="left", padx=4)

            tk.Entry(toolbar, textvariable=self.command_var, width=28).pack(side="left", padx=10)
            tk.Button(toolbar, text="Send Command", command=self.send_manual_command).pack(side="left", padx=4)

        def _build_content(self) -> None:
            content = tk.Frame(self.root, bg="#1c4a2a")
            content.pack(fill="both", expand=True, padx=12, pady=12)

            self.canvas = tk.Canvas(
                content,
                bg="#1c4a2a",
                highlightthickness=0,
                width=1040,
                height=700,
            )
            self.canvas.pack(side="left", fill="both", expand=True)

            side_panel = tk.Frame(content, bg="#102918", width=280, padx=14, pady=14)
            side_panel.pack(side="right", fill="y", padx=(14, 0))
            side_panel.pack_propagate(False)

            tk.Label(
                side_panel,
                text="Quick Info",
                bg="#102918",
                fg="#f4f0e6",
                anchor="w",
                font=("Courier", 16, "bold"),
            ).pack(fill="x", pady=(0, 10))

            tk.Label(
                side_panel,
                textvariable=self.phase_var,
                bg="#274c77",
                fg="#f4f0e6",
                anchor="w",
                justify="left",
                padx=10,
                pady=8,
                font=("Courier", 12, "bold"),
            ).pack(fill="x", pady=(0, 8))

            tk.Label(
                side_panel,
                textvariable=self.last_command_var,
                bg="#1f3b2c",
                fg="#f4f0e6",
                anchor="w",
                justify="left",
                padx=10,
                pady=8,
                wraplength=240,
                font=("Courier", 11),
            ).pack(fill="x", pady=(0, 14))

            tips = (
                "Command examples\n"
                "LD\n"
                "SW\n"
                "SI 26\n"
                "SR\n"
                "P\n"
                "C1->F1\n"
                "C6:4H->C4\n"
                "F1->C3\n"
                "Q\n\n"
                "GUI note\n"
                "For now the GUI uses the same command language as the terminal version.\n"
                "That sounds simple, but honestly it is pretty useful while we test the backend."
            )

            tk.Label(
                side_panel,
                text=tips,
                bg="#102918",
                fg="#d9e4d0",
                anchor="nw",
                justify="left",
                wraplength=240,
                font=("Courier", 11),
            ).pack(fill="both", expand=True)

        def _build_status(self) -> None:
            self.status_label = tk.Label(
                self.root,
                textvariable=self.status_var,
                bg="#13351d",
                fg="#f4f0e6",
                anchor="w",
                justify="left",
                padx=12,
                pady=10,
                font=("Courier", 12),
            )
            self.status_label.pack(fill="x")

        def load_file(self) -> None:
            filename = filedialog.askopenfilename(
                title="Load deck file",
                filetypes=[("Text files", "*.txt"), ("All files", "*.*")],
            )
            if filename:
                self.run_command(f"LD {filename}")

        def save_file(self) -> None:
            filename = filedialog.asksaveasfilename(
                title="Save deck file",
                defaultextension=".txt",
                filetypes=[("Text files", "*.txt"), ("All files", "*.*")],
            )
            if filename:
                self.run_command(f"SD {filename}")

        def send_manual_command(self) -> None:
            command = self.command_var.get().strip()
            if not command:
                return

            self.run_command(command)
            self.command_var.set("")

        def run_command(self, command: str) -> None:
            try:
                self.bridge.send_command(command)
                self.refresh()
            except Exception as exc:
                messagebox.showerror("Bridge Error", str(exc))

        def refresh(self) -> None:
            state = self.bridge.state
            self.canvas.delete("all")

            self._draw_title(state)
            self._draw_foundations(state)

            if state["phase"] == "STARTUP":
                self._draw_startup_deck(state)
            else:
                self._draw_tableau(state)

            self.phase_var.set(f"Phase: {state['phase']}\nDeck cards: {len(state['deck_cards'])}")
            self.last_command_var.set(f"Last Command:\n{state['last_command'] or '(none yet)'}")
            self.status_var.set(f"Message: {state['message'] or '(no message yet)'}")
            self._apply_status_style(state)
            self._show_popup_feedback(state)

        def _apply_status_style(self, state: dict) -> None:
            message = state["message"]

            if "won" in message.lower():
                background = "#365b2c"
            elif "invalid" in message.lower() or "not available" in message.lower():
                background = "#6b2d2d"
            elif message == "OK":
                background = "#234b3a"
            else:
                background = "#13351d"

            self.status_label.configure(bg=background)

        def _show_popup_feedback(self, state: dict) -> None:
            message = state["message"]

            # We only pop on the game-won message because invalid-move popups every
            # single time get annoying really fast. The red status bar is enough there.
            if "won" in message.lower() and message != self.last_popup_message:
                self.last_popup_message = message
                messagebox.showinfo("You won", message)
            elif "won" not in message.lower():
                self.last_popup_message = ""

        def _draw_title(self, state: dict) -> None:
            self.canvas.create_text(
                20,
                20,
                anchor="nw",
                text=f"Yukon Solitaire GUI  |  Phase: {state['phase']}",
                fill="#f4f0e6",
                font=("Courier", 18, "bold"),
            )
            self.canvas.create_text(
                20,
                48,
                anchor="nw",
                text="Shared C backend + Tkinter frontend",
                fill="#b8c7b4",
                font=("Courier", 11),
            )

        def _draw_card(self, x: int, y: int, code: str, face_up: bool) -> None:
            if face_up:
                fill = "#f9f4ea"
                outline = "#2a2a2a"
                text = code
                text_color = "#9a1f1f" if code.endswith(("H", "D")) else "#111111"
            else:
                fill = "#415a77"
                outline = "#223447"
                text = "[ ]"
                text_color = "#f4f0e6"

            self.canvas.create_rectangle(x, y, x + 78, y + 34, fill=fill, outline=outline, width=2)
            self.canvas.create_text(
                x + 39,
                y + 17,
                text=text,
                fill=text_color,
                font=("Courier", 13, "bold"),
            )

        def _draw_foundations(self, state: dict) -> None:
            start_x = 820
            y = 80

            for foundation_index in range(4):
                foundation = state["foundations"].get(
                    foundation_index,
                    {"size": 0, "assigned": False, "suit": -1, "top_code": "-"},
                )
                x = start_x
                top_code = foundation["top_code"]

                self.canvas.create_text(
                    x + 50,
                    y - 18,
                    text=f"F{foundation_index + 1}",
                    fill="#f4f0e6",
                    font=("Courier", 14, "bold"),
                )

                if top_code == "-":
                    self._draw_card(x, y, "[]", False)
                else:
                    self._draw_card(x, y, top_code, True)

                self.canvas.create_text(
                    x + 120,
                    y + 17,
                    text=f"size={foundation['size']}",
                    fill="#f4f0e6",
                    anchor="w",
                    font=("Courier", 11),
                )
                if foundation["assigned"]:
                    self.canvas.create_text(
                        x + 120,
                        y + 34,
                        text=f"suit={foundation['top_code'][-1]}",
                        fill="#b8c7b4",
                        anchor="w",
                        font=("Courier", 10),
                    )
                y += 105

        def _draw_startup_deck(self, state: dict) -> None:
            deck_cards = state["deck_cards"]
            x_positions = [40, 150, 260, 370, 480, 590, 700]
            y_start = 92
            deck_position = 0

            for previous_row in range(max(self.STARTUP_COLUMN_CARD_COUNTS)):
                for column_index, column_size in enumerate(self.STARTUP_COLUMN_CARD_COUNTS):
                    if previous_row < column_size:
                        if deck_position < len(deck_cards):
                            card = deck_cards[deck_position]
                            code = card["code"] if state["show_all"] else "[ ]"
                            face_up = state["show_all"]
                            self._draw_card(
                                x_positions[column_index],
                                y_start + previous_row * 50,
                                code,
                                face_up,
                            )
                        deck_position += 1

            for column_index in range(7):
                self.canvas.create_text(
                    x_positions[column_index] + 39,
                    72,
                    text=f"C{column_index + 1}",
                    fill="#f4f0e6",
                    font=("Courier", 14, "bold"),
                )

            if not deck_cards:
                self.canvas.create_text(
                    60,
                    120,
                    anchor="nw",
                    text="No deck loaded yet.\nUse Load Default or Load File to begin.",
                    fill="#d9e4d0",
                    font=("Courier", 13),
                )

        def _draw_tableau(self, state: dict) -> None:
            x_positions = [40, 150, 260, 370, 480, 590, 700]
            y_start = 92

            for column_index in range(7):
                self.canvas.create_text(
                    x_positions[column_index] + 39,
                    72,
                    text=f"C{column_index + 1}",
                    fill="#f4f0e6",
                    font=("Courier", 14, "bold"),
                )

                for row_index, card in enumerate(state["tableau"].get(column_index, [])):
                    self._draw_card(
                        x_positions[column_index],
                        y_start + row_index * 50,
                        card["code"],
                        card["face_up"],
                    )


WEB_PAGE = """<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Yukon Solitaire</title>
  <style>
    :root {
      --table: #1c4a2a;
      --panel: #102918;
      --panel2: #13351d;
      --ink: #f4f0e6;
      --muted: #c7d4c0;
      --paper: #f9f4ea;
      --back: #415a77;
      --red: #9a1f1f;
      --line: #223447;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: "Courier New", monospace;
      background: radial-gradient(circle at top, #2a6a3d 0%, var(--table) 40%, #173822 100%);
      color: var(--ink);
    }
    .app { padding: 18px; }
    .toolbar, .statusbar {
      display: flex;
      flex-wrap: wrap;
      gap: 8px;
      padding: 12px;
      border-radius: 14px;
      background: rgba(16, 41, 24, 0.9);
      backdrop-filter: blur(8px);
    }
    .toolbar { margin-bottom: 14px; }
    button, input {
      font: inherit;
      border-radius: 10px;
      border: none;
      padding: 10px 12px;
    }
    button {
      background: #e8d8b8;
      color: #1c1c1c;
      cursor: pointer;
    }
    input {
      min-width: 260px;
      background: #f6f1e8;
    }
    .layout {
      display: grid;
      grid-template-columns: minmax(0, 1fr) 280px;
      gap: 14px;
      margin-bottom: 14px;
    }
    .board, .side {
      background: rgba(16, 41, 24, 0.84);
      border-radius: 18px;
      padding: 18px;
      min-height: 720px;
    }
    .title {
      font-size: 28px;
      font-weight: 700;
      margin-bottom: 4px;
    }
    .subtitle {
      color: var(--muted);
      margin-bottom: 18px;
    }
    .foundations {
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      gap: 18px;
      margin-bottom: 22px;
    }
    .foundation {
      background: rgba(255,255,255,0.05);
      border-radius: 14px;
      padding: 10px;
      min-height: 120px;
    }
    .foundation-label, .column-label {
      font-weight: 700;
      margin-bottom: 8px;
    }
    .tableau, .startup {
      display: grid;
      grid-template-columns: repeat(7, minmax(80px, 1fr));
      gap: 12px;
      align-items: start;
    }
    .column {
      min-height: 520px;
    }
    .card {
      width: 78px;
      height: 34px;
      display: flex;
      align-items: center;
      justify-content: center;
      border-radius: 8px;
      border: 2px solid #2a2a2a;
      background: var(--paper);
      color: #111;
      margin-bottom: 10px;
      font-weight: 700;
      box-shadow: 0 8px 18px rgba(0,0,0,0.18);
    }
    .card.red { color: var(--red); }
    .card.back {
      background: var(--back);
      border-color: var(--line);
      color: var(--ink);
    }
    .sidebox {
      background: rgba(255,255,255,0.06);
      border-radius: 12px;
      padding: 12px;
      margin-bottom: 12px;
      white-space: pre-wrap;
    }
    .statusbar { align-items: center; }
    .hint { color: var(--muted); font-size: 14px; }
    .deck-path {
      min-width: 360px;
    }
    @media (max-width: 1100px) {
      .layout { grid-template-columns: 1fr; }
      .tableau, .startup { overflow-x: auto; }
    }
  </style>
</head>
<body>
  <div class="app">
    <div class="toolbar">
      <button onclick="runCommand('LD')">Load Default</button>
      <button onclick="runCommand('SW')">Show Deck</button>
      <button onclick="runCommand('SI')">Interleave</button>
      <button onclick="runCommand('SR')">Random Shuffle</button>
      <button onclick="runCommand('P')">Start Game</button>
      <button onclick="runCommand('Q')">Quit Game</button>
      <button onclick="refreshState()">Refresh</button>
      <input id="deckPath" class="deck-path" placeholder="deck file path or save path, like /tmp/deck.txt">
      <button onclick="loadFromPath()">Load File</button>
      <button onclick="saveToPath()">Save Deck</button>
      <input id="manualCommand" placeholder="type a command like C1->F1">
      <button onclick="sendManual()">Send Command</button>
    </div>

    <div class="layout">
      <div class="board">
        <div class="title">Yukon Solitaire</div>
        <div class="subtitle">Shared C backend + browser fallback frontend</div>
        <div id="foundations" class="foundations"></div>
        <div id="playArea"></div>
      </div>

      <div class="side">
        <div class="sidebox" id="phaseBox">Phase: STARTUP</div>
        <div class="sidebox" id="lastCommandBox">Last Command:\\n(none yet)</div>
        <div class="sidebox">
Command examples
LD
SW
SI 26
SR
P
C1->F1
C6:4H->C4
F1->C3
Q
        </div>
        <div class="sidebox hint">
This page exists because Tk is broken on this machine right now, but the backend is still the exact same C backend.
        </div>
      </div>
    </div>

    <div id="statusBar" class="statusbar">Message: waiting for first state...</div>
  </div>

  <script>
    function cardHtml(code, faceUp) {
      if (!faceUp) {
        return '<div class="card back">[ ]</div>';
      }
      const red = code.endsWith('H') || code.endsWith('D');
      return `<div class="card ${red ? 'red' : ''}">${code}</div>`;
    }

    function foundationHtml(index, foundation) {
      const top = foundation.top_code === '-' ? cardHtml('[ ]', false) : cardHtml(foundation.top_code, true);
      const suitLine = foundation.assigned ? `Suit: ${foundation.top_code.slice(-1)}` : 'Suit: -';
      return `
        <div class="foundation">
          <div class="foundation-label">F${index + 1}</div>
          ${top}
          <div class="hint">Size: ${foundation.size}</div>
          <div class="hint">${suitLine}</div>
        </div>
      `;
    }

    function renderStartup(state) {
      const counts = [1, 6, 7, 8, 9, 10, 11];
      let deckPos = 0;
      let html = '<div class="startup">';
      for (let col = 0; col < 7; col += 1) {
        html += `<div class="column"><div class="column-label">C${col + 1}</div>`;
        for (let row = 0; row < counts[col]; row += 1) {
          const card = state.deck_cards[deckPos];
          if (card) {
            html += cardHtml(card.code, state.show_all);
          }
          deckPos += 1;
        }
        html += '</div>';
      }
      html += '</div>';
      if (!state.deck_cards.length) {
        html += '<p class="hint">No deck loaded yet. Use Load Default or Load File to begin.</p>';
      }
      return html;
    }

    function renderPlay(state) {
      let html = '<div class="tableau">';
      for (let col = 0; col < 7; col += 1) {
        html += `<div class="column"><div class="column-label">C${col + 1}</div>`;
        const cards = state.tableau[col] || [];
        cards.forEach((card) => {
          html += cardHtml(card.code, card.face_up);
        });
        html += '</div>';
      }
      html += '</div>';
      return html;
    }

    function applyStatusStyle(message) {
      const statusBar = document.getElementById('statusBar');
      let background = '#13351d';

      if (message.toLowerCase().includes('won')) {
        background = '#365b2c';
      } else if (message.toLowerCase().includes('invalid') || message.toLowerCase().includes('not available')) {
        background = '#6b2d2d';
      } else if (message === 'OK') {
        background = '#234b3a';
      }

      statusBar.style.background = background;
    }

    async function refreshState() {
      const response = await fetch('/state');
      const state = await response.json();
      renderState(state);
    }

    function renderState(state) {
      const foundations = document.getElementById('foundations');
      const playArea = document.getElementById('playArea');

      foundations.innerHTML = [0, 1, 2, 3]
        .map((index) => foundationHtml(index, state.foundations[index] || {size: 0, assigned: false, top_code: '-'}))
        .join('');

      playArea.innerHTML = state.phase === 'STARTUP' ? renderStartup(state) : renderPlay(state);

      document.getElementById('phaseBox').textContent = `Phase: ${state.phase}\nDeck cards: ${state.deck_cards.length}`;
      document.getElementById('lastCommandBox').textContent = `Last Command:\n${state.last_command || '(none yet)'}`;
      document.getElementById('statusBar').textContent = `Message: ${state.message || '(no message yet)'}`;
      applyStatusStyle(state.message || '');
    }

    async function runCommand(command) {
      const response = await fetch('/command', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({command})
      });
      const state = await response.json();
      renderState(state);
    }

    function sendManual() {
      const input = document.getElementById('manualCommand');
      const command = input.value.trim();
      if (!command) {
        return;
      }
      runCommand(command);
      input.value = '';
    }

    function loadFromPath() {
      const path = document.getElementById('deckPath').value.trim();
      if (path) {
        runCommand(`LD ${path}`);
      }
    }

    function saveToPath() {
      const path = document.getElementById('deckPath').value.trim();
      if (path) {
        runCommand(`SD ${path}`);
      }
    }

    refreshState();
  </script>
</body>
</html>
"""


class WebGuiServer:
    def __init__(self, bridge: BackendBridge) -> None:
        self.bridge = bridge
        self.server = ThreadingHTTPServer(("127.0.0.1", 0), self._make_handler())
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)

    def _make_handler(self):
        bridge = self.bridge

        class YukonRequestHandler(BaseHTTPRequestHandler):
            def do_GET(self) -> None:
                if self.path == "/":
                    payload = WEB_PAGE.encode("utf-8")
                    self.send_response(HTTPStatus.OK)
                    self.send_header("Content-Type", "text/html; charset=utf-8")
                    self.send_header("Content-Length", str(len(payload)))
                    self.end_headers()
                    self.wfile.write(payload)
                    return

                if self.path == "/state":
                    payload = json.dumps(bridge.state).encode("utf-8")
                    self.send_response(HTTPStatus.OK)
                    self.send_header("Content-Type", "application/json; charset=utf-8")
                    self.send_header("Content-Length", str(len(payload)))
                    self.end_headers()
                    self.wfile.write(payload)
                    return

                self.send_error(HTTPStatus.NOT_FOUND, "Not found")

            def do_POST(self) -> None:
                if self.path != "/command":
                    self.send_error(HTTPStatus.NOT_FOUND, "Not found")
                    return

                content_length = int(self.headers.get("Content-Length", "0"))
                body = self.rfile.read(content_length).decode("utf-8")
                payload = json.loads(body or "{}")
                command = str(payload.get("command", "")).strip()

                if command:
                    bridge.send_command(command)

                response = json.dumps(bridge.state).encode("utf-8")
                self.send_response(HTTPStatus.OK)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Content-Length", str(len(response)))
                self.end_headers()
                self.wfile.write(response)

            def log_message(self, format_string: str, *args) -> None:
                # The browser polls state and sends commands, so default HTTP logs
                # get noisy really fast. Keeping this quiet makes CLion output readable.
                return

        return YukonRequestHandler

    @property
    def url(self) -> str:
        host, port = self.server.server_address
        return f"http://{host}:{port}/"

    def start(self) -> None:
        self.thread.start()

    def close(self) -> None:
        self.server.shutdown()
        self.server.server_close()


def run_tk_gui(bridge: BackendBridge) -> int:
    root = tk.Tk()
    app = YukonGui(root, bridge)

    def on_close() -> None:
        bridge.close()
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_close)
    root.mainloop()
    return 0


def run_web_gui(bridge: BackendBridge, reason: str) -> int:
    server = WebGuiServer(bridge)
    server.start()

    print("Tkinter GUI could not start cleanly on this machine.", file=sys.stderr)
    print(f"Fallback reason: {reason}", file=sys.stderr)
    print(f"Browser GUI is running at: {server.url}", file=sys.stderr)

    # This is the pragmatic fallback. We still give you a real GUI, just in the browser.
    # The backend is the same C bridge, so the game logic path stays unchanged.
    webbrowser.open(server.url)

    try:
        while True:
            threading.Event().wait(1.0)
    except KeyboardInterrupt:
        server.close()
        bridge.close()
        return 0


def main() -> int:
    bridge_path = None

    for argument in sys.argv[1:]:
        if argument != "--web":
            bridge_path = argument
            break

    try:
        bridge = BackendBridge(bridge_path)
    except Exception as exc:
        print(f"Failed to start GUI bridge: {exc}", file=sys.stderr)
        return 1

    if "--web" in sys.argv:
        return run_web_gui(bridge, "forced by --web flag")

    if TK_IMPORT_ERROR is not None:
        return run_web_gui(bridge, f"tkinter import failed: {TK_IMPORT_ERROR}")

    # This machine currently crashes inside the Command Line Tools Python when
    # Tk tries to create a real window. It is not a normal Python exception,
    # it is a hard abort, so we must avoid that path before it even starts.
    if "CommandLineTools" in sys.executable or sys.executable == "/usr/bin/python3":
        return run_web_gui(
            bridge,
            "skipping Tk because the current macOS Command Line Tools Python "
            "is linked to a broken Tk runtime on this machine",
        )

    try:
        return run_tk_gui(bridge)
    except Exception as exc:  # pragma: no cover - this depends on local GUI runtime
        return run_web_gui(bridge, f"tkinter runtime failed: {exc}")


if __name__ == "__main__":
    raise SystemExit(main())
