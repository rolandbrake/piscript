import PiExpression from "./PiExpression.js";

export default class PiFunctionExpression extends PiExpression {
  constructor(start, end, name, args) {
    super(name.getStartToken(), end);
    this._name = name;
    this._args = args; // Array of PiExpression's
    this._start = start;
    this._end = end;
  }

  format(indent, comments = []) {
    let length = this._args.length;
    let s = this._name.format(indent) + "(";

    for (let i = 0; i < length; i++) {
      s += this._args[i].format(0, comments);
      if (i < length - 1) {
        if (length > 4) s += ",\n " + this.indent(this._start.column - 4);
        else s += ", ";
      }
    }

    s += ")";

    return s;
  }

  minify(context) {
    const fnName = this._name.minify ? this._name.minify(context) : this._name;
    const args = this._args.map((arg) => arg.minify(context)).join(",");
    return fnName + `(${args})`;
  }
}
