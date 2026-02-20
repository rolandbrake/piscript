import PiExpression from "./PiExpression.js";

export default class PiIdentifier extends PiExpression {
  /**
   * @param {Token} token - The identifier token
   */
  constructor(token) {
    super(token, token);
    this._token = token;
    this._name = token.getValue();
  }

  getName() {
    return this._name;
  }

  /**
   * Pretty-print the identifier with comments
   */
  format(indent = 0) {
    let result = "";
    const _indent = this.indent(indent);

    // Leading comments
    result += this.formatComments(this._token, indent, "leading");

    // The identifier name itself
    result += _indent + this._name;

    // Trailing comments are handled by the statement.

    return result;
  }

  minify(context) {
    return context.getValue(this._name);
  }
}
