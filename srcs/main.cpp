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


static int	g_signalPipeFd[2];

void	createSignalPipe(void)
{
	if (pipe(g_signalPipeFd) == -1)
	{
		std::ostringstream oss;
		Console::log(Console::WARNING, "[SERVER] Failed to pipe for clean signal-handled exit... Server will still run, kill the process if you want to stop it");
		g_signalPipeFd[0] = -1;
		g_signalPipeFd[1] = -1;
	}
	else
		Console::log(Console::DEBUG, "[SERVER] Pipe fds for exit signal handling created : " + convToStr(g_signalPipeFd[0]) + " and " + convToStr(g_signalPipeFd[1]));
}

void	closeSignalPipe(void)
{
	if (g_signalPipeFd[0] > 0)
		close(g_signalPipeFd[0]);
	if (g_signalPipeFd[1] > 0)
		close(g_signalPipeFd[1]);

}

void	signalHandler(int signum)
{
	write(g_signalPipeFd[1], &signum, 1);

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
		Console::log(Console::INFO, "Starting server program, process : " + convToStr(getpid()));
		Builder builder;
		MakeConfig(av[1], builder);
		// builder.printRuntimeBuild();

		Server	server;
		createSignalPipe();
		server.makeServerReady(builder, g_signalPipeFd[0], g_signalPipeFd[1]);
		signal(SIGINT, signalHandler);
		server.run();
		closeSignalPipe();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return (0);
}