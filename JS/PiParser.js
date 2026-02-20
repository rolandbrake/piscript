import TokenType from "./TokenType.js";
import Token from "./Token.js";
import PiCompoundStatement from "./PiCompoundStatement.js";
import PiVarDeclarations from "./PiVarDeclarations.js";
import PiVarDeclaration from "./PiVarDeclaration.js";
import PiFuncDeclStatement from "./PiFuncDeclStatement.js";
import PiFuncDeclExpression from "./PiFuncDeclExpression.js";
import PiExpressionStatement from "./PiExpressionStatement.js";
import PiObjectExpression from "./PiObjectExpression.js";
import PiBlockStatement from "./PiBlockStatement.js";
import PiIfStatement from "./PiIfStatement.js";
import PiConditionalExpression from "./PiConditionalExpression.js";
import PiWhileStatement from "./PiWhileStatement.js";
import PiForStatement from "./PiForStatement.js";
import PiReturnStatement from "./PiReturnStatement.js";
import PiAssertStatement from "./PiAssertStatement.js";
import PiBreakStatement from "./PiBreakStatement.js";
import PiContinueStatement from "./PiContinueStatement.js";
import PiUnaryExpression from "./PiUnaryExpression.js";
import PiBinaryExpression from "./PiBinaryExpression.js";
import PiLiteralExpression from "./PiLiteralExpression.js";
import PiFunctionExpression from "./PiFunctionExpression.js";
import PiListExpression from "./PiListExpression.js";
import PiRangeExpression from "./PiRangeExpression.js";
import PiSliceExpression from "./PiSliceExpression.js";
import PiVariable from "./PiVariable.js";
import ParseError from "./ParseError.js";

export default class PiParser {
  FunctionDeclarations() {
    while (!this.isAtEnd()) {
      if (this.match(TokenType.FUN)) {
        this.statements.add(this.FunctionDeclaration());
      } else {
        this.advance();
      }
    }
  }

  parse(tokens) {
    this.tokens = tokens;
    this.current = 0;
    this.access = false;
    this.isStore = false;
    this.isReturn = false;
    this.loopDepth = 0;
    this.statements = new PiCompoundStatement(null, true);
    this.key = null;
    this.last = null;
    return this.Program();
  }

  /**
   * Parses the entire source code and returns the parsed program.
   * The program is a compound statement that contains all the declarations in the source code.
   * @returns {PiCompoundStatement} Parsed program
   */

  Program() {
    while (!this.isAtEnd()) {
      let current = this.current;

      // If only comments till EOF → stop cleanly
      if (this.isAtEnd()) {
        this.current = current;
        break;
      }

      const decl = this.Declaration();

      this.statements.add(decl);
    }

    this.statements.eofToken = this.peek();
    return this.statements;
  }

  /**
   * Parses a declaration statement.
   * It can be a variable declaration, function declaration, or a general statement.
   * @returns {PiVarDeclarations|PiFuncDeclStatement|PiStatement} Parsed statement
   */
  Declaration() {
    // Check if the declaration is a variable declaration
    if (this.match(TokenType.LET)) {
      return this.VarDeclaration(this.previous());
    }
    // Check if the declaration is a function declaration
    if (this.match(TokenType.FUN)) return this.FunctionDeclaration();

    // Otherwise, parse it as a general statement
    return this.Statement();
  }

  /**
   * Parses a variable declaration.
   * @param {Token} token the token marking the start of the declaration
   * @returns {PiVarDeclarations} Parsed variable declaration
   */
  VarDeclaration(token) {
    // Parse the variable declarations
    let vars = this.Variables();

    // Consume any optional delimiters (semicolon or newline)
    const semicolon = this.match(TokenType.SEMICOLON) ? this.previous() : null;
    // Return the parsed variable declaration
    return new PiVarDeclarations(token, vars, semicolon);
  }

  /**
   * Parses a list of variables.
   * @returns {PiVarDeclaration[]} List of parsed variable declarations
   */
  Variables() {
    let vars = [];
    do {
      vars.push(this.Variable());
    } while (this.match(TokenType.COMMA));
    // Return the list of parsed variable declarations
    return vars;
  }

  /**
   * Parses a single variable declaration.
   * @returns {PiVarDeclaration} Parsed variable declaration
   */
  Variable() {
    // Parse the variable name
    let name = this.consume(TokenType.ID, "Expect variable name");

    // Parse the optional assignment expression
    let eqToken = null;
    let init = null;
    if (this.match(TokenType.ASSIGN)) {
      eqToken = this.previous();
      init = this.AssignmentExpression();
    }

    // Return the parsed variable declaration
    return new PiVarDeclaration(name, eqToken, init);
  }

