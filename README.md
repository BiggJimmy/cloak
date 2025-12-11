# Cloak64 Payload Crafter (Research Tool)

**Cloak64** is a forensic and reverse-engineering research utility created to study
file-format manipulation and payload encapsulation techniques similar to those used
in security incidents involving **CVE-2024-7344** — a Secure Boot bypass vulnerability
publicly disclosed in 2024.

⚠️ **This tool does not exploit Secure Boot by itself.**  
It is intended strictly for **security analysis, detection research, incident response,
and education**.  
No part of this project should be used for unauthorized access, bypassing security
controls, or loading untrusted binaries.

---

## 📌 Project Purpose

This project serves as a **research implementation** of a payload wrapper ("crafting")
and un-wrapper ("uncrafting") mechanism useful for:

- analyzing how malicious actors encapsulate EFI binaries,
- understanding how custom headers, obfuscation, and checksums may appear in-the-wild,
- testing defensive detections for suspicious EFI payload packaging,
- teaching binary structure manipulation and integrity metadata.

The design intentionally mirrors patterns seen in the ecosystem surrounding
**CVE-2024-7344**, but includes **no exploit logic**.

---

## 🔧 Features

### ✔ Custom Payload Header
The crafter generates a 0x200-byte header containing:

- a magic identifier (`ALRM`)
- original payload size
- header size
- CRC-32 checksum of the unmodified payload

### ✔ Optional XOR Obfuscation
The payload data can be obfuscated using a simple 8-bit XOR cipher for research
purposes (default key: `0xB3`).

This is **not encryption** and should not be considered a security boundary.

### ✔ Crafting and Uncrafting Modes

#### Craft Mode
Wraps an input EFI binary into a structured container:
