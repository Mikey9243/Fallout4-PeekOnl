# PeekOnly (private-use prototype)

An independently written F4SE plugin for Fallout 4 Old-Gen 1.10.163. It does not include or load the Bodycam mod's DLL, assets, scripts, or configuration. While aiming in first person near a detected corner, it shifts the camera laterally toward the clearer side. Pressing the configured movement key toward the wall suppresses the peek.

The camera movement default is now 90 game units, increased from 60 based on user feedback that the original sightline remained too tight. The value is adjustable in the INI. A camera-to-target physics ray limits movement where nearby geometry blocks the offset.

## Requirements

- Fallout 4 1.10.163 (pre-next-gen)
- F4SE 0.6.23 for runtime 1.10.163
- Address Library for F4SE Plugins, version matching 1.10.163
- x64 Microsoft Visual C++ runtime, if not already installed

No ESP/ESL, Papyrus, MCM, Creation Kit, or Bodycam installation is required.

## Install with Vortex

1. Download the `PeekOnly-Vortex` artifact ZIP from GitHub Actions.
2. In Vortex, choose **Install From File** and select the ZIP.
3. Enable **PeekOnly** and deploy mods.
4. Confirm the files resolve as `Data/F4SE/Plugins/PeekOnly.dll` and `Data/F4SE/Plugins/PeekOnly.ini`.
5. Launch Fallout 4 through F4SE. Check the F4SE log for plugin initialization messages.

## Configuration

Edit `Data/F4SE/Plugins/PeekOnly.ini` (or Vortex's mod staging copy) and redeploy.

- `Enabled`: master toggle.
- `AimOnly`: require the configured aim key.
- `DetectDistance`, `FullDistance`: forward wall range in game units.
- `SideProbe`: lateral corner test offset.
- `CameraOffset`: maximum lateral view movement; default 90. Lower it if the shift is too large or clips into scenery, or raise it if the camera ray leaves room.
- `FadeIn`, `FadeOut`: smoothing speeds.
- `Aim`, `Left`, `Right`: decimal Win32 virtual-key values (defaults: right mouse 2, A 65, D 68).

## Current limitations

This plugin moves the camera only; it does not reproduce weapon yaw/cant, camera roll, or MCM. The new 90-unit default has not yet been verified in a fresh in-game session. The supplied earlier gameplay clip shows the existing variant in action according to the user, and their feedback was that it needed a larger peek. Disable the mod in Vortex if the camera behaves unexpectedly.

## Build

The repository's Windows GitHub Actions workflow builds with XMake/CommonLibF4 and packages a Vortex ZIP. Source is independent and clean-room; the original archive is not redistributed.
