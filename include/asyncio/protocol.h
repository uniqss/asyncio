﻿#pragma once
#include <memory>
#include <asyncio/type.h>
#include <asyncio/obj_counter.h>

namespace asyncio {

class Protocol : public ObjCounter<Protocol> {
public:
	Protocol() = default;
	virtual ~Protocol() = default;
	Protocol(const Protocol&) = delete;
	Protocol& operator=(const Protocol&) = delete;

	virtual std::pair<char*, size_t> GetRxBuffer() = 0;
	virtual void ConnectionMade(const TransportPtr& transport) = 0;
	virtual void ConnectionLost(const TransportPtr& transport, int err_code) = 0;
	virtual void DataReceived(size_t len) = 0;
	virtual size_t Write(const StringPtr& s) = 0;
	virtual void Close() = 0;
};

class ProtocolFactory {
public:
	virtual ~ProtocolFactory() = default;
	virtual ProtocolPtr CreateProtocol() = 0;
};

} // namespace asyncio
