﻿#pragma once
#include "codec.h"

namespace asyncio {

class CodecTrans : public Codec {
public:
	using USER_MSG_CALLBACK = std::function<void(uint64_t trans_id, uint32_t msg_id, std::shared_ptr<std::string>)>;
	CodecTrans(USER_MSG_CALLBACK&& func)
		: Codec(0)
		, m_user_msg_func(std::move(func)) {}
	virtual ~CodecTrans() {}

	virtual void Reset(size_t size = DEFAULT_RX_BUFFER_SIZE) override {
		Codec::Reset(size);
		bucket_.header.reset();
	}

	virtual void Decode(TransportPtr transport, size_t len) {
		// len是本次接收到的数据长度
		write_pos_ += len;					//需要更新一下最新的写入位置
		size_t left_len = GetRemainedLen(); //缓冲区内的数据总长度

		while (left_len > 0) {
			if (!bucket_.header.is_full()) {
				if (left_len < bucket_.header.size())
					break;

				if (bucket_.header.fill(read_pos_, left_len)) {
					uint32_t original_len = BigLittleSwap32(bucket_.header.get().len);
					if (packet_size_limit_ > 0 && original_len > packet_size_limit_) {
						transport->Close(EC_PACKET_OVER_SIZE);
						ASYNCIO_LOG_WARN("Close transport because of packet length(%d) over limit(%d)", original_len,
										 packet_size_limit_);
						return;
					}

					bucket_.data.reset(original_len);
				}
			}

			if (bucket_.data.is_full() || bucket_.data.fill(read_pos_, left_len)) {
				uint32_t original_msgid = BigLittleSwap32(bucket_.header.get().msg_id);
				m_user_msg_func(bucket_.header.get().trans_sid, original_msgid, bucket_.data.get());
				bucket_.header.reset();
			}
		}

		ReArrangePos();
	}

	std::shared_ptr<std::string> Encode(uint32_t msgID, uint64_t trans_sid, const char* buf, size_t len) {
		auto p = std::make_shared<std::string>(TcpMsgHeader::size() + len, 0);
		TcpMsgHeader* header = (TcpMsgHeader*)&p->at(0);
		header->len = BigLittleSwap32(len);
		header->msg_id = BigLittleSwap32(msgID);
		header->trans_sid = trans_sid;
		if (len > 0) {
			memcpy(&p->at(TcpMsgHeader::size()), buf, len);
		}
		return p;
	}

private:
	struct TcpMsgHeader {
		TcpMsgHeader() { memset(this, 0, size()); }
		constexpr static int size() { return sizeof(TcpMsgHeader); }

		uint32_t len; //这个长度是指报文体的长度，没有包括报文头的长度
		uint32_t msg_id;
		uint64_t trans_sid; //转发的SessionId
	};

	struct TcpMsgBucket {
		BucketPod<TcpMsgHeader> header;
		BucketString data;
	};

private:
	USER_MSG_CALLBACK m_user_msg_func;
	TcpMsgBucket bucket_;
};

} // namespace asyncio