  ParameterList() {
    let parameters = []; // Changed to array
    if (!this.check(TokenType.RPAREN)) {
      // If not empty parameter list
      do {
        if (parameters.length >= 32) {
          throw new Error("Can't have more than 32 parameters.");
        }
        const nameToken = this.consume(TokenType.ID, "Expect parameter name.");
        let defaultValue = null;
        if (this.match(TokenType.ASSIGN)) {
          defaultValue = this.Expression();
        }

        // Store the parameter details
        parameters.push({
          nameToken: nameToken,
          defaultValue: defaultValue,
        });

        // Check for a comma to see if there are more parameters
        if (this.match(TokenType.COMMA)) {
          // The comma token belongs to the *previous* parameter in the list
          // So, update the last parameter added with its comma token
          parameters[parameters.length - 1].commaToken = this.previous();
        } else {
          // No comma, so this is the last parameter
          break;
        }
      } while (true); // Loop until break
    }
    return parameters;
  }

  FunctionDeclaration() {
    let funToken = this.previous();

    // This is a function expression, not a declaration statement
    if (this.check(TokenType.LPAREN)) {
      this.current--;
      return this.ExpressionStatement();
    }

    let name = new PiVariable(
      this.consume(TokenType.ID, "Expect variable name")
    );

    const lparen = this.consume(
      TokenType.LPAREN,
      "Expect '(' after function name"
    );
    let params = this.ParameterList();
    const rparen = this.consume(
      TokenType.RPAREN,
      "Expect ')' after parameters."
    );
    const lbrace = this.consume(
      TokenType.LBRACE,
      "Expect '{' before function body."
    );
    let body = this.Block(lbrace);

    return new PiFuncDeclStatement(
      funToken,
      name,
      lparen,
      params,
      rparen,
      body
    );
  }

  Statement() {
    if (this.match(TokenType.LBRACE)) {
      // Look ahead to check if it's an object literal (key: value format)
      const current = this.current; // Save current position

      if (
        this.match(
          TokenType.STR,
          TokenType.ID,
          TokenType.NUM,
          TokenType.FALSE,
          TokenType.TRUE
        ) &&
        this.match(TokenType.COLON)
      ) {
        // If we find key-value pattern, reset position and parse as object
        this.current = current - 1;
        return this.ExpressionStatement();
      } else {
        // Otherwise, parse as a block
        this.current = current; // Restore position
        return this.Block();
      }
    } else if (this.match(TokenType.IF)) {
      return this.IfStatement();
    } else if (this.match(TokenType.WHILE)) {
      return this.WhileStatement();
    } else if (this.match(TokenType.FOR)) {
      return this.ForStatement();
    } else if (this.match(TokenType.BREAK)) {
      return this.BreakStatement();
    } else if (this.match(TokenType.CONTINUE)) {
      return this.ContinueStatement();
    } else if (this.match(TokenType.RETURN)) {
      return this.ReturnStatement();
    } else if (this.match(TokenType.ASSERT)) {
      return this.AssertStatement();
    } else if (this.match(TokenType.DEBUG)) {
      return this.DebugStatement();
    } else return this.ExpressionStatement();
  }

  CompoundStatement() {
    let token = this.previous();
    let statements = new PiCompoundStatement(token);
    while (!this.check(TokenType.RBRACE) && !this.isAtEnd()) {
      statements.add(this.Statement());
    }
    this.consume(TokenType.RBRACE, "Expect '}' after block.");
    return statements;
  }
  Block() {
    let lbrace = this.previous();
    let statements = new PiCompoundStatement(lbrace);
    while (!this.check(TokenType.RBRACE) && !this.isAtEnd()) {
      statements.add(this.Declaration());
    }
    let rbrace = this.consume(TokenType.RBRACE, "Expect '}' after block.");
    return new PiBlockStatement(lbrace, statements, rbrace);
  }

  AssertStatement() {
    let token = this.previous();
    let value = this.Expression();
    const semicolon = this.match(TokenType.SEMICOLON) ? this.previous() : null;
    return new PiAssertStatement(token, value, semicolon);
  }

  DebugStatement() {
    let token = this.previous();
    let value = this.Expression();
    const semicolon = this.match(TokenType.SEMICOLON) ? this.previous() : null;
    return new PiDebugStatement(token, value, semicolon);
  }

