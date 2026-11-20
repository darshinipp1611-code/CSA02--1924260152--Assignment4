#!/usr/bin/env python3
"""Render every Taskxx/compile_*.txt and output_*.txt as a terminal-style PNG next to it."""
import glob, os, sys
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.abspath(__file__))
FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
FONT_B = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf"
F = "gcc -Wall -Wextra"
CMD = {  # name -> (compile command, run command or None)
    "patient": (f"{F} -o patient Task01/patient.c", "./Task01/patient"),
    "parcel": (f"{F} -o parcel Task02/parcel.c", "./Task02/parcel"),
    "parcel_full": (f"{F} -o parcel_full Task02/parcel_full.c", "./Task02/parcel_full"),
    "sensor": (f"{F} -o sensor Task03/sensor.c", "./Task03/sensor"),
    "sensor_extra": (f"{F} -o sensor_extra Task03/sensor_extra.c", "./Task03/sensor_extra"),
    "buggy": (f"{F} -o buggy Task04/buggy.c", None),
    "evidence": (f"{F} -o evidence Task04/evidence.c", "./Task04/evidence"),
    "fixed": (f"{F} -o fixed Task04/fixed.c", "./Task04/fixed"),
    "byvalue_alt": (f"{F} -O0 -o byvalue_alt Task04/byvalue_alt.c", "./Task04/byvalue_alt"),
    "token": (f"{F} -o token Task05/token.c", "./Task05/token"),
    "order": (f"{F} -o order Task05/order.c", "./Task05/order"),
    "extern_demo": (f"{F} -o extern_demo Task05/extern_main.c Task05/extern_counter.c", "./Task05/extern_demo"),
    "register_loop": (f"{F} -o register_loop Task05/register_loop.c", "./Task05/register_loop"),
    "register_addr_error": (f"{F} -o register_addr_error Task05/register_addr_error.c", None),
    "two_counters": (f"{F} -o two_counters Task05/two_counters.c", "./Task05/two_counters"),
    "storage_demo": (f"{F} -o storage_demo Task05/storage_demo.c", "./Task05/storage_demo"),
    "canteen": (f"{F} -o canteen Task06/canteen.c", "./Task06/canteen"),
}
BG, FG, PROMPT, BAR = (30, 30, 30), (224, 224, 224), (120, 220, 120), (58, 58, 58)
ERR, WARN = (255, 110, 110), (240, 200, 90)

def render(txt_path, command, body, out_path):
    font, bold = ImageFont.truetype(FONT, 18), ImageFont.truetype(FONT_B, 18)
    prompt = "student@lab:~/CSA02-REGNO-Assignment4$ "
    lines = [(prompt + command, True)] + [(l.expandtabs(4), False) for l in body.rstrip("\n").split("\n")] * (1 if body.strip() else 0)
    cw = font.getlength("M"); lh = 26; pad = 20; bar = 34
    w = int(max(len(t) for t, _ in lines) * cw + 2 * pad + 10); w = max(w, 700)
    h = bar + pad + lh * (len(lines) + 1) + pad // 2
    img = Image.new("RGB", (w, h), BG); d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w, bar], fill=BAR)
    for i, c in enumerate([(255, 95, 86), (255, 189, 46), (39, 201, 63)]):
        d.ellipse([14 + i * 22, 11, 26 + i * 22, 23], fill=c)
    d.text((w // 2, bar // 2), "Terminal", font=font, fill=(190, 190, 190), anchor="mm")
    y = bar + pad
    for t, is_cmd in lines:
        if is_cmd:
            d.text((pad, y), prompt, font=bold, fill=PROMPT)
            d.text((pad + font.getlength(prompt), y), command, font=font, fill=FG)
        else:
            col = ERR if "error" in t else WARN if "warning" in t else FG
            d.text((pad, y), t, font=font, fill=col)
        y += lh
    d.text((pad, y), prompt + "█", font=font, fill=PROMPT)
    img.save(out_path, "PNG")

n = 0
for p in sorted(glob.glob(os.path.join(ROOT, "Task*", "*_*.txt"))):
    base = os.path.basename(p)
    kind, name = base[:-4].split("_", 1)
    if kind not in ("compile", "output") or name not in CMD:
        continue
    cmd = CMD[name][0] if kind == "compile" else CMD[name][1]
    body = open(p, encoding="utf-8", errors="replace").read()
    render(p, cmd, body, p[:-4] + ".png"); n += 1
print(f"wrote {n} PNG files")
