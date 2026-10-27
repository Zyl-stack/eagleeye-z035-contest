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
    def __init__(self, root, frames=None, scenario_name=None, start_frame=1,
                 autoplay=False, speed_ms=None):
        self.root = root
        self.root.title("EagleEye Soccer Foveated Vision - PC Simulation")
        self.root.geometry("1360x860")
        self.root.minsize(1100, 720)
        self.frames = frames or []
        self.external_frames = frames is not None
        self.index = 0
        self.initial_index = 0
        self.playing = False
        self.after_id = None
        self.speed_ms = speed_ms or 180
        self.trails = {}
        self.consistency_error = None
        self.recent_event = None
        self.recent_event_frames = 0

        self.scenario_var = tk.StringVar(value=scenario_name or list(SCENARIOS)[0])
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

        right = ttk.Frame(body, width=430)
        right.pack(side=tk.RIGHT, fill=tk.Y)
        right.pack_propagate(False)
        fovea_box = ttk.LabelFrame(right, text="2x Fovea View - Simulation / PC Demo")
        fovea_box.pack(fill=tk.X, pady=(0, 6))
        self.fovea_canvas = tk.Canvas(fovea_box, width=360, height=260,
                                      background="#17202a", highlightthickness=0)
        self.fovea_canvas.pack(fill=tk.X, padx=6, pady=6)

        decision_box = ttk.LabelFrame(right, text="Decision Panel - C Core Output")
        decision_box.pack(fill=tk.X, pady=(0, 6))
        self.decision_text = tk.Text(decision_box, width=46, height=15, state=tk.DISABLED,
                                     font=("Consolas", 9), relief=tk.FLAT)
        self.decision_text.pack(fill=tk.X, padx=6, pady=4)

        info_box = ttk.LabelFrame(right, text="Attention Breakdown - C Core Output")
        info_box.pack(fill=tk.X, pady=(0, 6))
        self.info_text = tk.Text(info_box, width=46, height=12, state=tk.DISABLED,
                                 font=("Consolas", 10), relief=tk.FLAT)
        self.info_text.pack(fill=tk.X, padx=6, pady=4)

        stats_box = ttk.LabelFrame(right, text="Statistics - Theoretical Pixel Workload")
        stats_box.pack(fill=tk.BOTH, expand=True)
        self.stats_text = tk.Text(stats_box, width=46, height=8, state=tk.DISABLED,
                                  font=("Consolas", 9), relief=tk.FLAT)
        self.stats_text.pack(fill=tk.BOTH, expand=True, padx=6, pady=4)
        ttk.Label(root, textvariable=self.info_var, anchor=tk.W).pack(fill=tk.X, padx=10, pady=(0, 8))

        if self.external_frames:
            self.set_frames(self.frames, start_frame)
        else:
            self.load_scenario()
        self.root.protocol("WM_DELETE_WINDOW", self.close)
        if autoplay:
            self.root.after(100, self.play)

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
            frames = [json.loads(line) for line in result.stdout.splitlines() if line.strip()]
        except (OSError, subprocess.CalledProcessError, json.JSONDecodeError) as exc:
            messagebox.showerror("Visualizer data error", str(exc))
            frames = []
        self.external_frames = False
        self.set_frames(frames, 1)

    def set_frames(self, frames, start_frame):
        self.frames = frames
        self.index = next((index for index, frame in enumerate(frames)
                           if frame["frame_id"] >= start_frame), 0)
        self.initial_index = self.index
        self.trails = {}
        self.consistency_error = None
        self.recent_event = None
        self.recent_event_frames = 0
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
        self.index = self.initial_index
        self.trails = {}
        self.consistency_error = None
        self.recent_event = None
        self.recent_event_frames = 0
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
        if frame.get("desired_roi_w", 0) and frame.get("desired_roi_h", 0):
            desired_x = frame["desired_roi_x"] + frame["desired_roi_w"] / 2
            desired_y = frame["desired_roi_y"] + frame["desired_roi_h"] / 2
            dx, dy = self.scale(c, desired_x, desired_y)
            c.create_line(dx - 5, dy, dx + 5, dy, fill="#ff75d1", width=2)
            c.create_line(dx, dy - 5, dx, dy + 5, fill="#ff75d1", width=2)
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
        c.create_text(10, 30, anchor=tk.NW, text="C ROI crop / PC demo only",
                      fill="#aab7c4", font=("Arial", 9))
        roi_x = self.current.get("roi_x", 0)
        roi_y = self.current.get("roi_y", 0)
        roi_w = self.current.get("roi_w", 0)
        roi_h = self.current.get("roi_h", 0)
        if not selected or not roi_w or not roi_h:
            c.create_text(180, 130, text="No valid Fovea request", fill="white")
            return
        left, top, right, bottom = 18, 52, 342, 232
        scale = min((right - left) / roi_w, (bottom - top) / roi_h)
        c.create_rectangle(left, top, right, bottom, fill="#1f7a45", outline="#ffdf00", width=2)
        for target in self.current["targets"]:
            if not target.get("valid", 1) or not target.get("visible", 1):
                continue
            if (target["x"] + target["w"] <= roi_x or target["x"] >= roi_x + roi_w or
                    target["y"] + target["h"] <= roi_y or target["y"] >= roi_y + roi_h):
                continue
            x1 = left + (target["x"] - roi_x) * scale
            y1 = top + (target["y"] - roi_y) * scale
            x2 = x1 + target["w"] * scale
            y2 = y1 + target["h"] * scale
            is_selected = target["target_id"] == selected["target_id"]
            color = "#ffdf00" if is_selected or target["target_type"] == "BALL" else "#d9e7ff"
            if target["target_type"] == "BALL":
                c.create_oval(x1, y1, x2, y2, fill=color, outline="black", width=2)
            else:
                c.create_rectangle(x1, y1, x2, y2, outline=color, width=3 if is_selected else 2)
            c.create_text(x1, y1 - 3, anchor=tk.SW, text="#%s" % target["target_id"],
                          fill=color, font=("Arial", 8, "bold"))
        c.create_text(180, 246, text="ROI=(%s,%s,%s,%s) | display crop only" %
                      (roi_x, roi_y, roi_w, roi_h), fill="white", font=("Arial", 9))

    def write_info(self, frame, selected):
        self.decision_text.configure(state=tk.NORMAL)
        self.decision_text.delete("1.0", tk.END)
        possession = frame.get("estimated_ball_carrier_id", 65535)
        decision_lines = [
            "Frame / Request  : %s / %s" % (frame["frame_id"], frame["request_id"]),
            "Attention Mode   : %s" % frame.get("attention_mode", "SOCCER_V2"),
            "Selected Target  : #%s" % frame["selected"],
            "Previous Target  : #%s" % frame.get("previous_selected", 65535),
            "Base Attention   : %s" % frame.get("base_attention", 0),
            "Feedback Influence: %+d" % frame.get("feedback_adjustment", 0),
            "Feedback Reason  : %s" % frame.get("feedback_attention_reason", "NO_FEEDBACK"),
            "Final Attention  : %s" % frame.get("final_attention", 0),
            "Possession       : %s" % ("#%s" % possession if possession != 65535 else "NONE"),
            "Switch Reason    : %s" % frame.get("switch_reason", "-"),
            "Director State   : %s" % frame.get("director_state", "-"),
            "Director Event   : %s" % frame.get("director_event", "-"),
            "ROI Motion       : %s" % frame.get("roi_motion_mode", "TRACK"),
            "Fast Jump Left   : %s" % frame.get("fast_jump_remaining", 0),
            "Fovea Feedback   : %s" % frame.get("feedback_status", "NONE"),
            "Feedback Request : %s" % frame.get("feedback_request_id", 0),
            "Feedback Target  : #%s" % frame.get("feedback_target_id", 65535),
            "Quality Flags    : 0x%08X" % frame.get("feedback_quality_flags", 0),
            "Enhance Mode     : %s" % frame.get("enhance_mode_name", "-"),
            "Enhance Reason   : %s" % frame.get("enhance_reason", "-"),
            "ROI              : (%s,%s,%s,%s)" %
            (frame["roi_x"], frame["roi_y"], frame["roi_w"], frame["roi_h"]),
        ]
        if self.recent_event and frame.get("director_event") == "NONE":
            decision_lines.append("Recent Event UI : %s" % self.recent_event)
        self.decision_text.insert("1.0", "\n".join(decision_lines))
        self.decision_text.configure(state=tk.DISABLED)

        self.info_text.configure(state=tk.NORMAL)
        self.info_text.delete("1.0", tk.END)
        lines = []
        if selected and frame.get("attention_mode") == "PLAYER_DRIVEN_V3":
            lines += [
                "Motion          : %s" % selected.get("v3_motion", 0),
                "Interaction     : %s" % selected.get("v3_interaction", 0),
                "Goal Zone       : %s" % selected.get("v3_goal_zone", 0),
                "Motion Saliency : %s" % selected.get("v3_motion_saliency", 0),
                "Uncertainty     : %s" % selected.get("v3_uncertainty", 0),
                "Aging           : %s" % selected.get("v3_aging", 0),
                "Base Attention  : %s" % selected.get("base_attention", 0),
                "Optional Ball   : %s" % selected.get("v3_optional_ball_bonus", 0),
                "Total Attention : %s" % selected.get("final_attention", selected["attention"]),
            ]
        elif selected:
            lines += [
                "Ball Priority   : %s" % selected["ball_priority"],
                "Motion          : %s" % selected["motion"],
                "Goal Zone       : %s" % selected["goal_zone"],
                "Small Target    : %s" % selected["small_target"],
                "Uncertainty     : %s" % selected["uncertainty"],
                "Ball Proximity  : %s" % selected["ball_proximity"],
                "Aging           : %s" % selected["aging"],
                "Base Attention  : %s" % selected.get("base_attention", 0),
                "Feedback        : %+d (%s)" %
                (selected.get("feedback_adjustment", 0),
                 selected.get("feedback_attention_reason", "NO_FEEDBACK")),
                "Final Attention : %s" % selected.get("final_attention", selected["attention"]),
            ]
        else:
            lines.append("No current selected target")
        self.info_text.insert("1.0", "\n".join(lines))
        self.info_text.configure(state=tk.DISABLED)

        self.stats_text.configure(state=tk.NORMAL)
        self.stats_text.delete("1.0", tk.END)
        stats_lines = [
            "Frames / Switches : %s / %s" %
            (frame.get("stats_frames", 0), frame.get("stats_switches", 0)),
            "Passes / Ball Lost: %s / %s" %
            (frame.get("stats_passes", 0), frame.get("stats_ball_lost", 0)),
            "Bypass             : %s" % frame.get("stats_bypass", 0),
            "Bilinear           : %s" % frame.get("stats_bilinear", 0),
            "Lanczos            : %s" % frame.get("stats_lanczos", 0),
            "Average Attention  : %.1f" % frame.get("average_attention", 0.0),
            "Average ROI Move   : %.1f px" % frame.get("average_roi_movement", 0.0),
            "ROI Area           : %.0f%%" % (frame.get("roi_ratio", 0.0) * 100.0),
            "Theoretical Pixel Reduction: %.0f%%" %
            (frame.get("theoretical_pixel_reduction", 0.0) * 100.0),
            "Not measured bandwidth or power savings.",
        ]
        self.stats_text.insert("1.0", "\n".join(stats_lines))
        self.stats_text.configure(state=tk.DISABLED)

    def render(self):
        if not self.frames:
            self.draw_field()
            return
        self.current = self.frames[self.index]
        if self.current.get("director_event") not in (None, "NONE"):
            self.recent_event = self.current["director_event"]
            self.recent_event_frames = 20
        elif self.recent_event_frames:
            self.recent_event_frames -= 1
            if not self.recent_event_frames:
                self.recent_event = None
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


def launch_frames(frames, start_frame=1, playback="realtime"):
    root = tk.Tk()
    speed_ms = 40 if playback == "realtime" else 10
    SoccerVisualizer(root, frames=frames, scenario_name="6 - Mini Match Simulation",
                     start_frame=start_frame, autoplay=True, speed_ms=speed_ms)
    root.mainloop()


if __name__ == "__main__":
    app = tk.Tk()
    SoccerVisualizer(app)
    app.mainloop()
