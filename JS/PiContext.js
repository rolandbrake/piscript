import PiMangler from "./PiMangler.js";
import PiScopeStack from "./PiScopeStack.js";
import PiScope from "./PiScope.js";

export default class PiContext {
  constructor(builtins = [], mangle = true) {
    this.scopeStack = new PiScopeStack();

    if (mangle) this.mangler = new PiMangler();
    this.builtins = new Set(builtins);

    // Create global scope and preload builtins as identity mappings
    const global = new PiScope();
    builtins.forEach((name) => {
      global.setValue(name, name); // builtin -> itself initially
    });
    this.scopeStack.push(global);
  }

  pushScope() {
    this.scopeStack.push(new PiScope());
  }

  popScope() {
    this.scopeStack.pop();
  }

  /**
   * Declare a variable in the current scope (called for `let`, function params, for-loop vars, etc.)
   * Behavior:
   *  - If current scope already has a mapping:
   *      - If it's global and the existing mapping is the builtin identity (e.g. builtin was preloaded),
   *        override it with a new mangled name (this implements global redeclaration overriding a builtin).
   *      - Otherwise return the existing mapping (don't remangle).
   *  - If not present in current scope: create a new mangled mapping in current scope.
   *
   * Returns the mangled name for the declared identifier.
   */
  setValue(name) {
    const current = this.scopeStack.peek();
    const global = this.scopeStack.peekFirst();

    // If mangling disabled → always store identity mapping
    if (!this.mangler) {
      current.setValue(name, name);
      return name;
    }

    if (current.hasValue(name)) {
      const existing = current.getValue(name);
      // If we're in the global scope and the existing mapping is exactly the builtin identity,
      // that means the name was a preloaded builtin and the user now redeclares it at global level.
      // Override it with a new mangled name.
      if (current === global && existing === name && this.builtins.has(name)) {
        const mangled = this.mangler.getName(name);
        current.setValue(name, mangled);
        return mangled;
      }
      // Otherwise, keep the existing mapping.
      return existing;
    }

    // Not declared in current scope -> create new mangled mapping (shadowing possible)
    const mangled = this.mangler.getName(name);
    current.setValue(name, mangled);
    return mangled;
  }

  /**
   * Resolve a variable reference.
   * - If found in any scope: return its mapping (mangled or identity).
   * - If builtin and not shadowed: return builtin name (unmangled).
   * - Otherwise: treat as implicit global -> create a mangled entry in global scope and return it.
   */
  getValue(name) {


    // If mangling is disabled → always return the original name
    if (!this.mangler) return name;

    // If declared anywhere, return the mapping
    const found = this.scopeStack.getValue(name);
    if (found) return found;

    // If it's a builtin not shadowed, keep the builtin identity
    if (this.builtins.has(name)) {
      return name;
    }

    // Not found anywhere: treat as implicit global
    const global = this.scopeStack.peekFirst();
    const mangled = this.mangler.getName(name);
    global.setValue(name, mangled);
    return mangled;
  }
}
