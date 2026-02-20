import PiStatement from "./PiStatement.js";
import TokenType from "./TokenType.js";

export default class PiIfStatement extends PiStatement {
  constructor(
    ifToken,
    lparen,
    cond,
    rparen,
    thenStmt,
    elseToken = null,
    elseStmt = null
  ) {
    super(ifToken, (elseStmt || thenStmt).getLastToken());
    this._ifToken = ifToken;
    this._lparen = lparen;
    this._cond = cond;
    this._rparen = rparen;
    this._then = thenStmt;
    this._elseToken = elseToken;
    this._else = elseStmt;
  }

  format(indent = 0, isElifChain = false) {
    let result = "";
    const leadingComments = this.formatComments(
      this._ifToken,
      indent,
      "leading"
    );
    if (leadingComments.length > 0) {
      result += leadingComments;
    }
    if (result.length === 0 || result.endsWith("\n")) {
      if (!isElifChain) {
        result += this.indent(indent);
      }
    }

    if (!isElifChain) {
      // Regular if statement
      result += "if";
      result += this.formatComments(this._ifToken, indent, "trailing");
      result += " "; // Space after "if"
    } else {
      // elif chain - we already have the "elif" keyword from parent
      // No space needed here, the space will be added after "elif"
    }

    result += this.formatComments(this._lparen, indent, "leading");
    result += "(";
    result += this.formatComments(this._lparen, indent, "trailing");

    result += this._cond.format(0);

    result += this.formatComments(this._rparen, indent, "leading");
    result += ")";
    result += this.formatComments(this._rparen, indent, "trailing");

    // --- THEN BODY ---
    result += this._formatBody(this._then, indent);

    // --- ELSE / ELIF ---
    if (this._else) {
      const thenIsBlock = !!this._then.isBlock;

      if (thenIsBlock) {
        // Inline on same line as closing brace
        result = result.replace(/\s+$/u, ""); // Trim trailing newlines
        result += " ";
      } else {
        // Same indentation level as 'if', not deeper
        result += "\n" + this.indent(indent);
      }

      let elseBlockStr = "";
      const leadingComments = this.formatComments(
        this._elseToken,
        indent,
        "leading"
      );
      if (leadingComments.length > 0) {
        elseBlockStr += leadingComments;
        // If leading comments end with newline, re-indent properly
        if (elseBlockStr.endsWith("\n")) {
          elseBlockStr += this.indent(indent);
        }
      }

      if (this._elseToken.type === TokenType.ELIF) {
        elseBlockStr += "elif";
        elseBlockStr += this.formatComments(
          this._elseToken,
          indent,
          "trailing"
        );
        elseBlockStr += " "; // CRITICAL: Add space after elif before condition
      } else {
        elseBlockStr += "else";
        elseBlockStr += this.formatComments(
          this._elseToken,
          indent,
          "trailing"
        );
        // For else, we don't add space here since _formatBody will handle it
      }

      result += elseBlockStr;

      if (this._else instanceof PiIfStatement) {
        // For elif chains, pass true to indicate it's part of a chain
        result += this._else.format(indent, true);
      } else {
        result += this._formatBody(this._else, indent);
      }
    }

    return result;
  }

  _formatBody(stmt, indent) {
    if (stmt.isBlock) {
      return stmt.format(indent, true);
    } else {
      const formattedBody = stmt.format(0).trim();
      if (formattedBody.includes("\n")) {
        return "\n" + stmt.format(indent + 2);
      } else {
        return " " + formattedBody;
      }
    }
  }

  minify(context) {
    let s = "if(" + this._cond.minify(context) + ")";
    s += this._then.minify(context);

    if (this._else != null) s += this._minify(this._else, context);

    return s;
  }

  _minify(st, context) {
    if (st instanceof PiIfStatement) {
      return (
        "elif(" +
        st._cond.minify(context) +
        ")" +
        st._then.minify(context) +
        (st._else ? this._minify(st._else, context) : "")
      );
    } else return "else " + st.minify(context);
  }
}