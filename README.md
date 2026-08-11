# vectornav-sdk

VectorNav SDK v1.2.0 with local fixes and a verified working configuration for a **VN-200T-CR**
GNSS/INS unit.

Everything in the "Verified" sections below was measured on hardware, not taken from
documentation. Where a claim came from reasoning rather than measurement, it says so.

---

## Unit under test

| | |
|---|---|
| Model | `VN-200T-CR` |
| Hardware revision | 3 |
| Serial number | `0100062351` |
| Firmware | `2.0.0.1` |
| Interface | FTDI USB-RS232 cable (`FT232`), `/dev/ttyUSB*` |
| Host | Ubuntu 24.04, ROS 2 Jazzy, gcc 13.3 |

---

## Changes from the upstream SDK

Two tracked files differ from VectorNav's release. Both matter.

### 1. `cpp/plugins/ros2/vectornav/include/vectornav.hpp` — segfault fix

The ROS 2 plugin passed `std::string_view` through printf-style varargs:

```cpp
RCLCPP_INFO(get_logger(), "[RX] %s", VN::errorCodeToString(vnError));   // UB
```

`VN::errorCodeToString()` returns `std::string_view` (`Interface/Errors.hpp:135`), which is not
null-terminated and is not a POD safe to pass through `...`. Any failed transaction therefore
crashed the node with **SIGSEGV**. Reproduced reliably by reading a register the unit does not
implement:

```
$VNRRG,157   ->   process has died [exit code -11]
```

Fixed at 4 call sites (lines ~206, 232, 244, 419) by materialising a `std::string` first. The
compiler had been warning about all of them (`-Wformat`). After the fix the same read returns
cleanly and the node survives:

```
recv='Error: InvalidRegister'      # node still streaming at 800 Hz
```

### 2. `cpp/plugins/ros2/vectornav/config/vectornav.yaml` — working config

Replaced with a measured max-throughput configuration. Three substantive differences from
VectorNav's stock file:

- `asyncMode: 1` (serial1 only) instead of `3` (both ports). With `3` the sensor bills the packet
  against serial2's baud as well and rejects writes with `InsufficientBaudRate` far earlier than
  the link actually requires.
- `syncOutMode: 6` / `syncOutPolarity: 1` preserved from the unit's own settings. The stock file
  writes **zeros** here, which silently repurposes the sync output pin — on this unit that pin was
  configured to emit GPS PPS.
- `BaudRate: 921600`, `BO1` at 800 Hz, `BO2` GNSS at 5 Hz.

The stock file is recoverable from git history.

---

## Quick start

### ROS 2 node

The package uses relative paths (`../../../src`), so build it in place:

```bash
source /opt/ros/jazzy/setup.bash
colcon build --base-paths cpp/plugins/ros2 --cmake-args -DCMAKE_BUILD_TYPE=Release
source install/setup.bash
ros2 launch vectornav vectornav.launch.py
```

Topics published: `/vectornav/raw/{common,time,imu,gnss,gnss2,attitude,ins}` plus a
`/vectornav/transaction` service that accepts raw commands (`RRG,55`, `WRG,6,22`, …).

### C++ example

No external dependencies:

```bash
cmake -S cpp/examples/GettingStarted -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/GettingStarted /dev/ttyUSB0
```

### Serial port renumbering

`/dev/ttyUSBn` is **not stable** — this unit moved `ttyUSB0 → ttyUSB1 → ttyUSB0` inside one
session, and each move breaks a hardcoded `port:` in the yaml. The stable path
(`/dev/serial/by-id/usb-FTDI_USB-RS232_Cable_*`) is 62 characters and **cannot be used**: the SDK
caps port names at 32 (`Config::Serial::PortNameMaxLength`, `cpp/include/vectornav/Config.hpp`).

Resolve it at launch instead:

```bash
PORT="/dev/$(basename "$(readlink -f /dev/serial/by-id/*FTDI*)")"
ros2 run vectornav vectornav --ros-args --params-file <yaml> -p "port:=$PORT"
```

A udev rule giving a short fixed name (`/dev/vn200`) is the permanent fix but needs root.

---

## Verified: throughput

