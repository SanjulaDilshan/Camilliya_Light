# Camiliya LED Sign — System & Build Documentation

## 1. Project Overview
The **Camiliya** sign is a Wi-Fi–connected, 12V RGB LED display controlled by a 38-pin ESP32 microcontroller and managed through the Blynk IoT mobile app. It supports smooth PWM color transitions across a 10-color palette on a configurable 1-minute interval alongside real-time manual color override from a smartphone.

---

## 2. System Architecture & Hardware

### Core Components
* **Microcontroller:** 38-pin ESP32 Dev Module (`ESP32-D0WD-V3`), running at 3.3V logic on GPIO pins.
* **LED Load:** 12V common-anode RGB LED strip (driven low via N-channel MOSFETs on R, G, and B channels).
* **Switching Stage:** Three logic-level N-channel MOSFETs (such as **IRLZ44N**) controlled via PWM on GPIO pins `4` (Red), `5` (Green), and `18` (Blue).
* **Power Architecture:** Single 12V DC power supply split into two parallel branches:
  1. **LED Branch:** Direct 12V positive feed to the LED strip and switched negative (Drain) paths through the MOSFETs back to a shared ground rail.
  2. **Controller Branch:** 12V input into a step-down **buck converter** (e.g., **LM2596** or **MP1584EN**) regulated to **5.0V**, connected to the ESP32's `VIN` / `5V` pin. *(A switching buck converter is used instead of a linear regulator like the LD1117 to prevent excessive heat dissipation from dropping 12V to 3.3V under Wi-Fi current spikes.)*

### Key Assembly Rules
* **Common Ground:** Tie the 12V power supply ground, buck converter ground, MOSFET Source legs, and ESP32 `GND` together on a shared ground rail.
* **Current Capacity:** Route high-current 12V and ground paths with thick copper wire (18–20 AWG) and soldered joints or screw terminals rather than thin jumper wires or breadboard clips, which cause voltage drop and LED dimming.
* **Direct LED Feed:** Never route the LED strip's `+12V` power through the buck converter; split the 12V supply directly to the strip.

---

## 3. Pin-by-Pin Wiring Tables

> **Important Safety Warning:** Power the LM2596/MP1584EN buck converter from the 12V supply first and adjust its potentiometer until a multimeter reads **5.0V** on `OUT+` / `OUT-` before connecting it to the ESP32 `VIN` pin. Feeding 12V directly into the ESP32 will permanently damage the board.

### Power Distribution Wiring

| Source Component & Pin | Destination Component & Pin | Wire Gauge / Notes |
| :--- | :--- | :--- |
| **12V Power Supply (`+12V`)** | **12V RGB LED Strip (`+12V`)** | **18–20 AWG** (Direct feed; do not route through buck converter) |
| **12V Power Supply (`+12V`)** | **Buck Converter (`IN+`)** | 22 AWG (Feeds the step-down module) |
| **12V Power Supply (`GND / -`)** | **Shared Ground Rail** | **18–20 AWG** (Main return path for high LED current) |
| **Buck Converter (`IN-`)** | **Shared Ground Rail** | 22 AWG (Common ground reference) |
| **Buck Converter (`OUT+`, set to 5V)** | **ESP32 (`VIN` or `5V`)** | 22 AWG (Powers the microcontroller) |
| **Buck Converter (`OUT-`)** | **Shared Ground Rail** | 22 AWG (Common ground reference) |
| **ESP32 (`GND`)** | **Shared Ground Rail** | 22 AWG (Required for PWM gate signals to work) |

### RGB Channel & MOSFET Switching Wiring

*Pin orientation assumes standard TO-220 MOSFETs (such as IRLZ44N) viewed from the front with the black plastic face toward you and legs pointing down: **Left = Gate (G)**, **Middle = Drain (D)**, **Right = Source (S)**.*

