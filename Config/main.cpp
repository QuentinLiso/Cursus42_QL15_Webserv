# include "Includes.hpp"
# include "Console.hpp"
# include "0_Utils.hpp"
# include "1_Lexing.hpp"
# include "2_Parsing.hpp"
# include "3_Build.hpp"

void		MakeConfig(const TStr& filename, Builder& builder)
{
	Lexer lexer(filename);
	lexer.throwInvalid();
	
	Parser parser(lexer.getTokens());
	parser.throwInvalid();
	
	builder.parsingToBuild(parser.getStatements());
	builder.validParsingToBuild();
	builder.throwInvalid("Parsing to build failed");
	builder.makeRuntimeBuild();
	builder.validRuntimeBuild();
	builder.throwInvalid("Making runtime build failed");
}


int main(int ac, char **av)
{
	try
	{
		Builder builder;
		MakeConfig(av[1], builder);
		// builder.printRuntimeBuild();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	return (0);
}