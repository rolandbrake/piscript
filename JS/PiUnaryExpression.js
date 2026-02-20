import PiExpression from "./PiExpression.js";

export default class PiUnaryExpression extends PiExpression {
  constructor(op, sub, prefix = true, access = false) {
    let startToken, lastToken;
    if (prefix) {
      startToken = op;
      lastToken = sub.getLastToken();
    } else {
      // postfix
      startToken = sub.getStartToken();
      lastToken = op;
    }
    super(startToken, lastToken);
    this._type = op.type;
    this._name = op.value;
    this._sub = sub;
    this._prefix = prefix;
    this._access = access;
  }

  format(indent = 0, comments = [], parentPrecedence = Infinity) {
    // precedence of unary ops (higher than binary, but not atoms)
    const precedence = 7;

    let expr;
    if (this._prefix) {
      expr = this._name + this._sub.format(0, comments, precedence);
    } else {
      expr = this._sub.format(0, comments, precedence) + this._name;
    }

    // wrap in parens if parent needs stronger precedence
    if (precedence < parentPrecedence) expr = "(" + expr + ")";

    // indent only at statement level
    return indent > 0 ? this.indent(indent) + expr : expr;
  }

  minify(context, parentPrecedence = Infinity) {
    const precedence = 7; // precedence of unary ops

    let expr;
    if (this._prefix) {
      // prefix form: -x, !y, etc.
      expr = this._name + this._sub.minify(context, precedence);
    } else {
      // postfix form: x++, x--, etc.
      expr = this._sub.minify(context, precedence) + this._name;
    }

    // Only wrap in parens if this expression binds weaker than parent
    if (precedence > parentPrecedence) {
      expr = "(" + expr + ")";
    }

    return expr;
  }
}
