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
        self.root.geometry("1260x860")
        self.root.configure(bg="#1c4a2a")

        self.status_var = tk.StringVar()
        self.command_var = tk.StringVar()

        self._build_toolbar()
        self._build_canvas()
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
            ("Start Game", lambda: self.run_command("P")),
            ("Quit Game", lambda: self.run_command("Q")),
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

    def _build_canvas(self) -> None:
        self.canvas = tk.Canvas(
            self.root,
            bg="#1c4a2a",
            highlightthickness=0,
            width=1220,
            height=700,
        )
        self.canvas.pack(fill="both", expand=True, padx=12, pady=12)

    def _build_status(self) -> None:
        status = tk.Label(
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
        status.pack(fill="x")

    def load_file(self) -> None:
        filename = filedialog.askopenfilename(
            title="Load deck file",
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")],
        )
        if filename:
            self.run_command(f"LD {filename}")

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

        self.status_var.set(
            f"Phase: {state['phase']}    Last Command: {state['last_command']}    Message: {state['message']}"
        )

    def _draw_title(self, state: dict) -> None:
        self.canvas.create_text(
            20,
            20,
            anchor="nw",
            text=f"Yukon Solitaire GUI  |  Phase: {state['phase']}",
            fill="#f4f0e6",
            font=("Courier", 18, "bold"),
        )

    def _draw_card(self, x: int, y: int, code: str, face_up: bool) -> None:
        if face_up:
            fill = "#f9f4ea"
            outline = "#2a2a2a"
            text = code
            text_color = "#111111"
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
            y += 105

    def _draw_startup_deck(self, state: dict) -> None:
        deck_cards = state["deck_cards"]
        x_positions = [40, 150, 260, 370, 480, 590, 700]
        y_start = 80
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
                            y_start + previous_row * 48,
                            code,
                            face_up,
                        )
                    deck_position += 1

        for column_index in range(7):
            self.canvas.create_text(
                x_positions[column_index] + 39,
                58,
                text=f"C{column_index + 1}",
                fill="#f4f0e6",
                font=("Courier", 14, "bold"),
            )

    def _draw_tableau(self, state: dict) -> None:
        x_positions = [40, 150, 260, 370, 480, 590, 700]
        y_start = 80

        for column_index in range(7):
            self.canvas.create_text(
                x_positions[column_index] + 39,
                58,
                text=f"C{column_index + 1}",
                fill="#f4f0e6",
                font=("Courier", 14, "bold"),
            )

            for row_index, card in enumerate(state["tableau"].get(column_index, [])):
                self._draw_card(
                    x_positions[column_index],
                    y_start + row_index * 48,
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
