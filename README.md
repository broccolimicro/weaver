# Weaver

Weaver is a compiler framework for hardware description languages within the Loom CAD ecosystem. It provides a flexible architecture for representing, analyzing, and transforming hierarchical hardware designs with a focus on asynchronous circuits.

## Overview

Weaver serves as the foundational layer of the Loom hardware compilation and verification system. It provides:

- A modular architecture for organizing hardware designs
- A type system supporting both data types and interfaces
- A flexible process/function representation system
- Support for multiple language dialects (CHP, HSE, PRS)
- Variable expansion and scope management
- Cross-module type and function resolution

Weaver is designed to accommodate hardware specifications at various levels of abstraction, from high-level communicating processes (CHP) down to production rule sets (PRS) for asynchronous circuit implementation.

## Architecture

Weaver uses a hierarchical organization model:

- **Program**: Top-level container for all modules
- **Module**: Compilation units containing terms and types
- **Term**: Process/function definitions that can be written in various dialects
- **Type**: Data structures and interfaces
- **Instance**: Variable declarations with type information

### Key Features

- **Module System**: Organizes hardware components into namespaces
- **Type Hierarchy**: Supports both regular data types and interfaces
- **Dialect System**: Allows for multiple language representations within the same framework
- **Variable Expansion**: Flattens hierarchical designs for analysis and transformation
- **Symbol Management**: Maintains variable scoping and visibility

## Components

### Core Classes

- `Program`: Manages modules and cross-module type/term resolution
- `Module`: Contains terms and types within a namespace
- `Term`: Represents processes or functions with dialect-specific implementations
- `Type`: Defines data types and interfaces
- `Instance`: Represents variable declarations
- `Decl`: Represents function/method prototypes
- `SymbolTable`: Manages variable scoping and lookup
- `VariableExpander`: Traverses and flattens nested type hierarchies

### Type System

Weaver supports two fundamental kinds of types:
- **Regular Types** (`TYPE`): Can have both data members and methods
- **Interfaces** (`INTERFACE`): Can only have methods

The global module provides built-in interfaces:
- `chan`: For communication channels
- `fixed`: For fixed-point numbers
- `ufixed`: For unsigned fixed-point numbers
- `bool`: For boolean values

### Dialect System

The dialect system allows for multiple hardware description languages to coexist within the same framework. Each dialect can have its own syntax and semantics but shares the common term declaration structure. This enables support for:

- CHP (Communicating Hardware Processes)
- HSE (Handshaking Expansions)
- PRS (Production Rule Sets)

## Building

To build the weaver library:

```bash
make lib
```

To build and run tests:

```bash
make tests
```

For code coverage analysis:

```bash
make coverage
```

## Integration

Weaver is designed to integrate with other components in the Loom ecosystem:

- Relies on `parse` and `common` libraries
- Potentially targets LLVM/MLIR for code generation
- Supports FPGA implementations through dataflow representations

## License

Licensed by Broccoli, LLC under GNU GPL v3.

Written by Ned Bingham.
Copyright © 2025 Broccoli, LLC.

Loom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Loom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

A copy of the GNU General Public License may be found in COPYRIGHT.
Otherwise, see <https://www.gnu.org/licenses/>.

