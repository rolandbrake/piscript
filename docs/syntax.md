# 🧭 Piscript Overview

**Piscript** is a lightweight, expressive programming language designed for **learning**, **creativity**, and **rapid prototyping**. It draws inspiration from Python and JavaScript, blending readable syntax with unique capabilities like matrix operations and pixel-based graphics designed for tiny, low-res consoles (128×128).

Whether you're building a math experiment, a retro-style game, or just exploring functional programming, Piscript offers a minimal yet powerful playground for developers and learners alike.

---

## 🌟 Key Features

- **Clean, familiar syntax** (inspired by Python & JS)
- **Matrix & vector operations** for linear algebra and simulations
- **Functional programming** support: `map`, `filter`, `reduce`, `compose`
- **Pixel graphics** rendering on a 128×128 screen
- **Lightweight runtime** ideal for web-based or embedded environments
- **Custom assignment operator `<-`** for inline expressions
- **Compact standard library** designed for rapid iteration

---

## 🏗️ Language Structure

Piscript programs are structured around the following core elements:

### 1. Variables & Expressions

- Assignment with `=`
- Inline assignment with `<-`
- Supports numbers, strings, booleans, lists, and matrices

### 2. Control Flow

- `if`, `else`, `for`, and `while` constructs
- Indentation-based blocks (like Python)

### 3. Functions

- Named functions with `fun`
- Anonymous and arrow functions
- First-class support: pass, store, and return functions

### 4. Functional Programming

- Built-in `map`, `filter`, `reduce`, and `compose`
- Combine logic elegantly using pure functions

### 5. Matrix Utilities

- Native support for common matrix ops: `zeros`, `eye`, `mult`, etc.
- Ideal for physics sims, math tools, or procedural generation

### 6. Graphics Layer

- 128×128 pixel canvas
- Functions like `set_pixel(x, y, color)`
- Works inside a special `draw()` function for frame-based rendering

### 7. Comments

- Single-line: `\\ Comment`
- Multi-line: `\* ... *\`

---

## 🔭 What’s Next?

Piscript is evolving. Upcoming features may include:

- Object-oriented constructs
- Modular code organization
- Error handling and debugging tools
- File I/O (in sandboxed or virtual environments)
