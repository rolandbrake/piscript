import PiStatement from "./PiStatement.js";

export default class PiBreakStatement extends PiStatement {
  constructor(token, semicolonToken = null) {
    super(token, semicolonToken || token);
    this._semicolon = semicolonToken;
  }

  format(indent = 0) {
    let result = "";
    const leadingComments = this.formatComments(this.getStartToken(), indent, "leading");
    if (leadingComments.length > 0) {
      result += leadingComments;
    }
    if (result.length === 0 || result.endsWith('\n')) {
      result += this.indent(indent);
    }

    result += "break";
    const breakKeywordFormatted = result; // Store the result up to here.

    const trailingComments = this.formatComments(this.getStartToken(), indent, "trailing");

    if (this._semicolon) {
      result += trailingComments; // Add trailing comments if explicit semicolon
      result += this.formatComments(this._semicolon, indent, "leading");
      result += ";";
      result += this.formatComments(this._semicolon, indent, "trailing");
    } else {
      // No explicit semicolon, add one automatically.
      // We want: break; /* comment */
      result = breakKeywordFormatted; // Reset result to just "break"
      result += ";";
      result += trailingComments; // Add trailing comments after semicolon
    }

    return result;
  }

  minify(context) {
    // break doesn’t depend on variables or scope
    return "break;";
  }
}
