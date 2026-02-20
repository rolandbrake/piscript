import { readFileSync } from "fs";
import ParseError from "./ParseError.js";
import PiException from "./PiException.js";
import PiParser from "./PiParser.js";
import PiScanner from "./PiScanner.js";
import PiContext from "./PiContext.js";

export default class PiTester {
  static runFromFile(filePath, screen) {
    try {
      // Read the file synchronously (use readFile for async version)
      const source = readFileSync(filePath, "utf-8");
      return this.run(source, screen);
    } catch (e) {
      if (e.code === "ENOENT") {
        console.error(
          `\u001B[31mError: File not found at ${filePath}\u001B[0m`
        );
      } else {
        console.error(`\u001B[31mError reading file: ${e.message}\u001B[0m`);
      }
      process.exit(1);
    }
  }

  static run(source, screen) {
    const BUILTINS = [
      "this",
      // Audio Processing Functions
      "audio",
      "sound",
      "play",
      "stop",

      // Collection Manipulation Functions
      "pop",
      "push",
      "peek",
      "empty",
      "sort",
      "insert",
      "remove",
      "unshift",
      "append",
      "find",
      "contains",
      "index_of",
      "len",
      "range",

      // Map/Object Manipulation Functions
      "keys",
      "vals",

      // Functional Programming Functions
      "map",
      "reduce",
      "fill",
      "join",
      "reverse",
      "slice",
      "filter",
      "clone",
      "cont",

      // Input/Output Functions
      "println",
      "print",
      "printf",
      "text",
      "key",
      "input",

      // Matrix Manipulation Functions
      "zeros",
      "ones",
      "eye",
      "mult",
      "dot",
      "cross",
      "rand_m",
      "is_mat",
      "size",

      // Mathematical Functions
      "floor",
      "ceil",
      "round",
      "rand",
      "rand_n",
      "rand_i",
      "sqrt",
      "sin",
      "cos",
      "tan",
      "asin",
      "acos",
      "atan",
      "deg",
      "rad",
      "sum",
      "exp",
      "log2",
      "log10",
      "pow",
      "abs",
      "mean",
      "avg",
      "var",
      "dev",
      "median",
      "std",
      "mode",
      "max",
      "min",

      // Screen Rendering Functions
      "pixel",
      "line",
      "draw",
      "clear",
      "circ",
      "rect",
      "poly",
      "sprite",
      "color",

      // String Manipulation Functions
      "char",
      "ord",
      "trim",
      "upper",
      "lower",
      "replace",
      "split",
      "join",
      "is_upper",
      "is_lower",
      "is_digit",
      "is_numeric",
      "is_alpha",
      "is_alnum",

      // Time Functions
      "sleep",
      "time",

      // Type Functions
      "type",
      "is_list",
      "is_map",
      "is_num",
      "is_str",
      "is_bool",
      "is_inf",
      "as_list",
      "as_num",
      "as_str",
      "as_bool",

      // System Functions
      "fps",
      "error",
      "cursor",
      "mouse",

      // File Handling
      "open",
      "read",
      "write",
      "seek",
      "close",

      // 3D Graphics Rendering
      "load3d",
      "proj3d",
      "rot3d",
      "scale3d",
      "tran3d",

      // Image Manipulation
      "image",
      "crop",
      "resize",
      "flip",
      "rend2d",
      "scale2d",
      "tran2d",
      "rot2d",
      "copy2d",

      // Builtin Constants
      "PI",
      "E",
      "WIDTH",
      "HEIGHT",
      "KEYS",
    ];

    console.log("PiScript Tester Version 0.1");

    try {
      const scanner = new PiScanner(source);
      const tokens = scanner.scanTokens();

      const parser = new PiParser();
      const statements = parser.parse(tokens);

      let code = statements.format(0, scanner.comments);
      // let code = statements.minify(new PiContext(BUILTINS));
      console.log(code);
    } catch (e) {
      if (e instanceof PiException) {
        console.error(
          `\u001B[31mPiScript Version 0.1:  Error: [PiException] \u001B[0m${e}`
        );
      } else if (e instanceof ParseError) {
        console.log(e);
        console.error(
          `\u001B[31mPiScript Version 0.1: Syntax error: [ParseError] \u001B[0m${e}`
        );
      } else {
        console.log(e);
        console.error(
          `\u001B[31mPiScript Version 0.1: Error: Something went wrong!: ${e.constructor.name}\u001B[0m`
        );
      }
    }
  }
}

// Usage example:
// Get file path from command line arguments
const filePath = process.argv[2]; // node PiTester.js yourscript.pi

if (!filePath) {
  console.error("Usage: node PiTester.js <script-file>");
  process.exit(1);
}

PiTester.runFromFile(filePath, null);
