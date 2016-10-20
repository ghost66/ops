﻿///////////////////////////////////////////////////////////
//  Channel.cs
//  Implementation of the Class Channel
//  Created on:      18-oct-2016
//  Author:
///////////////////////////////////////////////////////////

namespace Ops 
{
	public class Channel : OPSObject 
    {
        public string channelID = "";
        public string linktype = "";
        public string localInterface = "";     // If multicast, this specifies interface to use
        public string domainAddress = "";
        public int timeToLive = -1;            // if multicast, this specifies the ttl parameter
        public int port = 0;
        public long outSocketBufferSize = -1;
        public long inSocketBufferSize = -1;

        public static readonly string LINKTYPE_MC = "multicast";
        public static readonly string LINKTYPE_TCP = "tcp";
        public static readonly string LINKTYPE_UDP = "udp";

        public Channel()
        {
            AppendType("Channel");
        }

        public override void Serialize(IArchiverInOut archive)
        {
            // NOTE. Keep this in sync with the C++ version, so it in theory is possible to send these as objects.
            // We need to serialize fields in the same order as C++.
            //OPSObject::serialize(archiver);
            base.Serialize(archive);

            //archiver->inout(std::string("name"), channelID);
            //archiver->inout(std::string("linktype"), linktype);
            //archiver->inout(std::string("localInterface"), localInterface);
            //archiver->inout(std::string("address"), domainAddress);
            channelID = archive.Inout("name", channelID);
            linktype = archive.Inout("linktype", linktype);
            localInterface = archive.Inout("localInterface", localInterface);
            domainAddress = archive.Inout("address", domainAddress);

            //archiver->inout(std::string("timeToLive"), timeToLive);
            //archiver->inout(std::string("port"), port);
            //archiver->inout(std::string("outSocketBufferSize"), outSocketBufferSize);
            //archiver->inout(std::string("inSocketBufferSize"), inSocketBufferSize);
            timeToLive = archive.Inout("timeToLive", timeToLive);
            port = archive.Inout("port", port);
            outSocketBufferSize = archive.Inout("outSocketBufferSize", outSocketBufferSize);
            inSocketBufferSize = archive.Inout("inSocketBufferSize", inSocketBufferSize);

            if (linktype.Equals(""))
            {
                linktype = LINKTYPE_MC;
            }
            else if ((linktype != LINKTYPE_MC) && (linktype != LINKTYPE_TCP) && (linktype != LINKTYPE_UDP))
            {
                throw new ConfigException(
                    "Illegal linktype: '" + linktype +
                    "'. Linktype for Channel must be either 'multicast', 'tcp', 'udp' or left blank( = multicast)");
            }
        }

        public void populateTopic(Topic top)
        {
            // If Topic doesn't specify a transport it will default to 'multicast', therefore
            // we can't just check for an empty 'top.getTransport()' to know when to replace value.
            // Therfore, if a topic is listed in a 'Transport/Channel', we assume it shall
            // use the channel values, so replace all values.
            top.SetTransport(linktype);
            top.SetLocalInterface(localInterface);
            top.SetDomainAddress(domainAddress);
            top.SetPort(port);
            top.SetOutSocketBufferSize((int)outSocketBufferSize);
            top.SetInSocketBufferSize((int)inSocketBufferSize);
            top.SetTimeToLive(timeToLive);
        }

	}
}
