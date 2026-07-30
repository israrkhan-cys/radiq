# radiq (Radial Launcher)

A Wayland-native, game-inspired radial application launcher designed for Arch Linux + Hyprland. Built with **Qt 6**, **QML**, and **C++20** for instant startup and minimal system resource usage.

![Tech Stack](https://img.shields.io/badge/C++-20-blue.svg)
![Qt](https://img.shields.io/badge/Qt-6.5+-green.svg)
![Wayland](https://img.shields.io/badge/Wayland-LayerShell-orange.svg)

---
## Preview
![Preview](/preview/2026-07-29-024259_hyprshot.png)
![Preview](/preview/radiq.mp4)

---
##  Features

- **Pie-Slice Weapon Wheel UI:** Weapon-select wheel inspired by games like *Call of Duty* and *PUBG*. Divided into equal angular segments rendered dynamically via Canvas with smooth hover scaling and color transitions.
- **Freedesktop Desktop Entry Discovery:** Automatically scans `$XDG_DATA_DIRS` and `~/.local/share/applications` for `.desktop` entries while filtering hidden/no-display entries and stripping desktop field codes (`%f`, `%u`, `%F`, `%U`, etc.).
- **System Icon Lookup & Fallback:** Resolves icons via `QIcon::fromTheme` and absolute file paths. If an icon cannot be resolved, falls back gracefully to a stylized colored circle with the application's initial letter (logging clear terminal warnings).
- **Centered Search & Fuzzy Matching:** Search bar embedded directly inside the ring center. Typing immediately triggers relevance-scored fuzzy search filtering across all installed system applications.
- **Hyprland Native Dispatch:** Launches applications via `hyprctl dispatch exec -- <cleaned_exec>` so custom Hyprland window rules are always respected.
- **Daemon-Client Architecture:** Resident `radiqd` background daemon holds a hidden `LayerShellQt` overlay window and communicates over a Unix socket (`radiq-launcher.sock`). The lightweight `radiqctl` CLI sends instant `show`, `hide`, or `toggle` commands.
- **JSON Configuration (`config.json`):** User configuration auto-created at `~/.config/radiq/config.json` on first run to customize pinned apps, ring radii, icon sizes, colors, font families, and animation speeds.

---

##  Requirements & Dependencies

Ensure the following dependencies are installed on your system (e.g., via `pacman -S` on Arch Linux):

- **CMake** (v3.21+)
- **C++20 Compiler** (`gcc` or `clang`)
- **Qt 6** (`qt6-base`, `qt6-declarative`, `qt6-5compat`)
- **LayerShellQt** (`layer-shell-qt`)
- **Hyprland** (or any Wayland compositor supporting LayerShell)

---
##  Installation

### Arch Linux (AUR) — recommended
```bash
yay -S radiq
# or
paru -S radiq
```



###  Building from Source

```bash
# Clone the repository
git clone https://github.com/israrkhan-cys/radiq.git
cd radiq

# Configure the build
cmake -B build -S .

# Build radiqd daemon and radiqctl client
cmake --build build
```

The resulting binaries will be placed in:
- `build/src/radiqd` (Daemon)
- `build/tools/radiqctl/radiqctl` (CLI Tool)

---

##  Usage & Setup

### 1. Test It Manually First

Before wiring it into `exec-once`, it's worth running it once by hand to
confirm everything works on your system.

**If installed via AUR:**
```bash
radiqd &
radiqctl toggle
```

**If built from source:**
```bash
./build/src/radiqd &
./build/tools/radiqctl/radiqctl toggle
```
## 2. Add to Hyprland Config

Add these lines to `~/.config/hypr/hyprland.conf`:

**If installed via AUR** (`radiqd`/`radiqctl` are already on your `PATH`):
```conf
exec-once = radiqd
bind = SUPER, A, exec, radiqctl toggle
```

**If built manually from source**, use the full path to your build output:
```conf
exec-once = /full/path/to/radiq/build/src/radiqd
bind = SUPER, A, exec, /full/path/to/radiq/build/tools/radiqctl/radiqctl toggle
```

Then reload Hyprland:
```bash
hyprctl reload
```

Press `Super + A` to summon the wheel.

---

## ⚙️ Configuration (`config.json`)

On daemon startup, `radiqd` checks for a JSON configuration file at:
```
~/.config/radiq/config.json
```
If the file does not exist, a default starter configuration is automatically created.

### Example `config.json`
```json
{
  "pinnedApps": [
    "firefox.desktop",
    "Alacritty.desktop",
    "org.kde.dolphin.desktop",
    "code.desktop",
    "discord.desktop",
    "gimp.desktop",
    "btop.desktop",
    "com.obsproject.Studio.desktop"
  ],
  "ringOuterRadius": 230,
  "ringInnerRadius": 100,
  "iconSize": 56,
  "wedgeColor": "#CC1F2335",
  "wedgeHighlightColor": "#2D3F76",
  "borderColor": "#414868",
  "borderHighlightColor": "#7AA2F7",
  "animationDurationMs": 130,
  "fontFamily": "Inter, Roboto, sans-serif",
  "hotkeyDisplay": "SUPER + A",
  "blurAmount": 10
  "scrimColor": "#CC0F0F15"
```

### Configuration Options
| Key | Type | Description |
|---|---|---|
| `pinnedApps` | Array of strings | `.desktop` file IDs to pin on the radial wheel slots in order. |
| `ringOuterRadius` | Number (px) | Outer radius of the radial wedge ring. |
| `ringInnerRadius` | Number (px) | Inner radius of the radial wedge ring. |
| `iconSize` | Number (px) | Size of application icons within the slots. |
| `wedgeColor` | Hex String | Fill color for unselected radial slots. |
| `wedgeHighlightColor` | Hex String | Fill color for selected/hovered radial slots. |
| `borderColor` | Hex String | Border stroke color for unselected slots. |
| `borderHighlightColor` | Hex String | Border stroke color for selected/hovered slots. |
| `animationDurationMs` | Number (ms) | Duration for scale and color transitions. |
| `fontFamily` | String | Font family stack for text labels. |
| `scrimColor` | Hex String | Background dimming behind the wheel (alpha channel controls transparency). |

---

> if you liked kindly start the repo and vote it on AUR Thankuu

## 📄 License

Distributed under the MIT [LICENSE](LICENSE).
