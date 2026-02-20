import PiExpression from "./PiExpression.js";

export default class PiListExpression extends PiExpression {
  constructor(start, exprs, commas, end) {
    super(start, end); // Pass the start token to the base class
    this.start = start; // The '[' token
    this._exprs = exprs; // Array of PiExpression nodes
    this.commas = commas; // Array of ',' tokens
    this.end = end; // The ']' token
  }

  getSize() {
    return this._exprs.length;
  }

  format(indent = 0) {
    // Handle the simple empty list case first
    if (this._exprs.length === 0) {
      let result = this.indent(indent);
      result += this.formatComments(this.start, indent, "leading");
      result += "[]";
      return result;
    }

    const maxLineLength = 80;
    const innerIndent = indent + 2;

    // 1. Format all element expressions and check for complexity.
    const formattedItems = this._exprs.map((expr) =>
      expr.format(0).replace(/[ \t]+$/, "")
    );
    const hasComplexItems = formattedItems.some(
      (item) => item.includes("\n") || item.includes("{") || item.includes("[")
    );
    const singleLineLength = formattedItems.join(", ").length + 2; // +2 for brackets

    const hasTrailingComment = this.commas.some((comma) => {
      if (!comma) {
        return false;
      }
      const trailingComments = this.formatComments(comma, 0, "trailing");
      return trailingComments && trailingComments.trim().length > 0;
    });

    // 2. Decide on the formatting strategy.
    const useMultiLine =
      hasComplexItems || singleLineLength > maxLineLength || hasTrailingComment;

    // 3. Build the final string.
    let result = this.indent(indent);
    result += this.formatComments(this.start, indent, "leading");
    result += "[";
    result += this.formatComments(this.start, indent, "trailing");

    if (useMultiLine) {
      result += "\n";
      const lines = [];
      let currentLineItems = [];
      let currentLineLength = innerIndent;

      this._exprs.forEach((expr, index) => {
        const formattedItem = formattedItems[index];
        const isLastItem = index === this._exprs.length - 1;

        // Build the string for the item, including comma and its comments
        let itemWithComma = formattedItem;
        let hasTrailingComment = false;
        if (!isLastItem) {
          const commaToken = this.commas[index];
          if (commaToken) {
            itemWithComma += this.formatComments(commaToken, 0, "leading");
            itemWithComma += ",";
            const trailingComments = this.formatComments(
              commaToken,
              0,
              "trailing"
            );
            if (trailingComments.trim().length > 0) {
              hasTrailingComment = true;
            }
            itemWithComma += trailingComments;
          }
        }

        const itemLength =
          itemWithComma.length + (currentLineItems.length > 0 ? 1 : 0);

        if (
          currentLineItems.length > 0 &&
          currentLineLength + itemLength > maxLineLength
        ) {
          lines.push(this.indent(innerIndent) + currentLineItems.join(" "));
          currentLineItems = [];
          currentLineLength = innerIndent;
        }

        currentLineItems.push(itemWithComma);
        currentLineLength += itemLength;

        if (hasTrailingComment) {
          lines.push(this.indent(innerIndent) + currentLineItems.join(" "));
          currentLineItems = [];
          currentLineLength = innerIndent;
        }
      });

      if (currentLineItems.length > 0) {
        lines.push(this.indent(innerIndent) + currentLineItems.join(" "));
      }

      result += lines.join("\n");
      result += "\n" + this.indent(indent);
    } else {
      // Single-line format
      this._exprs.forEach((expr, index) => {
        result += " ";
        result += formattedItems[index];
        if (index < this._exprs.length - 1) {
          const commaToken = this.commas[index];
          if (commaToken) {
            result += this.formatComments(commaToken, 0, "leading");
            result += ",";
            result += this.formatComments(commaToken, 0, "trailing");
          }
        }
      });
      result += " ";
    }

    result += this.formatComments(this.end, indent, "leading");
    result += "]";
    // Trailing comments for the whole list are handled by the parent statement

    return result;
  }

  minify(context) {
    if (this._exprs.length === 0) return "[]";

    let s = "[";
    for (let i = 0; i < this._exprs.length; i++) {
      s += this._exprs[i].minify(context);
      if (i < this._exprs.length - 1) s += ",";
    }
    s += "]";
    return s;
  }
}
