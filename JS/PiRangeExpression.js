import PiExpression from "./PiExpression.js";

export default class PiRangeExpression extends PiExpression {
  constructor(token, start, end, step = null) {
    super(start.getStartToken(), (step || end).getLastToken());
    this._start = start;
    this._end = end;
    this._step = step;
  }

  format(indent, comments = []) {
    return (
      this._start.format(0, comments) +
      ".." +
      this._end.format(0, comments) +
      (this._step ? ":" + this._step.format(0, comments) : "")
    );
  }

  minify(context) {
    let s = this._start.minify(context) + ".." + this._end.minify(context);
    if (this._step) s += ":" + this._step.minify(context);
    return s;
  }
}
