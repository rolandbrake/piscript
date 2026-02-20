import PiExpression from "./PiExpression.js";
/**
 * Definition of expression tree for ?: operator.
 */

export default class PiConditionalExpression extends PiExpression {
  constructor(t, cond, thenExpr, elseExpr) {
    super(cond.getStartToken(), elseExpr.getLastToken());
    this._cond = cond;
    this._then = thenExpr;
    this._else = elseExpr;
  }

  format(indent = 0, comments = [], parentPrecedence = Infinity) {
    const precedence = 14;

    // format children
    let _cond = this._cond.format(0, comments, precedence);
    let _then = this._then.format(0, comments, precedence);
    let _else = this._else ? this._else.format(0, comments, precedence) : "";

    let result = _cond + " ? " + _then + " : " + _else;

    // add parentheses if nested in tighter expression
    if (precedence > parentPrecedence) {
      result = "(" + result + ")";
    }

    if (indent > 0) {
      return this.indent(indent) + result;
    }
    return result;
  }

  minify(context, parentPrecedence = Infinity) {
    const precedence = 14;

    let _cond = this._cond.minify(context, precedence);
    let _then = this._then.minify(context, precedence);
    let _else = this._else ? this._else.minify(context, precedence) : "";

    let result = _cond + "?" + _then + ":" + _else;

    if (precedence > parentPrecedence) {
      result = "(" + result + ")";
    }
    return result;
  }
}