Packet loss was measured from gaps in the sensor's own `timeStartup` counter, which catches both
wire corruption and SDK queue overflow. Wall-clock rate alone cannot distinguish "slow" from
"lossy".

| baud | rate | payload | result |
|---|---|---|---|
| 230400 / 460800 / 921600 | 100 Hz | full common `0x7FFF` | 0 drops each |
| 921600 | 200 Hz | full common | 0 drops |
| 921600 | 400 Hz | full common | 8001 samples / 20 s, **0 drops** |
| 921600 | 800 Hz | full common | **rejected**: `InsufficientBaudRate` |
| 921600 | 800 Hz | lean `0x0139` | 16000 samples / 20 s, **0 drops** |

- **921600 is the sensor ceiling** (`BaudRate` enum, `Interface/Registers.hpp`).
- **800 Hz is the IMU's native rate**, reachable only with a trimmed payload. The full 15-field
  common group is ~198 B; 198 B × 800 Hz = 158 kB/s against the 92 kB/s available at 921600 8N1,
  and the sensor's own bandwidth check refuses the register write.
- `commonField 0x0139` = `timeStartup|ypr|quaternion|angularRate|accel` (~60 B, 52% link use).
- For mag/pressure/temperature, INS status and deltas, use `0x7FFF` at 400 Hz instead.

Useful bitfields:

| field | value | contents |
|---|---|---|
| `commonField` | `0x0139` | timeStartup, ypr, quaternion, angularRate, accel |
| `commonField` | `0x7FFF` | all 15 common fields |
| `gpsField` | `0x226BE` | tow, week, numSats, **fix type**, posLLA, velNED, **pos/vel uncertainty**, DOP, status |
| `imuField` | `0x0406` | uncompMag, uncompAccel, angularRate |
| `timeField` | `0x03E7` | timeStartup, timeGps, timeGpsTow, timeGpsPps, timeUtc, syncInCnt, syncOutCnt, timeStatus |

---

## Verified: magnetometer calibration

### Register 23 convention — determined by probing, not assumed

Writing the calibration in the wrong sense is worse than none, so the convention was measured by
writing known matrices and reading back the effect:

```
C = diag(2,1,1), b = (0.1, 0, 0)
  C*(m - b) predicts x = -0.44960   measured -0.44960   error 0.00000
  C*m - b   predicts x = -0.34960
```

**`m_comp = C · (m − b)`.** Bias is *subtracted*.

Two non-obvious consequences:

1. **`reg 23` is applied upstream of the `uncompmag` field.** With gain `diag(1,2,3)` the
   "uncompensated" mag in `reg 54` scaled by 3 on the z axis. So raw capture for a fit **requires
   `reg 23` set to identity first**, or the fit double-applies.
2. `reg 17` (compensated) and `reg 54` (uncompensated) read nearly identically under a normal
   calibration, which makes it easy to conclude wrongly that `reg 23` is being ignored.

### Result achieved

Full ellipsoid fit, 25,896 samples at 92.5% sphere coverage:

| | before | after |
|---|---|---|
| \|B\| spread | **123.1%** | **4.8%** |
| \|B\| mean | 0.6626 | 0.4749 gauss |

The defect was a **0.4069 gauss hard-iron offset** — comparable to Earth's field locally, which is
why heading was unusable. Soft iron was negligible (axis ratio 1.011, off-diagonals ≤ 1.3e-3).

The gain was then scaled by 0.875055 so output magnitude matches the sensor's own reference vector
in `reg 21` (0.4738 gauss) rather than the fit's volume-preserving 0.5414. Heading is unchanged by
scaling, but VPE compares magnitude against that reference when judging magnetic disturbance, and a
persistent 14% mismatch invites false disturbance flags.

### What does not work

**The firmware's real-time HSI (`reg 44`) cannot fit soft iron on this unit.** Over 162 samples the
estimated gain stayed uniformly diagonal (`0.272839` on all three axes, off-diagonals exactly
`0.000000`) — it fits a *sphere*, i.e. one uniform scale plus 3 bias terms, not an ellipsoid. It
also diverged rather than converged: bias grew monotonically 0.107 → 0.747 gauss (7×) while the
gain collapsed, leaving |B| spread at 63.9%. Do not persist its output.

