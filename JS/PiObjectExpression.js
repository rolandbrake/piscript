import PiExpression from "./PiExpression.js";

export default class PiObjectExpression extends PiExpression {
  constructor(startToken, properties, endToken) {
    super(startToken, endToken);
    this._startToken = startToken;
    this._properties = properties; // Array of { keyToken, colonToken, value, commaToken }
    this._endToken = endToken;
  }

  format(indent = 0) {
    // 1. Handle Empty Object
    if (this._properties.length === 0) {
      let result = this.indent(indent);
      result += this.formatComments(this._startToken, indent, "leading");
      result += "{}";
      result += this.formatComments(this._endToken, indent, "trailing");
      return result;
    }

    const maxLineLength = 80;
    const innerIndent = indent + 2;
    const innerIndentStr = this.indent(innerIndent);

    // 2. Analyze Contents for complexity and length
    let singleLineLength = 4; // Account for "{  }"
    let hasComplexItems = false;
    const formattedPropertyStrings = []; // Store formatted key: value strings for single-line check

    this._properties.forEach((prop, index) => {
      const valueStr = prop.value.format(0).trim(); // Format value without initial indent for length check
      if (
        valueStr.includes("\n") ||
        valueStr.includes("{") ||
        valueStr.includes("[")
      ) {
        hasComplexItems = true;
      }

      let keyStr = prop.keyToken.value;
      // Check if key needs quotes (not a simple identifier)
      if (!/^[a-zA-Z_][a-zA-Z0-9_]*$/.test(keyStr)) {
        keyStr = `"${keyStr}"`;
      }

      let entryPart = keyStr;
      if (prop.colonToken) {
        // Not a method shorthand
        entryPart += ": ";
      }
      entryPart += valueStr;
      formattedPropertyStrings.push(entryPart);
      singleLineLength +=
        entryPart.length + (index < this._properties.length - 1 ? 2 : 0); // +2 for ", "
    });

    // 3. Decide Strategy
    const useMultiLine = hasComplexItems || singleLineLength > maxLineLength;

    // 4. Build Output String
    let result = this.indent(indent);
    result += this.formatComments(this._startToken, indent, "leading");
    result += "{";
    result += this.formatComments(this._startToken, indent, "trailing");

    if (useMultiLine) {
      result += "\n";
      this._properties.forEach((prop, index) => {
        // Indent for the property line
        result += innerIndentStr;

        // Key and its comments
        result += this.formatComments(prop.keyToken, 0, "leading");
        let keyStr = prop.keyToken.value;
        if (!/^[a-zA-Z_][a-zA-Z0-9_]*$/.test(keyStr)) {
          keyStr = `"${keyStr}"`;
        }
        result += keyStr;
        result += this.formatComments(prop.keyToken, 0, "trailing");

        // Colon and its comments (if not a method shorthand)
        if (prop.colonToken) {
          result += this.formatComments(prop.colonToken, 0, "leading");
          result += ":";
          result += this.formatComments(prop.colonToken, 0, "trailing");
          result += " "; // Space after colon
        }

        // Value
        result += prop.value.format(innerIndent);

        // Comma and its comments (if not the last property)
        if (index < this._properties.length - 1) {
          if (prop.commaToken) {
            result += this.formatComments(prop.commaToken, 0, "leading");
            result += ",";
            result += this.formatComments(prop.commaToken, 0, "trailing");
          } else {
            result += ","; // Add comma even if token not present (e.g., for trailing comma)
          }
        }
        result += "\n";
      });
      result += this.indent(indent); // Indent for closing brace
    } else {
      // Single-line format
      result += " ";
      this._properties.forEach((prop, index) => {
        // Key and its comments
        result += this.formatComments(prop.keyToken, 0, "leading");
        let keyStr = prop.keyToken.value;
        if (!/^[a-zA-Z_][a-zA-Z0-9_]*$/.test(keyStr)) {
          keyStr = `"${keyStr}"`;
        }
        result += keyStr;
        result += this.formatComments(prop.keyToken, 0, "trailing");

        // Colon and its comments (if not a method shorthand)
        if (prop.colonToken) {
          result += this.formatComments(prop.colonToken, 0, "leading");
          result += ":";
          result += this.formatComments(prop.colonToken, 0, "trailing");
          result += " "; // Space after colon
        }

        // Value
        result += prop.value.format(0); // No extra indent for single line

        // Comma and its comments (if not the last property)
        if (index < this._properties.length - 1) {
          if (prop.commaToken) {
            result += this.formatComments(prop.commaToken, 0, "leading");
            result += ",";
            result += this.formatComments(prop.commaToken, 0, "trailing");
          } else {
            result += ","; // Add comma even if token not present
          }
          result += " "; // Space after comma
        }
      });
      result += " ";
    }

    result += this.formatComments(this._endToken, indent, "leading");
    result += "}";
    result += this.formatComments(this._endToken, indent, "trailing");
    // Trailing comments for '}' are handled by PiExpressionStatement

    return result;
  }

  minify(context) {
    if (this._properties.length === 0) return "{}";

    const parts = this._properties.map((prop) => {
      const key = prop.keyToken.value;
      const expr = prop.value;
      // If key needs quotes, add them for minified output
      let minifiedKey = /^[a-zA-Z_][a-zA-Z0-9_]*$/.test(key) ? key : `"${key}"`;
      return minifiedKey + (prop.colonToken ? ":" : "") + expr.minify(context);
    });

    return "{" + parts.join(",") + "}";
  }
}
