import PiStatement from "./PiStatement.js";

export default class PiFuncDeclStatement extends PiStatement {
  constructor(funToken, id, lparen, params, rparen, body) {
    super(funToken, body.getLastToken());
    this._funToken = funToken;
    this._id = id;
    this._lparen = lparen;
    this._params = params; // Map of parameters
    this._rparen = rparen;
    this._body = body;
    this._func = null; // Placeholder for PiFunctionValue if needed
  }

  format(indent = 0) {
    let result = this.indent(indent);

    result += this.formatComments(this._funToken, indent, "leading");
    result += "fun";
    result += this.formatComments(this._funToken, indent, "trailing");
    result += " ";

    result += this._id.format(0);

    result += this.formatComments(this._lparen, indent, "leading");
    result += "(";
    result += this.formatComments(this._lparen, indent, "trailing");

    this._params.forEach((param, index) => {
      // Format comments for the parameter's name token
      result += this.formatComments(param.nameToken, indent, "leading");
      result += param.nameToken.value; // Parameter name
      result += this.formatComments(param.nameToken, indent, "trailing");

      // Format default value if present
      if (param.defaultValue !== null) {
        result += " = ";
        result += param.defaultValue.format(0);
      }

      // Format comma and its comments if present
      // Ensure no comma is added after the last parameter
      if (param.commaToken !== null && index < this._params.length - 1) {
        result += this.formatComments(param.commaToken, indent, "leading");
        result += ",";
        result += this.formatComments(param.commaToken, indent, "trailing");
        result += " "; // Add space after comma for readability
      }
    });

    result += this.formatComments(this._rparen, indent, "leading");
    result += ")";
    result += this.formatComments(this._rparen, indent, "trailing");

    if (this._body.isBlock) {
      result += this._body.format(indent, true);
    } else {
      result += " " + this._body.format(0);
    }

    return result + "\n";
  }

  minify(context) {
    // declare function name in current scope
    const mangledName = context.setValue(this._id.getName());

    // new scope for parameters
    context.pushScope();

    let s = "fun " + mangledName + "(";
    this._params.forEach((param, index) => {
      const mangledParam = context.setValue(param.nameToken.value);
      s += mangledParam;
      if (param.defaultValue !== null) {
        s += "=" + param.defaultValue.minify(context);
      }
      if (index < this._params.length - 1) s += ",";
    });
    s += ")";

    s += this._body.minify(context);

    context.popScope();
    return s;
  }
}
