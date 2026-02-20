export default class PiExpression {
  constructor(token, lastToken) {
    this._token = token;
    this._line = token.line;
    this._col = token.column;
    this._lastToken = lastToken || token;
  }

  getStartToken() {
    return this._token;
  }

  getLastToken() {
    return this._lastToken;
  }
  getLine() {
    return this._line;
  }

  getColumn() {
    return this._col;
  }

  indent(indent) {
    let s = "";
    for (let i = 0; i < indent; i++) {
      s += " ";
    }
    return s;
  }

  hashCode() {
    const str = JSON.stringify(this);
    let hash = 0;
    for (let i = 0; i < str.length; i++) {
      const char = str.charCodeAt(i);
      hash = (hash << 5) - hash + char;
      hash |= 0; // Convert to 32bit integer
    }
    return hash >>> 0; // Convert to unsigned 32bit integer
  }

  format(indent) {
    // This method needs to be implemented in subclasses
    throw new Error("Abstract method format() must be implemented");
  }

  /**
   * Formats the comments for the given token based on the type.
   * @param {Object} token - The token to format comments for.
   * @param {Number} indent - The indentation level for block comments.
   * @param {String} type - The type of comments to format, either "leading" or "trailing".
   * @returns {String} The formatted comments.
   */
  formatComments(token, indent, type) {
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
          if (c.kind === "line") {
            result += ` ${comment}`;
          } else {
            result += ` ${comment}`;
          }
        }
      } else {
        // Block/leading comments on separate lines
        result += `\n${indentStr}${comment}\n`;
      }
    }
    return result;
  }

  minify() {
    // This method needs to be implemented in subclasses
    throw new Error("Abstract method minify() must be implemented");
  }

  // for future use
  gen(compiler) {
    throw new Error("Unimplemented method");
  }
}
