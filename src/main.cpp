#include "ptp-ip.hpp"
#include <iostream>

int main() {
	auto loop = uvw::Loop::getDefault();

	const uvw::Addr remoteAddr{ "192.168.0.50", 15740 };

	PTP_IP::Connection connection{ remoteAddr };
	connection.connect(*loop);

	loop->run();

	return 0;
}
