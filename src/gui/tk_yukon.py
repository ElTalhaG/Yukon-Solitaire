from __future__ import annotations

import pathlib
import subprocess
import sys
import tkinter as tk
from tkinter import filedialog, messagebox


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


def main() -> int:
    try:
        bridge = BackendBridge(sys.argv[1] if len(sys.argv) > 1 else None)
    except Exception as exc:
        print(f"Failed to start GUI bridge: {exc}", file=sys.stderr)
        return 1

    root = tk.Tk()
    app = YukonGui(root, bridge)

    def on_close() -> None:
        bridge.close()
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_close)
    root.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