### Capture requirements

Both are hard gates; failing either makes the fit unsolvable, not merely inaccurate.

- **Coverage ≥ 40%**, ideally > 60%, of the orientation sphere, and every axis hemisphere non-zero.
  Two attempts failed at 26–29% coverage with **−Z = 0%** (the unit was never inverted). Fitting a
  quadric to a patch is ill-posed — the fit returned a *hyperboloid* (eigenvalue −2.76), because the
  centre can slide along the unsampled direction and still fit the patch.
- **A magnetically uniform location.** Calibration assumes |B| is constant and only its *direction*
  changes in body frame. One 47,078-sample capture at 100% coverage still failed because the sensor
  was carried through a field gradient: fitting the first and second halves independently gave
  \|bias\| of 0.043 vs 0.353 gauss — an 8× difference in what is supposed to be a fixed property of
  the sensor. Check |B| stability *before* capturing by holding one orientation and moving the unit
  around the intended volume; > 15% variation means relocate.

Calibrate in the final mounting. A bench calibration captures the bench's iron and is discarded the
moment the unit is remounted.

---

## Verified: GPS PPS and timing

**Timestamps are GPS-referenced; IMU sampling is not PPS phase-locked.**

PPS is present and used:

- `syncOutCnt` advanced 3931 → 3956 in 25 s = **exactly 1 Hz** (`syncOutMode: 6 = GpsPps`, so the
  unit re-emits PPS on its SyncOut pin).
- `reg 55` `ppsSource = 0 (PpsPinRising)` — PPS is consumed to discipline the time base.
- `timeStatus` = `towValid, dateValid, utcValid` (0x07).

Sampling is not locked to it, by three agreeing measurements:

| evidence | value |
|---|---|
| sample phase within the GPS second | sweeps 16.7 → 996.9 ms (the whole second) |
| drift of the 1.25 ms IMU grid vs GPS | 0.245 ms / 25 s |
| `timeGps − timeStartup` drift | 243.9 µs / 25 s ≈ **9.76 ppm** |

The last two are the same figure derived independently: the IMU sample clock free-runs ~10 ppm off
GPS (ordinary TCXO behaviour). `syncInMode = 3 (Count)`, not `4 (ImuSample)`, and `syncInCnt` stayed
`0 → 0` — nothing triggers sampling.

Note `timeGpsPps` is **time since the last PPS event** (range [0, 1 s)), not an absolute timestamp.

To tighten sync: post-correct per sample using `timeGps` (cheapest, lossless); or wire PPS into
SyncIn with `syncInMode = 3` and use `syncInCnt`/`timeSyncIn` as a once-per-second anchor; or use
`syncInMode = 4 (ImuSample)` with a higher-rate trigger derived from PPS (1 Hz is useless as a
sample clock); or feed the SyncOut PPS to other sensors.

---

## RTK: not supported on this unit

Corrections cannot be delivered to a VN-200T-CR through this SDK or this firmware:

- The full register inventory (IDs 0–255, scanned live) contains **no RTCM or correction-input
  register**. `grep -ri rtcm` over the SDK returns nothing.
- `reg 157` (`ExtGnssOffset`) returns **`InvalidRegister`** — no external GNSS position aiding.
- `reg 103`/`104` (`Gnss2Sol*`) also `InvalidRegister` — single antenna.
- The only external aiding available is **velocity** (`reg 50`/`51`).
- Observed fix type is `4 = SBAS`. `RtkFloat = 7` / `RtkFix = 8` exist in the SDK's `Gnss1Fix` enum
  but that enum is shared across VectorNav's product line; its presence is not a capability claim
  for this model.

An NTRIP caster was confirmed reachable and serving RTCM3 (MSM7, multi-constellation, base ~2 km
away — a good baseline), so the corrections are fine; there is simply no ingest path on the sensor.
Getting RTK here requires a separate RTK-capable receiver as rover, with its solution fused with VN
attitude off-board.

---

## Gotchas

