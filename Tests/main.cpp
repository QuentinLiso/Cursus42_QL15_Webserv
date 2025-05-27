# include "Includes.hpp"
# include "Console.hpp"
# include "0_Utils.hpp"
# include "1_Lexing.hpp"
# include "2_Parsing.hpp"
# include "3_Build.hpp"


void	testLexerParser(const std::string& filename)
{
	try
	{
		Lexer lexer(filename);
		// lexer.printTokens();
		lexer.throwInvalid();
		Parser parser(lexer.getTokens());
		// parser.printStatements();
		parser.throwInvalid();
		Builder	builder(parser.getStatements());
		builder.printBuild();
		builder.throwInvalid();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
}

int main(int ac, char **av)
{
	(void)ac;

	testLexerParser(av[1]);

	// ABlock<ServerBlock>*	block = new ServerBlock(0, 0, args);
	return (0);
}