  /**
   * Parses an 'if' statement.
   * Handles optional 'elif' and 'else' clauses.
   * @returns {PiIfStatement} Parsed if statement
   */
  IfStatement() {
    let ifToken = this.previous();

    const lparen = this.match(TokenType.LPAREN) ? this.previous() : null;
    let cond = this.Expression();
    let rparen = null;
    if (lparen) {
      rparen = this.consume(TokenType.RPAREN, "Expect ')' after condition.");
    }

    let thenStmt;
    if (this.match(TokenType.LBRACE)) {
      thenStmt = this.Block();
    } else {
      thenStmt = this.Statement();
    }

    let elseToken = null;
    let elseStmt = null;

    if (this.match(TokenType.ELIF)) {
      elseToken = this.previous();
      elseStmt = this.IfStatement();
    } else if (this.match(TokenType.ELSE)) {
      elseToken = this.previous();
      elseStmt = this.Statement();
    }

    return new PiIfStatement(
      ifToken,
      lparen,
      cond,
      rparen,
      thenStmt,
      elseToken,
      elseStmt
    );
  }

  /**
   * Parses a 'while' statement.
   * @returns {PiWhileStatement} Parsed while statement
   */
  WhileStatement() {
    let token = this.previous();
    const lparen = this.match(TokenType.LPAREN) ? this.previous() : null;
    let cond = this.Expression();
    let rparen = null;
    if (lparen) {
      rparen = this.consume(TokenType.RPAREN, "Expect ')' after condition.");
    }
    let body;

    this.pushLoop();
    // Check if the loop body is enclosed in braces and parse accordingly
    if (this.match(TokenType.LBRACE)) {
      // If the body is enclosed in braces, parse as a block
      body = this.Block();
    } else {
      // Otherwise, parse the body as a single statement
      body = this.Statement();
    }
    this.popLoop();
    return new PiWhileStatement(token, lparen, cond, rparen, body);
  }

  /**
   * Parses a 'for' statement.
   * @returns {PiForStatement} Parsed for statement
   */
  ForStatement() {
    let forToken = this.previous();

    const lparen = this.match(TokenType.LPAREN) ? this.previous() : null;

    let init;
    // Parse the left-hand side of the for-loop
    if (this.match(TokenType.ID)) {
      init = new PiVariable(this.previous());
    } else {
      throw new Error("Invalid for-loop left-hand side. Expect identifier.");
    }

    // Consume the 'in' keyword
    const inToken = this.consume(
      TokenType.IN,
      "Expect 'in' keyword after loop variable."
    );
    // Parse the right-hand side of the for-loop
    let expr = this.Expression();

    let rparen = null;
    if (lparen) {
      // Consume the ')'
      rparen = this.consume(
        TokenType.RPAREN,
        "Expect ')' after iterable expression."
      );
    }

    let body;
    this.pushLoop();
    // Check if the loop body is enclosed in braces and parse accordingly
    if (this.match(TokenType.LBRACE)) {
      body = this.Block();
    } else {
      body = this.Statement();
    }
    this.popLoop();

    return new PiForStatement(
      forToken,
      lparen,
      init,
      inToken,
      expr,
      rparen,
      body
    );
  }

  BreakStatement() {
    let token = this.previous();

    // Check if we're inside a loop
    if (!this.inLoop()) {
      throw new ParseError(
        "'break' used outside of a loop",
        token.line,
        token.column
      );
    }
    const semicolon = this.match(TokenType.SEMICOLON) ? this.previous() : null;
    return new PiBreakStatement(token, semicolon);
  }

  ContinueStatement() {
    let token = this.previous();

    // Check if we're inside a loop
    if (!this.inLoop()) {
      throw new ParseError(
        "'continue' used outside of a loop",
        token.line,
        token.column
      );
    }
    const semicolon = this.match(TokenType.SEMICOLON) ? this.previous() : null;
    return new PiContinueStatement(token, semicolon);
  }

  /**
   * Parses a 'return' statement.
   * @returns {PiReturnStatement} Parsed return statement
   */
  ReturnStatement() {
    let token = this.previous();
    let value = null;
    // Parse the expression that the return statement is returning
    if (!this.check(TokenType.SEMICOLON) && !this.check(TokenType.RBRACE)) {
      value = this.Expression();
    }

    const semicolon = this.match(TokenType.SEMICOLON) ? this.previous() : null;

    // Return the parsed return statement
    return new PiReturnStatement(token, value, semicolon);
  }

  /**
   * Parses an expression statement.
   * @returns {PiExpressionStatement} Parsed expression statement
   */
  ExpressionStatement() {
    let expression = this.Expression();

    // Consume any optional delimiters (semicolon or newline)
    const semicolon = this.match(TokenType.SEMICOLON) ? this.previous() : null;
    this.consumeIfExist(TokenType.NEWLINE);
    // Create a new expression statement with the expression and its location
    return new PiExpressionStatement(expression, semicolon);
  }

