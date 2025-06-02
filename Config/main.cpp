# include "Includes.hpp"
# include "Console.hpp"
# include "0_Utils.hpp"
# include "1_Lexing.hpp"
# include "2_Parsing.hpp"
# include "3_Build.hpp"
# include "4_ListeningSocket.hpp"

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
		const std::set<TIPPort>	ips = builder.getBoundSockets();
		std::vector<ListeningSocket>	sockets;
		sockets.reserve(ips.size());
		for (std::set<TIPPort>::const_iterator it = ips.begin(); it != ips.end(); it++)
		{
			ListeningSocket	listeningSocket(*it);
			listeningSocket.makeListeningSocketReady();
			sockets.push_back(listeningSocket);
		}

		// Create epoll instance and register each listening socket -> epoll instance will monitor a bunch of fds
		int	epollfd = epoll_create1(0);
		for (std::vector<ListeningSocket>::const_iterator it = sockets.begin(); it != sockets.end(); it++)
		{
			struct	epoll_event	ev;
			ev.events = EPOLLIN;
			ev.data.fd = it->getSockFd();
			epoll_ctl(epollfd, EPOLL_CTL_ADD, it->getSockFd(), &ev);
		}

		struct epoll_event	events[64];
		int		nbEventsReady = epoll_wait(epollfd, events, 64, -1);
		for (int i = 0; i < nbEventsReady; i++)
		{
			std::cout << events[i].data.fd << std::endl;
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	return (0);
}