import PiStatement from "./PiStatement.js";

export default class PiWhileStatement extends PiStatement {
  constructor(whileToken, lparen, cond, rparen, body) {
    super(whileToken, body.getLastToken());
    this._whileToken = whileToken;
    this._lparen = lparen;
    this._cond = cond;
    this._rparen = rparen;
    this._body = body;
  }

  format(indent = 0) {
    let result = "";
    const leadingComments = this.formatComments(this._whileToken, indent, "leading");
    if (leadingComments.length > 0) {
      result += leadingComments;
    }
    if (result.length === 0 || result.endsWith('\n')) {
      result += this.indent(indent);
    }

    result += "while";
    result += this.formatComments(this._whileToken, indent, "trailing");
    result += " ";

    result += this.formatComments(this._lparen, indent, "leading");
    result += "(";
    result += this.formatComments(this._lparen, indent, "trailing");

    result += this._cond.format(0);

    result += this.formatComments(this._rparen, indent, "leading");
    result += ")";
    result += this.formatComments(this._rparen, indent, "trailing");

    if (this._body.isBlock) {
      result += this._body.format(indent, true);
    } else {
      const formattedBody = this._body.format(0).trim();
      if (formattedBody.includes('\n')) {
        result += "\n" + this._body.format(indent + 2);
      } else {
        result += " " + formattedBody;
      }
    }

    return result;
  }

  minify(context) {
    let s = "while(" + this._cond.minify(context) + ")";
    s += this._body.minify(context);
    return s;
  }
}