  /**
   * Parses an expression.
   * Delegates to AssignmentExpression for parsing assignment operations.
   * @returns {PiExpression} Parsed expression
   */

  Expression() {
    // Parse the assignment expression
    return this.AssignmentExpression();
  }

  /**
   * Parses an assignment expression.
   * Delegates to ConditionalExpression for the left-hand side.
   * @returns {PiExpression} Parsed assignment expression or conditional expression
   * @throws {Error} If the assignment target is invalid
   */
  AssignmentExpression() {
    // Parse the left-hand side of the assignment
    let expression = this.ConditionalExpression();

    // Check for assignment operators
    if (
      this.match(
        TokenType.ASSIGN,
        TokenType.PLUS_ASSIGN,
        TokenType.MINUS_ASSIGN,
        TokenType.DIV_ASSIGN,
        TokenType.MULT_ASSIGN,
        TokenType.MOD_ASSIGN,
        TokenType.BITOR_ASSIGN,
        TokenType.XOR_ASSIGN,
        TokenType.BITAND_ASSIGN
      )
    ) {
      // Store the assignment operator
      let operator = this.previous();
      // Parse the right-hand side of the assignment
      let right = this.AssignmentExpression();

      // Ensure the left-hand side is a valid assignment target
      if (expression instanceof PiVariable) {
        let varNode = expression;
        // Return the parsed binary assignment expression
        return new PiBinaryExpression(varNode, operator, right);
      }

      // Throw an error if the assignment target is invalid
      throw new ParseError(
        "Invalid assignment target",
        expression.getLine(),
        expression.getColumn()
      );
      // throw new Error("Invalid assignment target");
    }

    // Return the parsed expression if not an assignment
    return expression;
  }

  /**
   * Parses a conditional expression.
   * @returns {PiExpression} The parsed expression
   */
  ConditionalExpression() {
    let expression = this.LogicalOrExpression();
    // Check if the expression is a conditional expression
    if (this.match(TokenType.QUESTION)) {
      // Consume the '?' token
      let t = this.previous();
      // Parse the 'then' expression
      let _then = this.Expression();
      // Consume the ':' token
      this.consume(TokenType.COLON, "Expect ':' after '?'");
      // Parse the 'else' expression
      let _else = this.ConditionalExpression();
      // Construct a new conditional expression

      expression = new PiConditionalExpression(t, expression, _then, _else);
    }
    // Return the parsed expression
    return expression;
  }

  LogicalOrExpression() {
    let expression = this.LogicalAndExpression();
    while (this.match(TokenType.OR)) {
      let operator = this.previous();
      let right = this.LogicalAndExpression();
      expression = new PiBinaryExpression(expression, operator, right);
    }
    return expression;
  }

  LogicalAndExpression() {
    let expression = this.IncludeExpression();
    while (this.match(TokenType.AND)) {
      let operator = this.previous();
      let right = this.IncludeExpression();
      expression = new PiBinaryExpression(expression, operator, right);
    }
    return expression;
  }

  IncludeExpression() {
    let expression = this.RangeExpression();
    while (this.match(TokenType.IN)) {
      let operator = this.previous();
      let right = this.RangeExpression();
      expression = new PiBinaryExpression(expression, operator, right);
    }
    return expression;
  }

  RangeExpression() {
    let expression = this.BitwiseOrExpression();
    if (this.match(TokenType.DBDOTS)) {
      let operator = this.previous();
      let right = this.BitwiseOrExpression();
      if (this.match(TokenType.COLON)) {
        let step = this.Expression();
        return new PiRangeExpression(operator, expression, right, step);
      } else {
        return new PiRangeExpression(operator, expression, right);
      }
    } else {
      return expression;
    }
  }

  BitwiseOrExpression() {
    let expression = this.BitwiseXorExpression();
    while (this.match(TokenType.BITOR)) {
      let operator = this.previous();
      let right = this.BitwiseXorExpression();
      expression = new PiBinaryExpression(expression, operator, right);
    }
    return expression;
  }

  BitwiseXorExpression() {
    let expression = this.BitwiseAndExpression();
    while (this.match(TokenType.XOR)) {
      let operator = this.previous();
      let right = this.BitwiseAndExpression();
      expression = new PiBinaryExpression(expression, operator, right);
    }
    return expression;
  }

