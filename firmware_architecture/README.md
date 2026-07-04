# Firmware Architecture
This folder documents the firmware architecture of the ATxmega128A1 measurement platform.
The complete source code is not published. Instead, this folder explains the structure, layers, module responsibilities and selected interfaces.

## Architecture Concept
The firmware is divided into three main layers:
1. Driver Layer
2. Service Layer
3. Application Layer
This separation keeps hardware access, signal processing and user interaction independent.

## Documents
- `folder_structure.md`  
  Shows the original firmware organization.
- `module_overview.md`  
  Describes the responsibility of each module.
- `driver_layer.md`  
  Explains the low-level hardware drivers.
- `service_layer.md`  
  Explains acquisition, trigger, FFT, rendering and signal generation services.
- `application_layer.md`  
  Explains menu handling, mode control and UI logic.
- `selected_interfaces.md`  
  Shows selected function prototypes without publishing the complete implementation.