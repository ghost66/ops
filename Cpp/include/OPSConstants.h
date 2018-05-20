#ifndef ops_OPSConstants_h
#define ops_OPSConstants_h

#include <climits>

#include "OPSTypeDefs.h"

namespace ops
{
	class OPSConstants
	{
	public:
		const static int PACKET_MAX_SIZE = 60000;
		const static int MESSAGE_MAX_SIZE = 2400000;
		const static int64_t MAX_DEADLINE_TIMEOUT = LLONG_MAX;
		const static ObjectName_T DEFAULT_PARTICIPANT_ID;
	};
}
#endif
