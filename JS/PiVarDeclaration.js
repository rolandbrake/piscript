import PiStatement from "./PiStatement.js";

export default class PiVarDeclaration extends PiStatement {
  constructor(nameToken, eqToken = null, initExpr = null) {
    super(nameToken, initExpr ? initExpr.getLastToken() : nameToken);
    this._nameToken = nameToken;
    this._name = nameToken.value;
    this._eqToken = eqToken;
    this._init = initExpr;
  }

  getName() {
    return this._name;
  }

  getInit() {
    return this._init;
  }

  format(indent = 0) {
    let result = "";

    result += this.formatComments(this._nameToken, indent, "leading");
    result += this._name;
    result += this.formatComments(this._nameToken, indent, "trailing");

    if (this._init) {
      result += " ";
      if (this._eqToken) {
        result += this.formatComments(this._eqToken, indent, "leading");
        result += "=";
        result += this.formatComments(this._eqToken, indent, "trailing");
      }
      result += " " + this._init.format(0);
    }

    return result;
  }
}
