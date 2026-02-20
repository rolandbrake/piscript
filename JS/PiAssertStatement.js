import PiStatement from "./PiStatement.js";

export default class PiAssertStatement extends PiStatement {
  /**
   * Constructor for a PiAssertStatement.
   * @param {Object} assertToken - The token of the assert keyword.
   * @param {Object} value - The value to be asserted.
   * @param {Object} semicolonToken - The semicolon token, which is optional.
   */
  constructor(assertToken, value, semicolonToken = null) {
    const lastToken = semicolonToken || value.getLastToken();
    super(assertToken, lastToken);
    this._assertToken = assertToken;
    this._value = value;
    this._semicolon = semicolonToken;
  }

  /**
   * Formats the assert statement.
   * @param {number} indent - The indentation level to format the statement.
   * @returns {string} The formatted assert statement.
   */
  format(indent = 0) {
    let result = this.indent(indent);

    result += this.formatComments(this._assertToken, indent, "leading");
    result += "assert";
    result += this.formatComments(this._assertToken, indent, "trailing");
    result += " ";

    result += this._value.format(0);

    if (this._semicolon) {
      result += this.formatComments(this._semicolon, indent, "leading");
      result += ";";
      result += this.formatComments(this._semicolon, indent, "trailing");
    }

    return result;
  }

  /**
   * Minifies the assert statement.
   * @param {Object} context - The context to minimize the statement with.
   * @returns {string} The minimized assert statement.
   */
  minify(context) {
    return "assert " + this._value.minify(context) + ";";
  }
}
