import PiStatement from "./PiStatement.js";
import PiExpressionStatement from "./PiExpressionStatement.js";

export default class PiCompoundStatement extends PiStatement {
  constructor(token, isProgram = false) {
    super(token, token);
    this._v = [];
    this.isProgram = isProgram;
    this.eofToken = null;
  }

  add(s) {
    if (s) {
      this._v.push(s);
      this._lastToken = s.getLastToken();
    }
  }

  addFirst(s) {
    this._v.unshift(s);
    if (this._v.length === 1) {
      this._lastToken = s.getLastToken();
    }
  }

  get(i) {
    return this._v[i];
  }

  lastElement() {
    return this._v.length !== 0 ? this._v[this._v.length - 1] : null;
  }

  size() {
    return this._v.length;
  }

  format(indent = 0) {
    let result = "";
    for (let i = 0; i < this._v.length; i++) {
      const stmt = this._v[i];
      result += stmt.format(indent);
      if (i < this._v.length - 1) result += "\n";
    }
    if (this.isProgram) {
      result += "\n";
      if (this.eofToken) {
        // Format comments attached to the EOF token.
        // These are typically comments at the very end of the file.
        result += this.formatComments(this.eofToken, indent, "leading");
      }
    }
    return result;
  }

  minify(context) {
    let parts = this._v.map((stmt) => stmt.minify(context));

    // join with semicolons (safe even for empty return etc.)
    return parts.filter(Boolean).join("");
  }
}
