import ParseError from "./ParseError.js";
import PiException from "./PiException.js";
import PiParser from "./PiParser.js";
import PiScanner from "./PiScanner.js";

export default class PiFormatter {
  static format(source) {
    try {
      const scanner = new PiScanner(source);
      const tokens = scanner.scanTokens();
      const parser = new PiParser();
      const statements = parser.parse(tokens);
      return {
        success: true,
        code: statements.format(0, scanner.comments),
      };
    } catch (e) {
      let message;
      if (e instanceof PiException) {
        message = `PiException: ${e}`;
      } else if (e instanceof ParseError) {
        message = `ParseError: ${e}`;
      } else {
        message = `Error: ${e.constructor.name}: ${e.message}`;
      }
      return {
        success: false,
        error: message,
      };
    }
  }
}
