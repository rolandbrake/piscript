import PiScope from "./PiScope.js";
export default class PiScopeStack {
  constructor() {
    this.stack = [];
  }

  push(scope = new PiScope()) {
    this.stack.push(scope);
  }

  pop() {
    if (this.stack.length > 1) {
      // global scope cannot be popped
      this.stack.pop();
    }
  }

  peek() {
    return this.stack[this.stack.length - 1];
  }

  depth() {
    return this.stack.length;
  }

  isGlobalScope() {
    return this.stack.length === 1;
  }

  peekFirst() {
    return this.stack[0];
  }

  /**
   * @param {string} name
   * @return {?string} Returns the original name if found in the scope stack, otherwise null.
   */
  // search from innermost → outermost
  getValue(name) {
    for (let i = this.stack.length - 1; i >= 0; i--) {
      let val = this.stack[i].getValue(name);
      if (val) return val;
    }
    return null;
  }
}
