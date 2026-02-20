import TokenType from "./TokenType.js";
import Token from "./Token.js";

export default class PiScanner {
  constructor(source) {
    this.source = source;
    this.tokens = [];
    this.start = 0;
    this.current = 0;
    this.line = 1;
    this.column = 1;
    this.ch = null;
    this.pendingComments = []; // Buffer for comments waiting to be attached

    this.keywords = {
      false: TokenType.FALSE,
      true: TokenType.TRUE,
      for: TokenType.FOR,
      in: TokenType.IN,
      while: TokenType.WHILE,
      fun: TokenType.FUN,
      let: TokenType.LET,
      INF: TokenType.INF,
      NaN: TokenType.NaN,
      break: TokenType.BREAK,
      continue: TokenType.CONTINUE,
      goto: TokenType.GOTO,
      if: TokenType.IF,
      else: TokenType.ELSE,
      elif: TokenType.ELIF,
      nil: TokenType.NIL,
      return: TokenType.RETURN,
      class: TokenType.CLASS,
      this: TokenType.THIS,
      println: TokenType.PRINTLN,
      assert: TokenType.ASSERT,
      typeof: TokenType.TYPEOF,
      debug: TokenType.DEBUG,
      import: TokenType.IMPORT,
    };
  }

  scanTokens() {
    while (!this.isAtEnd()) {
      this.start = this.current;
      this.scanToken();
    }

    if (this.pendingComments.length > 0) {
      const lastToken =
        this.tokens.length > 0 ? this.tokens[this.tokens.length - 1] : null;
      if (lastToken) {
        const trailingComments = [];
        const remainingComments = [];
        for (const comment of this.pendingComments) {
          if (comment.line === lastToken.line) {
            trailingComments.push(comment);
          } else {
            remainingComments.push(comment);
          }
        }
        if (trailingComments.length > 0) {
          trailingComments.forEach((c) => lastToken.addTrailingComment(c));
        }
        this.pendingComments = remainingComments;
      }
    }

    // Attach any remaining pending comments to the EOF token
    const eofToken = new Token(TokenType.EOF, null, this.line, this.column);
    if (this.pendingComments.length > 0) {
      this.pendingComments.forEach((c) => eofToken.addLeadingComment(c));
      this.pendingComments = [];
    }
    this.tokens.push(eofToken);
    return this.tokens;
  }

