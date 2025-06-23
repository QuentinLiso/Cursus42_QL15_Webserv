# include "Includes.hpp"
# include "Console.hpp"
# include "0_Utils.hpp"
# include "1_Lexing.hpp"
# include "2_Parsing.hpp"
# include "3_Build.hpp"
# include "4_Server.hpp"
# include "5_ListeningSocket.hpp"
# include "8_HttpResponse.hpp"

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
	if (ac != 2)
	{
		Console::log(Console::ERROR, "Wrong number of arguments");
		return (1);
	}
	try
	{
		Builder builder;
		MakeConfig(av[1], builder);
		builder.printRuntimeBuild();
		Server	server;
		server.makeServerReady(builder);
		server.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	// HttpRequest	request;
	// TStr		clientRequest = "GET /docs/in%21%20dex.html HTTP/1.1\r\n"
	// 							"host: 127.0.0.1:80\r\n"
	// 							"user-agent: lolilou\r\n"
	// 							"trucmachin:sssss\r\n"
	// 							"Content-Type: multipart/form-data; boundary=iii\r\n"
	// 							"\r\n"
	// 							;

	// request.setBuffer(clientRequest);
	// std::cout << request.getBuffer() << std::endl;

	// request.parseRequestAndHeaders();

	return (0);
}