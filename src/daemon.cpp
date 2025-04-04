#include "daemon.hpp"
#include <stdexcept>
#include <unistd.h>
#include <sys/stat.h>
#include "file.hpp"
#include "server.hpp"
#include "signal.hpp"

void daemon::_is_root() {
	if (::getuid() != 0) {
		throw std::runtime_error("You must be root to run this program.");
	}
}

pid_t daemon::_fork() {
	const pid_t pid = ::fork();
	if (pid == -1) {
		throw std::runtime_error("Fork failed.");
	}
	return pid;
}

void daemon::_new_session() {
	if (::setsid() == -1) {
		throw std::runtime_error("Failed to create a new session.");
	}
}

void daemon::_go_to_root() {
	if (::chdir("/") == -1) {
		throw std::runtime_error("Failed to change directory to root.");
	}
}

void daemon::_close_fds() {
		::close(STDIN_FILENO);
		::close(STDOUT_FILENO);
		::close(STDERR_FILENO);
}

void daemon::run() {

	sig::init();

	_is_root();

	if (_fork() != 0) {
		return;
	}

	_new_session();

	if (_fork() != 0) {
		return;
	}

	_go_to_root();

	::umask(0);
	_close_fds();


	file fl("/var/lock/matt_daemon.lock", O_CREAT, 0440);
	guard_lock guard(fl);

	server{}.run();
}
