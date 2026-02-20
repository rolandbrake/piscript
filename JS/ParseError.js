import Token from "./Token.js";
export default class ParseError extends Error {
  /**
   * Constructor for ParseError
   * @param {string|Token} message The error message or a Token object
   * @param {number} [line] The line number of the error
   * @param {number} [column] The column number of the error
   */
  constructor(message, line, column) {
    if (line !== undefined && column !== undefined) {
      super(`[line: ${line}, column: ${column}] ${message}`);
    } else if (message instanceof Token) {
      const token = message;
      super(`[line: ${token.line}, column: ${token.column}] ${token.message}`);
    } else {
      super(message);
    }
  }

  /**
   * Returns a string representation of the error
   * @return {string} The error message
   */
  toString() {
    return this.message;
  }
}
