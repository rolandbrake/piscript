import PiStatement from "./PiStatement.js";

export default class PiReturnStatement extends PiStatement {
  constructor(token, ret, semicolonToken = null, isImplicit = false) {
    const lastToken = semicolonToken || (ret ? ret.getLastToken() : token);
    super(token, lastToken);
    this._ret = ret;
    this._semicolon = semicolonToken;
    this._isImplicit = isImplicit;
  }

  format(indent = 0) {
    if (this._isImplicit) {
      if (this._ret !== null) return this._ret.format(indent);
      return "";
    }

    let s = "";
    const leadingComments = this.formatComments(this.getStartToken(), indent, "leading");

    if (leadingComments.length > 0) {
      s += leadingComments;
    }

    if (s.length === 0 || s.endsWith('\n')) {
      s += this.indent(indent);
    }
    
    s += "return";
    s += this.formatComments(this.getStartToken(), indent, "trailing");

    if (this._ret !== null) {
      s += " " + this._ret.format(0);
    }

    if (this._semicolon) {
      s += this.formatComments(this._semicolon, indent, "leading");
      s += ";";
      s += this.formatComments(this._semicolon, indent, "trailing");
    } else {
      const lastToken = this._ret ? this._ret.getLastToken() : this.getStartToken();
      const trailingComments = this.formatComments(lastToken, indent, "trailing");

      if (trailingComments.length > 0 && s.endsWith(trailingComments)) {
        s = s.slice(0, -trailingComments.length);
      }
      
      s += ";";
      s += trailingComments;
    }
    return s;
  }

  minify(context) {
    if (this._isImplicit) return this._ret ? this._ret.minify(context) : "";
    return "return " + (this._ret ? this._ret.minify(context) : "") + ";";
  }
}
