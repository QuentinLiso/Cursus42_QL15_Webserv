# include "Includes.hpp"
# include "Console.hpp"
# include "0_Utils.hpp"
# include "1_Lexing.hpp"
# include "2_Parsing.hpp"


// void	testDirectives(char **av)
// {
	// TStrVect args;
	// args.push_back(av[1]);
	// args.push_back(av[2]);
	// args.push_back(av[3]);
	// TStrVect	args1;
	// TStrVect	args2;
	// args1.push_back("404");
	// args1.push_back("405");
	// args1.push_back("406");
	// args1.push_back("407");
	// args1.push_back("/404.html");
	// args2.push_back("404");
	// args2.push_back("515");
	// args2.push_back("37");
	// args2.push_back("dpdpldpl");
	// args2.push_back("/418.html");
	// AConfigNode* node = new	Listen(1, 1, args);
	// AConfigNode* node = new ServerName(1, 1, args);
	// AConfigNode* node = new Alias(1, 1, args);
	// AConfigNode* node = new AllowedMethods(1, 1, args);
	// AConfigNode* node = new CgiPass(1, 1, args);
	// AConfigNode* node = new Root(1, 1, args);
	// AConfigNode* node = new Index(1, 1, args);
	// AConfigNode* node = new Autoindex(1, 1, args);
	// ErrorPage* node = new ErrorPage(1, 1, args1);
	// node->addErrorPages(args2);
	// AConfigNode* node = new ClientMaxBodySize(1, 1, args);
	// node->print(std::cout);
// }

void	testLexerParser(const std::string& filename)
{
	try
	{
		Lexer lexer(filename);
		// lexer.printTokens();
		lexer.throwInvalid();
		Parser parser(lexer.getTokens());
		// parser.printNodes();
		// parser.throwInvalid();
		Builder	builder(parser.getAst());
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