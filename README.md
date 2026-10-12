# Fallout4-PeekOnl — independent peek plugin prototype

This repository contains a clean-room prototype for a standalone Fallout 4 F4SE peek plugin. It does not include the Bodycam DLL or its assets.

## Current status

**Not a working or installable mod yet.** The plugin currently logs initialization only. The camera hook, corner and collision probes, input handling, transform application, INI parsing, in-game validation, and Vortex package are still outstanding. This first commit prepares the source/build pipeline and does not provide playable peek behavior.

Target runtime: Fallout 4 1.10.163 with F4SE 0.6.23. Planned build: CommonLibF4 with XMake on a Windows GitHub Actions runner. CommonLibF4 is GPL-3.0-or-later with modding/linking exceptions; comply with its corresponding-source terms before distributing linked binaries.

## Build pipeline

The Windows plugin build workflow is configured to run on pushes to `main` or by manual dispatch. It compiles a prototype DLL and uploads it as a GitHub Actions artifact. That DLL is only a build milestone until the actual runtime feature is implemented.

## Planned Vortex layout

The eventual package will place `PeekOnly.dll` and `PeekOnly.ini` at `Data/F4SE/Plugins/`. Dependencies are expected to include F4SE 0.6.23, and possibly the matching Address Library depending on the final hook/API strategy.