  scanToken() {
    this.ch = this.next();
    switch (this.ch) {
      case "\r":
      case "\t":
      case " ":
        // Whitespace, ignore
        break;
      case "\n":
        this.line++;
        this.column = 1;
        this.start = this.current;
        // Newline itself acts as a separator for comments
        // Any buffered comments before this newline that were on a different line
        // than the *next* token would be leading comments.
        // We handle this in addToken, so just advance for now.
        break;

      case "/":
        if (this.match("/")) {
          // Single-line comment
          let comment = "";
          let line = this.line;
          let column = this.column - 1; // The '/' position

          while (!this.isAtEnd() && this.peek() !== "\n") {
            // Use peek for comment text
            comment += this.next(); // Consume character to build comment
          }

          this.pendingComments.push({
            text: comment,
            kind: "line",
            line: line,
            column: column,
          });
          // Do NOT add a regular token; just buffer the comment
          return;
        } else if (this.match("*")) {
          // Multi-line comment
          let comment = "";
          let line = this.line;
          let column = this.column - 1; // Position of '/'
          let closed = false;

          while (!this.isAtEnd()) {
            if (this.peek() === "*" && this.peek(1) === "/") {
              this.next();
              this.next(); // Consume '*' and '/'
              closed = true;
              break;
            }
            if (this.peek() === "\n") {
              // Handle newlines within multi-line comments
              this.line++;
              this.column = 1;
            }
            comment += this.next();
          }

          if (!closed) {
            this.error("Unclosed Comment");
          }

          this.pendingComments.push({
            text: comment,
            kind: "block",
            line: line,
            column: column,
          });
          return; // Do NOT add a regular token; just buffer the comment
        } else if (this.match("=")) {
          this.addToken(TokenType.DIV_ASSIGN);
        } else {
          this.addToken(TokenType.DIV);
        }
        break;

      case "[":
        this.addToken(TokenType.LBRACKET);
        break;
      case "]":
        this.addToken(TokenType.RBRACKET);
        break;
      case "{":
        this.addToken(TokenType.LBRACE);
        break;
      case "}":
        this.addToken(TokenType.RBRACE);
        break;
      case "(":
        this.addToken(TokenType.LPAREN);
        break;
      case ")":
        this.addToken(TokenType.RPAREN);
        break;
      case ";":
        this.addToken(TokenType.SEMICOLON);
        break;
      case ":":
        this.addToken(TokenType.COLON);
        break;
      case ",":
        this.addToken(TokenType.COMMA);
        break;
      case "?":
        this.addToken(TokenType.QUESTION);
        break;
      case "#":
        this.addToken(TokenType.HASH);
        break;
      case "=":
        if (this.match("=")) this.addToken(TokenType.EQUAL);
        else this.addToken(TokenType.ASSIGN);
        break;
      case "*":
        if (this.match("*")) {
          this.addToken(TokenType.POWER);
        } else if (this.match("=")) {
          this.addToken(TokenType.MULT_ASSIGN);
        } else if (this.match(".")) {
          this.addToken(TokenType.DOT_PROD);
        } else this.addToken(TokenType.MULT);
        break;
      case "+":
        if (this.match("=")) this.addToken(TokenType.PLUS_ASSIGN);
        else if (this.match("+")) this.addToken(TokenType.INCR);
        else this.addToken(TokenType.PLUS);
        break;
      case "-":
        if (this.match("=")) this.addToken(TokenType.MINUS_ASSIGN);
        else if (this.match("-")) this.addToken(TokenType.DECR);
        else if (this.match(">")) {
          this.addToken(TokenType.RARROW);
        } else this.addToken(TokenType.MINUS);
        break;
      case "%":
        if (this.match("=")) this.addToken(TokenType.MOD_ASSIGN);
        else this.addToken(TokenType.MOD);
        break;
      case "|":
        if (this.match("=")) this.addToken(TokenType.BITOR_ASSIGN);
        else if (this.match("|")) this.addToken(TokenType.OR);
        else this.addToken(TokenType.BITOR);
        break;
      case "&":
        if (this.match("=")) this.addToken(TokenType.BITAND_ASSIGN);
        else if (this.match("&")) this.addToken(TokenType.AND);
        else this.addToken(TokenType.BITAND);
        break;
      case "^":
        if (this.match("=")) this.addToken(TokenType.XOR_ASSIGN);
        else this.addToken(TokenType.XOR);
        break;
      case "~":
        this.addToken(TokenType.BITNEG);
        break;
      case "!":
        if (this.match("=")) this.addToken(TokenType.NOT_EQUAL);
        else this.addToken(TokenType.NOT);
        break;
      case "<":
        if (this.match("=")) this.addToken(TokenType.LESS_EQUAL);
        else if (this.match("<")) this.addToken(TokenType.LSHIFT);
        else if (this.match("-")) this.addToken(TokenType.LARROW);
        else this.addToken(TokenType.LESS);
        break;
      case ">":
        if (this.match("=")) this.addToken(TokenType.GREATER_EQUAL);
        else if (this.match(">")) {
          if (this.match(">")) {
            if (this.match("=")) this.addToken(TokenType.URSHIFT_ASSIGN);
            else this.addToken(TokenType.URSHIFT);
          } else this.addToken(TokenType.RSHIFT);
        } else this.addToken(TokenType.GREATER);
        break;
      case '"':
      case "'":
        let _ch = this.ch;
        while (!this.match(_ch)) {
          this.next();
          if (this.match("\n")) {
            this.line++;
            this.column = 1;
          } else if (this.isAtEnd()) this.error("Unterminated String");
        }
        this.addToken(
          TokenType.STR,
          this.source.substring(this.start, this.current)
        );
        break;
      case ".":
        let _prev = this.previous();
        this.ch = this.next();
        if (this.isDigit(this.ch) && _prev !== "]" && !this.isAlpha(_prev)) {
          this.decimal();
          this.addToken(
            TokenType.NUM,
            parseFloat(this.source.substring(this.start, this.current))
          );
        } else if (this.ch === ".") {
          this.ch = this.next();
          if (this.ch === ".") this.addToken(TokenType.ELLIPSIS);
          else {
            this.current--;
            this.addToken(TokenType.DBDOTS);
          }
        } else {
          this.current--;
          this.addToken(TokenType.DOT);
        }
        break;
      default:
        if (this.isDigit(this.ch)) {
          if (this.ch === "0") {
            if (this.match("xX")) {
              do {
                if (!this.isHexDigit(this.peek()))
                  this.error("invalid hexadecimal literal");
                do {
                  this.ch = this.next();
                } while (this.isHexDigit(this.peek()));
              } while (this.match("_"));
              this.addToken(
                TokenType.NUM,
                this.source.substring(this.start, this.current)
              );
            } else if (this.match("oO")) {
              do {
                if (!this.isOctDigit(this.peek()))
                  this.error("invalid octal literal");
                this.ch = this.next();
              } while (this.match(".") || this.isDigit(this.peek()));
              this.addToken(
                TokenType.NUM,
                this.parseOct(this.source.substring(this.start, this.current))
              );
            } else if (this.match("bB")) {
              do {
                if (!this.isBinDigit(this.peek()))
                  this.error("invalid binary literal");
                this.ch = this.next();
              } while (this.match(".") || this.isDigit(this.peek()));
              this.addToken(
                TokenType.NUM,
                this.parseBin(this.source.substring(this.start, this.current))
              );
            } else if (this.peek() === "." && this.peek(1) !== ".") {
              this.ch = this.next();
              if (this.isDigit(this.peek())) this.decimal();
              this.addToken(
                TokenType.NUM,
                parseFloat(this.source.substring(this.start, this.current))
              );
            } else if (this.isDigit(this.peek()))
              this.error(
                "leading zeros in decimal integer literals are not permitted"
              );
            else this.addToken(TokenType.NUM, 0);
          } else {
            while (this.isDigit(this.peek())) this.ch = this.next();
            if (this.peek() === "." && this.peek(1) !== ".") {
              this.ch = this.next();
              if (this.isDigit(this.peek())) this.decimal();
            }
            this.addToken(
              TokenType.NUM,
              parseFloat(this.source.substring(this.start, this.current))
            );
          }
        } else if (this.isAlpha(this.ch)) {
          while (this.isValidIdentifier(this.peek())) this.next();
          let name = this.source.substring(this.start, this.current);
          let type = name === "constructor" ? undefined : this.keywords[name];
          if (type === undefined) this.addToken(TokenType.ID);
          else this.addToken(type);
        }
        break;
    }
  }