| Color Channel | ESP32 Pin | MOSFET Pin (Destination) | LED Strip / Rail Connection |
| :--- | :--- | :--- | :--- |
| **Red (R)** | **GPIO 4** | Red MOSFET — **Gate (Left)** | — |
| **Red (R)** | — | Red MOSFET — **Drain (Middle)** | **LED Strip `R` Pad** (18–20 AWG) |
| **Red (R)** | — | Red MOSFET — **Source (Right)** | **Shared Ground Rail** (18–20 AWG) |
| **Green (G)** | **GPIO 5** | Green MOSFET — **Gate (Left)** | — |
| **Green (G)** | — | Green MOSFET — **Drain (Middle)** | **LED Strip `G` Pad** (18–20 AWG) |
| **Green (G)** | — | Green MOSFET — **Source (Right)** | **Shared Ground Rail** (18–20 AWG) |
| **Blue (B)** | **GPIO 18** | Blue MOSFET — **Gate (Left)** | — |
| **Blue (B)** | — | Blue MOSFET — **Drain (Middle)** | **LED Strip `B` Pad** (18–20 AWG) |
| **Blue (B)** | — | Blue MOSFET — **Source (Right)** | **Shared Ground Rail** (18–20 AWG) |

### Optional Gate Protection Resistors (Recommended)
* **Series Gate Resistors ($100\,\Omega - 220\,\Omega$):** Place one between each ESP32 GPIO (`4`, `5`, `18`) and its MOSFET **Gate** to limit peak switching current spikes on the ESP32 pins.
* **Pull-Down Resistors ($10\,\text{k}\Omega$):** Place one between each MOSFET **Gate** and the **Shared Ground Rail** so the LEDs remain completely off while the ESP32 boots or resets.

---

## 4. Software & Blynk Configuration

### Blynk Setup
* **Template / Device:** `cam light` (`TMPL6sRGW4-kr`) connected via `BlynkSimpleEsp32.h` over Wi-Fi.
* **Virtual Pin `V1`:** Receives RGB color picker values (`param[0]`, `param[1]`, `param[2]`) in the `0–255` range. Both the zebra widget and circular color picker widget can be assigned to `V1`. *(Note: If a circular color widget sends a single HEX string instead of three RGB integer parameters, update `BLYNK_WRITE(V1)` to parse HEX values.)*

### Firmware Behavior
* **Smooth Auto-Fade:** Steps `currentR`, `currentG`, and `currentB` by 1 unit toward the active target color every `fadeInterval` ($20\,\text{ms}$).
* **Color Interval (`stateInterval`):** Advances `targetIndex` through the 10-color array (`Red`, `Orange`, `Yellow`, `Green`, `Cyan`, `Blue`, `Purple`, `Magenta`, `Deep Pink`, `White`) every $60,000\,\text{ms}$ (1 minute).
* **Manual Override:** Adjusting `V1` in the Blynk app immediately updates the R, G, and B PWM outputs, sets `manualOverride = true`, and resets `previousStateMillis` so the custom color holds for 1 full minute before auto-fading resumes.

---

## 5. Flashing & Verification Workflow

### Uploading Sketches in Arduino IDE
1. Paste the firmware sketch into Arduino IDE, select the ESP32 Dev Module board and active COM port (e.g., `COM9`), and close any open Serial Monitor tab locking the port.
2. Disconnect external wires on boot strapping pins (such as GPIO `5`) if they prevent the board from entering download mode.
3. Click **Upload**. The moment `Connecting...` appears in the console, firmly press and hold the **BOOT (IO0)** button on the ESP32 until `Writing at 0x...` begins, then release.
4. Confirm `100%` write, hash verification, and `Hard resetting via RTS pin...`. *(If upload errors persist, lower **Upload Speed** under **Tools** to `115200`.)*

### Functional & Brightness Checks
* **Serial Monitor (`115200` baud):** Press **EN** (Reset) to verify Wi-Fi/Blynk connection, `Fading to new color index` messages every 60 seconds, and `Manual Override -> R: ... | G: ... | B: ...` logs when adjusting colors in the app.
* **Troubleshooting Low Brightness:**
  1. **MOSFET Gate Drive:** Ensure logic-level MOSFETs (**IRLZ44N**) are used rather than standard MOSFETs (**IRFZ44N**), which only partially open at 3.3V gate voltage and overheat. If using standard MOSFETs or needing full saturation at 3.3V, add a logic-level shifter or transistor driver stage.
  2. **Wire & Connection Resistance:** Replace breadboards and thin Dupont wires on the 12V and Ground paths with soldered 18–20 AWG wire.
  3. **Power Supply Rating:** Verify the 12V adapter supplies sufficient current (typically **12V 5A** or higher for full-sign brightness).
  4. **Buck Converter Isolation:** Confirm the LED strip `+12V` wire connects directly to the 12V supply and not the buck converter output.