import json
import os
import subprocess
import tkinter as tk
from tkinter import messagebox, ttk


SCENARIOS = {
    "1 - Ball Tracking": 1,
    "2 - Fast Ball Motion": 2,
    "3 - Goal Zone": 3,
    "4 - Temporary Occlusion": 4,
    "5 - Switch Margin": 5,
    "6 - Mini Match Simulation": 6,
}


class SoccerVisualizer:
    def __init__(self, root):
        self.root = root
        self.root.title("EagleEye Soccer Foveated Vision - PC Simulation")
        self.root.geometry("1240x760")
        self.root.minsize(1000, 650)
        self.frames = []
        self.index = 0
        self.playing = False
        self.after_id = None
        self.speed_ms = 180
        self.trails = {}
        self.consistency_error = None

        self.scenario_var = tk.StringVar(value=list(SCENARIOS)[0])
        self.speed_var = tk.StringVar(value="Normal 1x")
        self.frame_var = tk.StringVar(value="Frame: -")
        self.status_var = tk.StringVar(value="Paused")
        self.info_var = tk.StringVar(value="Load a scenario to begin.")

        controls = ttk.Frame(root, padding=8)
        controls.pack(fill=tk.X)
        ttk.Label(controls, text="Scenario").pack(side=tk.LEFT)
        scenario_box = ttk.Combobox(
            controls, textvariable=self.scenario_var, values=list(SCENARIOS),
            state="readonly", width=24
        )
        scenario_box.pack(side=tk.LEFT, padx=(6, 12))
        scenario_box.bind("<<ComboboxSelected>>", self.change_scenario)
        ttk.Button(controls, text="Start / Play", command=self.play).pack(side=tk.LEFT, padx=3)
        ttk.Button(controls, text="Pause", command=self.pause).pack(side=tk.LEFT, padx=3)
        ttk.Button(controls, text="Next Frame", command=self.next_frame).pack(side=tk.LEFT, padx=3)
        ttk.Button(controls, text="Restart", command=self.restart).pack(side=tk.LEFT, padx=3)
        ttk.Label(controls, text="Speed").pack(side=tk.LEFT, padx=(18, 6))
        speed_box = ttk.Combobox(
            controls, textvariable=self.speed_var,
            values=("Slow 0.5x", "Normal 1x", "Fast 2x"),
            state="readonly", width=13
        )
        speed_box.pack(side=tk.LEFT)
        speed_box.bind("<<ComboboxSelected>>", self.change_speed)
        ttk.Label(controls, textvariable=self.frame_var).pack(side=tk.RIGHT)

        body = ttk.Frame(root, padding=(8, 0, 8, 8))
        body.pack(fill=tk.BOTH, expand=True)
        left = ttk.LabelFrame(body, text="Global Match View")
        left.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 6))
        self.global_canvas = tk.Canvas(left, background="#1f7a45", highlightthickness=0)
        self.global_canvas.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)

        right = ttk.Frame(body, width=390)
        right.pack(side=tk.RIGHT, fill=tk.Y)
        right.pack_propagate(False)
        fovea_box = ttk.LabelFrame(right, text="2x Fovea View - Simulation / PC Demo")
        fovea_box.pack(fill=tk.X, pady=(0, 6))
        self.fovea_canvas = tk.Canvas(fovea_box, width=360, height=260,
                                      background="#17202a", highlightthickness=0)
        self.fovea_canvas.pack(fill=tk.X, padx=6, pady=6)

        info_box = ttk.LabelFrame(right, text="Attention / Motion Prediction")
        info_box.pack(fill=tk.BOTH, expand=True)
        self.info_text = tk.Text(info_box, width=42, height=22, state=tk.DISABLED,
                                 font=("Consolas", 10), relief=tk.FLAT)
        self.info_text.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)
        ttk.Label(root, textvariable=self.info_var, anchor=tk.W).pack(fill=tk.X, padx=10, pady=(0, 8))

        self.load_scenario()
        self.root.protocol("WM_DELETE_WINDOW", self.close)

    def bridge_path(self):
        return os.path.join(os.path.dirname(__file__), "build", "visual_demo_bridge.exe")

    def load_scenario(self):
        path = self.bridge_path()
        if not os.path.exists(path):
            messagebox.showerror(
                "Visualizer bridge missing",
                "找不到 tools/build/visual_demo_bridge.exe。\n"
                "请先双击运行 build_visualizer_bridge.bat。"
            )
            return
        scenario = SCENARIOS[self.scenario_var.get()]
        try:
            result = subprocess.run([path, str(scenario)], check=True,
                                    capture_output=True, text=True)
            self.frames = [json.loads(line) for line in result.stdout.splitlines() if line.strip()]
        except (OSError, subprocess.CalledProcessError, json.JSONDecodeError) as exc:
            messagebox.showerror("Visualizer data error", str(exc))
            self.frames = []
        self.index = 0
        self.trails = {}
        self.consistency_error = None
        self.pause()
        self.render()

    def change_scenario(self, _event=None):
        self.load_scenario()

    def change_speed(self, _event=None):
        self.speed_ms = {"Slow 0.5x": 360, "Normal 1x": 180, "Fast 2x": 90}[self.speed_var.get()]

    def play(self):
        if not self.frames:
            return
        self.playing = True
        self.status_var.set("Playing")
        self.schedule_next()

    def pause(self):
        self.playing = False
        self.status_var.set("Paused")
        if self.after_id is not None:
            self.root.after_cancel(self.after_id)
            self.after_id = None

    def schedule_next(self):
        if self.playing:
            self.after_id = self.root.after(self.speed_ms, self.advance)

    def advance(self):
        self.after_id = None
        if not self.playing:
            return
        if self.index < len(self.frames) - 1:
            self.index += 1
            self.render()
            self.schedule_next()
        else:
            self.pause()

    def next_frame(self):
        self.pause()
        if self.frames and self.index < len(self.frames) - 1:
            self.index += 1
            self.render()

    def restart(self):
        self.pause()
        self.index = 0
        self.trails = {}
        self.consistency_error = None
        self.render()

    def scale(self, canvas, x, y):
        width = max(canvas.winfo_width(), 1)
        height = max(canvas.winfo_height(), 1)
        return x * width / 640.0, y * height / 360.0

    def draw_field(self):
        c = self.global_canvas
        c.delete("all")
        w, h = max(c.winfo_width(), 1), max(c.winfo_height(), 1)
        c.create_rectangle(2, 2, w - 2, h - 2, outline="white", width=3)
        c.create_line(w / 2, 2, w / 2, h - 2, fill="white", width=2)
        c.create_oval(w * .39, h * .28, w * .61, h * .72, outline="white", width=2)
        c.create_rectangle(2, h * .28, w * .11, h * .72, outline="white", width=2)
        c.create_rectangle(w * .89, h * .28, w - 2, h * .72, outline="white", width=2)
        c.create_text(12, 14, anchor=tk.NW, text="GLOBAL MATCH VIEW  640 x 360",
                      fill="white", font=("Arial", 10, "bold"))

    def draw_trails(self):
        c = self.global_canvas
        selected_id = self.current["selected"]
        for target_id, points in self.trails.items():
            if target_id != selected_id and target_id != 1:
                continue
            if len(points) < 2:
                continue
            color = "#fff3a6" if target_id == 1 else ("#f28b82" if target_id in (11, 17, 21, 24) else "#82b1ff")
            coords = []
            for x, y in points:
                px, py = self.scale(c, x, y)
                coords.extend((px, py))
            c.create_line(*coords, fill=color, width=2, dash=(3, 4))

    def draw_target(self, target, selected):
        c = self.global_canvas
        x, y = target["x"], target["y"]
        w, h = target["w"], target["h"]
        x1, y1 = self.scale(c, x, y)
        x2, y2 = self.scale(c, x + w, y + h)
        if target["target_type"] == "BALL":
            color = "#ffdf00"
        elif target.get("team") == "A":
            color = "#ff8a80"
        else:
            color = "#82b1ff"
        if selected:
            color = "#ffdf00"
        if target["target_type"] == "BALL":
            cx, cy = (x1 + x2) / 2, (y1 + y2) / 2
            r = max(5, min(x2 - x1, y2 - y1) / 2)
            c.create_oval(cx-r, cy-r, cx+r, cy+r, fill=color, outline="black", width=2)
        else:
            c.create_rectangle(x1, y1, x2, y2, outline=color, width=4 if selected else 2)
        label = "%s #%s" % (target["target_type"], target["target_id"])
        c.create_text(x1, max(24, y1 - 5), anchor=tk.SW, text=label,
                      fill=color, font=("Arial", 9, "bold"))
        if selected:
            c.create_rectangle(x1 - 5, y1 - 5, x2 + 5, y2 + 5,
                               outline="#ffdf00", width=3)

    def draw_prediction(self, frame):
        if not frame["prediction_valid"]:
            return
        c = self.global_canvas
        px, py = self.scale(c, frame["predicted_x"], frame["predicted_y"])
        c.create_oval(px - 9, py - 9, px + 9, py + 9,
                      outline="#ff75d1", width=2, dash=(4, 3))
        c.create_line(px - 12, py, px + 12, py, fill="#ff75d1")
        c.create_line(px, py - 12, px, py + 12, fill="#ff75d1")
        c.create_text(px + 12, py, anchor=tk.W, text="predicted",
                      fill="#ff75d1", font=("Arial", 9))

    def draw_roi(self, frame):
        if not frame["request_id"]:
            return
        c = self.global_canvas
        x1, y1 = self.scale(c, frame["roi_x"], frame["roi_y"])
        x2, y2 = self.scale(c, frame["roi_x"] + frame["roi_w"], frame["roi_y"] + frame["roi_h"])
        c.create_rectangle(x1, y1, x2, y2, outline="#ffdf00", width=2, dash=(6, 3))
        c.create_text(x1, y2 + 4, anchor=tk.NW,
                      text="FOVEA ROI -> #%s" % frame["selected"],
                      fill="#ffdf00", font=("Arial", 9, "bold"))
        if frame.get("ball_occluded"):
            c.create_text(c.winfo_width() - 12, 14, anchor=tk.NE,
                          text="BALL OCCLUDED", fill="#ff75d1",
                          font=("Arial", 11, "bold"))
        if "scenario_event" in frame:
            c.create_text(12, 32, anchor=tk.NW, text=frame["scenario_event"],
                          fill="#ffffff", font=("Arial", 10, "bold"))

    def draw_fovea(self, selected):
        c = self.fovea_canvas
        c.delete("all")
        c.create_text(10, 10, anchor=tk.NW, text="2x Fovea View",
                      fill="white", font=("Arial", 11, "bold"))
        c.create_text(10, 30, anchor=tk.NW, text="Simulation / PC Demo",
                      fill="#aab7c4", font=("Arial", 9))
        if not selected:
            c.create_text(180, 130, text="No selected target", fill="white")
            return
        cx, cy = 180, 145
        if selected["target_type"] == "BALL":
            c.create_oval(cx - 32, cy - 32, cx + 32, cy + 32,
                          fill="#ffdf00", outline="black", width=2)
        else:
            c.create_rectangle(cx - 35, cy - 65, cx + 35, cy + 65,
                               outline="#ffdf00", width=4)
            c.create_oval(cx - 20, cy - 85, cx + 20, cy - 45,
                          outline="#ffdf00", width=3)
            ball = next((target for target in self.current["targets"]
                         if target["target_type"] == "BALL" and target.get("valid", 1)), None)
            if ball:
                bx = cx + max(-70, min(70, (ball["x"] - selected["x"]) * 0.8))
                by = cy + max(-70, min(70, (ball["y"] - selected["y"]) * 0.8))
                c.create_oval(bx - 9, by - 9, bx + 9, by + 9,
                              fill="#ffffff", outline="#ffdf00", width=2)
                c.create_text(bx + 12, by, anchor=tk.W, text="BALL",
                              fill="#ffffff", font=("Arial", 8, "bold"))
        c.create_text(180, 245, text="%s #%s  |  2x display only" %
                      (selected["target_type"], selected["target_id"]),
                      fill="white", font=("Arial", 10))

    def write_info(self, frame, selected):
        self.info_text.configure(state=tk.NORMAL)
        self.info_text.delete("1.0", tk.END)
        lines = [
            "Frame ID        : %s" % frame["frame_id"],
            "Selected Target : %s" % frame["selected"],
            "Fovea View ID    : %s" % frame["fovea_view_target_id"],
            "Estimated Carrier: #%s  (%s/255)" %
            (frame.get("estimated_ball_carrier_id", "-"), frame.get("estimated_ball_carrier_score", 0)),
            "Focus State     : %s" % frame["focus_state"],
            "Match Event     : %s" % frame.get("scenario_event", frame.get("event", "-")),
            "Algorithm State : %s" % frame.get("algorithm_state", "-"),
            "Request ID      : %s" % frame["request_id"],
            "Challenger      : #%s" % frame.get("challenger_id", "-"),
            "Score Difference: %s" % frame.get("score_difference", "-"),
            "Fovea ROI       : (%s,%s,%s,%s)" %
            (frame["roi_x"], frame["roi_y"], frame["roi_w"], frame["roi_h"]),
            "",
        ]
        if selected:
            lines += [
                "Target Type     : %s" % selected["target_type"],
                "Team            : %s" % selected.get("team", "-"),
                "Ball Priority   : %s" % selected["ball_priority"],
                "Motion          : %s" % selected["motion"],
                "Goal Zone       : %s" % selected["goal_zone"],
                "Small Target    : %s" % selected["small_target"],
                "Uncertainty     : %s" % selected["uncertainty"],
                "Ball Proximity  : %s" % selected["ball_proximity"],
                "Possession      : %s/255" % selected.get("possession_score", 0),
                "Aging           : %s" % selected["aging"],
                "Total Attention : %s" % selected["attention"],
            ]
        else:
            lines.append("No current selected target")
        lines += [
            "",
            "Predicted X     : %s" % frame["predicted_x"],
            "Predicted Y     : %s" % frame["predicted_y"],
            "Prediction valid: %s" % bool(frame["prediction_valid"]),
            "Prediction src  : %s" % frame.get("prediction_source", "-"),
            "Switch Margin   : %s" % frame.get("switch_margin", "-"),
            "Hold Remaining  : %s" % frame.get("hold_remaining", "-"),
            "",
            "ATTENTION TOP 3",
            "PC demo only: 2x is a graphic enlargement,",
            "not FPGA Lanczos or super resolution.",
        ]
        ranked = sorted(frame["targets"], key=lambda target: target.get("attention", 0), reverse=True)[:3]
        top_lines = ["#%-3s %-6s %s" %
                     (target["target_id"], target.get("team", "-"), target["attention"])
                     for target in ranked]
        lines[lines.index("ATTENTION TOP 3") + 1:lines.index("PC demo only: 2x is a graphic enlargement,")] = top_lines
        self.info_text.insert("1.0", "\n".join(lines))
        self.info_text.configure(state=tk.DISABLED)

    def render(self):
        if not self.frames:
            self.draw_field()
            return
        self.current = self.frames[self.index]
        selected_id = self.current["selected"]
        fovea_id = self.current["fovea_view_target_id"]
        selected = next((target for target in self.current["targets"]
                         if target["target_id"] == selected_id and target.get("valid", 1)), None)
        if selected_id != fovea_id or (selected_id != 65535 and selected is None):
            self.consistency_error = "Selected Target / Fovea View mismatch: selected=%s fovea=%s" % (selected_id, fovea_id)
            self.pause()
            self.draw_field()
            self.global_canvas.create_text(20, 50, anchor=tk.NW,
                                           text="CONSISTENCY ERROR: " + self.consistency_error,
                                           fill="#ff5555", font=("Arial", 12, "bold"))
            messagebox.showerror("Visualizer consistency error", self.consistency_error)
            return
        self.consistency_error = None
        for target in self.current["targets"]:
            if target.get("valid", 1):
                points = self.trails.setdefault(target["target_id"], [])
                points.append((target["x"] + target["w"] / 2, target["y"] + target["h"] / 2))
                if len(points) > (8 if target["target_type"] == "BALL" else 5):
                    del points[:- (8 if target["target_type"] == "BALL" else 5)]
        self.draw_field()
        self.draw_trails()
        for target in self.current["targets"]:
            if target.get("valid", 1) and target.get("visible", 1):
                self.draw_target(target, target["target_id"] == selected_id)
        self.draw_roi(self.current)
        self.draw_prediction(self.current)
        self.draw_fovea(selected)
        self.write_info(self.current, selected)
        self.frame_var.set("Frame: %s    %s" % (self.current["frame_id"], self.status_var.get()))
        self.info_var.set("Scenario %s | Selected target and Attention data come from the C bridge." %
                          self.scenario_var.get())

    def close(self):
        self.pause()
        self.root.destroy()


if __name__ == "__main__":
    app = tk.Tk()
    SoccerVisualizer(app)
    app.mainloop()
