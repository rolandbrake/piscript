# 📘 Getting Started with Piscript

Welcome to **Piscript** — a lightweight, embeddable scripting language with built-in support for graphics, matrices, and functional programming. Piscript is designed to be expressive, minimal, and ideal for creative and educational projects, with inspiration drawn from Pico-8 and Python.

---

## 🚀 What is Piscript?

Piscript is a scripting language that:

- Compiles to WebAssembly using **Emscripten**
- Supports a **128×128 pixel graphics console**, ideal for retro-style visual output
- Features familiar syntax elements inspired by Python and JavaScript
- Includes **functional programming** tools like `map`, `filter`, `reduce`, and `compose`
- Supports matrices, dot/cross products, and array manipulation
- Allows anonymous and arrow functions
- Offers a REPL-style **online playground**

---

## 🔧 Requirements

To run Piscript locally or embed it in your project, you need:

- A modern browser (for the web-based version)
- Or: a C/C++ development environment with Emscripten if you're compiling the language yourself

---

## 🌐 Try It Online

You can try Piscript directly in your browser using the [Piscript Playground](https://piscript.netlify.app/playground). Just write your code and hit **Run**!

---

## 📦 Installation

Piscript is designed to run in the browser, but if you want to build it yourself:

### Clone and Build

```bash
git clone https://github.com/rolandbrake/piscript.git
cd piscript
emmake make
