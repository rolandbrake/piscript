import PiExpression from "./PiExpression.js";

export default class PiFuncDeclExpression extends PiExpression {
  constructor(
    token,
    params,
    body,
    name = null,
    isArrow = false,
    isMethod = false
  ) {
    super(token, body.getLastToken());
    this._params = params; // Map of parameters
    this._body = body;
    this._isArrow = isArrow;
    this._isMethod = isMethod;
    this._name = name;
  }

  setName(name) {
    this._name = name;
  }

  format(indent = 0) {
    let str = "";

    if (!this._isArrow && !this._isMethod) str += "fun ";
    str += "(";

    this._params.forEach((param, index) => {
      str += this.formatComments(param.nameToken, indent, "leading");
      str += param.nameToken.value;
      str += this.formatComments(param.nameToken, indent, "trailing");

      if (param.defaultValue !== null) {
        str += " = ";
        str += param.defaultValue.format(indent);
      }

      if (param.commaToken !== null && index < this._params.length - 1) {
        str += this.formatComments(param.commaToken, indent, "leading");
        str += ",";
        str += this.formatComments(param.commaToken, indent, "trailing");
        str += " ";
      }
    });

    str += ")";
    if (this._isArrow) str += " ->";

    if (this._body.isBlock) {
      str += this._body.format(indent, true);
    }
    else {
      str += " " + this._body.format(indent);
    }
    return str;
  }

  minify(context) {
    let str = "";

    if (!this._isArrow && !this._isMethod) {
      str += "fun ";
      if (this._name) str += this._name;
    }

    str += "(";

    // Create a new scope for the function parameters and body
    context.pushScope();

    this._params.forEach((param, index) => {
      const mangled = context.setValue(param.nameToken.value);
      str += mangled;
      if (param.defaultValue !== null) {
        str += "=" + param.defaultValue.minify(context);
      }
      if (param.commaToken !== null && index < this._params.length - 1) {
        str += ",";
      }
    });
    str += ")";

    if (this._isArrow) str += "->";

    str += this._body.minify(context);

    // Pop the scope after minifying the body
    context.popScope();

    return str;
  }
}
