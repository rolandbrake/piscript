import PiStatement from "./PiStatement.js";

export default class PiVarDeclarations extends PiStatement {
  constructor(letToken, declarations, semicolonToken = null) {
    const lastToken =
      semicolonToken || declarations[declarations.length - 1].getLastToken();
    super(letToken, lastToken);
    this._letToken = letToken;
    this._declarations = declarations;
    this._semicolon = semicolonToken;
  }

  format(indent = 0) {
    let result = "";
    const leadingComments = this.formatComments(this._letToken, indent, "leading");
    if (leadingComments.length > 0) {
      result += leadingComments;
    }
    if (result.length === 0 || result.endsWith('\n')) {
      result += this.indent(indent);
    }

    result += "let";
    result += this.formatComments(this._letToken, indent, "trailing");
    result += " ";

    const declStrings = this._declarations.map((decl) => decl.format(0));
    result += declStrings.join(", ");

    if (this._semicolon) {
      result += this.formatComments(this._semicolon, indent, "leading");
      result += ";";
      result += this.formatComments(this._semicolon, indent, "trailing");
    } else result += ";";

    return result;
  }

  minify(context) {
    let s = "let ";
    for (let i = 0; i < this._declarations.length; i++) {
      const d = this._declarations[i];
      const name = d.getName();

      // Register the declared variable in the current scope
      const mangled = context.setValue(name);

      s += mangled;
      if (d.getInit()) {
        s += "=" + d.getInit().minify(context);
      }
      s += i < this._declarations.length - 1 ? "," : "";
    }

    s += ";";
    return s;
  }
}
