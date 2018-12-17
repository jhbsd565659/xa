#include<iostream>
#include<string>
#include<list>
#include<thread>
#include<cstdlib>
#include<cstdio>
#include<cstring>

#ifdef _MSC_VER
	#define popen _popen
	#define pclose _pclose
#endif

void usage() {
	std::cout 
		<< "xa version 0.2.1" << std::endl
		<< "" << std::endl
		<< "usage:" << std::endl
		<< "xa [--dbg, --no-thread] command ?" << std::endl
		<< "--dbg:" << std::endl
		<< "  show built commands" << std::endl
		<< "--no-thread:" << std::endl
		<< "  disable thread execution" << std::endl
		<< "?:" << std::endl
		<< "  replace ? by previous stdin" << std::endl
	;
}

int main(int argc, char ** argv) {
	if(argc <= 1) {
		usage();
		return EXIT_FAILURE;
	}
	std::list<std::thread> li;
	static char cmd[4096] = {0};
	char c = 0;
	int n = 0;
	int z = 0;
	int t = std::thread::hardware_concurrency() * 2;
	if(t == 0) t = 8;
	while(scanf("%c", &c) != EOF) {
		sprintf(&cmd[n++], "%c", c);
		if(c != '\n') continue;
		if(cmd[n - 1] == '\n' || cmd[n - 1] == '\r') cmd[n - 1] = 0;
		if(cmd[n - 2] == '\n' || cmd[n - 2] == '\r') cmd[n - 2] = 0;
		std::string s;
		auto show_cmd = false;
		for(int i = 1; i < argc; ++i) {
			if(strcmp(argv[i], "--dbg") == 0) show_cmd = true;
			else if(strcmp(argv[i], "--no-thread") == 0) t = -1;
			else if(argv[i][0] == '?') {
#ifdef _MSC_VER
				s += "\"";
				s += cmd;
				s += "\"";
#else
				s += "'";
				s += cmd;
				s += "'";
#endif
			}
			else s += argv[i];
			s += " ";
		}
		memset(cmd, 0, 4096);
		n = 0;
		auto f = [&, s] {
			if(show_cmd) fprintf(stderr, "[dbg]%s\n", s.c_str());
			auto fp = popen(s.c_str(), "r");
			if( ! fp) {
				std::cerr << "xa:pipe error: " << s << std::endl;
				return;
			}
			std::string r;
			while ( ! feof(fp)) {
				char b[4096] = {0};
				fread(b, 1, 4095, fp);
				r += b;
			}
			pclose(fp);
			--z;
			printf("%s", r.c_str());
		};
		++z;
		if(z < t) li.emplace_back(f);
		else f();
	}
	for(auto & i : li) if(i.joinable()) i.join();
	return EXIT_SUCCESS;
}
