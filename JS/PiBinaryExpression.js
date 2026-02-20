import TokenType from "./TokenType.js";

import PiExpression from "./PiExpression.js";

export default class PiBinaryExpression extends PiExpression {
  //   // instance vars
  //   type;
  //   name;
  //   left;
  //   right;
  //   container;
  //   index;
  //   value = null;

  /**
   * Constructor for a PiBinaryExpression.
   * @param {PiExpression} left - The left operand.
   * @param {Object} op - The operator.
   * @param {PiExpression} right - The right operand.
   * @param {Object} [endToken=null] - The end token of the expression.
   */
  constructor(left, op, right, endToken = null) {
    super(left.getStartToken(), endToken || right.getLastToken());
    this.type = op.type;
    this.name = op.value;
    this.left = left;
    this.right = right;
    this._endToken = endToken;
  }
  /**
   * Formats the expression as a string.
   * @param {Number} [indent=0] - The indentation level.
   * @param {Array<String>} [comments=[]] - The comments to include.
   * @param {Number} [parentPrecedence=Infinity] - The precedence of the parent expression.
   * @returns {String} The formatted string.
   */
  format(indent = 0, comments = [], parentPrecedence = Infinity) {
    const precedence = PiBinaryExpression.getPrecedence(this.type);
    const maxLineLength = 80;

    const op = ` ${this.name} `;

    // First, try to format on a single line to check the length
    const leftSingle = this.left.format(0, [], precedence);
    const rightSingle = this.right.format(0, [], precedence);
    const singleLineLength =
      indent + leftSingle.length + op.length + rightSingle.length;

    const shouldBreak =
      (this.type === TokenType.AND || this.type === TokenType.OR) &&
      (singleLineLength > maxLineLength ||
        leftSingle.includes("\n") ||
        rightSingle.includes("\n"));

    let result;

    if (shouldBreak) {
      const multiLineOp = ` ${this.name}\n`;
      // When breaking, format left with indent, and manually indent the right side.
      const leftMulti = this.left.format(indent, comments, precedence);
      const rightMulti = this.right.format(0, comments, precedence); // Get unindented string
      result = leftMulti + multiLineOp + this.indent(indent) + rightMulti;
    } else {
      // Format on a single line
      const left = this.left.format(indent, comments, precedence);
      const right = this.right.format(0, comments, precedence); // right side has 0 indent as it's on the same line
      result = left + op + right;
    }

    result += this.name === "[" ? "]" : "";

    // Add parentheses if needed
    if (precedence > parentPrecedence && this.name !== "[") {
      result = `(${result})`;
    }

    return result;
  }

  /**
   * Minifies the expression.
   * @param {Object} [context={}] - The minification context.
   * @param {Number} [parentPrecedence=Infinity] - The precedence of the parent expression.
   * @returns {String} The minimized expression.
   */
  minify(context = {}, parentPrecedence = Infinity) {
    const precedence = PiBinaryExpression.getPrecedence(this.type);

    // Recursively minify children
    let left = this.left.minify(context, precedence);
    let right = this.right.minify(context, precedence);

    let op;
    if (this.name === "[") {
      return left + "[" + right + "]";
    } else {
      op = this.name;
    }

    // Prevent `<-` confusion
    let needsSpace = op === "<" && right.startsWith("-");

    let result = left + op + (needsSpace ? " " : "") + right;

    if (precedence >= parentPrecedence && this.name !== "[") {
      result = "(" + result + ")";
    }

    return result;
  }

  /**
   * Returns the precedence of the given token type.
   * The precedence is a measure of how "tightly bound" an operator is.
   * Operators with higher precedence are evaluated first.
   * @param {Object} type - The token type.
   * @returns {Number} The precedence of the token type.
   * @example
   * const precedence = PiBinaryExpression.getPrecedence(TokenType.PLUS);
   * console.log(precedence); // 6
   */
  static getPrecedence(type) {
    /**
     * Precedence levels:
     * 1. Member access
     * 2. Postfix operators
     * 3. Unary operators
     * 4. Exponentiation
     * 5. Multiplication, division, modulus
     * 6. Addition, subtraction
     * 7. Bitwise shift
     * 8. Comparison
     * 9. Bitwise AND
     * 10. Bitwise XOR
     * 11. Bitwise OR
     * 12. Logical AND
     * 13. Logical OR
     * 14. Conditional (ternary)
     * 15. Assignment
     */
    switch (type) {
      case TokenType.DOT:
        return 1;

      case TokenType.INCR:
      case TokenType.DECR:
      case TokenType.TICK:
      case TokenType.DBQUOTE:
      case TokenType.QUOTE:
      case TokenType.HASH:
        return 2;

      case TokenType.NOT:
      case TokenType.BITNEG:
        return 3;

      case TokenType.POWER:
        return 4;

      case TokenType.MULT:
      case TokenType.DIV:
      case TokenType.MOD:
      case TokenType.DOT_PROD:
        return 5;

      case TokenType.PLUS:
      case TokenType.MINUS:
        return 6;

      case TokenType.LSHIFT:
      case TokenType.RSHIFT:
      case TokenType.URSHIFT:
        return 7;

      case TokenType.LESS:
      case TokenType.GREATER:
      case TokenType.EQUAL:
      case TokenType.DBDOTS:
        return 8;

      case TokenType.BITAND:
        return 9;

      case TokenType.XOR:
        return 10;

      case TokenType.BITOR:
        return 11;

      case TokenType.AND:
        return 12;

      case TokenType.OR:
        return 13;

      case TokenType.QUESTION: // ternary
        return 14;

      case TokenType.ASSIGN:
      case TokenType.LARROW:
      case TokenType.RARROW:
        return 15;

      default:
        return 100; // lowest (safe fallback)
    }
  }

  getLeft() {
    return this.left;
  }

  getRight() {
    return this.right;
  }
}
