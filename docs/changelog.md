# Changelog

All notable changes to PiScript will be documented in this file. This project adheres to [Semantic Versioning](https://semver.org/).

---

## \[0.4.0] - 2025-05-28

### Added

* Matrix operations: `pi_dot`, `pi_cross`, `pi_mult`, `pi_eye`
* Functional programming support: `map`, `filter`, `reduce`, `compose`
* Support for assignment expressions using `<-`
* Matrix creation utilities: `pi_zeros`, `pi_ones`, `pi_size`

### Changed

* Improved parser structure for clarity and maintainability
* Optimized handling of `primary()` expressions

### Fixed

* Memory leak during long-running program execution
* Bug in modulo operation with zero (returns `NaN` instead of crashing)

---

## \[0.3.0] - 2025-04-10

### Added

* Support for anonymous functions and arrow functions
* Basic graphics API using 128x128 pixel virtual screen
* WebAssembly (WASM) support via Emscripten
* Online playground UI for Piscript in browser

---

## \[0.2.0] - 2025-03-01

### Added

* Arrays, loops, and conditionals
* First version of REPL and parser
* Basic standard library functions

---

## \[0.1.0] - 2025-01-15

### Added

* Initial language grammar and interpreter in C
* Tokenizer, parser, and evaluator core
* Basic arithmetic and printing support