- **`$VNWNV` (WriteSettings) times out under load.** The SDK allows 1200 ms
  (`Config::Sensor::wnvSendTimeoutLength`); a flash write with a saturated 48 kB/s link exceeds it,
  and the ASCII reply is lost in the binary flood. Suspend streaming first with `$VNASY,0`
  (a *command*, so it does not alter the output registers or what gets persisted), commit, then
  `$VNASY,1`.
- **`$VNWNV` persists the entire current configuration**, not just the register you just wrote —
  baud, ADOR, all binary output registers, sync settings. Know the full RAM state before committing.
- **Verify persistence with `$VNRST`** rather than trusting the write. A software reset reloads from
  flash; if the values survive that, they are genuinely committed.
- **A power cycle reverts everything not in flash**, including baud. The unit returns to 115200 and
  its stored output config; the SDK's `autoConnect` handles the baud sweep.
- **Quaternion convention:** the published quaternion, decoded with tf2's convention
  (`R = Rz(yaw)Ry(pitch)Rx(roll)`), reproduces the message's own `ypr` field exactly — verified
  against live data at a single instant. Pass it through unchanged for TF. Conjugating it flips yaw
  and corrupts pitch.
- **Frame convention is NED** (+x north, +y east, **+z down**), not the ENU of REP-103. Convert
  before feeding a ROS navigation stack.
- **`ros2 topic hz` and `ros2 topic echo` need the workspace overlay sourced**, or they report
  `The message type 'vectornav_msgs/msg/CommonGroup' is invalid` even while the node publishes fine.
- **Baud probing needs a settle and a flush.** Opening a port and reading immediately yields garbage
  at every baud, including the correct one. Add `reset_input_buffer()` and ~200 ms before reading.
- **`pkill -f <pattern>` will kill your own shell** if the pattern appears in its command line.
  Match on process name, or scan `/proc/*/comm`.

---

## Reverting the sensor to as-found

Values captured from this unit before any changes:

| reg | as-found value | meaning |
|---|---|---|
| 5 | `115200` | baud |
| 6 | `22` | AsyncOutputType — VNINS ASCII |
| 7 | `40` | AsyncOutputFreq — 40 Hz |
| 23 | `0.209,-0.454,-0.482,0.532,0.203,0.163,0.07,-0.243,0.124,-0.096,0.14,-0.915` | MagCal |
| 30 | `0,0,0,0,1,0,1` | ProtocolControl |
| 32 | `3,0,0,0,6,1,0,100000000,0` | SyncControl |
| 44 | `0,1,5` | RealTimeHsiControl — off |
| 55 | `0,0,5,0,1` | GnssBasicConfig |
| 57 | `0,0,0` | GnssAOffset |
| 75/76/77 | `0,0,00` | BinaryOutput1/2/3 — off |

Write `reg 5` **last**, since changing baud drops the link.

The as-found `reg 23` measured **worse than identity** (compensated \|B\| 0.82 vs 0.52 gauss), so
restoring it is a regression for the magnetometer. It is listed only so as-found is recoverable.

---

## Repository layout

| path | contents |
|---|---|
| `cpp/` | C++17 core library, headers, examples |
| `cpp/include/vectornav/Interface/Registers.hpp` | all register definitions and enums |
| `cpp/include/vectornav/Config.hpp` | compile-time limits and measurement-group enables |
| `cpp/plugins/` | DataExport, FirmwareProgrammer, Logger, Math, RegisterScan, ros2 |
| `cpp/libs/pugixml/` | vendored third-party XML parser |
| `python/` | pybind11 bindings |
| `cli/`, `net/`, `cs/`, `matlab/` | C++/CLI wrapper, `VnSdk_Net.dll`, C# and MATLAB examples |
| `documentation/`, `Documentation.html` | generated API docs |

A **Calibration plugin** (`hsi2d`, `hsi3d`, `hsiDipole`) is documented in
`python/documentation/plugins/calibration.html` but **its source is not shipped** in this SDK copy —
only the six plugins listed above. Request it from VectorNav if needed.

There is **no GUI** in this SDK: it is a headless library plus console examples. VectorNav's GUI is
Control Center, a separate Windows product. `RegisterScan` XML files are interoperable with it.
