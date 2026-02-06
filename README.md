# OpenWare

OpenWare is a hobby operating system project developed under the Ventryx umbrella.  
It is built for learning, experimentation, and understanding how operating systems
work from the bootloader up.

This project is **source-available**, not open source.

---

## What is OpenWare?

OpenWare is a custom OS with:
- A two-stage BIOS bootloader
- A protected-mode kernel
- A focus on clarity, structure, and learning
- No goal of mass adoption or production use (yet)

The code is intentionally readable and educational.

---

## Project Status

**Early development**

Current focus:
- Bootloader (Stage 1 + Stage 2)
- Protected mode entry
- Kernel loading
- Core kernel structure

Things like filesystems, multitasking, and userland are planned but not complete.

---

## License (Important)

This project is licensed under the **OpenWare Source View License**.

You are allowed to:
- View and study the source code
- Learn from the architecture and implementation
- Use small code snippets for educational or inspirational purposes

You are **not** allowed to:
- Copy the project in whole or in substantial part
- Redistribute the code or binaries
- Publish forks or derivative projects
- Rebrand or claim the work as your own

Any permitted snippet usage **must include attribution**:

> “OpenWare — a Ventryx brand”

See the `LICENSE` file for full terms.

---

## Building (Experimental)

OpenWare is currently built as a BIOS-bootable image.

Typical tools used:
- `WSL` (what we use to build)
- `nasm`
- `gcc` (cross-compiler recommended)
- `ld`
- `xorriso`
- `qemu` (for testing)

Exact build steps may change as the project evolves.

---

## Goals

- Learn low-level systems programming
- Understand bootloaders, memory, and CPU modes
- Build a clean, understandable OS foundation
- Stay simple and intentional

This is not meant to compete with existing OSes.

---

## Attribution

OpenWare is developed by **Ventryx Inc.**

If you reference this project publicly, please credit:
**OpenWare — a Ventryx brand**

---

## Disclaimer

This software is provided **AS IS**, without warranty of any kind.  
Use at your own risk.

---
