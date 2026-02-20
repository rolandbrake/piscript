import PiExpression from "./PiExpression.js";

export default class PiLiteralExpression extends PiExpression {
  constructor(token) {
    super(token, token);
    this._token = token;
    this._value = token.value;
    this._type = token.type;
    this._line = token.line;
    this._col = token.column;
  }

  format(indent = 0) {
    const _indent = this.indent(indent);
    let out = "";

    // 1. Comments that appear before the literal
    out += this.formatComments(this._token, indent, "leading");

    // 2. The literal value itself
    out += _indent + this._value.toString();

    // 3. Trailing comments that are on the same line
    out += this.formatComments(this._token, indent, "trailing");

    return out;
  }

  minify(context) {
    return this._value.toString();
  }
}