  BitwiseAndExpression() {
    let expression = this.ShiftExpression();
    while (this.match(TokenType.BITAND)) {
      let operator = this.previous();
      let right = this.ShiftExpression();
      expression = new PiBinaryExpression(expression, operator, right);
    }
    return expression;
  }

  ShiftExpression() {
    let expression = this.EqualityExpression();
    while (this.match(TokenType.LSHIFT, TokenType.RSHIFT, TokenType.URSHIFT)) {
      let operator = this.previous();
      let right = this.ShiftExpression();
      expression = new PiBinaryExpression(expression, operator, right);
    }
    return expression;
  }

  EqualityExpression() {
    let expression = this.ComparisonExpression();
    while (this.match(TokenType.NOT_EQUAL, TokenType.EQUAL)) {
      let operator = this.previous();
      let right = this.ComparisonExpression();
      expression = new PiBinaryExpression(expression, operator, right);
    }
    return expression;
  }

  // ComparisonExpression -> AdditionExpression ( ( ">" | ">=" | "<" | "<=" ) AdditionExpression )*
  ComparisonExpression() {
    let expression = this.AdditionExpression();

    while (
      this.match(
        TokenType.GREATER,
        TokenType.GREATER_EQUAL,
        TokenType.LESS,
        TokenType.LESS_EQUAL
      )
    ) {
      const operator = this.previous();
      let right = this.AdditionExpression();

      // Build a binary expression with the current comparison operator and right-hand side
      expression = new PiBinaryExpression(expression, operator, right);

      // Check if there's another comparison operator
      while (
        this.match(
          TokenType.GREATER,
          TokenType.GREATER_EQUAL,
          TokenType.LESS,
          TokenType.LESS_EQUAL
        )
      ) {
        // Build a binary expression with the previous operator and right-hand side
        let _right = right;
        const operator = this.previous();
        right = this.AdditionExpression();
        _right = new PiBinaryExpression(_right, operator, right);

        // Build a binary expression with the logical operator and previous binary expression
        expression = new PiBinaryExpression(
          expression,
          new Token(TokenType.AND, "&&", operator.line, operator.column),
          _right
        );
      }
    }

    return expression;
  }

  // AdditionExpression -> MultiplicationExpression ( ( "-" | "+" ) MultiplicationExpression )*
  AdditionExpression() {
    let expression = this.DotProductExpression();

    while (this.match(TokenType.MINUS, TokenType.PLUS)) {
      const operator = this.previous();
      const right = this.DotProductExpression();
      expression = new PiBinaryExpression(expression, operator, right);
    }

    return expression;
  }

  // DotProductExpression -> MultiplicationExpression ( "." MultiplicationExpression )*
  DotProductExpression() {
    let expression = this.MultiplicationExpression();

    while (this.match(TokenType.DOT_PRODUCT)) {
      const operator = this.previous();
      const right = this.MultiplicationExpression();
      expression = new PiBinaryExpression(expression, operator, right);
    }

    return expression;
  }

  // MultiplicationExpression -> ExponentiationExpression ( ( "/" | "*" | "%" ) ExponentiationExpression )*
  MultiplicationExpression() {
    let expression = this.ExponentiationExpression();

    while (this.match(TokenType.DIV, TokenType.MULT, TokenType.MOD)) {
      const operator = this.previous();
      const right = this.ExponentiationExpression();
      expression = new PiBinaryExpression(expression, operator, right);
    }

    return expression;
  }

  // ExponentiationExpression -> UnaryExpression ( "**" UnaryExpression )*
  ExponentiationExpression() {
    let expression = this.UnaryExpression();

    while (this.match(TokenType.POWER)) {
      const operator = this.previous();
      const right = this.ExponentiationExpression();
      expression = new PiBinaryExpression(expression, operator, right);
    }

    return expression;
  }

