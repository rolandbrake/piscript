import TokenType from "./TokenType.js";
export default class Token {
  constructor(type, value, line, column) {
    this.type = type;
    this.value = value;
    this.line = line;
    this.column = column;
    this.leadingComments = []; // New: Comments before this token
    this.trailingComments = []; // New: Comments after this token, on same line
  }

  // Optional: Helper methods to add to these arrays
  addLeadingComment(comment) {
    this.leadingComments.push(comment);
  }

  addTrailingComment(comment) {
    this.trailingComments.push(comment);
  }

  getType() {
    return this.type;
  }

  getValue() {
    return this.value;
  }

  getLine() {
    return this.line;
  }

  getColumn() {
    return this.column;
  }

  toString() {
    if (this.type === TokenType.COMMENT)
      return `<COMMENT, '${this.value.text}'>`;

    return `<${this.type}, ${this.value}>`;
  }
}
