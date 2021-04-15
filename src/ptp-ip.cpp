#include "ptp-ip.hpp"
#include "serialisation.tpp"

extern "C" {
#include <endian.h>
}

#include <iostream>
#include <utility>

namespace PTP_IP {
	Connection::Connection(uvw::Addr endpoint) : endpoint{ std::move(endpoint) } {}

	void Connection::connect(uvw::Loop& loop) {
		this->tcp = loop.resource<uvw::TCPHandle>();

		tcp->on<uvw::ErrorEvent>(&PTP_IP::Connection::on_tcp_error);
		tcp->once<uvw::ConnectEvent>([](const uvw::ConnectEvent& /*unused*/, uvw::TCPHandle& tcp) {
			const InitCommandRequest req{ { 0x64, 0x63, 0x38, 0x39, 0x35, 0x61, 0x30, 0x38, 0x2d, 0x36,
				                              0x35, 0x30, 0x36, 0x2d, 0x34, 0x34 },
				                            u"TESTPC",
				                            0x01,
				                            0x00 };
			auto bytes = req.as_bytes();
			tcp.write(bytes.data(), bytes.size());
		});

		tcp->connect(this->endpoint);
	}

	void Connection::on_tcp_error(const uvw::ErrorEvent& error, uvw::TCPHandle& handle) {
		std::cerr << "TCP error: " << error.name() << "\n\t" << error.what() << "\n";
	}

	Packet::Packet(PacketType type, Bytes bytes)
	    : length{ static_cast<std::uint32_t>(bytes.size() + 8) }, type{ type }, bytes{ std::move(
		                                                                              bytes) } {}

	Packet::Packet(const char* bytes, std::size_t bytesLength) {
		Serialisation::assign_from_bytes<Endian::LittleEndian>(bytes, bytesLength, this->length,
		                                                       this->type, this->bytes);
	}

	Packet::operator std::vector<char>() const {
		return Serialisation::to_bytes<Endian::LittleEndian, char>(this->length, this->type,
		                                                           this->bytes);
	}

	PTPCommand::PTPCommand(PacketType packetType) : packetType{ packetType } {}

	PTPCommand::operator std::vector<char>() const {
		const auto payload = this->dump_payload();

		return static_cast<std::vector<char>>(Packet{ this->packetType, payload });
	}

	[[nodiscard]] std::vector<char> PTPCommand::as_bytes() const {
		return this->operator std::vector<char>();
	}

	std::unique_ptr<PTPCommand> PTPCommand::from_packet(const Packet& packet) {
		switch (packet.type) {
			case PacketType::INIT_REQ:
				return std::make_unique<InitCommandRequest>(packet.bytes);
			case PacketType::INIT_ACK:
				return std::make_unique<InitCommandAck>(packet.bytes);
			case PacketType::INIT_EVENT_REQ:
			case PacketType::INIT_EVENT_ACK:
			case PacketType::INIT_FAIL:
			case PacketType::CMD_REQ:
			case PacketType::CMD_RESP:
			case PacketType::EVENT:
			case PacketType::START_DATA_PACKET:
			case PacketType::DATA_PACKET:
			case PacketType::CANCEL_TRANSACTION:
			case PacketType::END_DATA_PACKET:
			case PacketType::PING:
				// case PONG = 13:
				break;
		}

		return nullptr;
	}

	InitCommandRequest::InitCommandRequest(GUID guid, std::u16string computerName,
	                                       std::uint16_t majorVersion, std::uint16_t minorVersion)
	    : PTPCommand{ PacketType::INIT_REQ }, guid{ guid }, computerName{ std::move(computerName) },
	      majorVersion{ majorVersion }, minorVersion{ minorVersion } {}

	InitCommandRequest::InitCommandRequest(Bytes bytes) : PTPCommand{ PacketType::INIT_REQ } {
		Serialisation::assign_from_bytes<Endian::LittleEndian>(
		    std::move(bytes), this->guid, this->computerName, this->minorVersion, this->majorVersion);
	}

	[[nodiscard]] Bytes InitCommandRequest::dump_payload() const {
		return Serialisation::to_bytes<Endian::LittleEndian, std::uint8_t>(
		    this->guid, this->computerName, this->minorVersion, this->majorVersion);
	}

	InitCommandAck::InitCommandAck(std::uint32_t sessionId, GUID guid, std::u16string cameraName,
	                               std::uint16_t majorVersion, std::uint16_t minorVersion)
	    : PTPCommand{ PacketType::INIT_ACK }, sessionId{ sessionId }, guid{ guid },
	      cameraName{ std::move(cameraName) }, majorVersion{ majorVersion }, minorVersion{
		      minorVersion
	      } {}

	InitCommandAck::InitCommandAck(Bytes bytes) : PTPCommand{ PacketType::INIT_ACK } {
		Serialisation::assign_from_bytes<Endian::LittleEndian>(std::move(bytes), sessionId, guid,
		                                                       cameraName, minorVersion, majorVersion);
	}

	[[nodiscard]] Bytes InitCommandAck::dump_payload() const {
		return Serialisation::to_bytes<Endian::LittleEndian, std::uint8_t>(sessionId, guid, cameraName,
		                                                                   minorVersion, majorVersion);
	}
} // namespace PTP_IP
