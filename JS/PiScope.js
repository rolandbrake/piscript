export default class PiScope {
  constructor() {
    this.names = new Map(); // original -> minified
  }

  hasValue(name) {
    return this.names.has(name);
  }

  setValue(original, mangled) {
    this.names.set(original, mangled);
  }

  getValue(name) {
    return this.names.get(name);
  }
}
