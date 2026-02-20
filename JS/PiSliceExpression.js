import PiExpression from "./PiExpression.js";
export default class PiSliceExpression extends PiExpression {
  constructor(startToken, lastToken, start, end, step) {
    super(startToken, lastToken);
    this._start = start;
    this._end = end;
    this._step = step;
  }

  format(indent, comments = []) {
    let parts = [];

    parts.push(this._start ? this._start.format(0, comments) : "");
    parts.push(":");
    parts.push(this._end ? this._end.format(0, comments) : "");
    if (this._step !== null) {
      parts.push(":");
      parts.push(this._step.format(0, comments));
    }

    return parts.join("");
  }

  minify(context) {
    let s = "";
    s += this._start ? this._start.minify(context) : "";
    s += ":";
    s += this._end ? this._end.minify(context) : "";
    if (this._step !== null) {
      s += ":";
      s += this._step.minify(context);
    }
    return s;
  }
}
