# PeekOnly (private-use prototype)

An independently written F4SE plugin for Fallout 4 Old-Gen 1.10.163. It does not include or load the Bodycam mod's DLL, assets, scripts, or configuration. It currently implements a camera-only corner peek prototype: while aiming in first person near a detected corner, it shifts the camera laterally toward the clearer side. Pressing the configured movement key toward the wall suppresses the peek.

## Requirements

- Fallout 4 1.10.163 (pre-next-gen)
- F4SE 0.6.23 for runtime 1.10.163
- Address Library for F4SE Plugins, version matching 1.10.163
- x64 Microsoft Visual C++ runtime, if not already installed

No ESP/ESL, Papyrus, MCM, Creation Kit, or Bodycam installation is required.

## Install with Vortex

1. Download `PeekOnly-Vortex.zip` from the build artifact/release.
2. In Vortex, choose **Install From File** and select the ZIP.
3. Enable **PeekOnly** and deploy mods.
4. Confirm the files resolve as `Data/F4SE/Plugins/PeekOnly.dll` and `Data/F4SE/Plugins/PeekOnly.ini`.
5. Launch Fallout 4 through F4SE and check `Documents/My Games/Fallout4/F4SE/PeekOnly.log` for initialization messages.

## Configuration

Edit `Data/F4SE/Plugins/PeekOnly.ini` (or Vortex's mod staging copy) and redeploy.

- `Enabled`: master toggle.
- `AimOnly`: require the configured aim key.
- `DetectDistance`, `FullDistance`: forward wall range in game units.
- `SideProbe`: lateral corner test offset.
- `CameraOffset`: maximum lateral view movement.
- `FadeIn`, `FadeOut`: smoothing speeds.
- `Aim`, `Left`, `Right`: decimal Win32 virtual-key values (defaults: right mouse 2, A 65, D 68).

## Current limitations

This is a first implementation, not yet confirmed in a live game. It moves the camera only; it does not yet reproduce the source mod's weapon yaw/cant, camera roll, or MCM. Corner detection and the chosen left/right sign must be checked in-game. Disable the mod in Vortex if the camera behaves unexpectedly.

## Build

The repository's Windows GitHub Actions workflow builds with XMake/CommonLibF4 and packages a Vortex ZIP. Source is independent and clean-room; the original archive is not redistributed.