  isAtEnd() {
    return this.current >= this.source.length;
  }

  next() {
    this.current++;
    this.column++;
    return this.source.charAt(this.current - 1);
  }

  match(expected) {
    if (this.isAtEnd()) return false;

    // Handle both strings and single characters
    const character = this.source.charAt(this.current);

    // If expected is a string, check if character matches any character in it
    if (typeof expected === "string" && expected.length > 0) {
      if (expected.includes(character)) {
        this.current++;
        this.column++;
        return true;
      }
      return false;
    }

    // Fallback to original single character matching
    if (character === expected) {
      this.current++;
      this.column++;
      return true;
    }

    return false;
  }
  peek(offset = 0) {
    if (this.current + offset >= this.source.length) return "\0";
    return this.source.charAt(this.current + offset);
  }

  addToken(type, value = null) {
    if (value === null) {
      value = this.source.substring(this.start, this.current);
    }
    const newToken = new Token(type, value, this.line, this.column);

    if (this.pendingComments.length > 0) {
      // Special case: If the new token is a comma, check for any pending line
      // comments on the same line and attach them as trailing comments to the comma.
      if (newToken.type === TokenType.COMMA) {
        const remainingComments = [];
        for (const comment of this.pendingComments) {
          if (comment.kind === "line" && comment.line === newToken.line) {
            newToken.addTrailingComment(comment);
          } else {
            remainingComments.push(comment);
          }
        }
        this.pendingComments = remainingComments;
      }

      const lastToken =
        this.tokens.length > 0 ? this.tokens[this.tokens.length - 1] : null;
      // Separate leading and trailing comments
      // leading comments for the new token
      const leadingComments = [];
      // trailing comments for the previously added token
      const trailingComments = [];

      for (const comment of this.pendingComments) {
        if (!lastToken || comment.line > lastToken.line) {
          // If there's no previous token, or the comment is on a new line,
          // it must be a leading comment for the new token.
          leadingComments.push(comment);
        } else {
          // The comment is on the same line as the last token.
          if (newToken.line > lastToken.line) {
            // The new token is on a different line, so the comment is a trailing comment for the last token.
            trailingComments.push(comment);
          } else {
            // The new token is on the same line as the comment and the last token.
            // This is the T1 /*c*/ T2 case.
            if (newToken.type === TokenType.SEMICOLON) {
              // Special case: `token ;` -> comment is trailing for token.
              trailingComments.push(comment);
            } else {
              // Default: comment is leading for the new token.
              leadingComments.push(comment);
            }
          }
        }
      }

      if (trailingComments.length > 0 && lastToken)
        trailingComments.forEach((c) => lastToken.addTrailingComment(c));

      if (leadingComments.length > 0)
        leadingComments.forEach((c) => newToken.addLeadingComment(c));

      this.pendingComments = [];
    }

    this.tokens.push(newToken);
  }

