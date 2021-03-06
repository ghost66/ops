/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2018-2020 Lennart Andersson.
 *
 * This file is part of OPS (Open Publish Subscribe).
 *
 * OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
 */

#include "OPSTypeDefs.h"
#include "TCPReceiveDataHandler.h"
#include "TCPReceiveDataChannel.h"
#include "Participant.h"

namespace ops
{
    TCPReceiveDataHandler::TCPReceiveDataHandler(Topic top, Participant& part) :
		ReceiveDataHandler(top, part, nullptr)
    {
		// Handle TCP channels specified with an address and port
		if ((top.getTransport() == Topic::TRANSPORT_TCP) && (top.getPort() != 0)) {
			ReceiveDataChannel* rdc_ = new TCPReceiveDataChannel(top, part);
			rdc_->connect(this);
			rdc.push_back(rdc_);
			usingPartInfo = false;
		}
	}

	void TCPReceiveDataHandler::AddReceiveChannel(ObjectName_T& topicName, Address_T& ip, int port)
	{
        UNUSED(topicName);
		OPS_PIFO_TRACE("Partinfo: name: " << topicName << ", ip: " << ip << ", port: " << port << "\n");

		// We need to check if a new publisher has emerged that we need to connect to
		InternalKey_T key(ip);
		key += "::";
		key += NumberToString(port);

		// Look for it in rdc, if not there, create one
		bool found = false;
		for (auto x : rdc) {
			if (x->key == key) {
				found = true;
				break;
			}
		}
		if (!found) {
			OPS_PIFO_TRACE("Partinfo: CREATED name: " << topicName << ", ip: " << ip << ", port: " << port << "\n");
			topic.setDomainAddress(ip);
			topic.setPort(port);
			ReceiveDataChannel* rdc_ = new TCPReceiveDataChannel(topic, participant);
			rdc_->key = key;
			rdc_->connect(this);

			SafeLock lock(&messageLock);
			rdc.push_back(rdc_);

			if (Notifier<OPSMessage*>::getNrOfListeners() > 0) {
				rdc_->start();
			}
		}
	}

	void TCPReceiveDataHandler::topicUsage(Topic& top, bool used)
	{
        if (usingPartInfo) {
            SafeLock lock(&topicsLock);
            // We should only register unique topics
			auto it = topics.find(top.getName());
			int32_t count = 0;
			if (it != topics.end()) {
				count = topics[top.getName()];
			}
			// Register topic with participant info data handler/listener to get callbacks to handler above
			if (used) {
				++count;
				if (count == 1) {
					participant.registerTcpTopic(top.getName(), shared_from_this());
				}
			} else {
				--count;
				if (count == 0) {
					participant.unregisterTcpTopic(top.getName(), shared_from_this());
				}
			}
			topics[top.getName()] = count;
		}
	}

}
