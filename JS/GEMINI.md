# GEMINI.md

## Project Overview

This project is an interpreter for a custom scripting language called "Pi". It is written in JavaScript (ESM) and appears to be designed to run both in a web environment (via `worker.js`) and on the command line with Node.js (via `PiTester.js`).

The interpreter is composed of the following main components:

*   **Scanner (`PiScanner.js`):** A lexical analyzer that performs tokenization of the source code.
*   **Parser (`PiParser.js`):** A parser that builds an Abstract Syntax Tree (AST) from the tokens.
*   **Virtual Machine (`PiVM.js`):** (Inferred from `worker.js`) A VM that likely executes bytecode generated from the AST.

The "Pi" language itself seems to be a dynamically-typed language with a syntax that borrows from Python and JavaScript. It supports a rich set of built-in functions for a wide range of functionalities, including:

*   Standard library features (math, strings, collections).
*   2D and 3D graphics.
*   Audio processing.
*   File I/O.

## Building and Running

There are two primary ways to run a "Pi" script:

### 1. Command-Line (Node.js)

The `PiTester.js` script can be used to execute `.pi` files from the command line.

**To run a script:**

```bash
node PiTester.js <path_to_script.pi>
```

This will scan and parse the script, and then print the resulting AST to the console.

### 2. Web Environment

The `worker.js` file is set up to run "Pi" scripts in a web worker. It receives source code, processes it through the scanner and parser, and then executes it on the `PiVM`.

## Development Conventions

*   **Modularity:** The code is highly modular, with each class residing in its own file and using ES Modules (`import`/`export`).
*   **File Naming:** Files are named after the class they contain (e.g., `PiParser.js` contains the `PiParser` class).
*   **AST:** The parser generates a detailed Abstract Syntax Tree, with different node types for each language construct.
*   **Testing:** `PiTester.js` serves as the primary tool for testing and debugging the interpreter.