  // UnaryExpression -> ( "!" | "-" | "+" | "~" | "#" | "++" | "--" | "typeof" ) UnaryExpression | PrimaryExpression
  UnaryExpression() {
    let operator, expression;

    if (
      this.match(
        TokenType.PLUS,
        TokenType.MINUS,
        TokenType.NOT,
        TokenType.BITNEG,
        TokenType.HASH,
        TokenType.INCR,
        TokenType.DECR,
        TokenType.TYPEOF
      )
    ) {
      operator = this.previous();

      if (operator.type === TokenType.MINUS && this.match(TokenType.NUM)) {
        const number = this.previous();
        return new PiLiteralExpression(
          new Token(
            TokenType.NUM,
            -1 * number.value,
            number.line,
            number.column
          )
        );
      } else {
        expression = this.MemberExpression();

        if (
          (operator.type == TokenType.INCR ||
            operator.type == TokenType.DECR) &&
          (expression instanceof PiFunctionExpression ||
            expression instanceof PiLiteralExpression)
        ) {
          throw new ParseError(
            "Increment/Decrement operations cannot be applied to calls or literals.",
            this.last.line,
            this.last.column
          );
        }

        return new PiUnaryExpression(operator, expression, true, this.access);
      }
    } else {
      expression = this.MemberExpression();

      if (this.match(TokenType.INCR, TokenType.DECR)) {
        if (
          expression instanceof PiFunctionExpression ||
          expression instanceof PiLiteralExpression
        ) {
          throw new ParseError(
            "Unary increment/decrement operators cannot be applied to function calls or literals",
            this.last.line,
            this.last.column
          );
        }
        operator = this.previous();
        return new PiUnaryExpression(operator, expression, false, this.access);
      } else {
        return expression;
      }
    }
  }

  // MemberExpression -> PrimaryExpression "." IDENTIFIER | PrimaryExpression "[" Expression "]" | PrimaryExpression "(" ArgumentList* ")"
  MemberExpression() {
    let expression = this.PrimaryExpression();
    let container, token;

    while (true) {
      if (this.match(TokenType.DOT)) {
        token = this.previous();
        const name = new PiLiteralExpression(this.next());
        expression = new PiBinaryExpression(expression, token, name);

        if (
          this.match(
            TokenType.ASSIGN,
            TokenType.PLUS_ASSIGN,
            TokenType.MINUS_ASSIGN,
            TokenType.DIV_ASSIGN,
            TokenType.MULT_ASSIGN,
            TokenType.MOD_ASSIGN,
            TokenType.BITOR_ASSIGN,
            TokenType.XOR_ASSIGN,
            TokenType.BITAND_ASSIGN
          )
        ) {
          token = this.previous();
          const right = this.AssignmentExpression();
          return new PiBinaryExpression(expression, token, right);
        }
        this.access = true;
      } else if (this.match(TokenType.LBRACKET)) {
        token = this.previous(); // '[' token
        const index = this.SliceExpression();
        const rbracket = this.consume(
          TokenType.RBRACKET,
          "Expect ']' after list index expression"
        );
        expression = new PiBinaryExpression(expression, token, index, rbracket);

        if (
          this.match(
            TokenType.ASSIGN,
            TokenType.PLUS_ASSIGN,
            TokenType.MINUS_ASSIGN,
            TokenType.DIV_ASSIGN,
            TokenType.MULT_ASSIGN,
            TokenType.MOD_ASSIGN,
            TokenType.BITOR_ASSIGN,
            TokenType.XOR_ASSIGN,
            TokenType.BITAND_ASSIGN
          )
        ) {
          token = this.previous();
          const right = this.AssignmentExpression();
          return new PiBinaryExpression(expression, token, right);
        }
        this.access = true;
      } else if (this.match(TokenType.LPAREN)) {
        let _current = this.current - 1;
        let args = [];
        let start = this.previous();

        if (!this.check(TokenType.RPAREN)) {
          args = this.ArgumentList();
        }
        let end = this.consume(
          TokenType.RPAREN,
          "Expect ')' after function call"
        );
        if (this.check(TokenType.RARROW)) {
          this.current = _current;
          break;
        }
        expression = new PiFunctionExpression(start, end, expression, args);
      } else break;
    }

    return expression;
  }

  ArgumentList() {
    let args = [];
    let expr = this.AssignmentExpression();
    args.push(expr);

    while (this.match(TokenType.COMMA)) {
      expr = this.AssignmentExpression();
      args.push(expr);
    }

    return args;
  }

  SliceExpression() {
    let start = null,
      step = null,
      end = null;

    const startToken = this.peek();

    if (!this.check(TokenType.COLON)) {
      start = this.ConditionalExpression();
    }

    let lastToken = start ? start.getLastToken() : startToken;

    if (this.match(TokenType.COLON)) {
      lastToken = this.previous();
      if (!this.check(TokenType.RBRACKET) && !this.check(TokenType.COLON)) {
        end = this.ConditionalExpression();
        lastToken = end.getLastToken();
      }
      if (this.match(TokenType.COLON)) {
        lastToken = this.previous();
        if (!this.check(TokenType.RBRACKET)) {
          step = this.ConditionalExpression();
          lastToken = step.getLastToken();
        }
      }
    } else {
      return start;
    }

    return new PiSliceExpression(startToken, lastToken, start, end, step);
  }

