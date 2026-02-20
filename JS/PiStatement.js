export default class PiStatement {
  constructor(token, lastToken) {
    this._token = token;
    this._lastToken = lastToken || token;
    this._line = token ? token.line : 1;
    this._col = token ? token.column : 1;
    this.isBlock = false;
  }

  getStartToken() {
    return this._token;
  }

  getLastToken() {
    return this._lastToken;
  }

  setLineCol(context) {
    context.setLineCol(this._line, this._col);
  }

  indent(indent) {
    let s = "";
    for (let i = 0; i < indent; i++) {
      s += " ";
    }
    return s;
  }

  /**
   * Formats the comments for the given token based on the type.
   * @param {Object} token - The token to format comments for.
   * @param {Number} indent - The indentation level for block comments.
   * @param {String} type - The type of comments to format, either "leading" or "trailing".
   * @returns {String} The formatted comments.
   */
  formatComments(token, indent, type) {
    if (!token) {
      return "";
    }
    let result = "";
    const indentStr = this.indent(indent);
    const comments =
      (type === "leading" ? token.leadingComments : token.trailingComments) ||
      [];

    for (const c of comments) {
      const comment = c.kind === "line" ? `// ${c.text}` : `/* ${c.text} *\/`;
      if (c.line === token.line) {
        // Comment on the same line as the token.
        if (type === "leading") {
          // Leading on same line: /*C*/ token -> "/*C*/ "
          result += `${comment} `;
        } else {
          // Trailing on same line: token // C -> " // C"
          result += ` ${comment}`;
        }
      } else {
        // Block/leading comments on separate lines
        result += `${indentStr}${comment}\n`;
      }
    }
    return result;
  }

  format(indent) {
    throw new Error("Abstract method format must be implemented");
  }

  minify(context = {}) {
    throw new Error("Abstract method minify must be implemented");
  }
  // for future use
  gen(compiler) {
    throw new Error("Unimplemented method");
  }
}
