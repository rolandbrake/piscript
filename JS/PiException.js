export default class PiException extends Error {
  constructor(message, line, col) {
    super(message);
    if (line !== undefined && col !== undefined) {
      this.line = line;
      this.col = col;
    }
  }

  toString() {
    return `[line: ${this.line}, column: ${this.col}] ${this.message}`;
  }
}