  PrimaryExpression() {
    if (
      this.match(
        TokenType.NUM,
        TokenType.STR,
        TokenType.TRUE,
        TokenType.FALSE,
        TokenType.NIL,
        TokenType.INF,
        TokenType.NaN
      )
    ) {
      return new PiLiteralExpression(this.previous());
    }

    if (this.match(TokenType.LPAREN)) {
      const lparen = this.previous();
      // Check for arrow function, e.g., (a, b) -> ...
      try {
        const params = this.ParameterList();
        const rparen = this.consume(
          TokenType.RPAREN,
          "Expect ')' after parameters."
        );
        this.consume(TokenType.RARROW, "Expect '->' for arrow function.");

        let body;
        if (this.match(TokenType.LBRACE)) {
          body = this.Block(this.previous());
        } else {
          // Single expression body
          const returnExpr = this.Expression();
          body = new PiCompoundStatement(returnExpr.getStartToken());
          body.add(
            new PiReturnStatement(
              returnExpr.getStartToken(),
              returnExpr,
              null,
              true
            )
          );
        }
        return new PiFuncDeclExpression(lparen, params, body, null, true);
      } catch (e) {
        // Not an arrow function, so it must be a grouped expression.
        this.current = this.tokens.indexOf(lparen) + 1;
        let expression = this.Expression();
        this.consume(TokenType.RPAREN, "Expect ')' after expression.");
        return expression;
      }
    }

    if (this.match(TokenType.ID)) {
      const token = this.previous();
      // Simplified lambda syntax, e.g., x -> x * 2
      if (this.match(TokenType.RARROW)) {
        const params = new Map();
        params.set(token.value, null);
        const returnExpr = this.Expression();
        const body = new PiCompoundStatement(returnExpr.getStartToken());
        body.add(
          new PiReturnStatement(
            returnExpr.getStartToken(),
            returnExpr,
            null,
            true
          )
        );
        return new PiFuncDeclExpression(token, params, body, null, true);
      }
      return new PiVariable(token);
    }

    if (this.match(TokenType.LBRACKET)) {
      const startToken = this.previous();
      const elements = [];
      const commas = [];
      if (!this.check(TokenType.RBRACKET)) {
        do {
          // Handle trailing comma case: [a, b,]
          if (this.check(TokenType.RBRACKET)) {
            break;
          }
          elements.push(this.Expression());
          if (this.match(TokenType.COMMA)) {
            commas.push(this.previous());
          } else {
            break; // No comma, so it must be the last element
          }
        } while (!this.isAtEnd());
      }
      const endToken = this.consume(
        TokenType.RBRACKET,
        "Expect ']' at the end of list literal."
      );
      return new PiListExpression(startToken, elements, commas, endToken);
    }

    // Object literal parsing
    if (this.match(TokenType.LBRACE)) {
      const startToken = this.previous();
      const properties = [];

      if (!this.check(TokenType.RBRACE)) {
        do {
          if (this.check(TokenType.RBRACE)) break; // Dangling comma

          let keyToken;
          // Allow ID, STR, NUM, FALSE, TRUE for keys
          if (
            this.match(
              TokenType.STR,
              TokenType.ID,
              TokenType.NUM,
              TokenType.FALSE,
              TokenType.TRUE
            )
          ) {
            keyToken = this.previous();
            let key = keyToken.value.toString();
            if (key.length > 2 && key.substring(key.length - 2) === ".0") {
              key = key.substring(0, key.length - 2);
              keyToken = new Token(
                TokenType.NUM,
                parseFloat(key),
                keyToken.line,
                keyToken.column
              );
            }
          } else {
            throw new ParseError(
              "Expect key in object literal (identifier, string, or number).",
              this.peek()
            );
          }

          let value;
          let colonToken = null;

          // Check for method shorthand: key(params) { body }
          if (this.match(TokenType.LPAREN)) {
            /**
             * Parse a function expression as a value in the map.
             * The function expression is parsed as a lambda function
             * so it can be used as a value in the map.
             */
            const params = this.ParameterList();
            this.consume(TokenType.RPAREN, "Expect ')' after parameters.");

            let lbrace = this.consume(
              TokenType.LBRACE,
              "Expect '{' before function body."
            );
            const statements = new PiCompoundStatement(lbrace);
            while (!this.check(TokenType.RBRACE) && !this.isAtEnd()) {
              statements.add(this.Declaration());
            }
            const rbrace = this.consume(TokenType.RBRACE, "Expect '}' after function body.");
            const body = new PiBlockStatement(lbrace, statements, rbrace);

            value = new PiFuncDeclExpression(
              keyToken,
              params,
              body,
              this.key,
              false, // isArrow = false
              true // isMethod = true
            );
          } else {
            // Regular property
            colonToken = this.consume(
              TokenType.COLON,
              "Expect ':' after key in object literal."
            );
            value = this.Expression();
          }

          let commaToken = null;
          if (this.match(TokenType.COMMA)) {
            commaToken = this.previous();
          }

          properties.push({ keyToken, colonToken, value, commaToken });

          if (commaToken === null) {
            break; // Last property
          }
        } while (!this.isAtEnd());
      }
      const endToken = this.consume(
        TokenType.RBRACE,
        "Expect '}' at the end of object literal."
      );
      this.key = null;
      return new PiObjectExpression(startToken, properties, endToken);
    }

    if (this.match(TokenType.FUN)) {
      const funToken = this.previous();
      const lparen = this.consume(
        TokenType.LPAREN,
        "Expect '(' after function name"
      );
      const params = this.ParameterList();
      const rparen = this.consume(
        TokenType.RPAREN,
        "Expect ')' after parameters."
      );
      const lbrace = this.consume(
        TokenType.LBRACE,
        "Expect '{' before function body."
      );
      const body = this.Block(lbrace);
      return new PiFuncDeclExpression(funToken, params, body);
    }

    throw new ParseError(
      "Expect expression.",
      this.peek().line,
      this.peek().column
    );
  }

