export default class PiMangler {
  constructor() {
    this.counter = 0;
    this.map = new Map(); // original -> minified
    this.reverse = new Map(); // minified -> original
  }

  nextName() {
    let name = "";
    let n = this.counter++;
    do {
      name = String.fromCharCode(97 + (n % 26)) + name; // a-z
      n = Math.floor(n / 26);
    } while (n > 0);
    return name;
  }

  /**
   * @param {string} original
   * @return {string} minified
   */
  getName(original) {
    if (!this.map.has(original)) {
      const newName = this.nextName();
      this.map.set(original, newName);
      this.reverse.set(newName, original);
    }
    return this.map.get(original);
  }
}
