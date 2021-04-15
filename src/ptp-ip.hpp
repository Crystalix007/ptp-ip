#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <uvw.hpp>

namespace PTP_IP {
	using GUID = std::array<std::uint8_t, 16>;
	using Bytes = std::vector<std::uint8_t>;

	class Connection {
	public:
		Connection(uvw::Addr endpoint);
		virtual ~Connection() = default;

		void connect(uvw::Loop& loop);
		static void on_tcp_error(const uvw::ErrorEvent& error, uvw::TCPHandle& handle);

	protected:
		uvw::Addr endpoint;
		std::shared_ptr<uvw::TCPHandle> tcp;
	};

	enum class PacketType : std::uint32_t {
		INIT_REQ = 1,
		INIT_ACK,
		INIT_EVENT_REQ,
		INIT_EVENT_ACK,
		INIT_FAIL,
		CMD_REQ,
		CMD_RESP,
		EVENT,
		START_DATA_PACKET,
		DATA_PACKET,
		CANCEL_TRANSACTION,
		END_DATA_PACKET,
		PING = 13,
		PONG = 13,
	};

	struct Packet {
		std::uint32_t length;
		PacketType type;
		Bytes bytes;

		Packet(PacketType type, Bytes bytes);
		explicit Packet(const char* bytes, std::size_t bytesLength);

		[[nodiscard]] explicit operator std::vector<char>() const;
	};

	class PTPCommand {
	public:
		PTPCommand(PacketType packetType);
		virtual ~PTPCommand() = default;

		[[nodiscard]] explicit operator std::vector<char>() const;
		[[nodiscard]] std::vector<char> as_bytes() const;

		[[nodiscard]] virtual Bytes dump_payload() const = 0;

		static std::unique_ptr<PTPCommand> from_packet(const Packet& packet);

	protected:
		PacketType packetType;
	};

	class InitCommandRequest : public PTPCommand {
	public:
		InitCommandRequest(GUID guid, std::u16string computerName, std::uint16_t majorVersion,
		                   std::uint16_t minorVersion);
		InitCommandRequest(Bytes bytes);
		~InitCommandRequest() override = default;

		[[nodiscard]] Bytes dump_payload() const override;

	protected:
		GUID guid;
		std::u16string computerName;
		std::uint16_t majorVersion;
		std::uint16_t minorVersion;
	};

	class InitCommandAck : public PTPCommand {
	public:
		InitCommandAck(std::uint32_t sessionId, GUID guid, std::u16string cameraName,
		               std::uint16_t majorVersion, std::uint16_t minorVersion);
		InitCommandAck(Bytes bytes);
		~InitCommandAck() override = default;

		[[nodiscard]] Bytes dump_payload() const override;

	protected:
		std::uint32_t sessionId;
		GUID guid;
		std::u16string cameraName;
		std::uint16_t majorVersion;
		std::uint16_t minorVersion;
	};
} // namespace PTP_IP
