# Shellcode Loader

A sophisticated Windows-based shellcode execution framework featuring encrypted payload embedding, anti-debugging mechanisms, and a user-friendly Qt interface for security research and penetration testing.

## Features

- **Encrypted Shellcode Execution**: XOR-based encryption with position-dependent keys for payload obfuscation
- **Anti-Debugging Protection**: Built-in debugger detection using INT 3 breakpoint techniques
- **GUI Interface**: Clean Qt-based interface for easy shellcode loading and execution
- **Payload Generator**: Command-line tool for encrypting and embedding payloads into loader executables
- **Memory Protection**: Dynamic memory allocation with proper executable page permissions
- **Thread-based Execution**: Asynchronous shellcode execution in separate threads
- **Lazy Import Resolution**: Runtime API resolution to evade static analysis
- **Memory Obfuscation**: Custom memory copy functions to avoid standard library calls
- **Debugger Evasion**: Exception-based debugger detection mechanisms
- **Encrypted Storage**: Payloads are encrypted and embedded within the executable

## Requirements

### System Requirements
- Windows 10/11
- Visual Studio 2022
- Qt Framework 5.x or 6.x

### Dependencies
- Qt Widgets module
- Windows API libraries
- C++ Runtime libraries

## Installation

### Building

- Clone the repository

- Open the solution file (.sln).

- Select **Build Solution** from the **Build** menu.

## Usage

### Generating Encrypted Payloads

Use the ShellMaker tool to encrypt and embed your shellcode:

```bash
ShellMaker.exe input_payload.bin output_loader.exe
```

**Example:**
```bash
ShellMaker.exe meterpreter.bin encrypted_loader.exe
```

### Loading and Executing Shellcode

1. **GUI Method**: Run the Loading.exe application and it will automatically execute embedded shellcode
2. **Command Line**: The loader automatically detects and executes embedded payloads

### Code Integration Example

```cpp
#include "code.h"

int main() {
    // Initialize the loader
    if (start() == 0) {
        printf("Shellcode executed successfully\n");
    }
    return 0;
}
```

## Configuration

### Payload Size Limits
- Maximum payload size: 27,136 bytes (defined by `DATA_SIZE`)
- Minimum marker sequence: 66 bytes (0x42)

### Encryption Parameters
- **Algorithm**: XOR with position-based key
- **Key Generation**: `(byte ^ (position + 1)) + 1`
- **Marker Byte**: 0x41 (used for payload location)

### Anti-Debug Settings
The loader includes several configurable anti-debugging mechanisms:

```cpp
// Debugger detection via INT 3
bool Tesbuer() {
    __try {
        __asm {
            _emit 0xCD  // INT 03
            _emit 0x03
            _emit 0xC3  // RET
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;  // Debugger detected
    }
    return true;
}
```

## Testing

### Unit Tests
```bash
# Build test configurations in Visual Studio
# Run with appropriate test payloads
```

### Compatibility Testing
- Tested on Windows 10 (1909, 2004, 21H1, 21H2)
- Tested on Windows 11
- Compatible with both x86 and x64 architectures

## Disclaimer

This software is provided for educational and research purposes only. The authors and contributors are not responsible for any misuse or damage caused by this software.


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.


