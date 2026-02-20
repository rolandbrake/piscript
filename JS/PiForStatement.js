import PiStatement from "./PiStatement.js";

export default class PiForStatement extends PiStatement {
  constructor(forToken, lparen, init, inToken, expr, rparen, body) {
    super(forToken, body.getLastToken());
    this._forToken = forToken;
    this._lparen = lparen;
    this._init = init;
    this._inToken = inToken;
    this._expr = expr;
    this._rparen = rparen;
    this._body = body;
  }

  format(indent = 0) {
    let result = "";
    const leadingComments = this.formatComments(this._forToken, indent, "leading");
    if (leadingComments.length > 0) {
      result += leadingComments;
    }
    if (result.length === 0 || result.endsWith('\n')) {
      result += this.indent(indent);
    }

    result += "for";
    result += this.formatComments(this._forToken, indent, "trailing");

    result += " ";
    result += this.formatComments(this._lparen, indent, "leading");
    result += "(";
    result += this.formatComments(this._lparen, indent, "trailing");

    result += this._init.format(0);

    result += " ";
    result += this.formatComments(this._inToken, indent, "leading");
    result += "in";
    result += this.formatComments(this._inToken, indent, "trailing");
    result += " ";

    result += this._expr.format(0);

    result += this.formatComments(this._rparen, indent, "leading");
    result += ")";
    result += this.formatComments(this._rparen, indent, "trailing");

    if (this._body.isBlock) {
      // The format method of PiBlockStatement handles the space and newlines
      result += this._body.format(indent, true); // isStatement = true
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
    // _init is always a variable
    let name = this._init._name;
    let mangled = context.setValue(name);
    let expr = this._expr.minify(context);

    let head = `for(${mangled} in ${expr})`;

    return head + this._body.minify(context);
  }
}