  isDigit(ch) {
    return ch >= "0" && ch <= "9";
  }

  isHexDigit(ch) {
    return /[0-9A-Fa-f]/.test(ch);
  }

  isOctDigit(ch) {
    return /[0-7]/.test(ch);
  }

  isBinDigit(ch) {
    return /[01]/.test(ch);
  }

  parseHex(num) {
    let digits = "0123456789ABCDEF";
    num = num.toUpperCase();
    let val = 0.0;
    for (let i = 2; i < num.length; i++) {
      let c = num.charAt(i);
      if (c !== "_" && c !== ".") {
        let d = digits.indexOf(c);
        val = 16 * val + d;
      }
    }
    return val;
  }

  parseOct(num) {
    let digits = "01234567";
    let val = 0.0;
    if (num.charAt(1) === "O" || num.charAt(1) === "o") num = num.substring(2);
    else num = num.substring(1);
    for (let i = 0; i < num.length; i++) {
      let c = num.charAt(i);
      if (c !== "_") {
        let d = digits.indexOf(c);
        val = 8 * val + d;
      }
    }
    return val;
  }

  parseBin(num) {
    let val = 0.0;
    for (let i = 2; i < num.length; i++)
      val = 2 * val + (num.charAt(i) === "1" ? 1.0 : 0.0);
    return val;
  }

  decimal() {
    while (this.isDigit(this.peek())) this.ch = this.next();
    if (this.match("eE")) {
      if (this.match("+-")) {
        this.ch = this.next();
        if (!this.isDigit(this.peek())) this.error("invalid decimal literal");
        while (this.isDigit(this.peek())) this.next();
      } else if (!this.isDigit(this.peek()))
        this.error("invalid decimal literal");
      while (this.isDigit(this.peek())) this.ch = this.next();
    }
  }

  isAlpha(ch) {
    return (ch >= "a" && ch <= "z") || (ch >= "A" && ch <= "Z") || ch === "_";
  }

  isValidIdentifier(ch) {
    return this.isAlpha(ch) || this.isDigit(ch);
  }

  error(message) {
    throw new Error(
      `Syntax Error: ${message} at line ${this.line}, column ${this.column}`
    );
  }

  previous() {
    let i = this.current - 2;
    while (i >= 0) {
      if (!this.isSpace(this.source.charAt(i))) return this.source.charAt(i);
      i--;
    }
    return "\0";
  }

  isSpace(ch) {
    return ch === " " || ch === "\t" || ch === "\r";
  }
}
