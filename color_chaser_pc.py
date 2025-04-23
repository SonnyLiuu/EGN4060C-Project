"""
ESP32-CAM HSV picker + centroid tracker + UDP command sender
(no kernelling, stray-blob removal)
"""

import cv2, numpy as np, datetime as dt, threading, time, socket

# ─── SETTINGS ─────────────────────────────────────────────────────
STREAM_URL   = "http://192.168.1.149/stream"
WRAP_RED     = True
CENTER_TOL   = 30
MIN_PIXELS   = 200
GOAL_PIXELS = 2000
TARGET_FPS   = 30.0
EMA_ALPHA    = 0.1

# UDP control socket
CTRL_IP      = "192.168.1.150"   # <-- set to ESP8266 IP
CTRL_PORT    = 5005              # <-- listening port
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
last_cmd = None

# Default HSV window (reset state)
DEFAULTS = {
    "H_min": 138, "H_max":   27,
    "S_min":  54, "S_max": 180,
    "V_min":  78, "V_max": 255,
}

# ─── SHARED STATE ─────────────────────────────────────────────────
latest_frame = None
lock         = threading.Lock()
stopped      = False

# ─── FRAME‐GRABBER THREAD ─────────────────────────────────────────
def frame_grabber():
    global latest_frame, stopped
    cap = cv2.VideoCapture(STREAM_URL, cv2.CAP_FFMPEG)
    if not cap.isOpened():
        print("ERROR: cannot open stream", STREAM_URL)
        stopped = True
        return
    while not stopped:
        ret, frm = cap.read()
        if ret:
            with lock:
                latest_frame = frm
        else:
            time.sleep(0.01)
    cap.release()

threading.Thread(target=frame_grabber, daemon=True).start()

# ─── UI SETUP ─────────────────────────────────────────────────────
cv2.namedWindow("frame",    cv2.WINDOW_NORMAL)
cv2.namedWindow("mask",     cv2.WINDOW_NORMAL)
cv2.namedWindow("controls", cv2.WINDOW_NORMAL)

def _noop(x): pass

# HSV sliders only
sliders = [
    ("H_min", DEFAULTS["H_min"], 179),
    ("H_max", DEFAULTS["H_max"], 179),
    ("S_min", DEFAULTS["S_min"], 255),
    ("S_max", DEFAULTS["S_max"], 255),
    ("V_min", DEFAULTS["V_min"], 255),
    ("V_max", DEFAULTS["V_max"], 255),
]
for name, ini, maxi in sliders:
    cv2.createTrackbar(name, "controls", ini, maxi, _noop)

print("S=print HSV • C=save snapshot • Q=quit")

# ─── MAIN LOOP ─────────────────────────────────────────────────────
ema_fps   = TARGET_FPS
period    = 1.0 / TARGET_FPS
last_vals = {n: None for n,_,_ in sliders}
lower1 = upper1 = lower2 = upper2 = None
ds = 2  # downsample factor

while True:
    t0 = time.time()

    # grab latest frame
    with lock:
        frame = None if latest_frame is None else latest_frame.copy()
    if frame is None:
        time.sleep(period)
        continue

    h, w = frame.shape[:2]
    small = frame[::ds, ::ds]
    hsv   = cv2.cvtColor(small, cv2.COLOR_BGR2HSV)

    # read sliders
    vals = {n: cv2.getTrackbarPos(n, "controls") for n,_,_ in sliders}

    # rebuild thresholds if changed
    if any(vals[n] != last_vals[n] for n in vals):
        hmin, hmax = vals["H_min"], vals["H_max"]
        smin, smax = vals["S_min"], vals["S_max"]
        vmin, vmax = vals["V_min"], vals["V_max"]
        lower1 = np.array([hmin, smin, vmin], dtype=np.uint8)
        upper1 = np.array([179,  smax, vmax], dtype=np.uint8)
        lower2 = np.array([0,    smin, vmin], dtype=np.uint8)
        upper2 = np.array([hmax, smax, vmax], dtype=np.uint8)
    last_vals.update(vals)

    # create mask with hue-wrap support
    m1 = cv2.inRange(hsv, lower1, upper1)
    if WRAP_RED and vals["H_min"] > vals["H_max"]:
        m2 = cv2.inRange(hsv, lower2, upper2)
        mask = cv2.bitwise_or(m1, m2)
    else:
        mask = m1

    # ─── FILTER OUT STRAY BLOBS ─────────────────────────────────────
    num_labels, labels, stats, _ = cv2.connectedComponentsWithStats(mask, connectivity=8)
    if num_labels > 1:
        # stats[1:, cv2.CC_STAT_AREA] → areas of each blob
        largest_blob = 1 + np.argmax(stats[1:, cv2.CC_STAT_AREA])
        mask = np.where(labels == largest_blob, 255, 0).astype(np.uint8)
    # ────────────────────────────────────────────────────────────────

    # find centroid & decide cmd
    M      = cv2.moments(mask)
    pixels = int(M["m00"] / 255) if M["m00"] else 0
    cmd    = 'S'
    cx = cy = None

    if pixels >= MIN_PIXELS:
        cx = int(M["m10"] / M["m00"]) * ds
        cy = int(M["m01"] / M["m00"]) * ds
        if   pixels > GOAL_PIXELS:    cmd = 'S'
        elif cx < w//2 - CENTER_TOL:   cmd = 'A'
        elif cx > w//2 + CENTER_TOL:   cmd = 'D'
        else:                          cmd = 'W'
        cv2.circle(frame, (cx, cy), 6, (255,255,255), -1)

    # send UDP only on change
    if cmd != last_cmd:
        try:
            sock.sendto(cmd.encode(), (CTRL_IP, CTRL_PORT))
        except Exception as e:
            print("UDP send error:", e)
        last_cmd = cmd

    # draw centerline
    cv2.line(frame, (w//2,0), (w//2,h), (200,200,200), 1)

    # upscale mask & apply
    bool_mask = mask > 0
    mask_full = np.repeat(np.repeat(bool_mask, ds, axis=0), ds, axis=1)
    masked_view = frame.copy()
    masked_view[~mask_full] = 0

    # smooth FPS
    t1      = time.time()
    raw_fps = 1.0 / (t1 - t0) if t1 > t0 else TARGET_FPS
    ema_fps = ema_fps*(1-EMA_ALPHA) + raw_fps*EMA_ALPHA

    cv2.putText(frame,
                f"CMD:{cmd} Pix:{pixels} FPS:{ema_fps:.1f}",
                (10,30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0,0,255), 2)

    # show
    cv2.imshow("frame", frame)
    cv2.imshow("mask",  masked_view)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        sock.sendto('S'.encode(), (CTRL_IP, CTRL_PORT))
        break
    if key == ord('s'):
        now = dt.datetime.now().isoformat(timespec='seconds')
        print(f"[{now}] H({hmin},{hmax}) S({smin},{smax}) V({vmin},{vmax})")
    if key == ord('c'):
        ts = dt.datetime.now().strftime("%Y%m%d_%H%M%S")
        cv2.imwrite(f"frame_{ts}.png", frame)
        cv2.imwrite(f"mask_{ts}.png",  masked_view)
        print("Saved snapshots")

    # throttle
    t2 = time.time()
    to_sleep = period - (t2 - t0)
    if to_sleep > 0:
        time.sleep(to_sleep)

stopped = True
sock.close()
cv2.destroyAllWindows()
