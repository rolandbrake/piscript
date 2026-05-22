## PiScript JS Formatter + Minifier

This project provides a JavaScript implementation of a formatter and minifier for the PiScript programming language. It includes a scanner, parser, AST nodes, and formatting/minification utilities.

If you want a quick way to see it in action, `PiTester.js` reads a `.pi` file, parses it, and prints the formatted output (with a minification option in code).

## Features

- Parses PiScript source into an AST
- Stable, readable formatting with comment preservation
- Minification support (with optional identifier mangling)
- CLI-style tester for quick local runs

## Requirements

- Node.js (ESM modules enabled)

## Quick Start

Format a file using the tester:

```bash
node PiTester.js path\to\script.pi
```

## Programmatic Usage

### Formatter

```js
import PiFormatter from "./PiFormatter.js";

const source = `let x=1+2;`;
const result = PiFormatter.format(source);

if (result.success) {
  console.log(result.code);
} else {
  console.error(result.error);
}
```

### Minifier

Minification is driven by AST nodes and a `PiContext` (which can mangle identifiers). Use the scanner + parser, then call `minify` on the statements.

```js
import PiScanner from "./PiScanner.js";
import PiParser from "./PiParser.js";
import PiContext from "./PiContext.js";

const source = `let counter = 1 + 2;`;
const scanner = new PiScanner(source);
const tokens = scanner.scanTokens();

const parser = new PiParser();
const statements = parser.parse(tokens);

const builtins = ["print", "println"]; // add PiScript built-ins you want to preserve
const ctx = new PiContext(builtins, true); // true = enable mangling

const minified = statements.minify(ctx);
console.log(minified);
```

## Notes

- Formatting preserves leading/trailing comments and uses indentation rules implemented in the AST node classes.
- Minification can mangle identifiers while keeping built-in names intact.
- The list of built-ins used in `PiTester.js` is a good starting point for real scripts.

## Files of Interest

- `PiFormatter.js` - formatting entry point
- `PiMangler.js` - name mangling for minification
- `PiContext.js` - scope handling for minification
- `PiTester.js` - CLI-style runner that formats a `.pi` file
