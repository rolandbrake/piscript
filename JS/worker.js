importScripts(
  "PiTester.js",
  "PiScanner.js",
  "PiParser.js",
  "TokenType.js",
  "OpCode.js",
  "vm/PiVM.js",
  "screen/Screen.js"
);

self.onmessage = function (event) {
  const { source } = event.data;

  const scanner = new PiScanner();
  const parser = new PiParser();

  const tokens = scanner.scan(source);

  const vm = new PiVM();

  if (tokens[0].getType() !== TokenType.EOF) {
    const statements = parser.parse(tokens);
    const compiler = statements.gen();

    // Emit HALT_OP at the end of generating instructions
    compiler.emit(OpCode.HALT_OP);

    compiler.dis();

    vm.run(compiler);
  }
};