  /**
   * Checks if the current token matches any of the provided types.
   * If a match is found, it consumes the token and returns true.
   * If no match is found, it returns false.
   * It also consumes any optional semicolon or newline before checking the current token.
   * @param {TokenType[]} types List of token types to match
   * @returns {boolean} True if a match is found, false otherwise
   */
  match(...types) {
    // this.consumeIfExist(TokenType.SEMICOLON, TokenType.NEWLINE);
    for (let type of types) {
      if (this.check(type)) {
        this.next();
        return true;
      }
    }
    return false;
  }

  check(type) {
    return !this.isAtEnd() && this.peek().type === type;
  }

  check(...types) {
    if (this.isAtEnd()) return false;
    for (let type of types) {
      if (this.peek().type === type) return true;
    }
    return false;
  }

  isDelimiter(token = null) {
    // If token is provided, check if it's a delimiter
    if (token !== null) {
      return (
        token.type === TokenType.SEMICOLON || token.type === TokenType.NEWLINE
      );
    }

    // Original isDelimiter() behavior
    let res = false;
    while (true) {
      const type = this.peek().type;
      if (type === TokenType.SEMICOLON) {
        this.next();
        res = true;
      } else if (type === TokenType.RBRACE) {
        res = true;
        break;
      } else {
        break;
      }
    }
    return this.isAtEnd() || res;
  }

  needDelimiter() {
    // If there's no explicit semicolon or newline,
    // and it's not a line break,
    // and the next token is not a closing brace,
    // then we should insert a semicolon.
    if (!this.consumeIfExist(TokenType.SEMICOLON)) {
      if (!this.isLineBreak()) {
        if (!this.check(TokenType.RBRACE)) {
          return true;
        }
      }
    }
    // If we get here, we don't need a delimiter
    return false;
  }

  peek() {
    return this.tokens[this.current];
  }

  isAtEnd() {
    return this.peek().type === TokenType.EOF;
  }

  next() {
    if (!this.isAtEnd()) {
      this.current++;
      if (!this.isDelimiter(this.peek())) {
        this.last = this.peek();
      }
    }
    return this.previous();
  }

  previous() {
    return this.tokens[this.current - 1];
  }

  consume(type, message = null) {
    if (this.check(type)) {
      const token = this.next();
      // this.consumeIfExist(TokenType.SEMICOLON, TokenType.NEWLINE);
      return token;
    } else if (message != null) {
      throw new ParseError(message, this.peek().line, this.peek().column);
    } else {
      throw new ParseError();
    }
  }

  advance() {
    if (!this.isAtEnd()) {
      this.current++;
    }
  }

  consumeIfExist(...types) {
    let consumed = false;
    while (this.check(...types)) {
      this.advance();
      consumed = true;
    }
    return consumed;
  }

  isLineBreak() {
    // Compare line numbers of previous and current tokens
    return (
      this.previous().line < this.peek().line ||
      this.peek().type === TokenType.EOF
    );
  }

  pushLoop() {
    this.loopDepth++;
  }

  popLoop() {
    this.loopDepth--;
  }

  inLoop() {
    return this.loopDepth > 0;
  }
}
