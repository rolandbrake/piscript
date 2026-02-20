import PiStatement from "./PiStatement.js";

export default class PiComment extends PiStatement {
  constructor(token) {
    super(token.line, token.col);
    this.text = token.value; // The actual comment string
  }

  format(indent) {
    return this.indent(indent) + this.text;
  }
}
