import PiStatement from "./PiStatement.js";

export default class PiBlockStatement extends PiStatement {
  _compoundStatement;

  constructor(startToken, compoundStatement, endToken) {
    super(startToken, endToken);
    this._compoundStatement = compoundStatement;
    this.isBlock = true;
  }

  format(indent = 0, isStatement = false) {
    // TODO: Review indentation logic if issues persist.
    let result = "";

    if (isStatement) {
      // Opening brace directly after a statement header (e.g., if, for)
      result += " ";
    } else {
      result += this.indent(indent);
    }

    result += this.formatComments(this.getStartToken(), indent, "leading");
    result += "{";
    result += this.formatComments(this.getStartToken(), indent, "trailing");

    // Body: delegate to compoundStatement if present
    if (this._compoundStatement && this._compoundStatement.size() > 0) {
      const bodyStr = this._compoundStatement.format(indent + 2);
      if (!bodyStr.startsWith('\n')) {
        result += "\n";
      }
      result += bodyStr;
      result += "\n";
    } else {
      result += "\n";
    }

    // Closing brace
    const leadingCommentsForBrace = this.formatComments(this.getLastToken(), indent, "leading");
    if (leadingCommentsForBrace.length > 0) {
      result += leadingCommentsForBrace;
    }
    if (result.length === 0 || result.endsWith('\n') || leadingCommentsForBrace.length === 0) {
      result += this.indent(indent);
    }
    result += "}";
    // Trailing comments for '}' are handled by the parent statement.

    return result;
  }

  minify(context) {
    let s = "{";

    // enter new scope
    context.pushScope();

    if (this._compoundStatement && this._compoundStatement.size() > 0) {
      for (let i = 0; i < this._compoundStatement.size(); i++) {
        const stmt = this._compoundStatement.get(i);
        s += stmt.minify(context);
      }
    }

    // exit scope
    context.popScope();

    s += "}";
    return s;
  }
}
