export default class PiReturnException extends Error {
  constructor(val) {
    super();
    this.retVal = val;
  }
}
